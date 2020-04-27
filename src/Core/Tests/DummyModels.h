/**
 * @file DummyInterfaceAndModels.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DUMMY_MODELS_H
#define DUMMY_MODELS_H
/* External Includes */
#include "DummyInterface.h"
#include <string>

namespace sample_namespace {

struct DummyModelA : public Scine::Core::DummyInterface {
  static constexpr const char* model = "dummy_a";
  std::string name() const final;
};

struct DummyModelB : public Scine::Core::DummyInterface {
  static constexpr const char* model = "dummy_b";
  std::string name() const final;
};

} // namespace sample_namespace

#endif /* DUMMY_INTERFACE_H */
