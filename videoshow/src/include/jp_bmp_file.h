#ifndef IMAGES_BMP_INCLUDE_JP_BMP_FILE_H_
#define IMAGES_BMP_INCLUDE_JP_BMP_FILE_H_
#include "jp_object.h"
 
#define BPP1 (1)
#define BPP4 (4)
#define BPP8 (8)
#define FILE_HEADER_SZIE (14)
 
 enum{
    BI_RGB=0,
    BI_RLE8,
    BI_RLE4,
    BI_BITFIELDS,
    BI_JPEG,
    BI_PNG
};
 
//调色板信息
typedef struct{
    U8 rgbBlue;
    U8 rgbGreen;
    U8 rgbRed;
    U8 rgbReserved;
}RGBQUAD;
 
//文件头
typedef struct {
    U32 file_size;//文件大小
    U32 reserved;
    U32 bm_data_offset;//图片实际图像数据在文件中的偏移
}BM_FILE_HEADER;
 
//文件信息头部
typedef struct {
    U32 bm_header_size;
    U32 width;
    U32 height;
    U16 planes;
    U16 bits_per_pixel;
    U32 compression;
    U32 bitmap_data_size;
    U32 hresolution;
    U32 vresolution;
    U32 colors;
    U32 important_colors;
}BM_INFORMATION_HEADER;
 
//文件结构
typedef struct {
    U8 identifier[2];
    BM_FILE_HEADER file_header;
    BM_INFORMATION_HEADER information_header;
    RGBQUAD *palette;
    U8 *data;
}BMP;
 
 
 
#endif /* IMAGES_BMP_INCLUDE_JP_BMP_FILE_H_ */

