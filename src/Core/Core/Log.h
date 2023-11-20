/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef INCLUDE_SCINE_UTILS_LOG_H
#define INCLUDE_SCINE_UTILS_LOG_H

#include "Core/ExportControl.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

namespace Scine {
namespace Core {
namespace Traits {

template<typename>
struct sfinae_true : std::true_type {};

template<typename T>
static auto testIsStreamWriter(int) -> sfinae_true<decltype(std::declval<T>()(std::declval<std::ostream&>()))>;

template<typename T>
static auto testIsStreamWriter(long) -> std::false_type;

template<typename T>
struct isStreamWriter : decltype(testIsStreamWriter<T>(0)) {};

} // namespace Traits

/**
 * @brief Multi-domain multi-sink logger
 *
 * A log object consists of multiple domains. These domains are debug, warning,
 * error and output, in ascending order of importance to an end user.
 *
 * Each domain has its own set of sinks into which information can be fed.
 * Multiple domains can have the same sink. Imagine a breadboard with all the
 * little cables that you have to manually connect.
 *
 * By default, the debug domain has no sinks.
 *
 * Some opinionated guidelines:
 * - This object is purposely not thread-safe. We think it disavisable to log
 *   to any output in parallel sections. If each thread must collect logging
 *   information, do so separately to thread-local objects and deal with it
 *   outside of parallel sections instead of contending with a single log
 *   object, stalling work.
 * - Library functions should not log nor write to any standard output unless
 *   their interface accepts a log instance.
 * - Libraries should not have log singletons.
 */
struct CORE_EXPORT Log {
  //!@name Types
  //!@{
  /*! @brief Type used to represent stream sinks for logging domains
   *
   * Ownership is shared since multiple levels may refer to e.g. the same file
   */
  using SinkPtr = std::shared_ptr<std::ostream>;

  /*! @brief Multi-sink object representing a single logging domain
   *
   * Usage:
   * @code{.cpp}
   * Log::Domain domain;
   * domain << "This text is never seen because domain has no sinks";
   *
   * domain.add("cout", Log::coutSink());
   * domain.line("No line breaks necessary here, but this fn only takes strings");
   *
   * // Domain operator << does not work with std::endl (it's an overload set)!
   * domain << "Add endline, do not force flush" << Log::nl;
   * domain << "Add endline and force flush" << Log::endl;
   *
   * // Can selectively remove error sinks
   * domain.remove("cerr");
   * domain << std::setw(25) << "Never seen" << Log::endl;
   * @endcode
   */
  class CORE_EXPORT Domain {
   public:
    //! No-sink default constructor
    inline Domain() = default;
    //! Single-sink constructor
    inline Domain(std::string n, SinkPtr s) {
      sinks_.emplace(std::move(n), std::move(s));
    }

    //! Add a sink
    inline void add(std::string n, SinkPtr s) {
      sinks_.emplace(std::move(n), std::move(s));
    }

    //! Remove all sinks
    inline void clear() {
      sinks_.clear();
    }

    /*! @brief Remove a sink by name
     *
     * @throws std::out_of_range If no sink of that name exists in the Domain
     */
    inline void remove(const std::string& name) {
      sinks_.erase(expectantFind(name));
    }

    /*! @brief Extracts a sink by name
     *
     * Removes the link from the underlying map and returns it
     *
     * @throws std::out_of_range If no sink of that name exists in the Domain
     */
    inline SinkPtr extract(const std::string& name) {
      auto findIter = expectantFind(name);
      auto ptr = std::move(findIter->second);
      sinks_.erase(findIter);
      return ptr;
    }

    //! Returns whether this Domain has any sinks
    inline explicit operator bool() const {
      return !sinks_.empty();
    }

    /*! @brief Formatted logging analog to pfrintf.
     *
     * @code{.cpp}
     * domain.printf("Akerman #%3d", 3);
     * @endcode
     *
     * Variadic in nullary callables.
     */
    template<typename... Args>
    inline void printf(Args&&... args) {
      const int sz = snprintf(NULL, 0, std::forward<Args>(args)...);
      char* buf = new char[sz + 1];
      snprintf(buf, sz + 1, std::forward<Args>(args)...);
      (*this) << std::string(buf);
      delete[] buf;
    }

    /*! @brief Left-shifting with unary functions accepting ostream&
     *
     * @tparam T a callable accepting std::ostream&. Return type is unimportant.
     * Accepts lambdas and bind expressions.
     */
    template<typename T>
    std::enable_if_t<Traits::isStreamWriter<T>::value, Domain&> operator<<(T&& f) {
      for (auto& nameSinkPair : sinks_) {
        f(*nameSinkPair.second);
      }

      return *this;
    }

