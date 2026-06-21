cmake_minimum_required(VERSION 3.21)
project(ForgPackageConsumer LANGUAGES CXX)

find_package(Forg 1 CONFIG REQUIRED)

add_executable(forg_package_consumer main.cpp)
target_link_libraries(forg_package_consumer PRIVATE Forg::forg)
