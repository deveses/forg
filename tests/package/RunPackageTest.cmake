set(prefix "${FORG_BINARY_DIR}/package-test/prefix")
set(consumer_build "${FORG_BINARY_DIR}/package-test/consumer")

file(REMOVE_RECURSE "${prefix}" "${consumer_build}")

set(config_args)
if(FORG_TEST_CONFIG)
    list(APPEND config_args --config "${FORG_TEST_CONFIG}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${FORG_BINARY_DIR}" --prefix "${prefix}" ${config_args}
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${FORG_SOURCE_DIR}" -B "${consumer_build}"
            "-DCMAKE_PREFIX_PATH=${prefix}"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}" ${config_args}
    COMMAND_ERROR_IS_FATAL ANY)
