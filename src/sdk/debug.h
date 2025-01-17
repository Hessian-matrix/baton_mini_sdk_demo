

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

#define DEBUG
#ifdef DEBUG 
#define dbg(arg...) {\
    printf("[dbg]:%s:%s:%d ---->",__FILE__,__FUNCTION__,__LINE__);\
    printf(arg);\
    fflush(stdout);\
}
#else
#define dbg(arg...){ /*printf(arg);fflush(stdout);*/}
#endif 

#ifndef EXIT_SAFE
    #define EXIT_SAFE(){DBG("exit safe\n");exit(1);}
#endif

#endif


