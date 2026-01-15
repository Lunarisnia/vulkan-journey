# Set the minimum allowed cmake version
cmake_minimum_required(VERSION 3.10)
# Set the project name
project(VulkanJourney)

# Set so that cmake spits out compile_commands.txt to be used by clangd to import
# Solution: https://gist.github.com/Strus/042a92a00070a943053006bf46912ae9
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the compiler standard the program will be compiled in
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This splits the cmake file to its own file per directory, this one links to the one on the root of vendor
add_subdirectory(vendor EXCLUDE_FROM_ALL)

# This splits the cmake file to its own file per directory, this one links to the one on the root of src
add_subdirectory(src)

# Neat trick to copy all assets to build directory
# https://stackoverflow.com/questions/42476765/copying-assets-directory-from-source-to-build-directory-in-cmake
# file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
# file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
# file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
    )

foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  message(STATUS COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
	Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
	COMMENT "Compiling shaders to SPIR-V"
    )

