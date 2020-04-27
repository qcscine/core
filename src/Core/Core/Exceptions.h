/**
 * @file Exceptions.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_EXCEPTIONS_H_
#define CORE_EXCEPTIONS_H_
/* External Includes */
#include <exception>
#include <stdexcept>
#include <string>

namespace Scine {
namespace Utils {
class PropertyList;
}
namespace Core {
/**
 * @class SettingsKeyError Exceptions.h
 * @brief An Exception for an error when handling keys in settings.
 */
class SettingsKeyError : public std::exception {
  const char* what() const noexcept override {
    return "The key was not recognized in these settings.";
  }
};

/**
 * @class SettingsValueError Exceptions.h
 * @brief An Exception for an error when handling values in settings.
 */
class SettingsValueError : public std::exception {
  const char* what() const noexcept override {
    return "The value did not match the type defined by the given key.";
  }
};

/**
 * @class ClassNotImplementedError Exceptions.h
 * @brief An Exception for an error when generating classes through a module-interface.
 */
class ClassNotImplementedError : public std::exception {
  const char* what() const noexcept override {
    return "No class with the requested name is implemented.";
  }
};

/**
 * @class FunctionNotImplementedError Exceptions.h
 * @brief An Exception for an error when a function in a module-interface is not implemented.
 */
class FunctionNotImplementedError : public std::exception {
  const char* what() const noexcept override {
    return "This module does not provide the requested interface..";
  }
};

/**
 * @class StateCastingException @file Exception.h
 * @brief Exception to be thrown if the state cannot be cast to the desired type.
 */
class StateCastingException : public std::exception {
  const char* what() const noexcept final {
    return "State pointer does not have a compatible underlying type.";
  }
};

/**
 * @class InvalidPropertiesException Exceptions.h
 * @brief Exception thrown when one requires properties from a calculation which cannot be calculated.
 */
class InvalidPropertiesException : public std::runtime_error {
 public:
  explicit InvalidPropertiesException(const Utils::PropertyList& /*unused*/)
    : std::runtime_error("Calculator cannot calculate required properties.") {
  }
};

/**
 * @class CalculationException Exceptions.h
 * @brief Base class for exceptions thrown during calculations.
 *   */
class CalculationException : public std::runtime_error {
 protected:
  /** protected ctor: instantiation only in derived classes. */
  explicit CalculationException(const std::string& s) : std::runtime_error(s) {
  }
};

/**
 * @class InitializationException Exceptions.h
 * @brief Exception thrown when a problem arises in the calculator initialization.
 */
class InitializationException : public CalculationException {
 public:
  explicit InitializationException(const std::string& s) : CalculationException("Initialization error: " + s) {
  }
};

/**
 * @class EmptyMolecularStructureException Exceptions.h
 * @brief Exception thrown when launching a calculation with an empty structure.
 */
class EmptyMolecularStructureException : public CalculationException {
 public:
  explicit EmptyMolecularStructureException()
    : CalculationException("Cannot calculate properties for empty structure") {
  }
};

/**
 * @class UnsuccessfulCalculationException Exceptions.h
 * @brief Exception thrown when a calculation is unsuccessful
 *   */
class UnsuccessfulCalculationException : public CalculationException {
 public:
  explicit UnsuccessfulCalculationException(const std::string& s) : CalculationException(s) {
  }
};

/**
 * @class StateSavingException Exceptions.h
 * @brief Exception thrown for errors in state saving / resetting.
 */
class StateSavingException : public CalculationException {
 public:
  explicit StateSavingException(const std::string& s) : CalculationException("State-saving error: " + s) {
  }
};

} /* namespace Core */
} /* namespace Scine */

#endif /* CORE_EXCEPTIONS_H_ */
