# collect all imgui files
file(GLOB IMGUI_SOURCES vendor/imgui/*.cpp)

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
	second-triangle/vma-usage.cpp
	second-triangle/vk-descriptors.cpp
	second-triangle/vk-pipelines.cpp

	# core/shader/shader.cpp

	# Imgui
	${IMGUI_SOURCES}
	vendor/imgui/backends/imgui_impl_vulkan.cpp
	vendor/imgui/backends/imgui_impl_sdlrenderer2.cpp
	vendor/imgui/misc/cpp/imgui_stdlib.cpp
)

# inlcude
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
# VMA include
target_include_directories(${PROJECT_NAME} PUBLIC vendor/vma)
# imgui include
target_include_directories(${PROJECT_NAME} PUBLIC vendor/imgui)

# vulkan
find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY})
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

# Link to vendor
target_link_libraries(${PROJECT_NAME} vendor)

