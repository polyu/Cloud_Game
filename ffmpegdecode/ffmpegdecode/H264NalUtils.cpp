#include "H264NalUtils.h"
NALU_t *AllocNALU(int buffersize)  
{  
    NALU_t *n;  
    if ((n = (NALU_t*)calloc (1, sizeof(NALU_t))) == NULL)  
    {  
        printf("AllocNALU Error: Allocate Meory To NALU_t Failed ");  
        exit(0);  
    }  
    return n;  
}  
void FreeNALU(NALU_t *n)  
{  
    if (n)  
    {  
        free (n);  
    }  
}  