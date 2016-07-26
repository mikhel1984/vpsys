/** @file vpb_error.c
~~~ Base Errors ~~~
Managing errors and log */
/* ERROR FILE NUMBER - 0 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>

#include "vpb_error.h"
#include "vp_construct.h"

#define LOG_NAME "vpsys.log"  /* log file name */
#define LOG_MESS_LENGTH 64    /* additional error message max length */

E_RES newError = E_OK;        /* error number */
float errPos = 0;            /* error position */
char  errFunc[LOG_MESS_LENGTH];  /* additional message */

Bool  writeLog = True;       /* write log to file */
Bool  appendLog = False;     /* append data to the end of file */

extern jmp_buf execErr;     /* jump to dlg_exec_code() */
extern jmp_buf registrateErr;
extern jmp_buf addNewErr;

/** Find message for given error
    @fn errorMessage
    @param r - error number
    @return error text */
char* errorMessage(E_RES r);
/** Add current error to log file
    @fn logError */
void logError(void);

/* clear all */
void clearError(void)
{
    newError = E_OK;
    errPos = 0;
    errFunc[0] = '\0';
}

/* Set not fatal error */
int setError(E_RES err, float pos, const char* mess)
{
    /* copy data */
    newError = err;
    errPos = pos;

    if(mess)
        strncpy(errFunc, mess, LOG_MESS_LENGTH-1);
    else
        *errFunc = '\0';
    /* add to log */
    logError();

    return 0;
}

/* Set fatal error */
void fatalError(E_RES err, float pos, const char* mess, u_char type)
{
    extern Bool interactiveRunning;
    /* set data about error */
    setError(err, pos, mess);
    /* jump */
    switch(type) {
    case F_ARITHMETIC:
        longjmp(execErr, 1);
        break;
    case F_MEMORY:
        puts("NO FREE MEMORY IN SYSTEM!");
        if(interactiveRunning)
            longjmp(addNewErr, 1);
        else
            longjmp(registrateErr, 1);
        break;
    default:
        break;
    }
}

/* Print error message */
void printError(void)
{
    printf("ERROR: '");            /* begin */
    u_print(stdout, errorMessage(newError)); /* error type */
    printf("' in %.2lf", errPos);  /* position */
    /* message or end line */
    if(*errFunc)
        printf(" (%s)\n", errFunc);
    else
        printf("\n");
}

/* Error messages */
char* errorMessage(E_RES r)
{
    char* message = NULL;

    switch(r) {

    case E_OK:
        message = lngText("e_noerr", "No errors");
        break;
    case E_WRONG_TYPE:
        message = lngText("e_wrtype", "Wrong data type");
        break;
    case E_ZERO_DEVIDE:
        message = lngText("e_dzero", "Devide by zero");
        break;
    case E_NO_MEMORY:
        message = lngText("e_nomem", "No free memory");
        break;
    case E_WRONG_ARG:
        message = lngText("e_wrarg", "Wrong argument");
        break;
    case E_DIFF_TYPE:
        message = lngText("e_diftp", "Different data type");
        break;
    case E_DIFF_SIZE:
        message = lngText("e_arrdif", "Different array size");
        break;
    case E_FOPEN_ERR:
        message = lngText("e_fopne", "Can't open file");
        break;
    case E_SYNTAX:
        message = lngText("e_synt", "Sintax error");
        break;
    case E_SYMANTIC:
        message = lngText("e_sym", "Symantic error");
        break;
    case E_WRONG_INDEX:
        message = lngText("e_wrind", "Wrong index");
        break;
    case E_MEM_OUT:
        message = lngText("e_memoff", "No more memory for this operation");
        break;
    case E_NOVERFLOW:
        message = lngText("e_nover", "Number overflow");
        break;
    case E_BAD_VAR:
        message = lngText("e_badv", "Bad variable");
        break;
    case E_NOT_FOUND:
        message = lngText("e_nofn", "Not found");
        break;
    case E_NOT_DEF:
        message = lngText("e_ndef", "Operation is not defined");
        break;
    default:
        message = lngText("e_wtf", "Unexpected error");
        break;
    }

    return message;
}

/* Print current date to log file */
void logDate(void)
{
    time_t t;
    FILE *logFile = NULL;

    if(writeLog) {
        if(appendLog)
            logFile = fopen(LOG_NAME, "a");
        else
            logFile = fopen(LOG_NAME, "w");

        if(!logFile)  return;
        /* current time */
        t = time(NULL);
        /* write to file */
        fprintf(logFile, "#### %s\n", ctime(&t));

        fclose(logFile);
    }
}

/* Write current error to file */
void logError(void)
{
    FILE *logFile = NULL;

    if(writeLog) {
        if((logFile = fopen(LOG_NAME, "a")) == NULL) return;
        /* write */
        fprintf(logFile, "# ERROR: '");
        u_print(logFile, errorMessage(newError)); /* utf-8 */
        fprintf(logFile, "' in %.2f", errPos);

        if(*errFunc)
            fprintf(logFile, " (%s)\n", errFunc);
        else
            fprintf(logFile, "\n");

        fclose(logFile);
    }
}

/* Add text to log */
void logText(char* mes)
{
    FILE *logFile = NULL;

    if(writeLog && *mes) {

        if((logFile = fopen(LOG_NAME, "a")) == NULL) return;

        fprintf(logFile, "%s\n", mes);

        fclose(logFile);
    }
}
