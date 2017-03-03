#include "jp_convert.h"
 
/*
 * 创建32位图头部
 */
static BMP * create_header32(U32 w,U32 h){
    BMP *chbmp=NULL;
    JP_NEW_OBJ(BMP,chbmp);
    memcpy(chbmp->identifier,"BM",sizeof(chbmp->identifier));
    chbmp->file_header.reserved=0;
    chbmp->file_header.bm_data_offset=0x36;
    chbmp->information_header.bits_per_pixel=0x20;
    chbmp->information_header.bm_header_size=0x28;
    chbmp->information_header.height=h;
    chbmp->information_header.width=w;
    chbmp->information_header.bitmap_data_size=w*h*4;
    chbmp->file_header.file_size=chbmp->information_header.bitmap_data_size+chbmp->file_header.bm_data_offset;
    chbmp->information_header.planes=1;
    chbmp->information_header.hresolution=w*4;
    chbmp->information_header.vresolution=w*4;
    chbmp->data=NULL;
    chbmp->palette=NULL;
    return chbmp;
}
 
/*
 * 8位图转32位图
 */
static BMP * c8to32h(BMP *frombmp){
    BMP *tobmp=NULL;
    JP_NEW_OBJ(BMP,tobmp);
    memcpy(tobmp->identifier,frombmp->identifier,sizeof(frombmp->identifier));
    memcpy(&tobmp->file_header,&frombmp->file_header,sizeof(frombmp->file_header));
    memcpy(&tobmp->information_header,&frombmp->information_header,sizeof(frombmp->information_header));
    tobmp->file_header.reserved=0;
    tobmp->file_header.bm_data_offset=0x36;
    S32 iLineByteCnt =((31+(tobmp->information_header.width<<3)) >> 5) << 2;
    tobmp->information_header.bitmap_data_size=2+(tobmp->information_header.height*tobmp->information_header.width<<2);
    tobmp->information_header.bits_per_pixel=0x20;
    tobmp->information_header.bm_header_size=0x28;
    tobmp->file_header.file_size=tobmp->information_header.bitmap_data_size+tobmp->file_header.bm_data_offset;
 
    tobmp->data=NULL;
    tobmp->palette=NULL;
    return tobmp;
}
 
static BMP * jp_convertdata(BMP *desbmp,BMP *srcbmp){
    U32 i,j;
    U32 w=srcbmp->information_header.width;
    U32 h=srcbmp->information_header.height;
    S32 iLineByteCnt =((31+(w<<3)) >> 5) << 2;
    JP_NEW_OBJ_NUM(U8,desbmp->data,desbmp->information_header.bitmap_data_size);
 
    U32 tail=iLineByteCnt-w;//每行尾部的填充数据个数。
    S32 dt=-tail;
    S32 iw=-w;
    for(i=0;i<h;i++){
        iw+=w;
        dt+=tail;
        for(j=0;j<iLineByteCnt;j++){
            if(j<w){
                U32 data=*(srcbmp->data+iw+j+dt);//跳过每行尾部的填充数据。
                RGBQUAD *rq=srcbmp->palette+data;
                memcpy(desbmp->data+((iw+j)<<2),rq,sizeof(RGBQUAD));
            }
        }
    }
    return desbmp;
}
 
CONVERT * new_8to32_convert(){
    CONVERT *c8to32=NULL;
    JP_NEW_OBJ(CONVERT,c8to32);
    c8to32->convertheader=c8to32h;
    c8to32->createheader=create_header32;
    c8to32->convertdata=jp_convertdata;
    return c8to32;
}

