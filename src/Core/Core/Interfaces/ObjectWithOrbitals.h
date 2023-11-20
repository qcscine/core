/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_OBJECTWITHORBITALS_H
#define CORE_OBJECTWITHORBITALS_H

#include "Core/BaseClasses/StateHandableObject.h"
#include <ostream>

namespace Scine {
namespace Utils {
class MolecularOrbitals;
} // namespace Utils
namespace Core {

/**
 * @class ObjectWithOrbitals @file ObjectWithOrbitals.h
 * @brief Interface class defining an entity having molecular orbitals.
 */
class ObjectWithOrbitals : public StateHandableObject {
 public:
  static constexpr const char* interface = "object_with_orbitals";
  ObjectWithOrbitals() = default;
  ~ObjectWithOrbitals() override = default;
  /**
   * @brief Sets (or changes) the molecular orbitals.
   *
   * @param mos New Utils::MolecularOrbitals to set.
   */
  virtual void setOrbitals(const Utils::MolecularOrbitals& mos) = 0;
};

} // namespace Core
} // namespace Scine

#endif // CORE_OBJECTWITHORBITALS_H
