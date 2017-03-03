set(SRC_LIST new_8to24_convert.c new_8to32_convert.c new_8to16_convert.c jp_convert.c jp_bmp.c)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#
add_library(parsebmp_module STATIC ${SRC_LIST})

