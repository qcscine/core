/**
 * @file Calculator.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_CALCULATORWITHREFERENCE_H
#define CORE_CALCULATORWITHREFERENCE_H
/* Internal Includes */
#include "Core/BaseClasses/ObjectWithLog.h"
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
 * This can be, for example, excited states calculation (CIS, TD-DFT(B),..) and post-HF.
 */
class CalculatorWithReference : public ObjectWithLog {
 public:
  static constexpr const char* interface = "calculator_with_reference";
  /// @brief Default constructor.
  CalculatorWithReference() = default;
  /// @brief Virtual destructor.
  virtual ~CalculatorWithReference() = default;
  /**
   * @brief Sets the calculator to be used to perform the reference calculation.
   * In the derived classes care must be taken that the case where a method does not accept
   * some calculator types (i.e CIS with DFT, or TDDFT with HF) is checked and handled.
   * @throws if the referenceCalculator is not a valid reference calculator for this class instance.
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
   * @brief The main function running the calculation with reference.
   * @returns A const-ref of stored (and newly calculated) Results.
   */
  virtual const Utils::Results& calculate() = 0;

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
};

} // namespace Core
} // namespace Scine

#endif // CORE_CALCULATORWITHREFERENCE_H
