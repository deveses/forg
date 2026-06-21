add_executable(winapp WIN32
    Viewport.cpp
    stdafx.cpp
    winapp.cpp
    winapp.rc)

forg_configure_target(winapp)
target_precompile_headers(winapp PRIVATE stdafx.h)
target_compile_definitions(winapp PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(winapp PRIVATE forg emfc comctl32)
add_dependencies(winapp glrenderer swrenderer)

add_custom_command(TARGET winapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glrenderer> $<TARGET_FILE_DIR:winapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:swrenderer> $<TARGET_FILE_DIR:winapp>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/docs/config.example.xml $<TARGET_FILE_DIR:winapp>/config.xml)
