/** @file vpc_parser.c
~~~ Code Parser ~~~
Realisation of parser */

/* FILE ERROR: 10 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "vpc_parser.h"
#include "vpb_error.h"
#include "vpb_data.h"
#include "vpb_memory.h"
#include "vpb_element.h"
#include "vpb_vars.h"

#define TOKEN_LENGTH 256    /* memory length for 1 token */
#define SET_OPERATION 64    /* left token max length */

char *prog;                 /* pointer to programm text */
char token[TOKEN_LENGTH];   /* token text */
Token token_type;            /* token type */
Special token_spec;          /* delimeter type */
char* tokenPtr = token;     /* pointer to token */

/* find set operation */
char leftToken[SET_OPERATION];  /* left token in set operation */
char *leftTokenPtr = leftToken; /* pointer to left token */

LastState memState = {0,0,NULL};
extern char *dlgErr[];

/** Check character to delimeter
    @fn isdelim
    @param c - character
    @return true if delimeter */
Bool isdelim(char c);
/** Get number from text
    @fn get_num
    @param n - pointer to number
    @return true if was no errors */
Bool get_num(Number *n);
/** Multiply 2 numbers
    @fn eval_num1
    @param n - result
    @return true if was no errors */
Bool eval_num1(Number *n);
/** Single operator
    @fn eval_num2
    @param n - result
    @return true if was no errors */
Bool eval_num2(Number *n);
/** Work with parenthis
    @fn eval_num3
    @param n - result
    @return true if was no errors */
Bool eval_num3(Number *n);
/** Evaluate function
    @param eval_func
    @param n - result
    @param fn - function index
    @return true if was no errors */
Bool eval_func(Number *n, int fn);

Bool eval_arr_func(Data *d, Number *n);

Bool eval_data_arg(Data* dl, u_char* val, u_char max);

Bool eval_get_array(Data *d);

Bool eval_variable(Data* d, Data* varData);

/* Set pointer to code text */
void setProgramm(char* code) { prog = code; }

/* Find number in text */
Token getNumberType(int *length)
{
    char *tmp = prog;
    register int size = 0;
    Bool dot = False;
    /* check for bin or hex integers */
    if(*tmp == '0') {
        tmp ++; size ++;
        /* bin is '0b...' or '0B...' */
        if(*tmp == 'b' || *tmp == 'B') {
            tmp ++; size++;
            while(*tmp && (*tmp == '0' || *tmp == '1')) { size ++; tmp++; }
            *length = size;
            return (size - 2 > 0) ? T_BIN : T_ERR;
        }
        /* hex is '0x...' or '0X...' */
        else if(*tmp == 'x' || *tmp == 'X') {
            tmp++; size++;
            while(*tmp && isxdigit(*tmp)) { size++; tmp++; }
            *length = size;
            return (size - 2 > 0) ? T_HEX : T_ERR;
        }
    }
    /* dec number */
    /* first digits */
    while(isdigit(*tmp)) { size ++; tmp++; }

    *length = size;
    /* get integer */
    if(!*tmp || !strchr(".eE", *tmp))
        return T_INT;
    /* float point */
    if(*tmp == '.') {
        tmp ++; size ++;
        /* check rest */
        if(!isdigit(*tmp)) return T_INT;
        dot = True;
        while(isdigit(*tmp++)) size ++;

        *length = size;
        if(*tmp != 'e' && *tmp != 'E') return T_FLOAT;
    }
    /* exponent */
    if(*tmp == 'e' || *tmp == 'E') {
        tmp ++; size ++;
        if(!strchr("0123456789+-", *tmp))
            return dot ? T_FLOAT : T_INT;
        /* sign */
        if(*tmp == '-' || *tmp == '+') {
            tmp ++; size ++;
            if(!isdigit(*tmp))
                return dot ? T_FLOAT : T_INT;
        }
        /* count rest */
        while(isdigit(*tmp++)) size ++;

        *length = size;
    }

    return T_FLOAT;
}

/* check whitespace */
Bool iswhite(char c)
{
    return (c == ' ' || c == '\t' || c == '\r');
}

