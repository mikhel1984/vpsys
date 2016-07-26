/** @file vpe_user.c
Functions for user's type creation */

/* ERROR FILE NUMBER - 14 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpe_user.h"
#include "vpb_error.h"
#include "vpc_parser.h"
#include "vpc_translate.h"
#include "vpb_memory.h"
#include "vpb_ticket.h"
#include "vpb_connectors.h"
#include "vpc_dialog.h"
#include "vpb_element.h"
#include "vp_construct.h"

#define TYPE_END   ".t"

extern char *dlgErr[];

/* buffer for code text */
char fileBuffer[CODE_SIZE];

/** Create ticket for type
    @fn usr_tic_fill
    @param elt - ticket index
    @return true if no errors */
Bool usr_tic_fill(int elt);
/** Add connector
    @fn usr_con_init
    @param e - ticket index
    @param dir - data direction
    @return true if no errors */
Bool usr_con_init(int e, int dir);
/** Fill connector parameters
    @fn usr_con_fill
    @param t - connector ticket
    @return true if no errors */
Bool usr_con_fill(ticCon* t);
/** Add parameter
    @fn usr_par_init
    @param e - ticket index
    @return true if no errors */
Bool usr_par_init(int e);
/** Fill type parameter
    @fn usr_par_fill
    @param t - parameter ticket
    @return true if no errors */
Bool usr_par_fill(ticPar* t);

/* Registrate user's type */
Bool regUserType(const char* utype)
{
    /* ERROR 02 */
    int elt;
    char fName[MAXNAME];
    /* create file name */
    strncpy(fName, utype, MAXNAME-3);
    strcat(fName, TYPE_END);
    /* try to load file */
    if(!loadFile(fName, fileBuffer, CODE_SIZE))
        return setError(E_FOPEN_ERR, 14.02, fName);
    /* registrate type */
    elt = regElement(utype, NULL);
    if(elt == -1) return False;
    /* 0 - code */
    regParameter(elt, NULL, NULL, dataFree(KEY_CODE, 1, 0), False);
    /* 1 - constants */
    regParameter(elt,  NULL, NULL, dataArrayNull(), False);
    /* data processing */
    regExec(elt, transCodeExec);
    /* try to read and parse file */
    setProgramm(fileBuffer);
    if(!usr_tic_fill(elt)) return False;

    return True;
}

/* Prepare ticket */
Bool usr_tic_fill(int elt)
{
    /* ERROR 01 */
    Ticket *tick;

    /* comments or empty strings */
    if(getToken() == T_ENDLINE)
        return usr_tic_fill(elt);

    /* get description */
    if(token_type == T_STRING) {
        /* get ticket */
        tick = memGetTicket(elt);
        /* string have to be description */
        tick->u8_descript = stringCopy(tokenPtr, NULL);
        /* end line */
        if(getToken() == T_ENDLINE)
            return usr_tic_fill(elt);
        else
            return setError(E_SYNTAX, 14.01, tokenPtr);
    }
    /* get parameter of connector descriptions */
    while(token_spec == S_RANGE) {

        getToken();
        /* input connector */
        if(stringEqUp(tokenPtr, "IN")) {
            if(!usr_con_init(elt, C_IN)) return False;
        }
        /* output connector */
        else if(stringEqUp(tokenPtr, "OUT")) {
            if(!usr_con_init(elt, C_OUT)) return False;
        }
        /* parameter */
        else if(stringEqUp(tokenPtr, "PAR")) {
            if(!usr_par_init(elt)) return False;
        }
        /* function description */
        else if(stringEqUp(tokenPtr, "FUNC"))
            break;
        else
            return setError(E_SYNTAX, 14.01, "expected block");

    }
    /* at the end of file have to be function */
    if(stringEqUp(tokenPtr, "FUNC")) {

        getToken();
        /* read and compile code from file */
        if(!trans_code(elt)) return False;
        /* write code to element */
        if(!transWriteCode(elt)) return False;
    }
    else
        return setError(E_SYMANTIC, 14.01, "expected 'FUNC'");

    return True;
}

/* Add connector */
Bool usr_con_init(int e, int dir)
{
    /* ERROR 04 */
    int pos;
    ticCon* ptr;
    Ticket* t;

    /* comment or empty string */
    if(getToken() == T_ENDLINE)
        return usr_con_init(e, dir);

    if(token_type == T_END || token_spec == S_RANGE)
        return True;
    /* connector name */
    if(token_type != T_WORD) return setError(E_SYNTAX, 14.04, dlgErr[3]);
    /* check existing */
    pos = (dir == C_IN) ? cinPos(e, tokenPtr) : coutPos(e, tokenPtr);
    /* add new with default settings */
    if(pos == -1)
        pos = regConnector(e, tokenPtr, NULL, DAT_ONE, dir);
    if(pos == -1) return False;
    /* next */
    getToken();
    if(token_type == T_ENDLINE || *tokenPtr == ',')
        return usr_con_init(e, dir);
    /* fill values */
    else if(*tokenPtr == ':') {
        /* get connector ticket */
        t = memGetTicket(e);
        ptr = (dir == C_IN) ? t->inLast : t->outLast;
        /* get value */
        if(!usr_con_fill(ptr)) return False;
        /* get next */
        return usr_con_init(e, dir);
    }
    else if(token_type == T_END)
        return True;

    return setError(E_SYNTAX, 14.04, tokenPtr);
}

