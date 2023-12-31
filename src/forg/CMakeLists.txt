set(SOURCES 
    include/EApplication.h
    src/EApplication.cpp
)

add_library(forg STATIC ${SOURCES})

target_include_directories(forg PUBLIC include)
target_compile_features(forg PUBLIC cxx_std_17)