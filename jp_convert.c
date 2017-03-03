#include "jp_convert.h"
 
//´´½¨
CREATE * new_create(CREATETYPE type){
    CONVERT *c=NULL;
        switch(type){
            case CR32:{
                c=new_8to32_convert();
                break;
            }
            default:{
                c=null;
            }
        }
        return c;
}
 
//×ª»»
CONVERT * new_convert(CONTYPE type){
    CONVERT *c=NULL;
    switch(type){
        case C08T32:{
            c=new_8to32_convert();
            break;
        }
        case C08T24:{
            c=new_8to24_convert();
            break;
        }
        case C08T16:{
            c=new_8to16_convert();
            break;
        }
        default:{
            c=null;
        }
    }
    return c;
}

