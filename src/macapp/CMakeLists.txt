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