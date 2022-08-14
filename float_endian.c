/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *
 ***************************************************************************/

#include <stdio.h>
#include <sysexits.h>

int     main(int argc,char *argv[])

{
    float   num = 0.1;
    
    printf("%f %08lx %08x\n", num,
	    *(unsigned long *)&num, *(unsigned char *)&num);
    
    return EX_OK;
}

