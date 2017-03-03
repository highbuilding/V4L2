#ifndef __V4L2_OPR__H
#define __V4L2_OPR__H
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
#include <math.h>
#include "jp_object.h"


#define CLEAR(x) memset (&(x), 0, sizeof (x))
//3种I/O方式
//(1.1)   read/write: 直接使用 read 和 write 函数进行读写。这种方式最简单，但是这种方式会在 用户空间和内核空间不断拷贝数据 ，同时在用户空间和内核空间占用 了 大量内存， 效率不高。
//(1.2)   mmap :      把硬件设备 里的内存映射 到位于用户空间的应用程序中的内存地址上， 直接处理设备内存，这是一种有效的 方式。
//(1.3)   userptr :   内存由用户空间的应用程序分配，并把地址传递到内核中的驱动程序， 然后由 v4l2 驱动程序直接将数据填充到用户空间的内存中。
typedef unsigned char uint8_t;

typedef enum {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;
typedef struct buffer {
    void *  start;  //起始地址
    size_t  length; //数据长度
	size_t  width;  //宽度
	size_t  height; //高度
} video_buffer;

typedef void(*procss_callback)(video_buffer* buffer);//传入的回调函数

static void errno_exit (const char * s){
   fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
   exit (EXIT_FAILURE);
}


static int xioctl(int fd,int request,void * arg){
   int r;
   do r = ioctl (fd, request, arg);
   while (-1 == r && EINTR == errno);
   return r;
}

class v4l2_opr{
   private:
   	 U32  frame_cnt; //读取到的帧数
	 char 		 dev_name[16];
	 int		 fd 			 ;
	 video_buffer * 	 buffers;
	 unsigned int  n_buffers;
   	 io_method	 io 	;  
   public:
   	
   	 v4l2_opr(void){
	   	 int size;
		 frame_cnt=0;
		 io=IO_METHOD_MMAP;
		 fd=-1;
		 for(size=0;size<=16;size++){
            dev_name[size]='\0';
		 }
         strcpy(dev_name,"/dev/video");
   	 }
   	 v4l2_opr(const char*name,io_method method_io){
	   	 int size;
		 io=method_io;
		 frame_cnt=0;
		 
		 fd=-1;
         for(size=0;size<=16;size++){
            dev_name[size]='\0';
		 }
		 strcpy(dev_name,"/dev/video");  
   	 }
	   int get_video_file(){
           return fd;
	   	}
	   int open_device(void){
           struct stat st; 
           if (-1 == stat (dev_name, &st)) {
              fprintf (stderr, "Cannot identify '%s': %d, %s\n",dev_name, errno, strerror (errno));
              return 0;
           }
           if (!S_ISCHR (st.st_mode)) {
              fprintf (stderr, "%s is no device\n", dev_name);
              return 0;
           }

           fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
           if (-1 == fd) {
            fprintf (stderr, "Cannot open '%s': %d, %s\n",
                     dev_name, errno, strerror (errno));
             return 0;
           }
		   return 1;
     }
	   int close_device(void){
          if (-1 == close (fd)){
              errno_exit ("close");
			  return 0;
          }
		   fd = -1;
		   return 1;
          
      }
	   
	  int start_capturing(void){
		   unsigned int i;
		   enum v4l2_buf_type type;
		   switch (io) {
			 case IO_METHOD_READ:
			   /* Nothing to do. */
			   printf("nothing to do\n");
			   fflush(stdout);
			   return 0;//
			   break;
	   
			 case IO_METHOD_MMAP:
				 for (i = 0; i < n_buffers; ++i) {
					struct v4l2_buffer buf;
					CLEAR (buf);
					buf.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
					buf.memory		= V4L2_MEMORY_MMAP;
					buf.index		= i;
					if (-1 == xioctl (fd, VIDIOC_QBUF, &buf)){
						 errno_exit ("VIDIOC_QBUF");
						 return 0;
					}
				}
				type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				  if (-1 == xioctl (fd, VIDIOC_STREAMON, &type)){
				      errno_exit ("VIDIOC_STREAMON"); 
				      return 0;
					}
				 break;
	   
			  case IO_METHOD_USERPTR:
				 for (i = 0; i < n_buffers; ++i) {
				   struct v4l2_buffer buf;
				   CLEAR (buf);
				   buf.type 	   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				   buf.memory	   = V4L2_MEMORY_USERPTR;
				   buf.index	   = i;
				   buf.m.userptr   = (unsigned long) buffers[i].start;
				   buf.length	   = buffers[i].length;
				   if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
					   errno_exit ("VIDIOC_QBUF");
			   }
			   type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			   if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
				   errno_exit ("VIDIOC_STREAMON");
			   break;
		   }
		   return 1;
	   }
	void stop_capturing(void){
	    enum v4l2_buf_type type;
	    switch (io) {
		   case IO_METHOD_READ:
			  
			/* Nothing to do. */
			  break;

		   case IO_METHOD_MMAP:
		   case IO_METHOD_USERPTR:
			  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	          if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
				 errno_exit ("VIDIOC_STREAMOFF");
	          break;
		  }
    }
	void query_device(){
		struct v4l2_capability cap;
		struct v4l2_fmtdesc    fmtdesc;
		struct v4l2_format     fmt; 
		struct v4l2_cropcap    cropcap;
		ioctl(fd,VIDIOC_QUERYCAP,&cap);
		fprintf(stdout,"Driver Name:%s\nCard Name:%s\nBus info:%s\nDriver Version:%u.%u.%u\n",cap.driver,cap.card,cap.bus_info,(cap.version>>16)&0XFF, (cap.version>>8)&0XFF,cap.version&0XFF);
		fmtdesc.index=0; 
		fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
		printf("Support format:\n");
	    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1){
	      fprintf(stdout,"\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
	      fmtdesc.index++;
	    }

		fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
		ioctl(fd, VIDIOC_G_FMT, &fmt);
	    printf("Current data format information:\n\twidth:%d\n\theight:%d\n",
	    fmt.fmt.pix.width,fmt.fmt.pix.height);
	    ioctl(fd, VIDIOC_G_CROP , &cropcap);
		printf("cropcap.bounds information:\n\twidth:%d\n\theight:%d\tleft:%d\ttop:%d\n",
			cropcap.bounds.width,cropcap.bounds.height,cropcap.bounds.left,cropcap.bounds.top);
	    
		printf("cropcap.defrect information:\n\twidth:%d\n\theight:%d\tleft:%d\ttop:%d\n",
				cropcap.defrect.width,cropcap.defrect.height,cropcap.defrect.left,cropcap.defrect.top);

    }
    void uninit_device(void){
		  unsigned int i;
		  switch (io) {
		  case IO_METHOD_READ:
			  free (buffers[0].start);
			  break;
		  case IO_METHOD_MMAP:
			  for (i = 0; i < n_buffers; ++i)
				  if (-1 == munmap (buffers[i].start, buffers[i].length))
					  errno_exit ("munmap");
			  break;
		  case IO_METHOD_USERPTR:
			  for (i = 0; i < n_buffers; ++i)
				  free (buffers[i].start);
			  break;
		  }
		  free(buffers);
	  }
    int init_device(int width,int height)
      {
	    struct v4l2_capability cap;
	    struct v4l2_cropcap    cropcap;
	    struct v4l2_crop       crop;
	    struct v4l2_format     fmt;
		unsigned int min;
	    if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
	        if (EINVAL == errno) {
	            fprintf (stderr, "%s is no V4L2 device\n",dev_name);
				return 0;
	           
	        } 
			else{
				return 0;
	          
	        }
	    }

	    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
	        fprintf (stderr, "%s is no video capture device\n",dev_name);
	       // exit (EXIT_FAILURE);
	       return 0;
	    }
        printf("init_device io %d\n",io);
		switch (io) {
		  case IO_METHOD_READ:
			if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
				fprintf (stderr, "%s does not support read i/o\n",dev_name);
				
			}
			break;
		  case IO_METHOD_MMAP:
		  case IO_METHOD_USERPTR:
			if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
				fprintf (stderr, "%s does not support streaming i/o\n",dev_name);
				
				return 0;
			}
			break;
		}
	    /* Select video input, video standard and tune here. */
	    CLEAR (cropcap);
	    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
	        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	        crop.c = cropcap.defrect; /* reset to default */

	        if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
	            switch (errno) {
	            case EINVAL:
	                    /* Cropping not supported. */
						fprintf (stderr,"Cropping not supported.\n");
	                    break;
	            default:
	                    /* Errors ignored. */
						fprintf (stderr,"Errors ignored.\n");
	                    break;
	            }
	        }
	    } 
		else {	
	                /* Errors ignored. */
	        }

		//printf("gogogo\n");
	    CLEAR (fmt);
	    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    fmt.fmt.pix.width       = width; 
	    fmt.fmt.pix.height      = height;
	    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	    if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
	        errno_exit ("VIDIOC_S_FMT");

	        /* Note VIDIOC_S_FMT may change width and height. */

		/* Buggy driver paranoia. */
		min = fmt.fmt.pix.width * 2;
		if (fmt.fmt.pix.bytesperline < min)
			fmt.fmt.pix.bytesperline = min;
		min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
		if (fmt.fmt.pix.sizeimage < min)
			fmt.fmt.pix.sizeimage = min;
        //printf("test1\n");
		switch (io) {
		case IO_METHOD_READ:
			init_read (fmt.fmt.pix.sizeimage);
			break;

		case IO_METHOD_MMAP:
			init_mmap (fmt.fmt.pix.width,fmt.fmt.pix.height);
			break;

		case IO_METHOD_USERPTR:
			init_userp (fmt.fmt.pix.sizeimage);
			break;
		}
	}
    int read_frame(procss_callback process_func){
       struct v4l2_buffer buf;
       unsigned int i;
       switch (io) {
	      case IO_METHOD_READ:
    		if (-1 == read (fd, buffers[0].start, buffers[0].length)) {
            		switch (errno) {
            		case EAGAIN:
                    		return 0;
			        case EIO:
							/* Could ignore EIO, see spec. */

							/* fall through */
			        default:
							errno_exit ("read");
			     }
		     }
       // process_image (buffers[0].start);
        break;
	    case IO_METHOD_MMAP:
		    CLEAR (buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
			//VIDIOC_DQBUF 读取缓存
            if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            		switch (errno) {
            		case EAGAIN:
                       return 0;
			        case EIO:
			      	/* Could ignore EIO, see spec. */

				    /* fall through */

			        default:
				    errno_exit ("VIDIOC_DQBUF");
			    }
		    }
            assert (buf.index < n_buffers);
			frame_cnt++;
			//printf("frame_cnt %d\n",frame_cnt);
             process_func (&buffers[buf.index]);//调用回调函数简单处理收到的视频数据
			//重新放入缓存
            if (-1 == xioctl (fd, VIDIOC_QBUF, &buf)){
			   // errno_exit ("VIDIOC_QBUF");
			   printf("VIDIOC_QBUF error\n");
			   return 0;
            }
             return 1;
	    break;

	    case IO_METHOD_USERPTR:
		    CLEAR (buf);
    		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    		buf.memory = V4L2_MEMORY_USERPTR;
            if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
				switch (errno) {
				    case EAGAIN:
					   return 0;
	                case EIO:
					/* Could ignore EIO, see spec. */

					/* fall through */
				    default:
					errno_exit ("VIDIOC_DQBUF");
				}
		    }

		   for (i = 0; i < n_buffers; ++i)
			  if (buf.m.userptr == (unsigned long) buffers[i].start&& buf.length == buffers[i].length)
				 break;

		   assert (i < n_buffers);
           //process_image ((void *) buf.m.userptr);
           if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
			    errno_exit ("VIDIOC_QBUF");

		   break;
	}

	return 1;
  }
  private:
  	
   void init_read(unsigned int buffer_size){
		buffers =(video_buffer*) calloc (1, sizeof (*buffers));
		if (!buffers) {
					fprintf (stderr, "Out of memory\n");
					exit (EXIT_FAILURE);
			}
		buffers[0].length = buffer_size;
		buffers[0].start = malloc (buffer_size);
		if (!buffers[0].start) {
				fprintf (stderr, "Out of memory\n");
					exit (EXIT_FAILURE);
		}
	}
	
	int init_mmap(int width,int height){
		struct v4l2_requestbuffers req;
		CLEAR (req);
		req.count				= 4;//申请4个缓存
		req.type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory				= V4L2_MEMORY_MMAP; 
		if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
			if (EINVAL == errno) {
					fprintf (stderr, "%s does not support "
							 "memory mapping\n", dev_name);
					//exit (EXIT_FAILURE);
					return 0;
			} else {
					//errno_exit ("VIDIOC_REQBUFS");
					return 0;
			}
		}
	
		if (req.count < 2) {
			fprintf (stderr, "Insufficient buffer memory on %s\n",
					 dev_name);
			//exit (EXIT_FAILURE);
			return 0;
		}
	
		buffers = (video_buffer*)calloc (req.count, sizeof (*buffers));
	
		if (!buffers) {
			fprintf (stderr, "Out of memory\n");
			//exit (EXIT_FAILURE);
			return 0;
		}
	
		for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
			struct v4l2_buffer buf;
			CLEAR (buf);
			buf.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory		= V4L2_MEMORY_MMAP;
			buf.index		= n_buffers;
			if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
				 errno_exit ("VIDIOC_QUERYBUF");
			buffers[n_buffers].length = buf.length;
			buffers[n_buffers].start =mmap (NULL /* start anywhere */,buf.length,PROT_READ | PROT_WRITE /* required */,
				   MAP_SHARED /* recommended */,
						  fd, buf.m.offset);
			buffers[n_buffers].width=width;
			buffers[n_buffers].height=height;
			//printf("buffer.length:%d\n",buf.length);
			if (MAP_FAILED == buffers[n_buffers].start){
				 //errno_exit ("mmap");
				 fprintf (stderr, "mmap fail\n");
				 return 0;
				}
		}
		return 1;
	}
	
     void init_userp(unsigned int buffer_size)
	{
		struct v4l2_requestbuffers req;
		unsigned int page_size;
		page_size = getpagesize ();
		buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);
		CLEAR (req);
		req.count		= 4;
		req.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory		= V4L2_MEMORY_USERPTR;
		if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
			if (EINVAL == errno) {
			   fprintf (stderr, "%s does not support "
				"user pointer i/o\n", dev_name);
					exit (EXIT_FAILURE);
			} 
			else {
					errno_exit ("VIDIOC_REQBUFS");
			}
		}
	
		buffers = (video_buffer*)calloc (4, sizeof (*buffers));
		if (!buffers) {
				fprintf (stderr, "Out of memory\n");
				exit (EXIT_FAILURE);
		}
		for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
			buffers[n_buffers].length = buffer_size;
			buffers[n_buffers].start = memalign (/* boundary */ page_size,buffer_size);
			if (!buffers[n_buffers].start) {
			fprintf (stderr, "Out of memory\n");
				exit (EXIT_FAILURE);
			}
	   }
	}
   
  

};

#endif


