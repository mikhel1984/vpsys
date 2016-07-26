/* Test - interface */
#include <mcheck.h>
#include <stdio.h>
#include <stdlib.h>

#include "vp_sys.h"
#include "vpb_lang.h"
#include "vpb_error.h"


int main(int argc, char *argv[])
{
    /* memory control */
    mtrace();

    return vp_sys(argc, argv);
}
