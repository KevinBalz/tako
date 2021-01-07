include(tako)
add_executable(sandboxWren WIN32 "Sandbox.cpp")
tako_setup(sandboxWren)
target_link_libraries(sandboxWren PRIVATE tako)
if (DEFINED EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

tako_assets_dir("${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
