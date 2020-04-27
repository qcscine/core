/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_WAVEFUNCTIONOUTPUTGENERATOR_H
#define CORE_WAVEFUNCTIONOUTPUTGENERATOR_H

#include <ostream>

namespace Scine {
namespace Utils {
class AtomCollection;
} // namespace Utils
namespace Core {

/**
 * @class WavefunctionOutputGenerator @file WavefunctionOutputGenerator.h
 * @brief Interface class defining an entity able to generate a wavefunction output file.
 */
class WavefunctionOutputGenerator : public StateHandableObject, public ObjectWithStructure {
 public:
  static constexpr const char* interface = "wavefunction_output_generator";
  WavefunctionOutputGenerator() = default;
  ~WavefunctionOutputGenerator() override = default;

  /**
   * @brief Dumps the wavefunction information in a file.
   * This function is not const, as it might be necessary to perform an SCF calculation
   * beforehand or any other computation that changes members of a derived class.
   * @param out The name of the file to which to dump to.
   */
  virtual void generateWavefunctionInformation(const std::string& out) = 0;
  /**
   * @brief Dumps the wavefunction information in a stream.
   * This function can be used to implement the previous overload.
   * This function is not const, as it might be necessary to perform an SCF calculation
   * beforehand or any other computation that changes members of a derived class.
   * @param out The stream to which to dump to.
   */
  virtual void generateWavefunctionInformation(std::ostream& out) = 0;

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
};

} // namespace Core
} // namespace Scine

#endif // CORE_WAVEFUNCTIONOUTPUTGENERATOR_H
