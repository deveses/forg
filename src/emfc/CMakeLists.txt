set(SOURCES 
    include/EApplication.h
    src/EApplication.cpp
)

add_library(emfc STATIC ${SOURCES})

target_include_directories(emfc PUBLIC include)
target_compile_features(emfc PUBLIC cxx_std_17)