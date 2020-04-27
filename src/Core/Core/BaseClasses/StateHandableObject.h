/**
 * @file StateHandableObject.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_STATEHANDABLEOBJECT_H_
#define CORE_STATEHANDABLEOBJECT_H_
/* External Includes */
#include <memory>

namespace Scine {
namespace Core {

/**
 * @brief A naming interface for all states to be handled in SCINE
 */
class State {
 public:
  /// @brief Default constructor.
  State() = default;
  /// @brief Default destrucor.
  virtual ~State() = default;
};

/**
 * @brief An interface for all objects that should have a handable state.
 *
 * All objects that have a state or a configuration that should be extractable
 * and loadable should inherit from this interface.
 *
 * The state of such an object is to be encoded into a class derived from State.
 * A state should represent a momentary snapshot of a given object
 *
 * Each such object must then implement the loadState() and getState()
 * functions which are hooks for further utilities. These utilities, such as a
 * StatesHandler can be found in the Scine::Utils namespace/repository.
 */
class StateHandableObject {
 public:
  /// @brief Default constructor.
  StateHandableObject() = default;
  /// @brief Default destrucor.
  virtual ~StateHandableObject() = default;
  /**
   * @brief Loads a given state into the object.
   *
   * Note that the loaded state may be mutated by the object. It is not
   * necessarily copied into the object, even though this is likely the default
   * behaviour. Please read the documentation of the specific implementation for
   * further details.
   *
   * @param state The state to be loaded into the object.
   */
  virtual void loadState(std::shared_ptr<State> state) = 0;
  /**
   * @brief Get the current state of the object.
   *
   * Note that the state is possibly a mutable representation of the current state
   * of the object.It is not necessarily a deepcopy, eventhough this is likely the
   * default behaviour. Please read the documentation of the specific
   * implementation for further details.
   *
   * @return std::shared_ptr<State> The current state of the object.
   */
  virtual std::shared_ptr<State> getState() const = 0;
};

} /* namespace Core */
} /* namespace Scine */

#endif /* CORE_STATEHANDABLEOBJECT_H_ */
