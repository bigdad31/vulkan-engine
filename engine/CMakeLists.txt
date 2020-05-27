# CMakeList.txt : CMake project for vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

add_compile_options("/std:c++latest")
add_compile_options("/Zc:__cplusplus")

find_package(Vulkan)
find_package(SDL2 REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(libpng CONFIG REQUIRED)

include_directories(engine ${Boost_INCLUDE_DIRS})
include_directories(engine ${ASSIMP_INCLUDE_DIR})
include_directories(engine ${Vulkan_INCLUDE_DIRS})

set(KERNELS
	${CMAKE_CURRENT_SOURCE_DIR}/frag.frag
	${CMAKE_CURRENT_SOURCE_DIR}/vert.vert
)

set(COMPILED_KERNELS
	${CMAKE_CURRENT_SOURCE_DIR}/frag.frag.spv
	${CMAKE_CURRENT_SOURCE_DIR}/vert.vert.spv
)

add_executable (engine 
	"engine.cpp"
	"engine.h"
	"vulkancontext.h"
	"vulkancontext.cpp"
	"swapchain.h"
	"swapchain.cpp"
	"pipeline.h"
	"pipeline.cpp"
	"shader.h"
	"shader.cpp"
	${KERNELS}
	${COMPILED_KERNELS} "DrawCommand.h" "DrawCommand.cpp" "renderer.h" "renderer.cpp")

foreach(KERNEL ${KERNELS})
	add_custom_command(OUTPUT ${KERNEL}.spv
	COMMAND C:/VulkanSDK/1.2.135.0/Bin/glslc ${KERNEL} -g -o ${KERNEL}.spv
	DEPENDS ${KERNEL}
	COMMENT "Rebuilding ${KERNEL}.spv" )
	FILE(COPY ${KERNEL}.spv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	message(STATUS "Generating build commands for ${KERNEL}.spv")
endforeach()

target_link_libraries(engine PRIVATE assimp::assimp)
target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(engine PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(engine PRIVATE png)