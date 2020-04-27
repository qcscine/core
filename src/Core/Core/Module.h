/**
 * @file Module.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_MODULE_H_
#define CORE_MODULE_H_
/* Internal Includes */
#include "Core/Exceptions.h"

/* External Includes */
#include <boost/any.hpp>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace Scine {
namespace Core {

/**
 * @brief Abstract base class for a module, which flexibly provides consumers
 *   with common interface derived classes.
 *
 * Modules supply consumers with instances of types satisfying a particular
 * common interface concept. Any library that wishes to provide classes
 * satisfying interfaces defined in Core must implement a single derived class
 * of this interface.
 *
 * Implementation notes:
 * - The has / get interface of a module is double-blind so that it can be used
 *   polymorphically within the ModuleManager. Its interface does not require
 *   knowledge of the Interface class nor the type of the model that satisfies
 *   it. This is for ease of implementation for derived classes.
 * - Get returns a type-erased wrapper around a shared_ptr to the base class of
 *   the interface that the desired model satisfies.
 *
 * A class deriving from Module that will be part of a compilation unit
 * intended to become a runtime dynamically-loaded library must offer an entry
 * point for ModuleManager at global scope. A Module-derived class header file
 * should look roughly like this:
 *
 * @code{.cpp}
 * #include <Core/Module.h>
 * #include <boost/dll/alias.hpp>
 * #include <memory>
 *
 * namespace XYZ {
 *
 * class FooModule : public Core::Module {
 * public:
 *   // Implement name()
 *   // Use DerivedModule.h to implement has(...), get(...), announceInterfaces(),
 *   //   announceModels(...)
 * };
 *
 * std::vector<std::shared_ptr<Core::Module>> moduleFactory() {
 *   return {std::make_shared<FooModule>()};
 * }
 *
 * } // namespace XYZ
 *
 * // At global scope, declare an entry point called "moduleFactory"
 * BOOST_DLL_ALIAS(XYZ::moduleFactory, moduleFactory)
 * @endcode
 */
class Module {
 public:
  // Virtual destructor necessary
  virtual ~Module() = default;

  //!@name Mandatory virtual interface
  //!@{
  //! Announces the module's name
  virtual std::string name() const noexcept = 0;

  /*!
   * @brief Creates a type-erased wrapper around a shared_ptr to a model of a
   *   interface.
   *
   * @note Derived classes can implement this function using DerivedModule.h:
   * @code{.cpp}
   * boost::any FooModule::get(const std::string& interface, const std::string& model) const final {
   *  boost::any resolved = Scine::Core::DerivedModule::resolve<InterfaceModelMap>(interface, model);
   *
   *  if (resolved.empty()) {
   *    throw Scine::Core::ClassNotImplementedError();
   *  }
   *  return resolved;
   * }
   * @endcode
   * @throws X If the derived class does not supply models of the interface.
   *
   * @returns A type-erased interface model. Use try_cast to extract a pointer
   *   to the desired interface interface.
   */
  virtual boost::any get(const std::string& /* interface */, const std::string& /* model */
                         ) const = 0;

  /*!
   * @brief Checks if this module supplies a particular model of an interface.
   *
   * @note Derived classes can implement this function using DerivedModule.h:
   * @code{.cpp}
   * bool FooModule::has(const std::string& interface, const std::string& model) const noexcept final {
   *   return Core::DerivedModule::has<InterfaceModelMap>(interface, model);
   * }
   * @endcode
   */
  virtual bool has(const std::string& /* interface */, const std::string& /* model */
                   ) const noexcept = 0;

  /*!
   * @brief Announces all interfaces of which the module provides at least one model
   *
   * @note Derived classes can implement this function using DerivedModule.h:
   * @code{.cpp}
   * bool FooModule::announceInterfaces() const noexcept final {
   *   return Core::DerivedModule::announceInterfaces<InterfaceModelMap>();
   * }
   * @endcode
   */
  virtual std::vector<std::string> announceInterfaces() const noexcept = 0;

  /*! Announces all models of a particular interface that the module provides
   *
   * @note If the class supplies no models of a particular interface, this list
   *   is empty.
   *
   * @note Derived classes can implement this function using DerivedModule.h:
   * @code{.cpp}
   * bool FooModule::announceModels(const std::string& interface) const noexcept final {
   *   return Core::DerivedModule::announceModels<InterfaceModelMap>(interface);
   * }
   * @endcode
   */
  virtual std::vector<std::string> announceModels(const std::string& interface) const noexcept = 0;
  //!@}
};

} /* namespace Core */
} /* namespace Scine */

#endif /* CORE_MODULE_H_ */