/* check delimeter */
Bool notdelim(char c)
{
    return (isalnum(c) || c == '_');
}

/* Get token from stream */
Token getToken(void)
{
    /* ERROR: 01 */
    int size = 0;
    char* temp = token;

    /* prepare */
    *temp = '\0';
    token_type = T_NONE;
    token_spec = S_NONE;

    /* white space */
    while(*prog && iswhite(*prog)) prog++;
    /* text end */
    if(*prog == '\0')
        return (token_type = T_END);
    /* end line */
    if(*prog == '\n') {
        prog++;
        return (token_type = T_ENDLINE);
    }
    /* comment */
    if(*prog == COMMENT) {
        while(*prog && *prog != '\n') prog ++;
        if(*prog) {
            prog ++;
            return (token_type = T_ENDLINE);
        }
        else
            return (token_type = T_END);
    }
    /* key characters */
    if(strchr("@&%", *prog)) {
        *temp++ = *prog++;
        *temp = '\0';
        return (token_type = T_KEYCHAR);
    }
    /* delimeter */
    if(strchr(":=,.->'+*/[]();{}<?!", *prog)) {
        *temp = *prog;
        prog++;
        /* connection */
        if(*temp == '-' && *prog == '>') {
            temp ++; *temp = *prog; prog ++;
            token_spec = S_LINK;
        }
        /* range */
        if(*temp == '.' && *prog == '.') {
            temp++;
            while(*prog == '.') *temp++ = *prog++;
            temp --;
            token_spec = S_RANGE;
        }
        /* logical operators */
        if(*prog == '=') {
            switch(*temp) {
            /* equail */
            case '=': token_spec = S_EQ; break;
            /* not equail */
            case '!': token_spec = S_NE; break;
            /* lighter or equial */
            case '<': token_spec = S_LE; break;
            /* greater or equail */
            case '>': token_spec = S_GE; break;
            /* reference */
            case ':': token_spec = S_REF; break;
            default:
                setError(E_SYNTAX, 10.01, NULL);
                return (token_type = T_ERR);
            }
            temp ++; *temp = *prog; prog ++;
        }
        /* single logic operator */
        if(*temp == '>' && token_spec == S_NONE)
            token_spec = S_GT;
        else if(*temp == '<' && token_spec == S_NONE)
            token_spec = S_LT;

        temp ++;
        *temp = '\0';
        return (token_type = T_DELIMETER);
    }
    /* string */
    if(*prog == '"') {
        prog++;
        while(*prog && *prog != '"') *temp++ = *prog++;
        if(*prog == '\0') {
            setError(E_SYNTAX, 10.01, dlgErr[11]);
            return (token_type = T_ERR);
        }
        prog ++;
        *temp = '\0';
        return (token_type = T_STRING);
    }
    /* number */
    if(isdigit(*prog)) {
        token_type = getNumberType(&size);
        while(size-- > 0) *temp++ = *prog++;
        *temp = '\0';
        return token_type;
    }
    /* other - expected word */
    while(notdelim(*prog)) *temp++ = *prog++;
    *temp = '\0';
    token_type = T_WORD;

    return token_type;
}

/* return token to stream */
void putback(void)
{
    char* t;
    for(t = token; *t; t++) prog--;
}

/* evaluate number */
Bool get_num(Number *n)
{
    /* ERROR: 02 */
    int pos, var;
    Data* dat;
    Number *ptr;

    switch(token_type) {
    case T_INT:
        transformInt(n, atol(token));
        break;
    case T_FLOAT:
        transformDbl(n, atof(token));
        break;
    case T_BIN:
        transformInt(n, strtol(token+2, NULL, 2) );
        break;
    case T_HEX:
        transformInt(n, strtol(token+2, NULL, 16) );
        break;
    case T_KEYCHAR:
        /* variable or constant */
        if(*token != '%') return setError(E_SYNTAX, 10.02, dlgErr[15]);
        getToken();

        if((var = variableFind(token)) == -1 ||
            (dat = variableGet(var)) == NULL)
            return setError(E_BAD_VAR, 10.02, token);

        if(dat->type == tNumber)
            *n = dat->numVal;
        else if(dat->type == tArray) {
            getToken();
            if(*token == '.') {
                if(!eval_arr_func(dat, n)) return False;
            }
            else {
                putback();
                if((ptr = eval_index(dat)) == NULL) return False;
                *n = *ptr;
            }
        }
        else
            return setError(E_WRONG_TYPE, 10.02, NULL);

        break;
    case T_WORD:
        /* logical values */
        if(stringEqUp(token, "TRUE"))
            *n = constOne;
        else if(stringEqUp(token, "FALSE"))
            *n = constZero;
        /* function */
        else if((pos = getFunc(token)) != -1) {
            return eval_func(n, pos);
        }
        else
            goto err; /* error expected number */
        break;
err:
    default:
        return setError(E_SYNTAX, 10.02, token);
    }
    getToken();

    return True;
}

