add_executable(macapp)
forg_configure_target(macapp)
target_sources(macapp
    PRIVATE
    #main.cpp
    main.mm
)
find_library(COCOA_LIBRARY Cocoa)
target_link_libraries(macapp
    PRIVATE
    forg
    #${COCOA_LIBRARY}
    $<LINK_LIBRARY:FRAMEWORK,Cocoa>
)
if(FORG_USE_FREETYPE)
    target_compile_definitions(macapp PRIVATE FORG_USE_FREETYPE)
endif()
add_dependencies(macapp swrenderer metalrenderer)
add_custom_command(TARGET macapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:swrenderer> $<TARGET_FILE_DIR:macapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:metalrenderer> $<TARGET_FILE_DIR:macapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/config.yml $<TARGET_FILE_DIR:macapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/scene.yml $<TARGET_FILE_DIR:macapp>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data $<TARGET_FILE_DIR:macapp>/data
)
