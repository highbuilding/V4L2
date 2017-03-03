set(SRC_LIST lcd_ops.c color.c)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#
add_library(lcd_module STATIC ${SRC_LIST})

