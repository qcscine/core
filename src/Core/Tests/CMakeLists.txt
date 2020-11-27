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

set(CORE_TEST_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/CoreTests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Log.cpp
)

# Build the tests
add_executable(CoreTests ${CORE_TEST_FILES})
target_link_libraries(CoreTests PRIVATE Scine::Core GTest::Main GMock::GMock)
add_test(NAME Core COMMAND CoreTests)
