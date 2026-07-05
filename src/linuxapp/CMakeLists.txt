add_executable(linuxapp
    main.cpp)

forg_configure_target(linuxapp)
target_link_libraries(linuxapp PRIVATE forg ${FORG_SDL2_LIBRARIES})
if(FORG_SDL2_INCLUDE_DIRS)
    target_include_directories(linuxapp PRIVATE ${FORG_SDL2_INCLUDE_DIRS})
endif()
if(FORG_USE_FREETYPE)
    target_compile_definitions(linuxapp PRIVATE FORG_USE_FREETYPE)
endif()
add_dependencies(linuxapp swrenderer)

add_custom_command(TARGET linuxapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:swrenderer> $<TARGET_FILE_DIR:linuxapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/config.yml $<TARGET_FILE_DIR:linuxapp>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data $<TARGET_FILE_DIR:linuxapp>/data)