/* evaluate function */
Bool eval_func(Number *n, int fn)
{
    /* ERROR: 03 */
    Number arg[MAX_ARG_NUMBER];
    int i = 0;
    Bool ans = False;
    /* first parenthis */
    getToken();
    if(*token != '(') return setError(E_SYNTAX, 10.03, NULL);
    /* evaluate arguments, save to array  */
    do{
        getToken();
        if(!eval_num(arg + i)) return False;
        i ++;
    } while(*token == ',');
    /* second parenthis */
    if(*token == ')') {
        /* check argument number */
        if(i == (int) getFuncArg(fn))
            ans = evalFunc(fn, arg, n);
        else
            return setError(E_WRONG_ARG, 10.03, "wrong number");
    }
    else
        return setError(E_SYNTAX, 10.03, dlgErr[16]);

    getToken();

    return ans;
}

/* sum */
Bool eval_num(Number *n)
{
    Number tmpVal;
    char op;

    if(!eval_num1(n)) return False;

    while((op = *token) == '+' || op == '-') {
        getToken();

        if(!eval_num1(&tmpVal)) return False;

        switch(op) {
        case '+':  *n = nSum(*n, tmpVal); break;
        case '-':  *n = nSub(*n, tmpVal); break;
    } }

    return True;
}

/* product */
Bool eval_num1(Number *n)
{
    Number tmpVal;
    char op;

    if(!eval_num2(n)) return False;

    while((op = *token) == '*' || op == '/') {
        getToken();

        if(!eval_num2(&tmpVal)) return False;

        switch(op) {
        case '*':  *n = nProd(*n, tmpVal); break;
        case '/':  *n = nRat(*n, tmpVal); break;
    } }

    return True;
}

/* single sign */
Bool eval_num2(Number *n)
{
    char op = '\0';

    if(*token == '+' || *token == '-') {
        op = *token;
        getToken();
    }

    if(!eval_num3(n)) return False;

    if(op == '-')  numInvert(n);

    return True;
}

/* parenthis () */
Bool eval_num3(Number *n)
{
    /* ERROR: 04 */
    if(*token == '(') {
        getToken();

        if(!eval_num(n)) return False;

        if(*token != ')') return setError(E_SYNTAX, 10.04, dlgErr[16]);

        getToken();
    }
    else
        return get_num(n);

    return True;
}

/* Read file to buffer */
Bool loadFile(const char* fName, char* b, int buff)
{
    /* ERROR: 07 */
    FILE *fPtr;
    int i = 0;
    /* open */
    if((fPtr = fopen(fName, "r")) == NULL) return False;
    /* read */
    do {
        *b = getc(fPtr);
        b++; i++;
    } while(!feof(fPtr) && i < buff);
    /* end */
    if(*(b-2) == 0x1a)
        *(b-2) = '\0';
    else
        *(b-1) = '\0';
    /* close file */
    fclose(fPtr);

    return True;
}
/* get left part of constructions 'left = right' */
Bool getSetTokens(void)
{
    //getToken();
    /* looking for set operation */
    while(getToken() != T_END) {
        if(*token == '=')
            break;
        /* copy left part */
        else if(token_type == T_WORD)
            strncpy(leftToken, token, SET_OPERATION-1);
        //getToken();
    }

    return (*token == '=' && getToken() != T_END);
}

