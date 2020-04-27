/**
 * @file ModuleManager.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory for Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/*Internal Includes*/
#include "Core/ModuleManager.h"
/* External Includes */
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/dll/shared_library.hpp>

namespace Scine {
namespace Core {

namespace detail {

bool patternMatchModuleLibrary(const std::string& filename) {
  std::string expectedModuleSuffix = "module" + boost::dll::shared_library::suffix().string();
  return filename.find(expectedModuleSuffix) != std::string::npos;
}

} // namespace detail

struct ModuleManager::LibraryAndModules {
  using ModulePtr = std::shared_ptr<Module>;
  using ModuleList = std::vector<ModulePtr>;
  boost::dll::shared_library library;
  ModuleList modules;

  static boost::dll::shared_library tryLoad(const boost::filesystem::path& libraryPath) {
    boost::system::error_code error;
    boost::dll::shared_library library;

    library.load(libraryPath, error, boost::dll::load_mode::append_decorations);

    // Check for problems
    if (static_cast<bool>(error)) {
      throw std::runtime_error(error.message());
    }

    return library;
  }

  /* NOTE: default constructor is implicitly deleted since SharedLibrary is
   * not default-constructible
   */

  explicit LibraryAndModules(boost::dll::shared_library shlib) : library(std::move(shlib)) {
    if (!library.has("moduleFactory")) {
      throw std::runtime_error("Module loaded does not have signature for Core");
    }

    // Predefine the function signature of the module factory function
    using ModuleFactorySignature = std::vector<std::shared_ptr<Module>>();
    /* Import the module factory function symbol from the shared library and
     * immediately call it, constructing a module base class pointer
     */
    modules = library.get_alias<ModuleFactorySignature>("moduleFactory")();
  }
  explicit LibraryAndModules(const boost::filesystem::path& libraryPath) : LibraryAndModules(tryLoad(libraryPath)) {
  }
};

std::vector<ModuleManager::LibraryAndModules> ModuleManager::_sources;

void ModuleManager::load(const boost::filesystem::path& libraryPath) {
  // Forward the path to the LibraryAndModule constructor
  LibraryAndModules lib(libraryPath);

  // Throw if the module is already loaded
  for (auto& modulePtr : lib.modules) {
    if (moduleLoaded(modulePtr->name())) {
      throw std::runtime_error("Module is already loaded");
    }
  }

  _sources.push_back(std::move(lib));
}

void ModuleManager::load(boost::dll::shared_library library) {
  // Forward the shared_library to the LibraryAndModule constructor
  LibraryAndModules lib(std::move(library));

  // Throw if the module is already loaded
  for (auto& modulePtr : lib.modules) {
    if (moduleLoaded(modulePtr->name())) {
      throw std::runtime_error("Module is already loaded");
    }
  }

  _sources.push_back(std::move(lib));
}

std::vector<std::string> ModuleManager::getLoadedModuleNames() const {
  std::vector<std::string> list;
  list.reserve(_sources.size());
  for (const auto& source : _sources) {
    for (const auto& modulePtr : source.modules) {
      list.push_back(modulePtr->name());
    }
  }
  return list;
}

std::vector<std::string> ModuleManager::getLoadedInterfaces() const {
  // Treat this vector like a cheap set
  std::vector<std::string> interfaces;

  for (const auto& source : _sources) {
    for (const auto& modulePtr : source.modules) {
      auto sourceInterfaces = modulePtr->announceInterfaces();
      for (auto& interfaceName : sourceInterfaces) {
        auto findIter = std::lower_bound(std::begin(interfaces), std::end(interfaces), interfaceName);

        if (findIter == std::end(interfaces) || *findIter != interfaceName) {
          interfaces.insert(findIter, std::move(interfaceName));
        }
      }
    }
  }

  return interfaces;
}

std::vector<std::string> ModuleManager::getLoadedModels(const std::string& interface) const {
  std::vector<std::string> models;

  for (const auto& source : _sources) {
    for (const auto& modulePtr : source.modules) {
      auto sourceModels = modulePtr->announceModels(interface);
      std::move(std::begin(sourceModels), std::end(sourceModels), std::back_inserter(models));
    }
  }

  return models;
}

bool ModuleManager::has(const std::string& interface, const std::string& model, const std::string& moduleName) const {
  // If the module name is specified, look in detailed fashion
  if (!moduleName.empty()) {
    for (const auto& source : _sources) {
      for (const auto& modulePtr : source.modules) {
        if (modulePtr->name() != moduleName && modulePtr->name() != moduleName + "Module") {
          continue;
        }

        if (modulePtr->has(interface, model)) {
          return true;
        }
      }
    }
    return false;
  }

  for (const auto& source : _sources) {
    for (const auto& modulePtr : source.modules) {
      if (modulePtr->has(interface, model)) {
        return true;
      }
    }
  }

  return false;
}

bool ModuleManager::moduleLoaded(const std::string& moduleName) const {
  return std::find_if(std::begin(_sources), std::end(_sources), [&moduleName](const LibraryAndModules& libModule) -> bool {
           return std::find_if(std::begin(libModule.modules), std::end(libModule.modules),
                               [&moduleName](const auto& modulePtr) -> bool { return moduleName == modulePtr->name(); }) !=
                  std::end(libModule.modules);
         }) != std::end(_sources);
}

ModuleManager::ModuleManager() {
  /* Try to load modules in several paths: */
  auto executableParentPath = boost::dll::program_location().parent_path();
  const boost::filesystem::directory_iterator end;

  auto loadModulesInPath = [&](const boost::filesystem::path& directoryPath) {
    assert(boost::filesystem::is_directory(directoryPath));

    std::for_each(boost::filesystem::directory_iterator(directoryPath), end, [&](const auto& filePath) {
      if (detail::patternMatchModuleLibrary(filePath.path().filename().string())) {
        try {
          this->load(filePath);
        }
        catch (...) {
        }
      }
    });
  };

  /* Look for modules in the same path as the current executable (if all
   * installed into the same place)
   */
  loadModulesInPath(executableParentPath);

  /* Look for modules in named paths adjacent to that in which the executable is
   * (for when bin and module are adjacent)
   */
  std::vector<std::string> sameLevelDirectories{{"module", "modules"}};

  for (const auto& directoryName : sameLevelDirectories) {
    auto pathToDirectory = executableParentPath.parent_path() / directoryName;
    if (boost::filesystem::exists(pathToDirectory) && boost::filesystem::is_directory(pathToDirectory)) {
      loadModulesInPath(pathToDirectory);
    }
  }

  // SCINE_MODULE_PATH environment variable. Path separators are os-dependent
#ifdef _WIN32
  const char pathSep = ';';
#else
  const char pathSep = ':';
#endif

  if (const char* pathPtr = std::getenv("SCINE_MODULE_PATH")) {
    std::string modulePath = pathPtr;

    std::string::size_type startPos = 0, endPos;
    do {
      endPos = modulePath.find(pathSep, startPos);
      std::string singlePath = modulePath.substr(startPos, endPos);
      if (!singlePath.empty() && boost::filesystem::exists(singlePath) && boost::filesystem::is_directory(singlePath)) {
        loadModulesInPath(singlePath);
      }
      /* If endPos is not npos, then adding one merely skips the found delimiter
       * for the start of the next substring.
       *
       * If endPos is npos, then unsigned integer overflow: npos is the largest
       * representable unsigned integer, adding one to it is defined behavior
       * and wraps around to zero, terminating the loop.
       */
      startPos = endPos + 1;
    } while (startPos != 0);
  }
}

boost::any ModuleManager::_get(const std::string& interface, const std::string& model, const std::string& moduleName) const {
  if (!moduleName.empty()) {
    for (const auto& source : _sources) {
      for (const auto& modulePtr : source.modules) {
        if ((modulePtr->name() != moduleName) && (modulePtr->name() != moduleName + "Module")) {
          continue;
        }

        if (modulePtr->has(interface, model)) {
          return modulePtr->get(interface, model);
        }
      }
    }
  }

  for (const auto& source : _sources) {
    for (const auto& modulePtr : source.modules) {
      if (modulePtr->has(interface, model)) {
        return modulePtr->get(interface, model);
      }
    }
  }

  throw ClassNotImplementedError{};
}

std::vector<boost::any> ModuleManager::_getAll(const std::string& interface, const std::string& moduleName) const {
  std::vector<boost::any> models;

  if (!moduleName.empty()) {
    for (const auto& source : _sources) {
      for (const auto& modulePtr : source.modules) {
        if ((modulePtr->name() != moduleName) && (modulePtr->name() != moduleName + "Module")) {
          continue;
        }

        for (const auto& model : modulePtr->announceModels(interface)) {
          models.push_back(modulePtr->get(interface, model));
        }

        return models;
      }
    }
    return models;
  }

  for (const auto& source : _sources) {
    for (const auto& modulePtr : source.modules) {
      for (const auto& model : modulePtr->announceModels(interface)) {
        models.push_back(modulePtr->get(interface, model));
      }
    }
  }

  return models;
}

} /* namespace Core */
} /* namespace Scine */
