cmake_minimum_required(VERSION 3.12)
project(tq VERSION 1.0.0 LANGUAGES C)

add_subdirectory(lib/utils)
add_subdirectory(lib/termutils)

set(SRC
	src/cli.c
	src/tq.c
	src/subcmd/add.c
	src/subcmd/done.c
	src/subcmd/init.c
	src/subcmd/list.c
)
set(HDR
	src/cli.h
	src/tq.h)
# set(HDR src/game.h src/memory.h src/set.h)
set(ALL_SRC ${SRC} ${HDR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable(${PROJECT_NAME} ${ALL_SRC})

target_compile_features(${PROJECT_NAME} PUBLIC c_std_11)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Werror)
target_link_libraries(${PROJECT_NAME} PRIVATE termutils::termutils utils::utils)
