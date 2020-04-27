/**
 * @file
 * @brief Provides helpers for the general implementation of classes deriving from Module
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef CORE_DERIVED_MODULE_
#define CORE_DERIVED_MODULE_

#include "boost/any.hpp"
#include "boost/mpl/at.hpp"
#include "boost/mpl/find_if.hpp"
#include "boost/mpl/for_each.hpp"
#include "boost/mpl/map.hpp"
#include "boost/mpl/size.hpp"
#include "boost/mpl/size_t.hpp"
#include "boost/mpl/vector.hpp"
#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Scine {
namespace Core {
namespace DerivedModule {

namespace detail {

inline bool caseInsensitiveEqual(const std::string& a, const std::string& b) {
  return std::equal(std::begin(a), std::end(a), std::begin(b), std::end(b),
                    [](const char x, const char y) -> bool { return ::tolower(x) == ::tolower(y); });
}

// When at end of sequence and Exec has not been called, return the none()
template<bool done = true>
struct exec_if_impl {
  template<typename Iter, typename LastIter, typename Pred, typename Exec>
  static auto execute(Iter* /* i */, LastIter* /* l */, const Pred& /* p */, const Exec & /* e */) ->
      typename Exec::ResultType {
    return Exec::none();
  }
};

// Non-end iterator of type sequence: Test predicate
template<>
struct exec_if_impl<false> {
  template<typename Iter, typename LastIter, typename Pred, typename Exec>
  static auto execute(Iter* /* i */, LastIter* /* l */, const Pred& p, const Exec& e) -> typename Exec::ResultType {
    using Item = typename boost::mpl::deref<Iter>::type;

    if (!p(static_cast<Item*>(0))) {
      using Next = typename boost::mpl::next<Iter>::type;
      return exec_if_impl<boost::is_same<Next, LastIter>::value>::execute(static_cast<Next*>(0),
                                                                          static_cast<LastIter*>(0), p, e);
    }
    else {
      return e(static_cast<Item*>(0));
    }
  }
};

/*! @brief Run-time "iteration" through type list, if predicate matches, call
 *    another function, otherwise return its none.
 *
 * @tparam Sequence a boost::mpl compatible sequence type
 * @tparam Predicate A type with a template<T> bool operator() (T* = 0) const
 *   member specifying whether to execute @p Executable for the type T in @p
 *   Sequence
 * @tparam Executable A type fulfilling the following requirements:
 *   - Has a ResultType typedef
 *   - Has a template<T> ResultType operator() (T* = 0) const member which
 *     is executed if @Predicate returns true for a type in @p Sequence
 *   - Has a static ResultType none() const member returning the base case
 *
 * @returns @p Executable::none() if @p Predicate never matches, otherwise @p
 *   Executable(T) for the first T in @p Sequence that matches Predicate.
 */
template<typename Sequence, typename Predicate, typename Executable>
inline auto exec_if(const Predicate& p, const Executable& e, Sequence* = 0) -> typename Executable::ResultType {
  BOOST_MPL_ASSERT((boost::mpl::is_sequence<Sequence>));
  using First = typename boost::mpl::begin<Sequence>::type;
  using Last = typename boost::mpl::end<Sequence>::type;

  return exec_if_impl<boost::is_same<First, Last>::value>::execute(static_cast<First*>(0), static_cast<Last*>(0), p, e);
}

// Predicate type for exec_if
struct MapPairInterfaceIdentifierMatches {
  MapPairInterfaceIdentifierMatches(std::string id) : identifier(std::move(id)) {
  }

  template<typename PairType>
  bool operator()(PairType* = 0) const {
    using InterfaceType = typename boost::mpl::first<PairType>::type;
    return caseInsensitiveEqual(identifier, InterfaceType::interface);
  }

  std::string identifier;
};

// Predicate type for exec_if
struct ModelTypeIdentifierMatches {
  ModelTypeIdentifierMatches(std::string id) : identifier(std::move(id)) {
  }

  template<typename ModelType>
  bool operator()(ModelType* = 0) const {
    return caseInsensitiveEqual(identifier, ModelType::model);
  }

  std::string identifier;
};

// Executable type for exec_if
template<typename InterfaceType>
struct CreateInterfacePointer {
  using InterfaceTypePtr = std::shared_ptr<InterfaceType>;
  using ResultType = boost::any;

  template<typename ModelType>
  ResultType operator()(ModelType* = 0) const {
    return static_cast<InterfaceTypePtr>(std::make_shared<ModelType>());
  }

  static ResultType none() {
    return {};
  }
};

// Executable type for exec_if
struct ResolveModel {
  ResolveModel(std::string id) : identifier(std::move(id)) {
  }

