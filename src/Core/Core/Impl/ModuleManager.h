/**
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef INCLUDE_SCINE_CORE_MODULE_MANAGER_IMPL_H
#define INCLUDE_SCINE_CORE_MODULE_MANAGER_IMPL_H

#include "Core/ModuleManager.h"
#include <boost/algorithm/string.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/dll/shared_library.hpp>
#include <sstream>

namespace Scine {
namespace Core {
namespace Detail {

inline std::vector<std::string> split(const std::string& s, char delimiter) {
  std::vector<std::string> elements;

  std::stringstream ss;
  ss.str(s);
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    elements.push_back(item);
  }

  return elements;
}

} // namespace Detail

struct ModuleManager::Impl {
  //!@name Types
  //!@{
  using ModulePtr = std::shared_ptr<Module>;
  using ModuleList = std::vector<ModulePtr>;

  struct LibraryAndModules {
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
    explicit LibraryAndModules(ModulePtr module) : modules({module}) {
    }

    boost::dll::shared_library library;
    ModuleList modules;
  };
  //!@}

  //!@name Statics
  //!@{
  static bool patternMatchModuleLibrary(const std::string& filename) {
    std::string expectedModuleSuffix = ".module" + boost::dll::shared_library::suffix().string();
    return filename.find(expectedModuleSuffix) != std::string::npos;
  }
  //!@}

  Impl() {
    /* Try to load modules in several paths: */
    const auto executableParentPath = boost::dll::program_location().parent_path();
    tryLoadModulesInPath(executableParentPath);
    tryAdjacentDirectories(executableParentPath);
    const auto coreLocationParent = boost::dll::this_line_location().parent_path();
    tryLoadModulesInPath(coreLocationParent);
    if (coreLocationParent.parent_path() != executableParentPath.parent_path()) {
      tryAdjacentDirectories(coreLocationParent);
    }

    // SCINE_MODULE_PATH environment variable. Path separators are os-dependent!
#ifdef _WIN32
    const char pathSep = ';';
#else
    const char pathSep = ':';
