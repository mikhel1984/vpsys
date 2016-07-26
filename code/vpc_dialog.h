/** @file vpc_dialog.h
~~~ Code Dialog ~~~
Functions for dialog with user */

#ifndef VPC_DIALOG_H
#define VPC_DIALOG_H

#include "vpb_abstract.h"

/** Check end of command
    @fn dlg_end
    @param f - function's index
    @return true if no errors */
Bool dlg_end(float f);

/** Execute code
    @fn dlg_exec_code
    @param c - code string
    @return -1 if get error, 0 if no errors, 1 if have to exit */
int dlg_exec_code(char* c);

#endif /* VPC_DIALOG_H */
