# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")    # Disable shared builds on platforms where it does not make sense to use them
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

# SDL2
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# GLM
add_subdirectory(glm EXCLUDE_FROM_ALL)

# glfw
add_subdirectory(glfw EXCLUDE_FROM_ALL)

# vulkan bootstrap
add_subdirectory(vk-bootstrap EXCLUDE_FROM_ALL)

# fmt
add_subdirectory(fmt EXCLUDE_FROM_ALL)

# Add library named vendor as an interface
add_library(vendor INTERFACE)

# target_link_libraries(vendor INTERFACE glad)
target_link_libraries(vendor INTERFACE glfw)
target_link_libraries(vendor INTERFACE glm::glm)
target_link_libraries(vendor INTERFACE vk-bootstrap::vk-bootstrap)
target_link_libraries(vendor INTERFACE fmt::fmt)
target_link_libraries(vendor INTERFACE SDL2::SDL2)