#endif

    if (const char* pathPtr = std::getenv("SCINE_MODULE_PATH")) {
      std::string modulePath = pathPtr;

      for (const auto& singlePath : Detail::split(modulePath, pathSep)) {
        if (!singlePath.empty() && boost::filesystem::exists(singlePath) && boost::filesystem::is_directory(singlePath)) {
          tryLoadModulesInPath(singlePath);
        }
      }
    }
  }

  //!@name Modification
  //!@{
  void load(std::shared_ptr<Module> module) {
    LibraryAndModules lib{module};

    const bool alreadyLoaded = std::any_of(std::begin(lib.modules), std::end(lib.modules),
                                           [&](const auto& modulePtr) -> bool { return moduleLoaded(modulePtr->name()); });

    if (!alreadyLoaded) {
      _sources.push_back(std::move(lib));
    }
  }

  void load(const boost::filesystem::path& libraryPath) {
    LibraryAndModules lib{libraryPath};

    const bool alreadyLoaded = std::any_of(std::begin(lib.modules), std::end(lib.modules),
                                           [&](const auto& modulePtr) -> bool { return moduleLoaded(modulePtr->name()); });

    if (!alreadyLoaded) {
      _sources.push_back(std::move(lib));
    }
  }

  void load(boost::dll::shared_library library) {
    LibraryAndModules lib{std::move(library)};

    const bool alreadyLoaded = std::any_of(std::begin(lib.modules), std::end(lib.modules),
                                           [&](const auto& modulePtr) -> bool { return moduleLoaded(modulePtr->name()); });

    if (!alreadyLoaded) {
      _sources.push_back(std::move(lib));
    }
  }

  void tryLoadModulesInPath(const boost::filesystem::path& directory) {
    if (!boost::filesystem::is_directory(directory)) {
      return;
    }

    const boost::filesystem::directory_iterator end;
    std::for_each(boost::filesystem::directory_iterator(directory), end, [&](const auto& filePath) {
      if (patternMatchModuleLibrary(filePath.path().filename().string())) {
        try {
          this->load(filePath);
        }
        catch (...) {
        }
      }
    });
  }

  void tryAdjacentDirectories(const boost::filesystem::path& directory) {
    const std::vector<std::string> sameLevelDirectories{{"module", "modules", "lib"}};

    for (const auto& directoryName : sameLevelDirectories) {
      const auto pathToDirectory = directory.parent_path() / directoryName;
      try {
        if (boost::filesystem::exists(pathToDirectory) && boost::filesystem::is_directory(pathToDirectory)) {
          tryLoadModulesInPath(pathToDirectory);
        }
      }
      catch (boost::filesystem::filesystem_error& ex) {
      }
    }
  }
  //!@}

  //!@name Information
  //!@{
  std::vector<std::string> getLoadedModuleNames() const {
    std::vector<std::string> list;
    list.reserve(_sources.size());
    for (const auto& source : _sources) {
      for (const auto& modulePtr : source.modules) {
        list.push_back(modulePtr->name());
      }
    }
    return list;
  }

  std::vector<std::string> getLoadedInterfaces() const {
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

  std::vector<std::string> getLoadedModels(const std::string& interface) const {
    std::vector<std::string> models;

    for (const auto& source : _sources) {
      for (const auto& modulePtr : source.modules) {
        auto sourceModels = modulePtr->announceModels(interface);
        std::move(std::begin(sourceModels), std::end(sourceModels), std::back_inserter(models));
      }
    }

    return models;
  }

  bool has(const std::string& interface, const std::string& model, const std::string& moduleName) const {
    // If the module name is specified, look in detailed fashion
    std::string moduleNameLower = boost::algorithm::to_lower_copy(moduleName);
    if (!moduleName.empty() && moduleNameLower != "any") {
      for (const auto& source : _sources) {
        for (const auto& modulePtr : source.modules) {
          std::string modulePtrLower = boost::algorithm::to_lower_copy(modulePtr->name());
          if ((modulePtrLower != moduleNameLower) && (modulePtrLower != moduleNameLower + "module")) {
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

  bool moduleLoaded(const std::string& moduleName) const {
    return std::find_if(std::begin(_sources), std::end(_sources), [&moduleName](const LibraryAndModules& libModule) -> bool {
             return std::find_if(std::begin(libModule.modules), std::end(libModule.modules),
                                 [&moduleName](const auto& modulePtr) -> bool { return moduleName == modulePtr->name(); }) !=
                    std::end(libModule.modules);
           }) != std::end(_sources);
  }

  boost::any get(const std::string& interface, const std::string& model, const std::string& moduleName) const {
    std::string moduleNameLower = boost::algorithm::to_lower_copy(moduleName);
    if (!moduleName.empty() && moduleNameLower != "any") {
      for (const auto& source : _sources) {
        for (const auto& modulePtr : source.modules) {
          std::string modulePtrLower = boost::algorithm::to_lower_copy(modulePtr->name());
          if ((modulePtrLower != moduleNameLower) && (modulePtrLower != moduleNameLower + "module")) {
            continue;
          }

          if (modulePtr->has(interface, model)) {
            return modulePtr->get(interface, model);
          }
        }
      }
    }
    else {
      for (const auto& source : _sources) {
        for (const auto& modulePtr : source.modules) {
          if (modulePtr->has(interface, model)) {
            return modulePtr->get(interface, model);
          }
        }
      }
    }

    throw ClassNotImplementedError{};
  }

  std::vector<boost::any> getAll(const std::string& interface, const std::string& moduleName) const {
    std::vector<boost::any> models;

    std::string moduleNameLower = boost::algorithm::to_lower_copy(moduleName);
    if (!moduleName.empty() && moduleNameLower != "any") {
      for (const auto& source : _sources) {
        for (const auto& modulePtr : source.modules) {
          std::string modulePtrLower = boost::algorithm::to_lower_copy(modulePtr->name());
          if ((modulePtrLower != moduleNameLower) && (modulePtrLower != moduleNameLower + "module")) {
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
  //!@}

  //!@name Data
  //!@{
  std::vector<LibraryAndModules> _sources;
  //!@}
};

} /* namespace Core */
} /* namespace Scine */

#endif