/* remember position */
LastState setPoint(void)
{
    memState.type = token_type;
    memState.spec = token_spec;
    memState.pos  = prog;

    return memState;
}

/* go to position */
void goToPoint(LastState* ls)
{
    if(ls) {
        token_type = ls->type;
        token_spec = ls->spec;
        prog = ls->pos;
    }
    else {
        token_type = memState.type;
        token_spec = memState.spec;
        prog = memState.pos;
    }
}

Bool eval_concat(Data* res, Data* dat)
{
    /* ERROR: 08 */
    Number n;
    int var;

    if(!stringSet(res, dat ? dataAsText(*dat) : token))
        return False;

    getToken();

    while(*token == '+') {
        setPoint();

        getToken();
        if(*token == '%') {
            if(getToken() != T_WORD)
                return setError(E_SYNTAX, 10.08, dlgErr[3]);
            if((var = variableFind(token)) == -1 ||
                (dat = variableGet(var)) == NULL)
                return setError(E_BAD_VAR, 10.08, token);

            if(dat->type == tNumber)
                goToPoint(NULL);
            else {
                if(!stringAdd(res, dataAsText(*dat))) return False;
                getToken();
                continue;
        } }

        if(token_type == T_STRING) {
            if(!stringAdd(res, token)) return False;
            getToken();
        }
        else {
            if(!eval_num1(&n) || !stringAdd(res, numAsText(n)))
                return False;
    } }

    return True;
}

/* Find arguments of function (numbers) */
Bool eval_args(Number *n, u_char *val, u_char max)
{
    int i = 0;

    do{
        getToken();
        if(!eval_num(n)) return False;
        n++; i++;
    } while(*token == ',' && i < (int) max);

    *val = (u_char) i;

    return True;
}

/* Find index of element */
Number* eval_index(Data* d)
{
    Number arg[MAX_ARG_NUMBER];
    u_char sz = 0;

    getToken();
    if(*token != '(') return NULL;

    if(!eval_args(arg, &sz, MAX_ARG_NUMBER) || sz == 0 || *token != ')')
        return NULL;

    return arrayNumPos(d, arg, sz);
}

Bool eval_arr_func(Data *d, Number *n)
{
    /* ERROR: 09 */
    int fn;
    Data darg, *ans;
    u_char sz = 0;

    darg = dataInt(0);

    getToken();
    fn = arrDotFind(token);

    if(fn == -1 || fn >= ARR_NUM_FUNC)
        return setError(E_SYMANTIC, 10.09, token);

    getToken();
    if(*token != '(') return setError(E_SYNTAX, 10.09, dlgErr[17]);

    getToken();
    if(*token != ')') {
        if(!eval_num(&(darg.numVal))) return False;
        if(*token != ')') return setError(E_SYNTAX, 10.09, dlgErr[16]);
        sz = 1;
    }

    if((ans = arrDotEval(fn, d, &darg, sz)) == NULL) return False;

    *n = ans->numVal;

    return True;
}

Data* eval_method(Data* in)
{
    /* ERROR: 10 */
    int fn;
    u_char argNum;
    Data *dList = getTmpData(MAX_ARG_NUMBER);

    if(!in || !dList)
        return (void*) setError(E_WRONG_ARG, 10.10, NULL);

    if(getToken() != T_WORD)
        return (void*) setError(E_SYNTAX, 10.10, dlgErr[3]);

    switch(in->type) {
    case tNumber:
        return in; /* ? */
    case tArray:
        if((fn =arrDotFind(token)) == -1)
            return (void*) setError(E_SYMANTIC, 10.10, token);
        break;
    case tText:
        return in; /* ? */
    default:
        return (void*) setError(E_WRONG_TYPE, 10.10, NULL);
    }

    getToken();
    if(*token != '(')
        return (void*) setError(E_SYNTAX, 10.10, dlgErr[17]);

    if(!eval_data_arg(dList, &argNum, MAX_ARG_NUMBER)) return NULL;

    if(*token != ')')
        return (void*) setError(E_SYNTAX, 10.10, dlgErr[16]);

    return arrDotEval(fn, in, dList, argNum);
}


