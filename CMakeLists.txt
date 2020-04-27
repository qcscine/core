cmake_minimum_required(VERSION 3.9)

# Set the project name. The chosen project name must be in CamelCase. Your CMake
# tree must then provide a properly namespaced target with the same name as
# your project.
project(Core
  VERSION 2.0.0
  DESCRIPTION "Module management and core interface definitions"
)

# Set the module path for universal cmake files inclusion
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Component setup
include(ComponentSetup)
scine_setup_component()

# Testing
if(SCINE_BUILD_TESTS)
  enable_testing()
endif()

# Subdirectories
add_subdirectory(src)
