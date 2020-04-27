/**
 * @file DummyInterfaceAndModels.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DUMMY_INTERFACE_H
#define DUMMY_INTERFACE_H
/* External Includes */
#include <memory>
#include <string>

namespace Scine {
namespace Core {
/**
 * @class DummyInterface
 * @brief A dummy interface class
 */
class DummyInterface {
 public:
  static constexpr const char* interface = "dummy_interface";

  DummyInterface() = default;
  virtual ~DummyInterface() = default;

  virtual std::string name() const = 0;
};

} /* namespace Core */
} /* namespace Scine */

#endif /* DUMMY_INTERFACE_H */
