/**
 * @file
 * @brief Provides helpers for the general implementation of classes deriving from Module
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef CORE_DERIVED_MODULE_
#define CORE_DERIVED_MODULE_

// Minimal hana headers
#include "boost/hana/for_each.hpp"
#include "boost/hana/keys.hpp"
#include "boost/hana/members.hpp"
#include "boost/hana/none_of.hpp"
#include "boost/hana/size.hpp"
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

namespace Scine {
namespace Core {
namespace DerivedModule {

template<typename Derived>
bool has(const std::string& interface, const std::string& model, const Derived& derived) noexcept {
  return boost::hana::any_of(boost::hana::accessors<Derived>(), [&](const auto& accessorPair) {
    // Check that the member name matches the interface we are looking for
    if (boost::hana::first(accessorPair).c_str() == interface) {
      // Look for an interface model in that type's list of supplied interfaces
      const auto& models = boost::hana::second(accessorPair)(derived);
      auto findIter = std::find(std::begin(models), std::end(models), model);
      return findIter != std::end(models);
    }

    return false;
  });
}

template<typename Derived>
std::vector<std::string> announceInterfaces(const Derived& derived) noexcept {
  std::vector<std::string> interfaces;
  interfaces.reserve(boost::hana::size(boost::hana::keys(derived)));

  boost::hana::for_each(boost::hana::keys(derived), [&](const auto& x) { interfaces.emplace_back(x.c_str()); });

  return interfaces;
}

template<typename Derived>
std::vector<std::string> announceModels(const std::string& interface, const Derived& derived) noexcept {
  std::vector<std::string> models;

  boost::hana::any_of(boost::hana::accessors<Derived>(), [&](const auto& x) {
    if (boost::hana::first(x).c_str() == interface) {
      // Copy the models if a match is found
      models = boost::hana::second(x)(derived);

      // Early exit
      return true;
    }

    // Keep looking
    return false;
  });

  return models;
}

template<typename Derived>
void checkInvariants(const Derived& derived) {
  assert(boost::hana::none_of(boost::hana::members(derived), [&derived](const auto& x) { return x.empty(); }) &&
         "A module may not have empty interface lists!");
}

} // namespace DerivedModule
} // namespace Core
} // namespace Scine

#endif
