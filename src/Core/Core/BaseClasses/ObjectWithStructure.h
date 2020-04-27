/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_OBJECTWITHSTRUCTURE_H
#define CORE_OBJECTWITHSTRUCTURE_H

#include <Eigen/Core>
#include <memory>

namespace Scine {
namespace Utils {
using PositionCollection = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;
class AtomCollection;
} // namespace Utils
namespace Core {

/**
 * @class ObjectWithStructure @file ObjectWithStructure.h
 * @brief Interface class defining an entity having a molecular structure.
 * This solves the diamond inheritance problem where multiple different classes have a setStructure() method.
 * A derived class cannot know which method to choose from.
 * This way, there is only one setStructure() method.
 */
class ObjectWithStructure {
 public:
  ObjectWithStructure() = default;
  virtual ~ObjectWithStructure() = default;

  /**
   * @brief Sets (or changes) the molecular structure.
   *
   * @param structure A new Utils::AtomCollection to set.
   */
  virtual void setStructure(const Utils::AtomCollection& structure) = 0;
  /**
   * @brief Returns the molecular structure.
   *
   * @return The molecular structure as const Utils::AtomCollection&.
   */
  virtual std::unique_ptr<Utils::AtomCollection> getStructure() const = 0;
  /**
   * @brief Modifies the atomic coordinates of the molecular structure.
   *
   * @param newPositions The new atomic coordinates to be assigned to the underlying Utils::AtomCollection.
   */
  virtual void modifyPositions(Utils::PositionCollection newPositions) = 0;
  /**
   * @brief Getter for the atomic coordinates of the molecular structure.
   *
   * @return The atomic coordinates as const Utils::PositionCollection&.
   */
  virtual const Utils::PositionCollection& getPositions() const = 0;
};

} // namespace Core
} // namespace Scine

#endif // CORE_OBJECTWITHSTRUCTURE_H
