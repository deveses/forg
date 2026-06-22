if(NOT CLANG_FORMAT_EXECUTABLE)
    message(FATAL_ERROR "CLANG_FORMAT_EXECUTABLE is required")
endif()

execute_process(
    COMMAND git ls-files src tests
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE tracked_files
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)

string(REPLACE "\n" ";" tracked_files "${tracked_files}")
list(FILTER tracked_files INCLUDE REGEX "\\.(c|cc|cpp|cxx|h|hh|hpp|hxx|m|mm)$")
list(FILTER tracked_files EXCLUDE REGEX "^src/(amprenderer|clrenderer)/")
list(FILTER tracked_files EXCLUDE REGEX "^tests/data/")

if(NOT tracked_files)
    message(FATAL_ERROR "No first-party source files found for formatting")
endif()

execute_process(
    COMMAND "${CLANG_FORMAT_EXECUTABLE}" --dry-run --Werror ${tracked_files}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMAND_ERROR_IS_FATAL ANY)
