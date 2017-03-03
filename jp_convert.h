#ifndef IMAGES_BMP_INCLUDE_JP_CONVERT_H_
#define IMAGES_BMP_INCLUDE_JP_CONVERT_H_
#include "jp_bmp_file.h"
 
//文件转换类型
typedef enum{
    C08T32,
    C08T24,
    C08T16
}CONTYPE;
 
typedef enum{
    CR32,
    CR24,
    CR16,
    CR08
}CREATETYPE;
 
//文件转换
typedef struct{
    BMP * (*createheader)(U32 w,U32 h);
    BMP * (*convertheader)(BMP *srcbmp);
    BMP * (*convertdata)(BMP *desbmp,BMP *srcbmp);
}CONVERT;
 
typedef CONVERT CREATE;
 
CONVERT * new_8to16_convert();
CONVERT * new_8to32_convert();
CONVERT * new_8to24_convert();
 
CONVERT * new_convert(CONTYPE type);
CREATE * new_create(CREATETYPE type);
#endif /* IMAGES_BMP_INCLUDE_JP_CONVERT_H_ */

