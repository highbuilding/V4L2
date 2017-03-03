/***********Author:9crk 2014-12-24*****************************/
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "stdio.h"
/******************for time mesurement*************************/
#include <sys/time.h>
struct timeval tpstart,tpend;
unsigned long timeuses;
void timeRec()
{
    gettimeofday(&tpstart,0);
}
int timeRep()
{
    gettimeofday(&tpend,0);
    timeuses=(tpend.tv_sec-tpstart.tv_sec)*1000000+tpend.tv_usec-tpstart.tv_usec;
    printf("use time: %uus\n",timeuses);
    return timeuses;
}
/********************end**************************************/

int main(int argc, char* argv[])
{
   IplImage* img = NULL;
   IplImage* cutImg = NULL;
   CvMemStorage* storage = cvCreateMemStorage(0);
   CvHaarClassifierCascade* cascade = (CvHaarClassifierCascade*)cvLoad("./haarcascade_frontalface_alt2.xml", 0, 0, 0);
   CvSeq* faces; 
   img = cvLoadImage(argv[1], 0);
   timeRec();
   faces = cvHaarDetectObjects(img, cascade,  storage, 1.2, 2, 0, cvSize(25,25) );
   timeRep();
   if (faces->total == 0){
        printf("no face!\n");
   }
   cvSetImageROI(img, *((CvRect*)cvGetSeqElem( faces, 0))); 
   cvSaveImage("face.bmp", img);    
   cvResetImageROI(img);
   printf("face detected! in face.bmp!\n");
}
