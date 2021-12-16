project(parser-library)
set(parser-source-files "${PROJECT_SOURCE_DIR}/parser.cpp" CACHE INTERNAL "")

add_library(
    parser-library SHARED ${parser-source-files} ${YAParser_SOURCE_DIR}/include/LRParser.h
)


