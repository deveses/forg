file(GLOB_RECURSE forg_library_files
    RELATIVE "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src/forg/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/forg/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/forg/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/forg/src/*.cc"
    "${PROJECT_SOURCE_DIR}/src/forg/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/forg/src/*.cxx"
    "${PROJECT_SOURCE_DIR}/src/forg/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/forg/src/*.hpp")

if(NOT forg_library_files)
    message(FATAL_ERROR "No forg library source files found for exception check")
endif()

set(violations)

foreach(file IN LISTS forg_library_files)
    file(STRINGS "${PROJECT_SOURCE_DIR}/${file}" lines)
    set(line_number 0)

    foreach(line IN LISTS lines)
        math(EXPR line_number "${line_number} + 1")
        string(STRIP "${line}" stripped_line)

        if(stripped_line MATCHES "^(//|/\\*|\\*)")
            continue()
        endif()

        if(line MATCHES "#[ \t]*include[ \t]*<(stdexcept|exception)>"
            OR line MATCHES "(^|[^A-Za-z0-9_])(throw|try)[ \t(;{]"
            OR line MATCHES "(^|[^A-Za-z0-9_])catch[ \t]*\\("
            OR line MATCHES "(^|[^A-Za-z0-9_])std::(exception|runtime_error|logic_error|invalid_argument|out_of_range)([^A-Za-z0-9_]|$)"
            OR line MATCHES "(^|[^A-Za-z0-9_])(runtime_error|logic_error|invalid_argument|out_of_range)([^A-Za-z0-9_]|$)")
            list(APPEND violations "${file}:${line_number}: ${stripped_line}")
        endif()
    endforeach()
endforeach()

if(violations)
    list(JOIN violations "\n" violation_text)
    message(FATAL_ERROR
        "C++ exceptions are not allowed in forg library sources:\n${violation_text}")
endif()
