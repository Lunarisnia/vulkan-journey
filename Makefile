.PHONY: run
run:
	cmake . -DCMAKE_BUILD_TYPE=debug -B ./build
	cmake --build ./build/ --target Shaders
	cmake --build ./build/
	./build/debug/VulkanJourney

.PHONY: shader
shader:
	cmake --build ./build/ --target Shaders
