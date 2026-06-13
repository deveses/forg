add_executable(macapp)
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
add_dependencies(macapp swrenderer metalrenderer)
add_custom_command(TARGET macapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:swrenderer> $<TARGET_FILE_DIR:macapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:metalrenderer> $<TARGET_FILE_DIR:macapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/config.xml $<TARGET_FILE_DIR:macapp>
)