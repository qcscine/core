/**
 * @file MMParametrizer.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef CORE_MMPARAMETRIZER_H
#define CORE_MMPARAMETRIZER_H

#include <string>

namespace Scine {

namespace Utils {
class AtomCollection;
class Settings;
} // namespace Utils

namespace Core {
/**
 * @class MMParametrizer MMParametrizer.h
 * @brief The interface for all classes parametrizing a molecular mechanics model.
 */
class MMParametrizer {
 public:
  static constexpr const char* interface = "mm_parametrizer";

  /// @brief Default constructor.
  MMParametrizer() = default;
  /// @brief Default destructor.
  virtual ~MMParametrizer() = default;

  /**
   * @brief This function generates the MM parameters for a given structure.
   *
   *        Note, that the resulting parameters will be stored in a parameter file that
   *        is specified in the settings. The same is true for a connectivity file that is generated.
   *
   * @param structure The molecular system's structure.
   */
  virtual void parametrize(Utils::AtomCollection structure) = 0;
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
   * @brief Getter for the name of the MM parametrizer.
   * @return Returns the name of the MM parametrizer.
   */
  virtual std::string name() const = 0;
};

} // namespace Core
} // namespace Scine

#endif // CORE_MMPARAMETRIZER_H
