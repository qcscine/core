/**
 * @file CoreTests.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include "DummyInterface.h"
#include <Core/BaseClasses/ObjectWithLog.h>
#include <Core/Impl/ModuleManager.h>
#include <Core/Log.h>
#include <Core/ModuleManager.h>
#include <gmock/gmock.h>

using namespace Scine;
using namespace Core;

TEST(CoreModules, ModuleCorrectness) {
  auto& manager = ModuleManager::getInstance();

  if (!manager.moduleLoaded("SampleModule")) {
    manager.load("samplemodule");
  }

  const auto moduleNames = manager.getLoadedModuleNames();
  ASSERT_FALSE(moduleNames.empty());
  ASSERT_EQ(moduleNames.front(), "SampleModule");
  ASSERT_TRUE(manager.moduleLoaded("SampleModule"));

  const auto interfaces = manager.getLoadedInterfaces();
  ASSERT_FALSE(interfaces.empty());
  ASSERT_TRUE(std::find(interfaces.begin(), interfaces.end(), "dummy_interface") != interfaces.end());

  const auto models = manager.getLoadedModels("dummy_interface");
  ASSERT_EQ(models.size(), 2);
  ASSERT_EQ(models.front(), "dummy_a");
  ASSERT_EQ(models.back(), "dummy_b");

  ASSERT_TRUE(manager.has("dummy_interface", "dummy_a"));
  ASSERT_TRUE(manager.has("dummy_interface", "dummy_b"));
  ASSERT_TRUE(manager.has("dummy_interface", "dummy_a", "SampleModule"));
  ASSERT_FALSE(manager.has("dummy_interface", "nonexistent_model"));

  auto barPtr = manager.get<DummyInterface>("dummy_a");
  ASSERT_EQ(barPtr->name(), "DummyA");

  auto bazPtr = manager.get<DummyInterface>("dummy_b");
  ASSERT_EQ(bazPtr->name(), "DummyB");
}

TEST(CoreModules, StringSplit) {
  const std::vector<std::string> expected{{"a", "b", "c", "d"}};
  ASSERT_EQ(Detail::split("a;b;c;d", ';'), expected);
}

class FakeModule : public Scine::Core::Module {
 public:
  std::string name() const noexcept final {
    return "FakeModule";
  }

  boost::any get(const std::string& /* interface */, const std::string& /* model */) const final {
    return NULL;
  }

  bool has(const std::string& /* interface */, const std::string& /* model */) const noexcept final {
    return false;
  }

  std::vector<std::string> announceInterfaces() const noexcept final {
    return {};
  }

  std::vector<std::string> announceModels(const std::string& /* interface */) const noexcept final {
    return {};
  }

  static std::shared_ptr<Scine::Core::Module> make() {
    return std::make_shared<FakeModule>();
  }
};

TEST(CoreModules, DirectModuleLoad) {
  auto& manager = ModuleManager::getInstance();
  auto module = std::make_shared<FakeModule>();
  manager.load(module);
}

TEST(ObjectWithLog, Basics) {
  struct Foo : public ObjectWithLog {};

  Foo f;                // default ctor
  Foo g{Foo()};         // move ctor
  Foo h(g);             // copy ctor
  Foo i = std::move(f); // move assign
  g = i;                // copy assign

  h.getLog();
  i.setLog(Log::silent());
}
