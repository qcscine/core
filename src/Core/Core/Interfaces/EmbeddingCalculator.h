/**
 * @file Calculator.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_EMBEDDINGCALCULATOR_H
#define CORE_EMBEDDINGCALCULATOR_H
/* Internal Includes */
#include "Core/ExportControl.h"
#include "Core/Interfaces/Calculator.h"
/* External Includes */
#include <Eigen/Core>
#include <memory>
#include <string>
#include <vector>

namespace Scine {
namespace Utils {
class Settings;
class Results;
} // namespace Utils
namespace Core {
class Calculator;

class EmbeddingCalculator : public Calculator {
 public:
  static constexpr const char* interface = "embedding_calculator";

  /// @brief Default constructor.
  EmbeddingCalculator() = default;
  /**
   * @brief Sets the underlying calculators for a calculation performed with the embedding calculator.
   * @note In the derived class, care must be taken of the exakt embedding method (for instance, QM/QM or QM/MM) and
   * that the overall Hamiltonian is correctly constructed.
   * @param underlyingCalculators A vector of Calculators.
   */
  virtual void setUnderlyingCalculators(std::vector<std::shared_ptr<Calculator>> underlyingCalculators) = 0;
  /**
   * @brief Accessor for the underlying calculators.
   * @return std::vector<std::shared_ptr<Calculator>> A vector of underlying calculators.
   */
  virtual std::vector<std::shared_ptr<Calculator>> getUnderlyingCalculators() const = 0;
  /**
   * @brief Passes the settings of the underlying calculators to the settings of the embedding calculator.
   * @note In the derived class, care must be taken that the underlying calculators have been set before.
   */
  virtual void addUnderlyingSettings() = 0;
  /// @brief Default destructor.
  virtual ~EmbeddingCalculator() = default;

 private:
  // The underlying calculators.
  std::vector<std::shared_ptr<Calculator>> underlyingCalculators_;
};

} // namespace Core
} // namespace Scine

#endif // CORE_CALCULATORWITHREFERENCE_H
