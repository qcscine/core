/**
 * @file ModuleManager.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/*Internal Includes*/
#include "Core/ModuleManager.h"
#include "Core/Impl/ModuleManager.h"

namespace Scine {
namespace Core {

ModuleManager& ModuleManager::getInstance() {
  static ModuleManager instance;
  return instance;
}

void ModuleManager::load(const boost::filesystem::path& libraryPath) {
  _impl->load(libraryPath);
}

void ModuleManager::load(boost::dll::shared_library library) {
  _impl->load(std::move(library));
}

void ModuleManager::load(std::shared_ptr<Module> module) {
  _impl->load(module);
}

std::vector<std::string> ModuleManager::getLoadedModuleNames() const {
  return _impl->getLoadedModuleNames();
}

std::vector<std::string> ModuleManager::getLoadedInterfaces() const {
  return _impl->getLoadedInterfaces();
}

std::vector<std::string> ModuleManager::getLoadedModels(const std::string& interface) const {
  return _impl->getLoadedModels(interface);
}

bool ModuleManager::has(const std::string& interface, const std::string& model, const std::string& moduleName) const {
  return _impl->has(interface, model, moduleName);
}

bool ModuleManager::moduleLoaded(const std::string& moduleName) const {
  return _impl->moduleLoaded(moduleName);
}

ModuleManager::ModuleManager() : _impl(std::make_unique<Impl>()) {
}

ModuleManager::~ModuleManager() = default;

boost::any ModuleManager::_get(const std::string& interface, const std::string& model, const std::string& moduleName) const {
  return _impl->get(interface, model, moduleName);
}

std::vector<boost::any> ModuleManager::_getAll(const std::string& interface, const std::string& moduleName) const {
  return _impl->getAll(interface, moduleName);
}

} /* namespace Core */
} /* namespace Scine */
