/** @file vpe_user.h
~~~ User's Element ~~~
Create and register user defined type */

#ifndef VPE_USER_H
#define VPE_USER_H

#include "vpb_abstract.h"

#define CODE_SIZE 10240  /* max size of script file */

/** Register user's type
    @fn regUserType
    @param utype - type name
    @return True if registered successfully */
Bool regUserType(const char* utype);

#endif /* VPE_USER_H */
