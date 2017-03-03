/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#include "v4l2_opr.h"
#include "jp_bmp.h"
#include "lcd_ops.h"
#include "videodata_list.h"
#include "color.h" 
static LCD_OPS * lcd_ops=NULL;
static int flag=1;

/* translate YUV422Packed to rgb565 */

static unsigned int
Pyuv422torgb565(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned char *output_pt = output_ptr;

    unsigned int r, g, b;
    unsigned int color;
    
	size = image_width * image_height /2;
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;
		r = R_FROMYV(Y,V);
		g = G_FROMYUV(Y,U,V); //b
		b = B_FROMYU(Y,U); //v

        /* 把r,g,b三色构造为rgb565的16位值 */
        r = r >> 3;
        g = g >> 2;
        b = b >> 3;
        color = (r << 11) | (g << 5) | b;
        *output_pt++ = color & 0xff;
        *output_pt++ = (color >> 8) & 0xff;
			
		r = R_FROMYV(Y1,V);
		g = G_FROMYUV(Y1,U,V); //b
		b = B_FROMYU(Y1,U); //v
		
        /* 把r,g,b三色构造为rgb565的16位值 */
        r = r >> 3;
        g = g >> 2;
        b = b >> 3;
        color = (r << 11) | (g << 5) | b;
        *output_pt++ = color & 0xff;
        *output_pt++ = (color >> 8) & 0xff;
	}
	
	return 0;
} 


/* translate YUV422Packed to rgb24 */

static unsigned int
Pyuv422torgb32(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned int *output_pt = (unsigned int *)output_ptr;

    unsigned int r, g, b;
    unsigned int color;

	size = image_width * image_height /2;
	for (i = size; i > 0; i--) {
		/* bgr instead rgb ?? */
		Y = buff[0] ;
		U = buff[1] ;
		Y1 = buff[2];
		V = buff[3];
		buff += 4;

        r = R_FROMYV(Y,V);
		g = G_FROMYUV(Y,U,V); //b
		b = B_FROMYU(Y,U); //v
		/* rgb888 */
		color = (r << 16) | (g << 8) | b;
        *output_pt++ = color;
			
		r = R_FROMYV(Y1,V);
		g = G_FROMYUV(Y1,U,V); //b
		b = B_FROMYU(Y1,U); //v
		color = (r << 16) | (g << 8) | b;
        *output_pt++ = color;
	}
	
	return 0;
} 


videodata_list<video_buffer> list;
void process_image(video_buffer * buffer){
   video_buffer* tmp=(video_buffer*)malloc(sizeof(video_buffer));
   if(tmp){
   	 // printf("%d\n",++count);
      memcpy((void*)tmp,(void*)buffer,sizeof(video_buffer));
	  list.add(tmp);
   }
   
}

void* ShowVideoData(void* arg){
	int count;
   for(;;){
   	     if(flag){
		   video_buffer * buffer=list.get();
           if(buffer){
             uint8_t*  rgb565=(uint8_t*)malloc(buffer->height*buffer->width*2);
             Pyuv422torgb565((uint8_t*)buffer->start,rgb565,buffer->width,buffer->height);
             lcd_ops->lcd_merge(rgb565,0,0,buffer->width,buffer->height);
             lcd_ops->lcd_fflush();
             free(rgb565);
             rgb565=null;
			 free(buffer);
			 buffer=null;
           }
   	    }
		else{
		  	break;
		}
   }
}
void* CollectingandProcess_VideoData(void* arg){
	v4l2_opr* p_v4l2=(v4l2_opr*)arg;
	unsigned int count;
    count = 10000;
    while (count-- > 0) {
       for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;
            FD_ZERO (&fds);
            FD_SET (p_v4l2->get_video_file(), &fds);
           
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            r = select (p_v4l2->get_video_file() + 1, &fds, NULL, NULL, &tv);
            if(-1 == r) {
                if (EINTR == errno)
                   continue;
                errno_exit ("select");
            }
            if(0 == r) {
               fprintf (stderr, "select timeout\n");
               exit (EXIT_FAILURE);
            }
		    if(p_v4l2->read_frame(process_image)){
                break;
			}
			
         }
     }
	flag=0;
}


int main(int argc,char **argv)
{
	struct timeval tv;
	pthread_t video_thread[2];//两个线程
    int ret=0;
	initLut();
     v4l2_opr video0("/dev/video0",IO_METHOD_MMAP);
    //v4l2_opr video0;
    lcd_ops=new_lcd_ops();
	if(!lcd_ops){
       fprintf(stderr,"/dev/fb0 open fail\n");
	   return 0;
    }
	
	if(!lcd_ops->lcd_init(lcd_ops,"/dev/fb0")){
       fprintf(stderr,"fb init error\n");   
    }
    if(!video0.open_device()){
       fprintf(stderr,"video device open fail\n");
	   return 0;
    }
	if(!video0.init_device(500,400)){
       fprintf(stderr,"video device init fail\n");
	   return 0;
	}
	//video0.query_device();
	if(!video0.start_capturing()){
	   fprintf(stderr,"video device start_capturing fail\n");
	   return 0;
	}
	

    if(pthread_create(&video_thread[0],NULL,CollectingandProcess_VideoData, (void *)&video0))
        fprintf(stderr,"create thread1 error\n");
	if(pthread_create(&video_thread[1],NULL,ShowVideoData,(void*) NULL))
        fprintf(stderr,"create thread2 error\n");
    //4个参数：
    //第一个参数：指向线程标示符pthread_t的指针；
    //第二个参数：设置线程的属性
    //第三个参数：线程运行函数的起始地址
    //第四个参数：运行函数的参数
	 /* Make sure the thread has finished. */
     pthread_join (video_thread[0], NULL);
	 pthread_join (video_thread[1], NULL);
	 freeLut();
    //mainloop(&video0);
	video0.stop_capturing();
	video0.uninit_device();
	video0.close_device();
	/* 清屏: 全部设为黑色 */
    lcd_ops->lcd_clearBk(0x000000);
    return 0;
}









