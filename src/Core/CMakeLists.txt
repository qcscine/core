include(Files.cmake)

find_package(Boost REQUIRED COMPONENTS system filesystem)

add_library(CoreHeaders INTERFACE)
target_include_directories(CoreHeaders INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/Scine>
)
target_link_libraries(CoreHeaders INTERFACE Boost::boost)

# Core is always a shared lib because of the module manager singleton
add_library(Core SHARED ${CORE_HEADERS} ${CORE_CPPS})
set_target_properties(Core PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME core
  CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_libraries(Core
  PUBLIC
    CoreHeaders
    Boost::filesystem
  PRIVATE
    ${CMAKE_DL_LIBS}
)

if(MSVC)
  target_compile_definitions(Core PUBLIC BOOST_ALL_NO_LIB)
endif()

macro(determine_lib_type target)
  get_target_property(LIB_TYPE ${target} TYPE)
  if("${LIB_TYPE}" STREQUAL "UNKNOWN_LIBRARY")
    get_target_property(_target_location ${target} LOCATION)
    get_filename_component(_target_extension ${_target_location} EXT)
    if(${_target_extension} STREQUAL ${CMAKE_SHARED_LIBRARY_SUFFIX})
      set(LIB_TYPE "SHARED_LIBRARY")
    elseif(${_target_extension} STREQUAL ${CMAKE_STATIC_LIBRARY_SUFFIX})
      set(LIB_TYPE "STATIC_LIBRARY")
    endif()
    unset(_target_location)
    unset(_target_extension)
  endif()
endmacro()

determine_lib_type(Boost::filesystem)
if("${LIB_TYPE}" STREQUAL "SHARED_LIBRARY")
  target_compile_definitions(Core PUBLIC BOOST_ALL_DYN_LINK)
endif()
unset(LIB_TYPE)

if(NOT "${SCINE_MARCH}" STREQUAL "" AND NOT MSVC)
  target_compile_options(Core PUBLIC -march=${SCINE_MARCH})
  target_compile_options(CoreHeaders INTERFACE -march=${SCINE_MARCH})
endif()


# Add namespaced aliases
add_library(Scine::CoreHeaders ALIAS CoreHeaders)
add_library(Scine::Core ALIAS Core)

# -- Define Install
# Headers
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Core/
  DESTINATION include/Scine/Core
  FILES_MATCHING
    PATTERN "*.h"
    PATTERN "Impl*" EXCLUDE
)

# Targets
install(
  TARGETS CoreHeaders Core
  EXPORT CoreTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Component-level cmake package files
scine_install_component_cmake_files(
  COMPONENT ScineCore
  EXPORT_NAME CoreTargets
)

if(SCINE_BUILD_TESTS)
  add_subdirectory(Tests)
endif()
