cmake_minimum_required(VERSION 3.30)
project("tako")

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Magic to enable C++20 modules
set(CMAKE_CXX_EXTENSIONS OFF)
#set(CXX_SCAN_FOR_MODULES OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(EMSCRIPTEN)
    set(EMSCRIPTEN_PTHREADS_COMPILER_FLAGS "-pthread")
    set(EMSCRIPTEN_PTHREADS_LINKER_FLAGS "${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")

    string(APPEND CMAKE_C_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " ${EMSCRIPTEN_PTHREADS_LINKER_FLAGS}")
endif()

add_subdirectory("Utils")
add_subdirectory("dependencies/fmt" EXCLUDE_FROM_ALL)
add_subdirectory("dependencies/yaml-cpp" EXCLUDE_FROM_ALL)
add_subdirectory("dependencies/json" EXCLUDE_FROM_ALL)

set(FASTGLTF_COMPILE_AS_CPP20 ON CACHE BOOL "" FORCE)
#set(FASTGLTF_ENABLE_CPP_MODULES ON CACHE BOOL "" FORCE)
add_subdirectory("dependencies/fastgltf" EXCLUDE_FROM_ALL)
add_subdirectory("dependencies/RmlUi" EXCLUDE_FROM_ALL)

if (NOT EMSCRIPTEN)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	add_subdirectory("dependencies/glfw" EXCLUDE_FROM_ALL)

	FetchContent_Declare(
		dawn
		URL https://github.com/google/dawn/archive/66bf64e67071911fffd159a4e3d556d055e76d85.zip
		FIND_PACKAGE_ARGS NAMES Dawn
	)

	#add_subdirectory("dependencies/assimp" EXCLUDE_FROM_ALL)
endif()

add_subdirectory("Tako")

# Examples
add_subdirectory("Examples/Sandbox")
add_subdirectory("Examples/Sandbox3D")
add_subdirectory("Examples/JamPhysics2D")
add_subdirectory("Examples/LDtkImport")
