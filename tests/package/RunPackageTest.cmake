set(prefix "${FORG_BINARY_DIR}/package-test/prefix")
set(consumer_build "${FORG_BINARY_DIR}/package-test/consumer")

file(REMOVE_RECURSE "${prefix}" "${consumer_build}")

set(config_args)
set(ctest_config_args)
if(FORG_TEST_CONFIG)
    list(APPEND config_args --config "${FORG_TEST_CONFIG}")
    list(APPEND ctest_config_args -C "${FORG_TEST_CONFIG}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${FORG_BINARY_DIR}" --prefix "${prefix}" ${config_args}
    COMMAND_ERROR_IS_FATAL ANY)

set(required_installed_files
    "include/forg/base.h"
    "include/forg/rendering/Color.h")
foreach(required_file IN LISTS required_installed_files)
    if(NOT EXISTS "${prefix}/${required_file}")
        message(FATAL_ERROR "Expected installed file is missing: ${required_file}")
    endif()
endforeach()

file(GLOB_RECURSE installed_package_configs
    "${prefix}/*/cmake/Forg/ForgConfig.cmake")
file(GLOB_RECURSE installed_package_versions
    "${prefix}/*/cmake/Forg/ForgConfigVersion.cmake")
file(GLOB_RECURSE installed_target_exports
    "${prefix}/*/cmake/Forg/ForgTargets.cmake")
file(GLOB_RECURSE installed_library_artifacts
    "${prefix}/*/libforg.a"
    "${prefix}/*/forg.lib")

foreach(installed_group
        installed_package_configs
        installed_package_versions
        installed_target_exports
        installed_library_artifacts)
    list(LENGTH ${installed_group} installed_group_count)
    if(NOT installed_group_count EQUAL 1)
        message(FATAL_ERROR
            "Expected exactly one installed file in ${installed_group}, found ${installed_group_count}")
    endif()
endforeach()

file(GLOB_RECURSE installed_files
    LIST_DIRECTORIES false
    RELATIVE "${prefix}"
    "${prefix}/*")
foreach(installed_file IN LISTS installed_files)
    if(installed_file MATCHES "^include/forg/src/")
        message(FATAL_ERROR "Private source path was installed: ${installed_file}")
    endif()
    if(installed_file MATCHES "^include/forg/.+\\.(c|cc|cxx|cpp|m|mm)$")
        message(FATAL_ERROR "Implementation source was installed: ${installed_file}")
    endif()
endforeach()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${FORG_SOURCE_DIR}" -B "${consumer_build}"
            "-DCMAKE_PREFIX_PATH=${prefix}"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}" ${config_args}
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${consumer_build}" ${ctest_config_args}
            --output-on-failure
    COMMAND_ERROR_IS_FATAL ANY)
