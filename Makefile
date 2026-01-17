.PHONY: run
run:
	cmake . -DCMAKE_BUILD_TYPE=debug -B ./build
	cmake --build ./build/ --target Shaders
	cmake --build ./build/
	./build/debug/VulkanJourney

.PHONY: build
build:
	cmake . -DCMAKE_BUILD_TYPE=debug -B ./build
	cmake --build ./build/ --target Shaders
	cmake --build ./build/

.PHONY: shader
shader:
	cmake --build ./build/ --target Shaders
