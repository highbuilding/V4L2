#include "jp_convert.h"
 
/*
 * 创建16位图头部
 */
static BMP * create_header16(U32 w,U32 h){
    BMP *chbmp=NULL;
    JP_NEW_OBJ(BMP,chbmp);
    memcpy(chbmp->identifier,"BM",sizeof(chbmp->identifier));
    chbmp->file_header.reserved=0;
    chbmp->file_header.bm_data_offset=0x36;
    chbmp->information_header.bits_per_pixel=0x10;
    chbmp->information_header.bm_header_size=0x28;
    chbmp->data=NULL;
    chbmp->palette=NULL;
    return chbmp;
}
 
/*
 * 转换16位图头部
 */
static BMP * c8to16h(BMP *frombmp){
    BMP *tobmp=NULL;
    JP_NEW_OBJ(BMP,tobmp);
    memcpy(tobmp->identifier,frombmp->identifier,sizeof(frombmp->identifier));
    memcpy(&tobmp->file_header,&frombmp->file_header,sizeof(frombmp->file_header));
    memcpy(&tobmp->information_header,&frombmp->information_header,sizeof(frombmp->information_header));
    tobmp->file_header.reserved=0;
    tobmp->file_header.bm_data_offset=0x36;
    S32 iLineByteCnt =((31+(tobmp->information_header.width<<3)) >> 5) << 2;
    U32 wh=tobmp->information_header.height*iLineByteCnt;
    printf("wh:%d\n",wh);
    tobmp->information_header.bitmap_data_size=wh<<1;
    tobmp->information_header.bits_per_pixel=0x10;
    tobmp->information_header.bm_header_size=0x28;
    tobmp->file_header.file_size=tobmp->information_header.bitmap_data_size+tobmp->file_header.bm_data_offset;
    tobmp->information_header.compression=BI_RGB;
    tobmp->data=NULL;
    tobmp->palette=NULL;
    return tobmp;
}
 
/*
 * 数据转换
 */
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
                U32 dd=(iw+j+i)<<1;
                U16 rgb16=(rq->rgbRed>>3)<<10|(rq->rgbGreen>>3)<<5|(rq->rgbBlue>>3)<<0;
                *(desbmp->data+dd)=rgb16&(0xff);
                *(desbmp->data+1+dd)=rgb16>>8;
            }
        }
    }
    return desbmp;
}
 
CONVERT * new_8to16_convert(){
    CONVERT *c8to16=NULL;
    JP_NEW_OBJ(CONVERT,c8to16);
    c8to16->createheader=create_header16;
    c8to16->convertheader=c8to16h;
    c8to16->convertdata=jp_convertdata;
    return c8to16;
}






