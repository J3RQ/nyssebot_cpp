cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Nyssebot VERSION 0.0.1 DESCRIPTION "A simple Discord bot for getting Tampere public transportation information.")

add_subdirectory(libs/DPP)
add_subdirectory(libs/cpr)
add_subdirectory(libs/fmt)

add_executable(${PROJECT_NAME} src/main.cpp src/commands/stop.cpp src/commands/stop.h src/utils/utils.cpp src/utils/utils.h src/commands/map.cpp src/commands/map.h)

target_link_libraries(${PROJECT_NAME}
    dpp
    cpr
    fmt
)
target_include_directories(${PROJECT_NAME} PRIVATE
    libs/DPP/include
    libs/cpr/include/cpr
    libs/fmt/include
    src/utils
)

set_target_properties(${PROJECT_NAME} PROPERTIES
CXX_STANDARD 17
CXX_STANDARD_REQUIRED ON
)
