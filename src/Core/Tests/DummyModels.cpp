/**
 * @file DummyModels.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include "DummyModels.h"

namespace sample_namespace {

std::string DummyModelA::name() const {
  return "DummyA";
}

std::string DummyModelB::name() const {
  return "DummyB";
}

} // namespace sample_namespace
