set(PROJECT_NAME epassctl)

cmake_minimum_required(VERSION 3.10)
project(${PROJECT_NAME})

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()


# 在下面添加源代码文件，一行一个
add_executable(${PROJECT_NAME}
  src/main.c
  src/ipc_client.c
  src/ipc_common.c
  src/log.c
  src/uuid.c 
)

# 在下面添加头文件目录，一行一个
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME})