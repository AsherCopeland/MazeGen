cmake_minimum_required(VERSION 3.27)

if(NOT DEFINED CACHE{CMAKE_EXPORT_COMPILE_COMMANDS})
  set(CMAKE_EXPORT_COMPILE_COMMANDS
      ON
      CACHE BOOL "Enable/Disable output of compile commands during generation.")
  mark_as_advanced(CMAKE_EXPORT_COMPILE_COMMANDS)
endif()

project(Maze)
set(TARGET_NAME maze)

add_executable(${TARGET_NAME} main.c maze.c)

set_target_properties(
  ${TARGET_NAME}
  PROPERTIES C_STANDARD 99
             C_STANDARD_REQUIRED ON
             C_EXTENSIONS OFF)

set(CAN_USE_SANITIZER ON)

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang|GNU"
	OR CMAKE_C_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
  unset(CAN_USE_SANITIZER)
endif()

set_property(TARGET ${TARGET_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

message(${CAN_USE_SANITIZER})

if(CAN_USE_SANITIZER)
  target_compile_options(
	${TARGET_NAME}
	PUBLIC -fsanitize=address
	       -fsanitize=undefined -g -Wall -Wextra -Wconversion -Wpedantic -Werror)

  target_link_options(
	${TARGET_NAME}
	PUBLIC -fsanitize=address
	       -fsanitize=undefined -g)
endif()