  using ResultType = boost::any;

  template<typename PairType>
  boost::any operator()(PairType* = 0) const {
    using InterfaceType = typename boost::mpl::first<PairType>::type;
    using ModelTypeList = typename boost::mpl::second<PairType>::type;

    return detail::exec_if<ModelTypeList>(detail::ModelTypeIdentifierMatches{identifier},
                                          CreateInterfacePointer<InterfaceType>{});
  }

  static ResultType none() {
    return {};
  }

  std::string identifier;
};

// Executable type for exec_if
struct MatchFoundExecutor {
  using ResultType = bool;

  template<typename T>
  ResultType operator()(T* = 0) const {
    return true;
  }

  static ResultType none() {
    return false;
  }
};

// Executable type for exec_if
struct ModelExists {
  ModelExists(std::string id) : identifier(std::move(id)) {
  }

  using ResultType = bool;

  template<typename PairType>
  ResultType operator()(PairType* = 0) const {
    using ModelTypeList = typename boost::mpl::second<PairType>::type;

    return exec_if<ModelTypeList>(detail::ModelTypeIdentifierMatches{identifier}, MatchFoundExecutor{});
  }

  static ResultType none() {
    return false;
  }

  std::string identifier;
};

// Executable type for exec_if
struct ListModels {
  using ResultType = std::vector<std::string>;

  template<typename PairType>
  ResultType operator()(PairType* = 0) const {
    using ModelTypeList = typename boost::mpl::second<PairType>::type;

    std::vector<std::string> models;
    models.reserve(boost::mpl::size<ModelTypeList>::value);
    // "Iterate" through this entry's list of model types
    boost::mpl::for_each<ModelTypeList, boost::mpl::make_identity<boost::mpl::_1>>([&](auto t) {
      using ModelType = typename decltype(t)::type;
      models.push_back(ModelType::model);
    });

    return models;
  }

  static ResultType none() {
    return {};
  }
};

// Compile-time compare C-style strings
constexpr bool strEqual(const char* a, const char* b) {
  return *a == *b && (*a == '\0' || strEqual(a + 1, b + 1));
}

// For a list of model types, test whether any identifiers are duplicate
template<typename... ModelTypes>
constexpr bool identifiersOverlap() {
  constexpr std::size_t S = sizeof...(ModelTypes);

  const std::array<const char*, S> typeIdentifiers{ModelTypes::model...};

  for (unsigned i = 0; i < S; ++i) {
    for (unsigned j = i + 1; j < S; ++j) {
      if (strEqual(typeIdentifiers.at(i), typeIdentifiers.at(j))) {
        return true;
      }
    }
  }

  return false;
}

// expansion of all types in the mpl vector to variadic pack for identifiersOverlap
template<typename MPLVector, std::size_t... Inds>
constexpr bool identifierOverlapForwarder(std::index_sequence<Inds...> /* inds */) {
  return identifiersOverlap<typename boost::mpl::at<MPLVector, boost::mpl::size_t<Inds>>::type...>();
}

// MPL Metafunction predicate suitable for find_if, and also for our none_of
struct IdentifierOverlapPredicate {
  template<typename MPLMapIterPair>
  struct apply
    : std::integral_constant<
          bool, identifierOverlapForwarder<typename boost::mpl::second<MPLMapIterPair>::type>(
                    std::make_index_sequence<boost::mpl::size<typename boost::mpl::second<MPLMapIterPair>::type>::type::value>{})> {
  };
};

// MPL Metafunction predicate suitable for our none_of
struct ModelTypeListIsEmpty {
  template<typename MPLMapIterPair>
  struct apply
    : std::integral_constant<bool, boost::mpl::size<typename boost::mpl::second<MPLMapIterPair>::type>::value == 0> {};
};

template<typename Sequence, typename Predicate>
struct none_of
  : boost::is_same<typename boost::mpl::end<Sequence>::type, typename boost::mpl::find_if<Sequence, Predicate>::type> {};

// Value type stating whether all model lists in an mpl map have at least one member type
template<typename MPLMap>
struct ModelListsAreNotEmpty : std::integral_constant<bool, none_of<MPLMap, ModelTypeListIsEmpty>::value> {};

// Value type stating whether all model lists in an mpl map have no model identifier duplicates
template<typename MPLMap>
struct NoModelIdentifiersOverlap : std::integral_constant<bool, none_of<MPLMap, IdentifierOverlapPredicate>::value> {};

} // namespace detail

