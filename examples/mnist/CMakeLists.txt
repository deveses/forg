add_executable(forg_mnist mnist.cpp)
target_link_libraries(forg_mnist PRIVATE forg)
forg_configure_target(forg_mnist)

add_executable(forg_mnist_infer infer.cpp)
target_link_libraries(forg_mnist_infer PRIVATE forg)
forg_configure_target(forg_mnist_infer)
