#ifndef IMAGES_BMP_INCLUDE_JP_BMP_H_
#define IMAGES_BMP_INCLUDE_JP_BMP_H_
#include "jp_convert.h"
 
typedef struct bmp_ops BMP_OPS;
 
struct bmp_ops{
    BMP *bmp_file;
    //获取文件属性
    U32 (*get_file_size)(BMP_OPS *_this);//获得文件大小
    U32 (*get_width)(BMP_OPS *_this);
    U32 (*get_height)(BMP_OPS* _this);
    U32 (*get_hresolution)(BMP_OPS *_this);
    U32 (*get_vresolution)(BMP_OPS *_this);
    U32 (*get_bits_per_pixel)(BMP_OPS *_this);
    U32 (*get_compression)(BMP_OPS *_this);//文件是否压缩
    U32 (*get_data_offset)(BMP_OPS *_this);
 
    //文件读取、打印、装换
    void (*read_file)(BMP_OPS *_this,const char* path);
    void (*write_file)(BMP_OPS *_this,const char* path);
    void (*ptint_bmp_inf)(BMP_OPS *_this);
    BMP_OPS* (*convert)(BMP_OPS *_this,CONVERT *convert);
    BMP_OPS* (*create)(BMP_OPS *_this,CREATE *convert,U32 w,U32 h);
};
 
BMP_OPS *new_bmp_ops();
void delete_bmp_ops(BMP_OPS *_this);
#endif /* IMAGES_BMP_INCLUDE_JP_BMP_H_ */

