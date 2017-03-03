#include "lcd_ops.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>


static int fd_fb;
static struct fb_var_screeninfo var;	/* Current var */
static struct fb_fix_screeninfo fix;	/* Current fix */
static int screen_size;
 unsigned char *fbmem;
static unsigned char *fbmem_buf;



static unsigned int line_width;
static unsigned int pixel_width;

static LCD_OPS *init_ops(LCD_OPS *lcd_ops);
 
static U8 lcd_init(LCD_OPS *__this,const char*name){
	int i=0;
	int screen_size=0;
	fbmem_buf=null;
	if(name!=null){
	    fd_fb = open(name, O_RDWR);
		if (fd_fb < 0){
			return 0;
		}
		if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var)){
			printf("can't get var\n");
			close(fd_fb);
			return 0;
		}

		__this->lcd_bytesperpixel=var.bits_per_pixel>>3;
        __this->lcd_width        =var.xres;
		__this->lcd_height       =var.yres;
 		
		if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &fix)){
			printf("can't get fix\n");
			close(fd_fb);
			return 0;
		}
		screen_size=var.xres*var.yres*(var.bits_per_pixel>>3);
		fbmem = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
		if (fbmem == (unsigned char *)-1){
			printf("can't mmap\n");
			return 0;
		}
		fbmem_buf=(unsigned char*)malloc(screen_size);
		if(!fbmem_buf){
            printf("malloc buffer error\n");
			return 0;
		}
		for(i=0;i<16&&name[i]!='\0';i++){
		  __this->devname[i]=name[i];
		}
		__this->devname[i]='\0';
   }
	return 1;

}

static U8 lcd_exit(LCD_OPS *__this){
	int screen_size=var.xres*var.yres*(var.bits_per_pixel>>3);
	if(munmap(fbmem,screen_size)!=0){//定义函数 int munmap(void *start,size_t length);
       return 0;//失败
	}
	if(close(fd_fb)!=0){
       return 0;//失败
	}
	//free(__this);
	//__this=NULL;
}


static void lcd_clearBk(U32 color)
{
	unsigned char *pen_8 = fbmem;
	unsigned short *pen_16;	
	unsigned int *pen_32;	
	int i=0;
	unsigned int red, green, blue;
	unsigned short color_u16;
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;
	switch (var.bits_per_pixel)
	{
		case 8:
		{
		   while(i++<var.xres*var.yres){
			*pen_8 = color;
			pen_8++;
			
		   }
		   break;
		}
		case 16:
		{
			/* 565 */
		   while(i++<var.xres*var.yres){
			 red   = (color >> 16) & 0xff;
			 green = (color >> 8) & 0xff;
			 blue  = (color >> 0) & 0xff;
			 color_u16 = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			 *pen_16 = color_u16;
			 pen_16++;
			
		   }
			
			break;
		}
		case 32:
		{ 
		  while(i++<var.xres*var.yres){
			*pen_32 = color;
			pen_32++;
			
		  }
		  break;
		}
		default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
	}
	
	
}


/*


*/
static int lcd_fflush(){
  int screen_size=var.xres*var.yres*(var.bits_per_pixel>>3); 
  memcpy(fbmem,fbmem_buf,screen_size);
}


/*
该函数的数据源根据根据framebuffer的位数自动调整
如果是8位bpp深度，data_src是一个字节
如果是16位bpp，   data_src是两个字节

*/
static int lcd_merge(void* data_src,U32 xoffset,U32 yoffset,U32 width,U32 height){
	U8 *pen_8 = fbmem_buf;
	U16 *pen_16=NULL; 
	U32 *pen_32=NULL;	
	U8 (*fp8)[width]=NULL;//指向数组的地址
	U16 (*fp16)[width]=NULL;
	U32 (*fp32)[width]=NULL;
	U32 i=0;
	U32 red, green, blue;
	U16 color_u16;
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;
	if(!data_src){
       return 0;
    }
	if((xoffset+width>=var.xres)||(yoffset+height>=var.yres)){
       return 0;
	}
	fp8=(U8(*)[width])data_src;
	fp16=(U16(*)[width])data_src;
	fp32=(U32(*)[width])data_src;
	switch (var.bits_per_pixel)
		{
			case 8:
			{
			   pen_8+=(xoffset+yoffset*var.xres);
			   for(i=0;i<height;i++){
                  memcpy(pen_8+i*var.xres,fp8[i],width*sizeof(U8));
			   }
			   break;
			}
			case 16:
			{
				/* 565 */
			    pen_16+=(xoffset+yoffset*var.xres);
				for(i=0;i<height;i++){
                  memcpy(pen_16+i*var.xres,fp16[i],width*sizeof(U16));
			    }
                  
				break;
			}

			case 24:
			case 32:
			{ 
			    pen_32+=(xoffset+yoffset*var.xres);
				for(i=0;i<height;i++){
                  memcpy(pen_32+i*var.xres,fp32[i],width*sizeof(U32));
			    }
			    break;
			}
			default:
			{
				printf("can't surport %dbpp\n", var.bits_per_pixel);
				break;
			}
		}

}


 
//初始化
static LCD_OPS *init_ops(LCD_OPS *lcd_ops){
    lcd_ops->lcd_init=lcd_init;
	lcd_ops->lcd_clearBk=lcd_clearBk;
	lcd_ops->lcd_exit=lcd_exit;
	lcd_ops->lcd_fflush=lcd_fflush;
	lcd_ops->lcd_merge=lcd_merge;
    return lcd_ops;
}
 

 
LCD_OPS *new_lcd_ops(){
    LCD_OPS *lcd_ops=NULL;
    JP_NEW_OBJ(LCD_OPS,lcd_ops);
    lcd_ops=init_ops(lcd_ops);
    return lcd_ops;
}


