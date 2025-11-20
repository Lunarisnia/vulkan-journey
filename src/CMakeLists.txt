# collect all imgui files
# file(GLOB SOURCES vendor/imgui/*.cpp)

# Add the source files
add_executable(${PROJECT_NAME} 
	main.cpp 

	first-triangle/app.cpp

	# Second triangle
	second-triangle/second-triangle.cpp
	second-triangle/vk-engine.cpp
	second-triangle/vk-initializer.cpp
	second-triangle/vk-image.cpp
	second-triangle/deletion-queue.cpp

	# core/shader/shader.cpp

	# Imgui
	# ${SOURCES}
	# vendor/imgui/backends/imgui_impl_opengl3.cpp
	# vendor/imgui/backends/imgui_impl_glfw.cpp
	# vendor/imgui/misc/cpp/imgui_stdlib.cpp
)

# inlcude
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
# imgui include
# target_include_directories(${PROJECT_NAME} PUBLIC vendor/imgui)

# vulkan
find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY})
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

# Link to vendor
target_link_libraries(${PROJECT_NAME} vendor)

