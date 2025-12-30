cmake_minimum_required(VERSION 3.16)
project(wallpaper_aerials_fix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(wallpaper-aerials-fix src/main.cpp)

find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
find_library(CORESERVICES_FRAMEWORK CoreServices)
find_library(APPLICATIONSERVICES_FRAMEWORK ApplicationServices)

target_link_libraries(
  wallpaper-aerials-fix
  PRIVATE
    ${COREFOUNDATION_FRAMEWORK}
    ${CORESERVICES_FRAMEWORK}
    ${APPLICATIONSERVICES_FRAMEWORK}
)
