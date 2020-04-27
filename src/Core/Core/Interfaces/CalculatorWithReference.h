/**
 * @file Calculator.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_CALCULATORWITHREFERENCE_H
#define CORE_CALCULATORWITHREFERENCE_H
/* Internal Includes */
#include "Core/ExportControl.h"
/* External Includes */
#include <Eigen/Core>
#include <memory>
#include <string>

namespace Scine {
namespace Utils {
class Settings;
class Results;
} // namespace Utils
namespace Core {

class Calculator;

/**
 * @class CalculatorWithReference CalculatorWithReference.h
 * @brief The interface for all classes running calculations on top of a
 *        reference calculation.
 * This can be, for example, excited states calculation (CIS, TD-DFT,..), post-HF,
 * but also thermodynamics calculations and the CISE approach.
 */
class CalculatorWithReference {
 public:
  static constexpr const char* interface = "calculatorWithReference";
  /// @brief Default constructor.
  CalculatorWithReference() = default;
  /// @brief Virtual destructor.
  virtual ~CalculatorWithReference() = default;
  /**
   * @brief Sets the calculator to be used to perform the reference calculation.
   * In the derived classes care must be taken that the case where a method does not accept
   * some calculator types (i.e CIS with DFT, or TDDFT with HF) is checked and handled.
   */
  virtual void setReferenceCalculator(std::shared_ptr<Calculator> referenceCalculator) = 0;
  /**
   * @brief Performs a reference calculation.
   */
  virtual void referenceCalculation() = 0;

  /**
   * @brief Accessor for the reference calculator.
   * @return Core::Calculator& The reference calculator.
   */
  virtual Core::Calculator& getReferenceCalculator() = 0;
  /**
   * @brief Constant accessor for the reference calculator.
   * @return const Core::Calculator& The reference calculator.
   */
  virtual const Core::Calculator& getReferenceCalculator() const = 0;

  /**
   * @brief Accessor for the calculator, if present.
   * @return Core::Calculator& The calculator.
   */
  virtual Core::Calculator& getCalculator() = 0;
  /**
   * @brief Constant accessor for the calculator, if present.
   * @return const Core::Calculator& The calculator.
   */
  virtual const Core::Calculator& getCalculator() const = 0;

  /**
   * @brief The main function running the calculation.
   * @return ReturnType Since different methods can give different return types,
   *                    a templetized return type has been chosen.
   *                    The derived class must derive from the right version
   *                    of the CalculatorWithReference<ReturnType> class.
   */
  virtual Utils::Results calculate() = 0;

  /**
   * @brief Getter for the name of the calculator with reference.
   * @return Returns the name of the calculator with reference.
   */
  virtual std::string name() const = 0;

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
   * @brief Method to apply the settings stored in the settings data structure.
   */
  virtual void applySettings() = 0;
};

} // namespace Core
} // namespace Scine

#endif // CORE_CALCULATORWITHREFERENCE_H
