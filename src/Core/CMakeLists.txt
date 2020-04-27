include(Files.cmake)

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Shared lib
add_library(Core SHARED ${CORE_FILES})
target_include_directories(Core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)
set_target_properties(Core PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME core
  CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_libraries(Core
  PUBLIC
    Boost::boost
    Boost::filesystem
    ${CMAKE_DL_LIBS}
  PRIVATE
    Boost::system
)

if(NOT "${SCINE_MARCH}" STREQUAL "" AND NOT MSVC)
  target_compile_options(Core PUBLIC -march=${SCINE_MARCH})
endif()


# Add namespaced aliases
add_library(Scine::Core ALIAS Core)

# -- Define Install
# Headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Core/
  DESTINATION include/Core
        FILES_MATCHING PATTERN "*.h"
)

# Targets
install(TARGETS Core
  EXPORT CoreTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/Core
)

# Component-level cmake package files
scine_install_component_cmake_files(
  COMPONENT Core
  EXPORT_NAME CoreTargets
)

if(SCINE_BUILD_TESTS)
  add_subdirectory(Tests)
endif()