/* Initialize connector */
Bool usr_con_fill(ticCon* t)
{
    /* ERROR 05 */
    Number n;

    /* empty strings */
    if(getToken() == T_ENDLINE)
        return usr_con_fill(t);
    /* description */
    if(token_type == T_STRING) {
        t->u8_descript = stringCopy(tokenPtr, NULL);
        return dlg_end(14.05);
    }
    /* data type */
    if(token_type != T_WORD || !stringEqUp(tokenPtr, "TYPE"))
        return setError(E_SYNTAX, 14.05, "expected 'TYPE'");

    getToken();
    if(*tokenPtr != '=') return setError(E_SYNTAX, 14.05, dlgErr[7]);

    getToken();
    if(stringEqUp(tokenPtr, "ANY"))
        t->dat = DAT_ANYTYPE;
    else if(stringEqUp(tokenPtr, "NUMBER"))
        t->dat = DAT_ONE;
    else if(stringEqUp(tokenPtr, "ARRAY")) {
        /* get 'array[size]' */
        getToken();
        if(*tokenPtr == '[') {
            getToken();
            if(*tokenPtr != ']') {
                /* get size */
                if(!eval_num(&n) || n.type != nINT || n.iVal < 0)
                    return setError(E_SYMANTIC, 14.05, "wrong size");
                t->dat = n.iVal;
                if(*tokenPtr != ']') return setError(E_SYNTAX, 14.05, dlgErr[6]);
            }
            else t->dat = DAT_ANYSIZE;
        }
        else {
            t->dat = DAT_ANYSIZE;
    } }
    else if(stringEqUp(tokenPtr, "STRING"))
        t->dat = DAT_STRING;
    else if(stringEqUp(tokenPtr, "SIGNAL"))
        t->dat = DAT_SIGNAL;
    else
        return setError(E_SYMANTIC, 14.05, "unexpected parameter");

    /* end */
    getToken();

    if(*tokenPtr == ',')
        return usr_con_fill(t);
    else if(token_type == T_ENDLINE || token_type == T_END)
        return True;

    return setError(E_SYNTAX, 14.05, tokenPtr);
}

Bool usr_par_init(int e)
{
    /* ERROR 06 */
    int pos;
    ticPar *ptr;

    getToken();
    /* comment or empty string */
    if(token_type == T_ENDLINE)
        return usr_par_init(e);
    if(token_type == T_END || token_spec == S_RANGE)
        return True;
    /* parameter name */
    if(token_type != T_WORD) return setError(E_SYNTAX, 14.06, dlgErr[3]);
    /* check and create */
    pos = parPos(e, tokenPtr);
    if(pos == -1) {
        pos = regParameter(e, tokenPtr, NULL, dataInt(0), True);
        if(pos == -1)  return False;
    }
    /* fill or next parameter */
    getToken();
    if(token_type == T_ENDLINE || *tokenPtr == ',')
        return usr_par_init(e);
    else if(*tokenPtr == ':') {

        ptr = parameterFind(e, pos);

        if(!usr_par_fill(ptr)) return False;
        return usr_par_init(e);
    }
    else if(token_type == T_END)
        return True;

    return setError(E_SYNTAX, 14.06, tokenPtr);
}


Bool usr_par_fill(ticPar* t)
{
    /* ERROR 07 */
    Number n;
    char parBuff[MAXNAME];

    if(getToken() == T_ENDLINE)
        return usr_par_fill(t);
    /* description */
    if(token_type == T_STRING) {
        t->u8_descript = stringCopy(tokenPtr, NULL);
        return dlg_end(14.07);
    }
    /* parameter name */
    if(token_type != T_WORD) return setError(E_SYNTAX, 14.07, dlgErr[5]);
    strncpy(parBuff, tokenPtr, MAXNAME-1);
    /* set */
    getToken();
    if(*tokenPtr != '=') return setError(E_SYNTAX, 14.07, dlgErr[7]);
    /* value */
    getToken();
    if(stringEqUp(parBuff, "VAL")) {
        if(token_type == T_STRING) {
            /* string parameter */
            stringSet(&(t->dat), tokenPtr);
            getToken();
        }
        /* array - ? */
        else {
            /* boolean or number value */
            if(!eval_num(&n)) return False;

            t->dat.type = tNumber;
            t->dat.numVal = n;
        }
    }
    else
        return setError(E_SYNTAX, 14.07, NULL);
    /* continue */
    if(*tokenPtr == ',')
        return usr_par_fill(t);
    else if(token_type == T_ENDLINE || token_type == T_END)
        return True;

    return setError(E_SYNTAX, 14.07, tokenPtr);
}
