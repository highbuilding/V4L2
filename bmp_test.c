#include "jp_bmp.h"
 
int main(int argc,char ** argv){
    BMP_OPS *bmp=new_bmp_ops();
    bmp->read_file(bmp,"bd.bmp");
 
    //BMP_OPS *to_ops=bmp->convert(bmp,new_convert(C08T32));
 
   // to_ops->write_file(to_ops,"bmp32.bmp");
   // to_ops->ptint_bmp_inf(to_ops);
    bmp->ptint_bmp_inf(bmp);
    delete_bmp_ops(bmp);
   // delete_bmp_ops(to_ops);
    return 0;
}

