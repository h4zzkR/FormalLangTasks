project(parser-library)
set(parser-source-files "${PROJECT_SOURCE_DIR}/parser.cpp" CACHE INTERNAL "")

add_library(
#    parser-library SHARED ${parser-library-source-files} ${Testing_SOURCE_DIR}/include/parser-library.h
    parser-library SHARED ${parser-source-files} ${ReParser_SOURCE_DIR}/include/parser.h
)


