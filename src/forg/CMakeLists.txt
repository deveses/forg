
add_library(forg STATIC)

include(Sources.cmake)

###############################################################################

target_sources(forg 
    PUBLIC ${all_includes}
    PRIVATE ${all_sources}
)

target_precompile_headers(forg PRIVATE src/forg_pch.h)
target_include_directories(forg 
    PUBLIC include
    PRIVATE src
)
target_compile_features(forg PUBLIC cxx_std_17)

target_compile_options(forg INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)