/**
 * @brief Creates an InterfaceType shared_ptr of a matching model to an interface
 *
 * Helper function to implement a derived module's get function:
 * @code{cpp}
 * // Transforms this:
 * boost::any Derived::get(const std::string& interface, const std::string& model) const {
 *   if(interface == Calculator::interface) {
 *     if(model == PlusCalculator::model) {
 *       return static_cast<Calculator>(std::make_shared<PlusCalculator>());
 *     }
 *
 *     if(model == MinusCalculator::model) {
 *       return static_cast<Calculator>(std::make_shared<MinusCalculator>());
 *     }
 *
 *     ...
 *   }
 *
 *   if(interface == Printer::interface) {
 *     if(model == SlowPrinter::model) {
 *       return static_cast<Printer>(std::make_shared<SlowPrinter>());
 *     }
 *     if(model == EvenSlowerPrinter::model) {
 *       return static_cast<Printer>(std::make_shared<EvenSlowerPrinter>());
 *     }
 *
 *     ...
 *   }
 *
 *   ...
 *
 *   throw Core::ClassNotImplementedError();
 * }
 *
 * // Into this:
 * boost::any Derived::get(const std::string& interface, const std::string& model) const {
 *   // NOTE: Same map for has, announceInterfaces, anounceModels
 *   using Map = boost::mpl::map<
 *     boost::mpl::pair<Calculator, boost::mpl::vector<PlusCalculator, MinusCalculator>>,
 *     boost::mpl::pair<Printer, boost::mpl::vector<SlowPrinter, EvenSlowerPrinter>>
 *   >;
 *
 *   boost::any resolved = DerivedModule::resolve<Map>(interface, model);
 *   if(resolved.empty()) {
 *     throw Core::ClassNotImplementedError {};
 *   }
 *   return resolved;
 * }
 * @endcode
 *
 * @tparam MPLMap An mpl::map of InterfaceType -> mpl::vector<ModelType>
 *   InterfaceType::interface and ModelType::Model must be valid expressions
 * @param interface The interface to model
 * @param model The model of @p interface to create
 *
 * @returns a boost any containing a shared_ptr to the InterfaceType of the
 *   matched model, an empty boost::any otherwise.
 */
template<typename MPLMap>
boost::any resolve(const std::string& interface, const std::string& model) {
  static_assert(detail::ModelListsAreNotEmpty<MPLMap>::value, "Model type lists may not be empty!");

  static_assert(detail::NoModelIdentifiersOverlap<MPLMap>::value,
                "Model identifiers overlap within a single model type list! Model identifiers must be unique within an "
                "interface!");

  return detail::exec_if<MPLMap>(detail::MapPairInterfaceIdentifierMatches{interface}, detail::ResolveModel{model});
}

/**
 * @brief Checks whether a module has a particular model for a particular interface
 *
 * @tparam MPLMap An mpl::map of InterfaceType -> mpl::vector<ModelType>
 *   InterfaceType::interface and ModelType::Model must be valid expressions
 * @param interface The interface to check for
 * @param model The model to check for
 *
 * @returns whether the module has a model for the interface
 */
template<typename MPLMap>
bool has(const std::string& interface, const std::string& model) noexcept {
  return detail::exec_if<MPLMap>(detail::MapPairInterfaceIdentifierMatches{interface}, detail::ModelExists{model});
}

/**
 * @brief Announces all interface names
 *
 * @tparam MPLMap An mpl::map of InterfaceType -> mpl::vector<ModelType>
 *   InterfaceType::interface and ModelType::Model must be valid expressions
 *
 * @return A list of all interface names
 */
template<typename MPLMap>
std::vector<std::string> announceInterfaces() noexcept {
  std::vector<std::string> interface;
  interface.reserve(boost::mpl::size<MPLMap>::value);

  boost::mpl::for_each<MPLMap, boost::mpl::make_identity<boost::mpl::_1>>([&](auto p) {
    using PairType = typename decltype(p)::type;
    using InterfaceType = typename boost::mpl::first<PairType>::type;
    interface.push_back(InterfaceType::interface);
  });

  return interface;
}

/**
 * @brief Announces all model names for a particular interface
 *
 * @tparam MPLMap An mpl::map of InterfaceType -> mpl::vector<ModelType>
 *   InterfaceType::interface and ModelType::Model must be valid expressions
 * @param interface The interface for which to list model identifiers
 *
 * @return The list of model names for a particular interface, or an empty list
 *   if no models exist for that interface.
 */
template<typename MPLMap>
std::vector<std::string> announceModels(const std::string& interface) noexcept {
  return detail::exec_if<MPLMap>(detail::MapPairInterfaceIdentifierMatches{interface}, detail::ListModels{});
}

} // namespace DerivedModule
} // namespace Core
} // namespace Scine

#endif
