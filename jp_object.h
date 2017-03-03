
#ifndef _JP_OBJECT_INCLUDE_
#define _JP_OBJECT_INCLUDE_
 
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
 
 
//数据类型
typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned int    U32;
typedef unsigned long   U64;
 
typedef const unsigned char     CU8;
typedef const unsigned short    CU16;
typedef const unsigned int  CU32;
typedef const unsigned long CU64;
 
 
typedef signed char     S8;
typedef signed short    S16;
typedef signed int      S32;
typedef signed long     S64;
 
typedef double  DOU;
typedef float   FLO;
typedef unsigned char   BOOL;
typedef void *  JP_OBJECT;
 
//数据类型
#define FALSE           (0)
#define TRUE            (!(FALSE))
 
#define false           (FALSE)
#define true            (TRUE)
 
#define SUCESS          (0)
 
#ifndef NULL
#define NULL ((JP_OBJECT)0)
#endif
 
#ifndef null
#define null (NULL)
#endif
 
 
static BOOL is_little(){
    union {
        U8 c[4];
        U32 l;
    }endian={0x01,0x00,0x00,0x00};
    return endian.l&0x01;
}
 
//常用宏定义
#define JP_IS_LITTLE() (is_little())
 
//大小写装换
#define JP_TOUPPER(x) ((x>'a'&&x<'z')?(x&~0x20):x)
#define JP_TOLOWER(x) ((x>'A'&&x<'Z')?(x|0x20):x)
 
#define JP_ISUPPER(x) ((x>'A'&&x<'Z')?TRUE:FALSE)
 
//循环移动
#define LOOP_LEFT(x,n,s)  (((x) << (n)) | ((x) >> (s-(n))))
#define LOOP_RIGHT(x,n,s) (((x) >> (n)) | ((x) << (s-(n))))
 
#define SHIFT_SIZE8 (8)
#define SHIFT_LOOP_LEFT8(x, n)  LOOP_LEFT(x,n,SHIFT_SIZE8)
#define SHIFT_LOOP_RIGHT8(x, n) LOOP_RIGHT(x,n,SHIFT_SIZE8)
 
#define SHIFT_SIZE16 (16)
#define SHIFT_LOOP_LEFT16(x, n)  LOOP_LEFT(x,n,SHIFT_SIZE16)
#define SHIFT_LOOP_RIGHT16(x, n) LOOP_RIGHT(x,n,SHIFT_SIZE16)
 
#define SHIFT_SIZE32 (32)
#define SHIFT_LOOP_LEFT32(x, n)  LOOP_LEFT(x,n,SHIFT_SIZE32)
#define SHIFT_LOOP_RIGHT32(x, n) LOOP_RIGHT(x,n,SHIFT_SIZE32)
 
#define VALUE24(data,i) (data[i++]<<16|data[i++]<<8|data[i++]<<0);
#define VALUE32(data,i) (data[i++]<<24|data[i++]<<16|data[i++]<<8|data[i++]<<0);
 
#define JP_NEW_OBJ(type,parsrc)  {JP_NEW_OBJ_NUM(type,parsrc,1)}
 
#define JP_NEW_OBJ_NUM(type,parsrc,num)  {parsrc=(type *)malloc((sizeof(type)*(num)));\
                                if(parsrc!=NULL)memset(parsrc,0,(sizeof(type)*(num)));}
 
#define JP_DELETE_OBJ(parsrc) {free(parsrc);parsrc=null;}
 
 
//结构体成员获取结构体地址
#define JP_OFFSET(type,member)  ((size_t)(&((type *)0)->member))
#define JP_OF_CONTAINT(ptr,type,member) ((type *)(ptr-JP_OFFSET(type,member)))
 
 
 
 
#endif

