/**
 * @file ModuleManager.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef CORE_MODULEMANAGER_H_
#define CORE_MODULEMANAGER_H_

/* Internal Includes */
#include "Core/ExportControl.h"
#include "Core/Module.h"
/* External Includes */
#include <boost/filesystem.hpp>
#include <memory>
#include <vector>

namespace boost {
namespace dll {
class shared_library;
} // namespace dll
} // namespace boost

namespace Scine {
namespace Core {

/**
 * @class ModuleManager ModuleManager.h
 * @brief The manager for all dynamically loaded modules.
 *
 * This class is a singleton, in order to ensure that there is only ever one
 *   instance of this class controlling all modules loaded.
 *
 * Use this class by requesting a reference to the single existing instance:
 *
 * @code{.cpp}
 * const auto& manager = ModuleManager::getInstance()
 * if(manager.has<Calculator>("DFT")) {
 *   auto calculator = manager.get<Calculator>("DFT");
 * }
 * @endcode
 */
class CORE_EXPORT ModuleManager {
 public:
  /**
   * @brief Static instance getter.
   *
   * This static function assures the single instantiation of a ModuleManager
   * for more information google the singleton design pattern.
   *
   * @return Returns a reference to the ModuleManager instance.
   */
  static ModuleManager& getInstance() {
    static ModuleManager instance;
    return instance;
  }

  /**
   * @brief Deleted copy constructor, as required by the singleton design pattern.
   *
   * The constructor could also be private, however public deletion should
   * generate more sensible error messages.
   *
   * @param ModuleManager
   */
  ModuleManager(ModuleManager const&) = delete;

  /**
   * @brief Deleted assignment operator, as required by the singleton design pattern.
   *
   * The assignment operator could also be private, however public deletion should
   * generate more sensible error messages.
   *
   * @param ModuleManager
   */
  void operator=(ModuleManager const&) = delete;

  /**
   * @brief Loads a library as a module
   *
   * Loads a shared library from a specified path and constructs the derived
   * module class, adding it to the list of loaded modules.
   *
   * @param libraryPath Either a full qualified path to the library, or merely
   *   the path to it and the name of the library without system-specific pre-
   *   and suffixes. E.g. "/usr/lib/libblas.so" or just "/usr/lib/blas", both
   *   should work.
   *
   * @throws std::runtime_error Under any of the following circumstances:
   *   - The file is not found
   *   - There is not enough memory
   *   - The library does not provide the moduleFactory alias entry point to
   *     construct a module base class pointer
   */
  void load(const boost::filesystem::path& libraryPath);

  /**
   * @brief Loads a module from a shared_library instance
   *
   * @param library A shared_library instance that contains the symbol alias
   *   "moduleFactory" that is a nullary function yielding a shared_ptr<Module>
   */
  void load(boost::dll::shared_library library);

  /**
   * @brief Get a list of all loaded Modules.
   *
   * @return std::vector<std::string> Returns a vector of names corresponding to the Modules
   *                                  currently loaded.
   */
  std::vector<std::string> getLoadedModuleNames() const;

  //* Announces a list of all interfaces for which at least one model is loaded.
  std::vector<std::string> getLoadedInterfaces() const;

  /**
   * @brief Announces a list of all loaded models of a particular interface
   *
   * @tparam Interface The interface class type (e.g. Calculator)
   *
   * @return A list of string-identifiers to the loaded models
   */
  template<typename Interface>
  std::vector<std::string> getLoadedModels() const {
    return getLoadedModels(Interface::interface);
  }

  /**
   * @brief Announces a list of all loaded models of a particular interface
   *
   * @param interface The string identifier of a particular interface
   *
   * @return A list of string-identifiers to the loaded models
   */
  std::vector<std::string> getLoadedModels(const std::string& interface) const;

  /**
   * @brief Checks whether a particular model of an interface is available
   *
   * @tparam Interface The interface class type (e.g. Calculator).
   * @param model      The string identifier of a particular model.
   * @param moduleName The string identifier of a particular module.
   *                   If given, queries for the exact module-model combination.
   *                   Module names will be auto expanded in addition to being tested
   *                   literally: 'XYZ' will also query for 'XYZModule'.
   * @return Whether the model is available. If so, get() should be safe to call.
   */
  template<typename Interface>
  bool has(const std::string& model, const std::string& moduleName = "") const {
    return has(Interface::interface, model, moduleName);
  }

  /**
   * @brief Checks whether a particular model of an interface is available
   *
   * @param interface The string identifier of an interface class type.
   * @param model The string identifier of a particular model.
   * @param moduleName The string identifier of a particular module.
   *                   If given, queries for the exact module-model combination.
   *                   Module names will be auto expanded in addition to being tested
   *                   literally: 'XYZ' will also query for 'XYZModule'.
   *
   * @return Whether the model is available. If so, get() should be safe to call.
   */
  bool has(const std::string& interface, const std::string& model, const std::string& moduleName = "") const;

