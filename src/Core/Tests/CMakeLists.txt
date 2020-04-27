# Build a shared library module from SampleModule and the models
add_library(SampleModule SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/SampleModule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/DummyModels.cpp
)
set_target_properties(SampleModule PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME samplemodule
)
target_link_libraries(SampleModule PUBLIC Scine::Core)

# Build the tests
add_executable(CoreTests CoreTests.cpp)
target_link_libraries(CoreTests PRIVATE Scine::Core gtest_main gmock)
add_test(NAME Core COMMAND CoreTests)