    /*! @brief Left-shifting with overload set
     *
     * You can left-shift this with anything on the right that is part of the
     * overload set of std::ostream& operator << (std::ostream& os, T&& t).
     */
    template<typename T>
    std::enable_if_t<!Traits::isStreamWriter<T>::value, Domain&> operator<<(T&& t) {
      for (auto& nameSinkPair : sinks_) {
        *nameSinkPair.second << t;
      }
      return *this;
    }

    /*! @brief Conditional logging with nullary callables
     *
     * If the composition of a log message can take significant time, it is
     * preferable to avoid composing it if the Domain has no sinks. This
     * function is sugar for the relevant pattern:
     *
     * @code{.cpp}
     * // Ackermann fn is executed even if domain has no sinks
     * domain << ackermann(4, 1);
     *
     * // Ackermann fn is executed only if domain has a sink
     * if(domain) {
     *   domain << ackermann(4, 1);
     * }
     *
     * // Sugar with nullary callables
     * domain.lazy(std::bind(ackermann, 4, 1));
     * @endcode
     *
     * Variadic in nullary callables.
     */
    template<typename... Args>
    inline void lazy(Args&&... args) {
      if (*this) {
        sink(std::forward<Args>(args)...);
      }
    }

    //! @brief Prints a single string with a newline character
    inline void line(const std::string& s) {
      if (*this) {
        *this << s << Log::nl;
      }
    }

   private:
    inline void sink() {
    }

    template<typename F, typename... Args>
    inline void sink(F&& f, Args&&... args) {
      *this << f();
      return sink(args...);
    }

    using MapType = std::unordered_map<std::string, SinkPtr>;
    using iterator = typename MapType::iterator;

    inline iterator expectantFind(const std::string& sinkName) {
      const auto findIter = sinks_.find(sinkName);
      if (findIter == std::end(sinks_)) {
        throw std::out_of_range("Sink name not found");
      }
      return findIter;
    }

    std::unordered_map<std::string, SinkPtr> sinks_;
  };

  using iterator = Domain*;
  using const_iterator = const Domain*;
  using value_type = Domain;
  using reference = Domain&;
  using pointer = Domain*;
  //!@}

  //!@name Static members
  //!@{
  //! @brief Creates an appending file sink
  static inline SinkPtr fileSink(const std::string& filename) {
    return std::make_shared<std::ofstream>(filename, std::ios_base::out | std::ios_base::app);
  }

  //! @brief Sink referring to std::cout
  static inline SinkPtr coutSink() {
    return std::make_shared<std::ostream>(std::cout.rdbuf());
  }

  //! @brief Sink referring to std::cerr
  static inline SinkPtr cerrSink() {
    return std::make_shared<std::ostream>(std::cerr.rdbuf());
  }

  //! @brief Creates a silent Log instance (i.e. all levels are cleared)
  static inline Log silent() {
    return Log{Domain(), Domain(), Domain(), Domain()};
  }

  //! @brief Inserts a newline character into the stream
  static inline std::ostream& nl(std::ostream& os) {
    os << "\n";
    return os;
  }

  //! @brief Forces flush of stream
  static inline std::ostream& flush(std::ostream& os) {
    os << std::flush;
    return os;
  }

  //! @brief Inserts a newline character and flushes the stream
  static inline std::ostream& endl(std::ostream& os) {
    os << std::endl;
    return os;
  }
  //!@}

  //!@name Modification
  //!@{
  //! @brief Clears all levels
  inline void clear() {
    for (Domain& domain : *this) {
      domain.clear();
    }
  }

  /*! @brief Add a sink to file for all domains
   *
   * @note If @p sinkName is empty, the sink name is
   * `std::to_string(std::hash<std::string>{}(file))`.
   */
  inline void sinkAllToFile(const std::string& file, std::string sinkName = "") {
    if (sinkName.empty()) {
      sinkName = std::to_string(std::hash<std::string>{}(file));
    }

    auto sink = fileSink(file);
    for (Domain& domain : *this) {
      domain.add(sinkName, sink);
    }
  }
  //!@}

  //!@name Iterators
  //!@{
  // NOTE: Standard guarantees in-order sequential member memory layout
  inline iterator begin() {
    return std::addressof(debug);
  }
  inline iterator end() {
    return std::addressof(output) + 1;
  }
  inline const_iterator begin() const {
    return std::addressof(debug);
  }
  inline const_iterator end() const {
    return std::addressof(output) + 1;
  }
  //!@}

  //!@name Members
  //!@{
  /*! @brief Non-transient debug statements
   *
   * For non-bug-specific statements to help with repeat debugging of long,
   * tricky procedures. No debug statements should never show in release builds.
   * Should not replace transient debug statements for bug-specific quick
   * debugging.
   */
  Domain debug;
  //! Recoverable errors that may require user attention
  Domain warning = Domain("cerr", cerrSink());
  //! Non-recoverable errors prior to termination
  Domain error = Domain("cerr", cerrSink());
  //! User-facing progress, results, etc.
  Domain output = Domain("cout", coutSink());
  //!@}
};

} // namespace Core
} // namespace Scine

#endif
