/**
 * @file SampleModule.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 *
 * Steps to your own module implementation:
 *
 * - Adjust the first include according to the rename
 * - Remove the DummyInterface and DummyModels includes
 * - Add includes to all of your classes implementing a Core interface that you
 *   wish to provide
 * - Adjust the sample_namespace namespace
 * - Adjust the InterfaceModelMap using declaration
 * - Adjust all instances of the SampleModule class name
 * - Adjust the string in ::name to provide a string identifying your Module
 * - Delete this documentation
 */
#include "SampleModule.h"
#include "DummyInterface.h"
#include "DummyModels.h"
#include <Core/DerivedModule.h>
#include <Core/Exceptions.h>

namespace sample_namespace {

/* Instructions and examples to adjust the interface to model map typedef
 *
 * Welcome to template metaprogramming! This map contains only type names
 * instead of values. Apart from that little difference, conceptually it is
 * still a map and denotes which interface models belong to which interfaces.
 * To adjust this to the interface models your module will provide:
 *
 * - For each interface for which you have a class implementing it, add a pair
 *   as below to the map, separated by commas.
 * - If you have multiple classes implementing a particular interface, add them
 *   within the vector part of the pair.
 *
 * E.g. If, besides the `DummyInterface` and its models `DummyModelA` and
 * `DummyModelB`, there were an additional interface called `SampleInterface` and
 * you had a class implementing its interface `SampleModel`, you would
 * add the lines marked with a `+`:
 *
 *   using InterfaceModelMap = boost::mpl::map<
 *     boost::mpl::pair<
 *       Scine::Core::DummyInterface,
 *       boost::mpl::vector<DummyModelA, DummyModelB> // Comma between model names
 * +   >, // Comma needed here
 * +   boost::mpl::pair<
 * +     Scine::Core::SampleInterface,
 * +     boost::mpl::vector<SampleModel>
 * +   >
 *   >;
 *
 * After you are done, delete this documentation or leave it or write your own.
 * You can always refer to this source file again if you need to.
 */
using InterfaceModelMap =
    boost::mpl::map<boost::mpl::pair<Scine::Core::DummyInterface, boost::mpl::vector<DummyModelA, DummyModelB>>>;

std::string SampleModule::name() const noexcept {
  return "SampleModule";
}

boost::any SampleModule::get(const std::string& interface, const std::string& model) const {
  boost::any resolved = Scine::Core::DerivedModule::resolve<InterfaceModelMap>(interface, model);

  // Throw an exception if we could not match an interface or model
  if (resolved.empty()) {
    throw Scine::Core::ClassNotImplementedError();
  }

  return resolved;
}

bool SampleModule::has(const std::string& interface, const std::string& model) const noexcept {
  return Scine::Core::DerivedModule::has<InterfaceModelMap>(interface, model);
}

std::vector<std::string> SampleModule::announceInterfaces() const noexcept {
  return Scine::Core::DerivedModule::announceInterfaces<InterfaceModelMap>();
}

std::vector<std::string> SampleModule::announceModels(const std::string& interface) const noexcept {
  return Scine::Core::DerivedModule::announceModels<InterfaceModelMap>(interface);
}

std::shared_ptr<Scine::Core::Module> SampleModule::make() {
  return std::make_shared<SampleModule>();
}

std::vector<std::shared_ptr<Scine::Core::Module>> moduleFactory() {
  return {SampleModule::make()};
}

} /* namespace sample_namespace */