  /**
   * @brief Creates a model of an interface and returns a base-class pointer.
   *
   * Creates a derived class and yields a base-class pointer to it.
   *
   * @tparam Interface The interface class type (e.g. Calculator)
   * @param model      The string identifier of a particular model (e.g. DefaultCalculator)
   * @param moduleName The string identifier of a particular module.
   *                   If given, queries for the exact module-model combination.
   *                   Module names will be auto expanded in addition to being tested
   *                   literally: 'XYZ' will also query for 'XYZModule'.
   *
   * @throws std::runtime_error 1) If no loaded module provides that particular interface model.
   *                            2) If the requested module does not exist.
   *                            3) If the requested module does not provide the interface model.
   *
   * @return A base-class pointer to the class derived from the specified
   *   interface.
   */
  template<typename Interface>
  std::shared_ptr<Interface> get(const std::string& model, const std::string& moduleName = "") const {
    auto any = _get(Interface::interface, model, moduleName);
    assert(!any.empty() && "Contract of a module's get states that the any may not be empty!");
    return boost::any_cast<std::shared_ptr<Interface>>(any);
  }

  /**
   * @brief Find a model satisfying a predicate
   *
   * @tparam Interface The interface class type (e.g. Calculator)
   * @tparam UnaryPredicate Unary predicate function or functor of signature (std::shared_ptr<Interface> -> bool)
   * @param predicate instance of UnaryPredicate
   * @param moduleName String identifier of a module. If given, searches for
   *   models of @p Interface only in that module.
   *
   * @throws std::runtime_error If no models of this interface are loaded or if
   * no model matches the supplied predicate.
   *
   * @return A non-empty interface shared pointer
   */
  template<typename Interface, typename UnaryPredicate>
  std::enable_if_t<!std::is_convertible<UnaryPredicate, std::string>::value, std::shared_ptr<Interface>>
  get(UnaryPredicate&& predicate, const std::string& moduleName = "") const {
    /* NOTE: Yes, this implementation is a little wasteful in that it gathers
     * all models, and only then tests the predicate, but this is necessary
     * to maintain type erasure. Only here do we know what interface type the
     * models actually are and only here can we call a predicate on them.
     */
    auto anys = _getAll(Interface::interface, moduleName);

    if (anys.empty()) {
      throw std::runtime_error("There are no models of this interface loaded.");
    }

    for (auto& any : anys) {
      assert(!any.empty() && "Contract of a module's get states the any may not be empty!");
      auto interfacePtr = boost::any_cast<std::shared_ptr<Interface>>(any);
      assert(interfacePtr && "Contract of module's get states the wrapped pointer may not be empty!");
      if (predicate(interfacePtr)) {
        return interfacePtr;
      }
    }

    throw std::runtime_error("No model matches the supplied predicate!");
  }

  /**
   * @brief Checks whether a particular module is loaded
   *
   * @param moduleName The self-reported name of the module
   *
   * @return Whether the purported module is loaded
   */
  bool moduleLoaded(const std::string& moduleName) const;

 private:
  // Private Constructor, as required by the singleton design pattern.
  ModuleManager();

  /**
   * @brief Fetches the any from the appropriate module
   *
   * @param interface  The interface identifier that the desired module matches
   * @param model      The model's identifier
   * @param moduleName The string identifier of a particular module.
   *                   If given, queries for the exact module-model combination.
   *                   Module names will be auto expanded in addition to being tested
   *                   literally: 'XYZ' will also query for 'XYZModule'.
   *
   * @throws std::runtime_error 1) If no loaded module provides that particular interface model.
   *                            2) If the requested module does not exist.
   *                            3) If the requested module does not provide the interface model.
   *
   * @return A boost::any-wrapped std::shared_ptr<Interface>
   */
  boost::any _get(const std::string& interface, const std::string& model, const std::string& moduleName = "") const;

  /**
   * @brief Fetches all models of an interface, optionally just from a single module
   *
   * @param interface interface identifier that the module matches
   * @param moduleName The string identifier of a particular model. If not
   *                   empty, queries for models only in that particular
   *                   module. Module names will also be expanded: 'XYZ' will
   *                   also query for 'XYZModule'
   *
   * @return a vector of boost::anys wrapping std::shared_ptr<Interface>. The
   * vector may be empty, the shared_ptrs will not.
   */
  std::vector<boost::any> _getAll(const std::string& interface, const std::string& moduleName = "") const;

  /**
   * @brief Named union type to associate a shared library with its module
   */
  struct LibraryAndModules;

  static std::vector<LibraryAndModules> _sources;
};

} /* namespace Core */
} /* namespace Scine */

#endif /* CORE_MODULEMANAGER_H_ */
