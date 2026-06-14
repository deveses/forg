# Vendored, header-only third-party dependencies.

# cgltf - single-header glTF 2.0 parser (MIT). CGLTF_IMPLEMENTATION is defined
# in exactly one translation unit (src/forg/include/forg/mesh/GLTFLoader.cpp).
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cgltf)
