add_executable(forg_mnist mnist.cpp)
target_link_libraries(forg_mnist PRIVATE forg)
forg_configure_target(forg_mnist)
