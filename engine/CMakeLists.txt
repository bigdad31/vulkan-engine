# CMakeList.txt : CMake project for vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan)
find_package(SDL2 REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(libpng CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

include_directories(engine ${Boost_INCLUDE_DIRS})
include_directories(engine ${ASSIMP_INCLUDE_DIR})
include_directories(engine ${Vulkan_INCLUDE_DIRS})
include_directories(engine PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

set(KERNELS
	"shaders/default.frag"
	"shaders/default.vert"
)

set(MODELS
	"models/bruh.fbx"
	"models/ground.fbx"
)

set(OUTPUT_MODELS
	"${CMAKE_CURRENT_BINARY_DIR}/models/bruh.fbx"
	"${CMAKE_CURRENT_BINARY_DIR}/models/ground.fbx"
)

set(SCENES
	"scenes/scene.json"
)

set(OUTPUT_SCENES
	"${CMAKE_CURRENT_BINARY_DIR}/scenes/scene.json"
)

set(COMPILED_KERNELS
	"shaders/default.frag.spv"
	"shaders/default.vert.spv"
)

set(HEADERS
	"asynctransferhandler.h"
	"defaultuniform.h"
	"depthstencil.h"
	"gamestate.h"
	"model.h"
	"pipeline.h"
	"renderer.h"
	"shader.h"
	"swapchain.h"
	"vulkancontext.h"
)

set(IMPLEMENTATIONS
	"asynctransferhandler.cpp"
	"defaultuniform.cpp"  
	"depthstencil.cpp"
	"engine.cpp"
	"gamestate.cpp" 
	"model.cpp"
	"pipeline.cpp"
	"renderer.cpp"
	"shader.cpp"
	"swapchain.cpp"
	"vulkancontext.cpp"
)

add_executable (engine 
	${HEADERS}
	${IMPLEMENTATIONS}
	${KERNELS}
	${MODELS}
	${OUTPUT_MODELS}
	${SCENES}
	${OUTPUT_SCENES}
	${COMPILED_KERNELS}
)

foreach(KERNEL ${KERNELS})
	add_custom_command(OUTPUT ${KERNEL}.spv
	COMMAND C:/VulkanSDK/1.2.135.0/Bin/glslc ${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL} -g -o "${CMAKE_CURRENT_BINARY_DIR}/${KERNEL}.spv"
	DEPENDS ${KERNEL}
	COMMENT "Rebuilding ${KERNEL}.spv" )
	message(STATUS "Generating build commands for ${KERNEL}.spv")
endforeach()

foreach(MODEL ${MODELS})
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MODEL}
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL}" "${CMAKE_CURRENT_BINARY_DIR}/${MODEL}"
	DEPENDS ${MODEL})
endforeach()

foreach(SCENE ${SCENES})
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SCENE}
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${SCENE}" "${CMAKE_CURRENT_BINARY_DIR}/${SCENE}"
	DEPENDS ${SCENE})
endforeach()

target_link_libraries(engine PRIVATE assimp::assimp)
target_link_libraries(engine PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(engine PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(engine PRIVATE png)
target_link_libraries(engine PRIVATE LinearMath Bullet3Common BulletDynamics BulletCollision BulletSoftBody)