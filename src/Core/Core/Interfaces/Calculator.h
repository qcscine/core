/**
 * @file Calculator.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_CALCULATOR_H_
#define CORE_CALCULATOR_H_
/* Internal Includes */
#include "Core/ExportControl.h"
/* External Includes */
#include <Eigen/Core>
#include <memory>
#include <string>

namespace Scine {

namespace Utils {
class AtomCollection;
class Results;
class Settings;
class StatesHandler;
class PropertyList;
using PositionCollection = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;
} // namespace Utils

namespace Core {
/**
 * @class Calculator Calculator.h
 * @brief The interface for all classes running electronic structure calculations.
 */
class Calculator {
 public:
  static constexpr const char* interface = "calculator";

  /// @brief Default Constructor.
  Calculator() = default;
  /// @brief Default Destructor.
  virtual ~Calculator() = default;
  /**
   * @brief Changes the molecular structure to calculate.
   *
   * @param structure A new Utils::AtomCollection to save.
   */
  virtual void setStructure(const Utils::AtomCollection& structure) = 0;
  /**
   * @brief Gets the molecular structure as a const Utils::AtomCollection&.
   *
   * @return a const Utils::AtomCollection&.
   */
  virtual std::unique_ptr<Utils::AtomCollection> getStructure() const = 0;
  /**
   * @brief Allows to modify the positions of the underlying Utils::AtomCollection
   * @param newPositions the new positions to be assigned to the underlying Utils::AtomCollection
   */
  virtual void modifyPositions(Utils::PositionCollection newPositions) = 0;
  /**
   * @brief Getter for the coordinates of the underlying Utils::AtomCollection
   */
  virtual const Utils::PositionCollection& getPositions() const = 0;
  /**
   * @brief Sets the properties to calculate.
   * @param requiredProperties A Utils::PropertyList, a sequence of bits that represent the
   *        properties that must be calculated.
   */
  virtual void setRequiredProperties(const Utils::PropertyList& requiredProperties) = 0;
  /**
   * @brief Returns the list of the possible properties to calculate analytically.
   * By some method analytical hessian calculation is not possible. In this case the
   * hessian calculation is done seminumerically.
   */
  virtual Utils::PropertyList possibleProperties() const = 0;
  /**
   * @brief The main function running calculations (dummy).
   *
   * @param description   The calculation description.
   * @return Utils::Result Return the result of the calculation. The object contains the
   *                       properties that were given as requirement by the
   *                       Calculator::setRequiredProperties function.
   */
  virtual const Utils::Results& calculate(std::string description = "") = 0;
  /**
   * @brief Getter for the name of the Calculator.
   * @return Returns the name of the Calculator.
   */
  virtual std::string name() const = 0;
  /**
   * @brief Method allowing to clone the derived class into a Core::Calculator
   * The derived, leaf class needs to inherit from Utils::CloneInterface and,
   * if needed, implement a custom copy constructor. This reduces boilerplate
   * code.
   */
  std::unique_ptr<Core::Calculator> clone() const {
    return std::unique_ptr<Core::Calculator>(this->cloneImpl());
  }
  /**
   * @brief Accessor for the settings.
   * @return Utils::Settings& The settings.
   */
  virtual Utils::Settings& settings() = 0;
  /**
   * @brief Constant accessor for the settings.
   * @return const Utils::Settings& The settings.
   */
  virtual const Utils::Settings& settings() const = 0;
  /**
   * @brief Accessor for the StatesHandler.
   * @return Utils::StatesHandler& The StatesHandler.
   */
  virtual Utils::StatesHandler& statesHandler() = 0;
  /**
   * @brief Constant accessor for the StatesHandler.
   * @return const Utils::StatesHandler& The StatesHandler.
   */
  virtual const Utils::StatesHandler& statesHandler() const = 0;
  /**
   * @brief Accessor for the saved instance of Utils::Results.
   * @return Utils::Results& The results of the previous calculation.
   */
  virtual Utils::Results& results() = 0;
  /**
   * @brief Constant accessor for the Utils::Results.
   * @return const Utils::Results& The results of the previous calculation.
   */
  virtual const Utils::Results& results() const = 0;

 private:
  /*
   * Implementation of the clone() function, pure virtual private method.
   * It returns a pointer to allow for covariant return types in inheritance.
   */
  virtual Core::Calculator* cloneImpl() const = 0;
};

} /* namespace Core */
} /* namespace Scine */

#endif /* CORE_CALCULATOR_H_ */