Bool eval_data(Data *d, int expec)
{
    /* ERROR: 11 */
    Data *dv;
    Number n;

    setPoint();

    /* string */
    if(token_type == T_STRING) {
        if(expec != -1 && expec != tText)
            return setError(E_WRONG_TYPE, 10.11, "string");
        if(d->type != tText) {
            dataClear(d);
            *d = dataStringNull();
        }
        return eval_concat(d, NULL);
    }
    /* array */
    if(*token == '[') {
        if(expec != -1 && expec != tArray)
            return setError(E_WRONG_TYPE, 10.11, "array");
        return eval_get_array(d);
    }
    /* variable */
    if(*token == '%') {
        setPoint();
        getToken();
        if((dv = variableGet(variableFind(token))) == NULL)
            return setError(E_BAD_VAR, 10.11, token);
        if(expec != -1 && expec != dv->type)
            return setError(E_WRONG_TYPE, 10.11, NULL);

        return eval_variable(d, dv);
    }

    if(!eval_num(&n)) return False;
    if(expec != -1 && expec != tNumber)
        return setError(E_WRONG_TYPE, 10.11, "number");
    dataClear(d);
    d->type = tNumber;
    d->numVal = n;

    return True;
}

Bool eval_data_arg(Data* dl, u_char* val, u_char max)
{
    int i = 0;

    do{
        getToken();
        if(*token == ')' && i == 0) break;
        if(!eval_data(dl+i++, -1)) return False;
        //i ++;
    } while(*token == ',' && i < (int) max);

    *val = (u_char) i;

    return True;
}

Bool eval_get_array(Data *d)
{
    /* ERROR: 12 */
    int col = 0, raw = 0, sum = 0, i, j;
    Number *pn;
    char *c = setPoint().pos, digits = False;

    while(*c && *c != ']') {
        switch(*c++) {
            case ',': col ++; sum ++; break;
            case ';': raw ++; sum ++; col = 0; break;
            case '0': case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9': case '%':
                digits = True; break;
            default: break;
        }
    }
    if(*c != ']') return setError(E_SYNTAX, 10.12, dlgErr[6]);
    goToPoint(NULL);
    dataClear(d);
    *d = dataArrayNull();
    /* null size */
    if(!col && !raw && digits == False) {
        getToken(); /* ] */
        goto done;
    }
    /* check correct size */
    sum ++; col++; raw++;
    if(col*raw != sum) return setError(E_SYMANTIC, 10.12, "wrong size");
    /* fill */
    if(!resizeArray(&(d->arrVal), col * raw) ||
        !resizeDim(&(d->arrVal), 2, raw, col)) return False;
    pn = d->arrVal.numb;
    sum = i = j = 0;
    do {
        if(getToken() == T_ENDLINE) continue;
        if(!eval_num(pn + sum)) return False;
        sum += raw;
        if(++j == col) {
            j = 0; i ++;
            sum = i;
        }
    } while(strchr(",;\n", *token) && i < raw);

done:
    getToken(); /* next */

    return True;
}

Bool eval_variable(Data* d, Data* varData)
{
    /* ERROR: 13 */
    Number n;

    if(d != varData) dataClear(d);

    /* text */
    if(varData->type == tText) {
        *d = dataStringNull();
        return eval_concat(d, varData);
    }

    goToPoint(NULL);
    token[0] = '%'; token[1] = '\0';
    /* check for number */
    if(eval_num(&n)) {
        if(d != varData) {
            d->type = tNumber;
            d->numVal = n;
        }
        return True;
    }

    /* get array */
    goToPoint(NULL);

    getToken(); /* var name */
    getToken();
    if(varData->type == tArray) {
        if(*token == '.') {
            /* prepare array */
            if(d != varData)  dataCopy(d, *varData);
            /* evaluate */
            if(eval_method(d) == NULL) return False;
            getToken();
            return True;
        }
        else if(*token != '[') /* ? */
            return dataCopy(d, *varData);
    }
    else
        return setError(E_WRONG_TYPE, 10.13, NULL);

    return setError(E_WTF, 10.13, NULL);
}

