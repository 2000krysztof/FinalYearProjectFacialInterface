cmake_minimum_required(VERSION 3.21)
project(FacialInterface LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)

FetchContent_MakeAvailable(raylib)

file(GLOB SOURCES src/*.cpp src/*.h)
add_executable(FacialInterface ${SOURCES})

target_compile_features(FacialInterface PRIVATE cxx_std_17)

target_link_libraries(FacialInterface PRIVATE raylib)

if (WIN32 AND BUILD_SHARED_LIBS)
	add_custom_command(TARGET FacialInterface POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:> $<TARGET_FILE_DIR:FacialInterface> COMMAND_EXPAND_LISTS)
endif()

add_custom_target(copy_resources ALL COMMAND ${CMAKE_COMMAND}
	-E copy_directory
	"${PROJECT_SOURCE_DIR}/res"
	${PROJECT_BINARY_DIR}/res
)

install(TARGETS FacialInterface)
