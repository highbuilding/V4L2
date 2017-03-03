#ifndef __LCD_OPS_H__
#define __LCD_OPS_H__
#include "jp_object.h"
typedef struct lcd_ops LCD_OPS;

struct lcd_ops{
    char devname[16];
	U32 lcd_width;
	U32	lcd_height;
	U8  lcd_bytesperpixel;
	U8   (*lcd_init)(LCD_OPS *__this,const char* name);
	void (*lcd_clearBk)(U32 color);
	int (*lcd_fflush)(void);
	int (*lcd_merge)(void* data_src,U32 xres,U32 yres,U32 width,U32 height);
    U8   (*lcd_exit)(LCD_OPS *__this);
};


 
LCD_OPS *new_lcd_ops();
void delete_lcd_ops(LCD_OPS *_this);
#endif /* __LCD_OPS_H__ */


