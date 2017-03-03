#include "jp_bmp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
static BMP_OPS *init_ops(BMP_OPS *bmp_ops);
 
/*
 *get属性
 */
static U32 jp_get_file_size(BMP_OPS *_this){
    return _this->bmp_file->file_header.file_size;
}
 
static U32 jp_get_width(BMP_OPS *_this){
    return _this->bmp_file->information_header.width;
}
 
static  U32 jp_get_height(BMP_OPS* _this){
    return _this->bmp_file->information_header.height;
}
static U32 jp_get_hresolution(BMP_OPS *_this){
    return _this->bmp_file->information_header.hresolution;
}
static  U32 jp_get_vresolution(BMP_OPS *_this){
    return _this->bmp_file->information_header.vresolution;
}
static  U32 jp_get_bits_per_pixel(BMP_OPS *_this){
    return _this->bmp_file->information_header.bits_per_pixel;
}
 
static  U32 jp_get_compression(BMP_OPS *_this){
    return _this->bmp_file->information_header.compression;
}
static U32 jp_get_data_offset(BMP_OPS *_this){
    return _this->bmp_file->file_header.bm_data_offset;
}
 
/*
 *文件信息打印
 */
static void ptint_bmp_inf(BMP_OPS *_this){
    BMP *bmp=_this->bmp_file;
    printf("identifier:%c%c\n",bmp->identifier[0],bmp->identifier[1]);
    printf("file_header.file_size:%d\n",_this->get_file_size(_this));
    printf("file_header.bm_data_offset:%d\n",_this->get_data_offset(_this));
    printf("file_header.reserved:%d\n",bmp->file_header.reserved);
    printf("information_header.bitmap_data_size:%d\n",bmp->information_header.bitmap_data_size);
    printf("information_header.bits_per_pixel:%d\n",bmp->information_header.bits_per_pixel);
    printf("information_header.bm_header_size:%d\n",bmp->information_header.bm_header_size);
    printf("information_header.height:%d\n",bmp->information_header.height);
    printf("information_header.width:%d\n",bmp->information_header.width);
    printf("information_header.important_colors:%d\n",bmp->information_header.important_colors);
    printf("information_header.planes:%d\n",bmp->information_header.planes);
    printf("information_header.vresolution:%d\n",bmp->information_header.vresolution);
    printf("information_header.hresolution:%d\n",bmp->information_header.hresolution);
}
 
/*
 *文件信息读取
 */
static void jp_read_file(BMP_OPS *_this,const char* path){
    BMP *bmp=NULL;
    S32 fd=open(path,O_RDONLY);
    if(fd<0){
        perror("Can not open File");
        return;
    }
 
    JP_NEW_OBJ(BMP,bmp);
    read(fd,bmp->identifier,sizeof(bmp->identifier));
    read(fd,&bmp->file_header,sizeof(bmp->file_header));
    read(fd,&bmp->information_header,sizeof(bmp->information_header));
    if(bmp->information_header.bits_per_pixel==BPP1||bmp->information_header.bits_per_pixel==BPP4
            ||bmp->information_header.bits_per_pixel==BPP8){
        U32 pal_size=(bmp->file_header.bm_data_offset-bmp->information_header.bm_header_size-FILE_HEADER_SZIE)>>2;
        JP_NEW_OBJ_NUM(RGBQUAD,bmp->palette,pal_size);
        read(fd,bmp->palette,sizeof(RGBQUAD)*pal_size);
    }
    lseek(fd,bmp->file_header.bm_data_offset,SEEK_SET);
    JP_NEW_OBJ_NUM(U8,bmp->data,bmp->information_header.bitmap_data_size);
    read(fd,bmp->data,sizeof(U8)*bmp->information_header.bitmap_data_size);
    close(fd);
    _this->bmp_file=bmp;
}
 
 
/*
 *文件信息写入
 */
static void jp_write_file(BMP_OPS *_this,const char* path){
    S32 fd=open(path,O_RDWR|O_CREAT,S_IRWXO|S_IRWXU|S_IRWXG);
    if(fd<0){
        printf("ERROR:%s\n","Can not open or create file");
        return;
    }
    BMP *bmp=_this->bmp_file;
    write(fd,bmp->identifier,sizeof(bmp->identifier));
    write(fd,&bmp->file_header,sizeof(bmp->file_header));
    write(fd,&bmp->information_header,sizeof(bmp->information_header));
    if(bmp->palette!=NULL){
        U32 pal_size=(bmp->file_header.bm_data_offset-bmp->information_header.bm_header_size-FILE_HEADER_SZIE)>>2;
        write(fd,bmp->palette,sizeof(RGBQUAD)*pal_size);
    }
    if(bmp->data!=NULL)
        write(fd,bmp->data,bmp->information_header.bitmap_data_size);
    fsync(fd);
    close(fd);
}
 
static BMP_OPS* convert(BMP_OPS *_this,CONVERT *convert){
    BMP_OPS *ops=NULL;
    JP_NEW_OBJ(BMP_OPS,ops);
    ops=init_ops(ops);
    BMP *desbmp=convert->convertheader(_this->bmp_file);
    desbmp=convert->convertdata(desbmp,_this->bmp_file);
    ops->bmp_file=desbmp;
    return ops;
}
 
static BMP_OPS* create_header(BMP_OPS *_this,CREATE *convert,U32 w,U32 h){
    BMP_OPS *ops=NULL;
    JP_NEW_OBJ(BMP_OPS,ops);
    ops=init_ops(ops);
    BMP *desbmp=convert->createheader(w,h);
    ops->bmp_file=desbmp;
    return ops;
}
 
//初始化
static BMP_OPS *init_ops(BMP_OPS *bmp_ops){
    bmp_ops->ptint_bmp_inf=ptint_bmp_inf;
    bmp_ops->read_file=jp_read_file;
    bmp_ops->get_file_size=jp_get_file_size;
    bmp_ops->get_width=jp_get_width;
    bmp_ops->get_height=jp_get_height;
    bmp_ops->get_hresolution=jp_get_hresolution;
    bmp_ops->get_vresolution=jp_get_vresolution;
    bmp_ops->get_bits_per_pixel=jp_get_bits_per_pixel;
    bmp_ops->get_compression=jp_get_compression;
    bmp_ops->get_data_offset=jp_get_data_offset;
    bmp_ops->convert=convert;
    bmp_ops->write_file=jp_write_file;
    bmp_ops->create=create_header;
    return bmp_ops;
}
 
 
void delete_bmp_ops(BMP_OPS *_this){
    JP_DELETE_OBJ(_this->bmp_file->data);
    if(_this->bmp_file->palette==NULL)
        JP_DELETE_OBJ(_this->bmp_file->palette);
    JP_DELETE_OBJ(_this->bmp_file);
    JP_DELETE_OBJ(_this);
}
 
BMP_OPS *new_bmp_ops(){
    BMP_OPS *bmp_ops=NULL;
    JP_NEW_OBJ(BMP_OPS,bmp_ops);
    bmp_ops=init_ops(bmp_ops);
    return bmp_ops;
}

