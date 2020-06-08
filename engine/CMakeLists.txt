# CMakeList.txt : CMake project for vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan)
find_package(SDL2 REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(libpng CONFIG REQUIRED)

include_directories(engine ${Boost_INCLUDE_DIRS})
include_directories(engine ${ASSIMP_INCLUDE_DIR})
include_directories(engine ${Vulkan_INCLUDE_DIRS})

set(KERNELS
	shaders/default.frag
	shaders/default.vert
)

set(COMPILED_KERNELS
	shaders/default.frag.spv
	shaders/default.vert.spv
)

add_executable (engine 
	"engine.cpp"
	"vulkancontext.h"
	"vulkancontext.cpp"
	"swapchain.h"
	"swapchain.cpp"
	"pipeline.h"
	"pipeline.cpp"
	"shader.h"
	"shader.cpp"
	${KERNELS}
	${COMPILED_KERNELS} "renderer.h" "renderer.cpp" "model.h" "model.cpp" "asynctransferhandler.h" "asynctransferhandler.cpp" "depthstencil.h" "depthstencil.cpp" "defaultuniform.h" "defaultuniform.cpp" "gamestate.h" "gamestate.cpp")

foreach(KERNEL ${KERNELS})
	add_custom_command(OUTPUT ${KERNEL}.spv
	COMMAND C:/VulkanSDK/1.2.135.0/Bin/glslc ${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL} -g -o "${CMAKE_CURRENT_BINARY_DIR}/${KERNEL}.spv"
	DEPENDS ${KERNEL}
	COMMENT "Rebuilding ${KERNEL}.spv" )
	message(STATUS "Generating build commands for ${KERNEL}.spv")
endforeach()

target_link_libraries(engine PRIVATE assimp::assimp)
target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(engine PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(engine PRIVATE png)