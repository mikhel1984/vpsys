/** @file vpb_error.h
~~~ Base Errors ~~~
Error messages and work logging */

#ifndef VPB_ERROR_H
#define VPB_ERROR_H

#include "vpb_abstract.h"

#define _CRITICAL(X)  if((X) == NULL) {   \
                        fatalError(E_NO_MEMORY, 0.0, NULL, F_MEMORY); }

/** @enum ErrorType
    List of possible errors */
enum ErrorType
{
    E_OK,          /* no errors */
    E_WRONG_TYPE,  /* wrong data type */
    E_ZERO_DEVIDE, /* deviding on zero */
    E_NO_MEMORY,   /* no free memory */
    E_WRONG_ARG,   /* wrong arguments */
    E_DIFF_TYPE,   /* different data type */
    E_DIFF_SIZE,   /* different array size */
    E_FOPEN_ERR,   /* can't open new file */
    E_SYNTAX,      /* syntax error */
    E_SYMANTIC,    /* symatic error */
    E_WRONG_INDEX, /* wrong array index */
    E_MEM_OUT,     /* no more given memory */
    E_NOVERFLOW,   /* overflow number value */
    E_BAD_VAR,     /* bad variable */
    E_NOT_FOUND,   /* not found */
    E_NOT_DEF,     /* variable not defined */
    E_WTF          /* temporary */
};
typedef enum ErrorType E_RES;

extern E_RES newError;

/** @enum FatalType
    Fatal error types */
enum FatalType { F_ARITHMETIC = 0, F_MEMORY };

/** Not critical error
    @fn setError
    @param err - error type
    @param pos - error position code (file . function)
    @param mess - additional message
    @return False allways */
int setError(E_RES err, float pos, const char* mess);
/** Critical error
    @fn fatalError
    @param err - error type
    @param pos - error position code (file . function)
    @param mess - additional message
    @param type - type of fatal error */
void fatalError(E_RES err, float pos, const char* mess, u_char type);
/** Print message about error
    @fn printError */
void printError(void);
/** Print to log file current date
    @fn logDate */
void logDate(void);
/** Print to log file text message
    @fn logText
    @param mes - text */
void logText(char* mes);
/** Clear data about last error
    @fn clearError */
void clearError(void);

#endif /* VPB_ERROR_H */
