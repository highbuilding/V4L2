#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "jp_bmp.h"
#include "lcd_ops.h"
#include <math.h>


int screen_size;
extern  unsigned char *fbmem;
unsigned int line_width;
unsigned int pixel_width;


LCD_OPS * lcd_ops;




void ShowBMP(int x,int y,int width,int height,int bits_per_pixel,U8* buf_data){
  int offset=0;
  unsigned int red, green, blue;
  U16 color_u16;
  int i=0;
  U8 *pen_8 = fbmem;
  U16 *pen_16; 
  U32 *pen_32;  
  pen_16=(U16*)fbmem;
  pen_32=(U32*)fbmem;
  pen_8+=(y*lcd_ops->lcd_width+x);
  pen_16+=(y*lcd_ops->lcd_width+x);
  pen_32+=(y*lcd_ops->lcd_width+x);
  if(buf_data==null){
    return;
  }
   int iLineByteCnt = (((width * bits_per_pixel) + 31) >> 5) << 2;//4字节对齐，如果bmp格式下每行的实际有效数据不是4字节对齐，那么在保存时格式化为4字节对齐
    //这样，位图数据区的大小为：
   int skip = 4 - ((width * bits_per_pixel)>>3) & 3;
   U8* buf_tmp=malloc((iLineByteCnt-skip)*height);
   if(buf_tmp==null){
   	  printf("malloc buf_temp error\n");
      return ;
   }
   U8 (*p)[iLineByteCnt]=(U8 (*)[iLineByteCnt])buf_data;
 
   for(i=height-1;i>=0;i--){
     memmove(buf_tmp+offset,p[i],iLineByteCnt-skip);
	 offset+=(iLineByteCnt-skip);
  }

  if(bits_per_pixel==(lcd_ops->lcd_bytesperpixel*8)){
      memmove(fbmem,buf_tmp,(line_width-skip)*height);
      free(buf_tmp);
      buf_tmp=null;
  }

  else if(bits_per_pixel<(lcd_ops->lcd_bytesperpixel*8)){
     printf("temp no support\n");
	 return;
  }
  else {
	 
    if(bits_per_pixel==24){
       if(lcd_ops->lcd_bytesperpixel==2){
	   	   int pixel_pos=0;
		   int new_iLineByteCnt=lcd_ops->lcd_bytesperpixel*width;
		   printf("new_iLineByteCnt:%d\n",new_iLineByteCnt);
		   U16* buf_bpp16=malloc(new_iLineByteCnt*height);
		   if(buf_bpp16==null){
              printf("buf_bpp16 malloc error\n");
			  return;
		   	}
		   while(pixel_pos<width*height){
			    blue    = buf_tmp[3*pixel_pos];
				green = buf_tmp[3*pixel_pos+1];
				red  = buf_tmp[3*pixel_pos+2];
				color_u16 = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
				//printf("%x\n",color_u16);
				buf_bpp16[pixel_pos++] = color_u16;
					   
		  }
		  
		  for(i=0;i<height;i++){
              memmove((pen_16+lcd_ops->lcd_width*i),&(buf_bpp16[i*width]),new_iLineByteCnt);
		   }
           free(buf_bpp16);
           buf_bpp16=null;
		   free(buf_tmp);
           buf_tmp=null;

       	}

	}
    else{
      printf("do not support 8bpp\n");
	}
  }
}


int hexstring2int(char* hex_string){
	int temp = 0;
	int i=0;
	if(*hex_string != '\0'){
		while (*hex_string == ' '){
			hex_string++;
		}
	 }
		int len = strlen(hex_string);
		if (*hex_string != '\0'){
			if (*hex_string++ == '0' && (*hex_string == 'x' || *hex_string == 'X')){
				hex_string++;
				for ( i = 0; i < len - 2; i++){
					if (*hex_string >= '0'&&*hex_string <= '9'){
						temp += (*hex_string - '0') *(pow(16., (len - 2 - i)));
						hex_string++;
					}
					else if (*hex_string >= 'a'&&*hex_string <= 'f'){
						temp += (*hex_string - 'a' + 10) *(pow(16., (len - 2 - i)));
						hex_string++;
					}
					else if (*hex_string >= 'A'&&*hex_string <= 'F'){
						temp += (*hex_string - 'A' + 10) *(pow(16., (len - 2 - i)));
						hex_string++;
					}
					else return -1;
				}
			}

		}
	return temp;
}

U16 test_buffer[50][50]={0};
U16 color_array[5]={0xfe41,0x78ef,0x2589,0x001f,0x3a39};

int main(int argc, char **argv)
{
	int i,j,k;
	lcd_ops=new_lcd_ops();
    if(argc!=2){
       fprintf(stderr,"usage error:  example ./parse_bmp filename\n");
	   return -1;
    }
    if(!lcd_ops){
       fprintf(stderr,"/dev/fb0 open fail\n");
    }
	
	if(!lcd_ops->lcd_init(lcd_ops,"/dev/fb0")){
       fprintf(stderr,"fb init error\n");   
    }
    
	/* 清屏: 全部设为黑色 */
	
    lcd_ops->lcd_clearBk(0x000000);


	
	//for(i=0;i<5;i++){
   //  for(j=i*10;j<i*10+10;j++){
	//   for(k=0;k<50;k++){
   //       test_buffer[j][k]=color_array[i];
	//   }
    // }
	//}
	//lcd_ops->lcd_fflush(test_buffer,xres,yres,50,50);
	
    printf("file path %s\n",argv[1]);
	
	BMP_OPS *bmp=new_bmp_ops();
    bmp->read_file(bmp,argv[1]);
    bmp->ptint_bmp_inf(bmp);
    
	ShowBMP(100,0,bmp->get_width(bmp),bmp->get_height(bmp),bmp->get_bits_per_pixel(bmp),bmp->bmp_file->data);
    //delete_bmp_ops(bmp);
    //delete_bmp_ops(to_ops);

	
     lcd_ops->lcd_exit(lcd_ops);
     free(lcd_ops);
	 lcd_ops=NULL;


	return 0;	
}

