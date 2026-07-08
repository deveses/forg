add_executable(forg_gpt gpt.cpp)
target_link_libraries(forg_gpt PRIVATE forg)
forg_configure_target(forg_gpt)
