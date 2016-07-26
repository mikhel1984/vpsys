/** @file vpc_translate.c
~~~ Code Translate ~~~
Translator realisation */
/* ERROR FILE NUMBER - 11 (last 19)*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpc_translate.h"
#include "vpb_ticket.h"
#include "vp_construct.h"
#include "vpb_data.h"
#include "vpb_error.h"
#include "vpc_parser.h"
#include "vpb_element.h"
#include "vpb_memory.h"
#include "vpb_connectors.h"

#define ARRAY_WORD_SIZE  1000
#define ARRAY_CONST_SIZE 256
#define ARRAY_VAR_SIZE   256
#define ARRAY_PTR_SIZE   64

#define POS_CONST  1
#define POS_VAR    2
#define CELL_PAR   1

Word code[ARRAY_WORD_SIZE];              /* array of words */
Number numConst[ARRAY_CONST_SIZE];       /* array of constants */
u_char tmpLock[ARRAY_VAR_SIZE] = {1, 1}; /* array of variable positions */
u_char tmpDat[ARRAY_PTR_SIZE] = {1, 1};  /* first position for array of variables */

Data *currentData = NULL;     /* ? */
/* Keywords */
char *direct[] = {"IF", "FOR", "WHILE", "END", "ELSE", NULL};
enum directWord {DW_IF, DW_FOR, DW_WHILE, DW_END, DW_ELSE};

Bool translateCode = True;          /* translation/interpretation flag (not used) */

int wordPos = 1;           /* current word position, 0 - for info */
int constPos = 0;          /* current constant position */
int tmpLength = 2;         /* current variable position, 0 - t, 1 - dt*/

int currentElt = -1;       /* element number */
int currentParameter = -1; /* parameter number */
Element *thisElt = NULL;    /* element pointer */

/* add isConst to array */
int setCommand(u_char command, u_char val, int pos);
/* add constant to array */
int setConstant(Number n);
/* get position for variable */
int setTemporary(void);
/* remove variable */
void freeTemporary(int pos);
/* get position for data pointer */
int setDPointer(void);
/* remove data pointer */
void freeDPointer(int pos);
/* prepare memory */
void clearLockal(void);
/* find data as variable */
Bool findVariable(const char* nm, int* command, int *pos);
/* find constant position with the same value */
int  findConstant(Number n);

Bool trans_get_num(Number *n, int *isConst);
Bool trans_num(Number *n, int* isConst);
Bool trans_num1(Number *n, int* isConst);
Bool trans_num2(Number *n, int *isConst);
Bool trans_num3(Number *n, int *isConst);

Bool trans_num_arg(Number *n, int* isConst, int* size);
Bool trans_get_ind(Number *n, int *isConst);
Bool trans_func(Number *n, int *isConst, int fn);

Bool trans_log(Number *n, int* isConst);
Bool trans_log1(Number *n, int* isConst);
Bool trans_log2(Number *n, int* isConst);
Bool trans_log3(Number *n, int *isConst);
Bool trans_log4(Number *n, int *isConst);


Bool trans_set(int elt);
Bool trans_if(int elt);
Bool trans_while(int elt);

Bool trans_eval(int elt);

Bool transCodeExec(Element *e, Number t, Number dt);

u_char compInvert(u_char op);

int check_direct(void);
/* temporary */
void dbg_code(Word* w);
void dbg_const(Number *n);

/* Add command to list */
/* ERROR FUNCTION NUMBER - 1 */
int setCommand(u_char command, u_char val, int pos)
{
    if(wordPos < ARRAY_WORD_SIZE) {
        code[wordPos].command = command;
        code[wordPos].value = val;
        code[wordPos].position = pos;
    }
    else {
        setError(E_MEM_OUT, 11.01, NULL);
        return -1;
    }

    return wordPos ++;
}

/* Add constant if it is uniq */
/* ERROR FUNCTION NUMBER - 2 */
int setConstant(Number n)
{
    int res = findConstant(n);

    if(res != -1) return res;

    if(constPos < ARRAY_CONST_SIZE) {
        numConst[constPos] = n;
    }
    else {
        setError(E_MEM_OUT, 11.02, NULL);
        return -1;
    }

    return constPos ++;
}

/* Get position for variable */
int setTemporary(void)
{
    int i = 2; /* first 2 reserved */

    while(i < ARRAY_VAR_SIZE) {
        if(tmpLock[i])
            i ++;
        else {
            tmpLock[i] = True;
            return i;
    } }

    return -1;
}

int setDPointer(void)
{
    int i = 2; /* first two reserved */

    while(i < ARRAY_PTR_SIZE) {

        if(tmpDat[i])
            i ++;
        else {
            tmpDat[i] = True;
            return i;
    } }

    return -1;
}
/* return variable position */
void freeTemporary(int pos)
{
    if(pos != -1) tmpLock[pos] = False;
}

void freeDPointer(int pos)
{
    if(pos != -1) tmpDat[pos] = False;
}

/* Clear data */
void clearLockal(void)
{
    wordPos = 1;    /* 0 pos for information */
    constPos = 0;

    memset(&tmpDat, 0, sizeof(tmpDat));
    memset(&tmpLock, 0, sizeof(tmpLock));

    currentElt = -1;
}

/* Find data with given name */
Bool findVariable(const char* nm, int *command, int *pos)
{
    int result;

    if((result = parPos(currentElt, nm)) != -1) {
        *command = PTR_PAR;
        *pos = result;
        return True;
    }

    if((result = coutPos(currentElt, nm)) != -1) {
        *command = PTR_COT;
        *pos = result;
        return True;
    }

    if((result = cinPos(currentElt, nm)) != -1) {
        *command = PTR_CIN;
        *pos = result;
        return True;
    }

    return False;
}



/* Looking for same number */
int findConstant(Number n)
{
    int i;

    for(i = 0; i < constPos; i ++) {
        if(nCompare(n, numConst[i], EQ))
            return i;
    }

    return -1;
}

/* Get number */
/* ERROR FUNCTION NUMBER - 3 */
Bool trans_get_num(Number *n, int* isConst)
{
    int posDat, com, posPtr, pos;
    Number *ptr;

    *isConst = True;

    switch(token_type) {
    case T_INT:
        transformInt(n, atol(tokenPtr));
        break;
    case T_FLOAT:
        transformDbl(n, atof(tokenPtr));
        break;
    case T_KEYCHAR:
        if(*tokenPtr != '%') return setError(E_SYNTAX, 11.03, "expected '%'");
        getToken();
        ptr = findConst(tokenPtr, NULL);
        if(ptr)
            *n = *ptr;
        else
            return setError(E_NOT_FOUND, 11.03, tokenPtr);
        break;
    case T_WORD:
        if(stringEqUp(tokenPtr, "TRUE"))
            *n = constOne;
        else if(stringEqUp(tokenPtr, "FALSE"))
            *n = constZero;
        else if(stringEqUp(tokenPtr, "TAU")) {
            /* current time in var[0] */
            setCommand(GET_NUM, IND_VAR, 0);
            *isConst = False;
        }
        else if(stringEqUp(tokenPtr, "DTAU")) {
            /* current time stamp in var[1] */
            setCommand(GET_NUM, IND_VAR, 1);
            *isConst = False;
        }
        else if((pos = getFunc(tokenPtr)) != -1) {
            return trans_func(n, isConst, pos);
        }
        else if(findVariable(tokenPtr, &com, &posDat)) {
            posPtr = setDPointer();
            if(posPtr == -1) return setError(E_MEM_OUT, 11.03, NULL);
            /* get pointer */
            setCommand(com, (u_char) posDat, posPtr);
            /* number to accumulator */
            setCommand(GET_NUM, (u_char) posPtr, 0); /* arra y - ? */
            /* set flag */
            *isConst = False;
            /* free data pointer */
            freeDPointer(posPtr);
        }
        else
            return setError(E_SYMANTIC, 11.03, tokenPtr);
        break;
    default:
        return setError(E_SYNTAX, 11.03, NULL);
    }

    getToken();

    return True;
}

/* ERROR FUNCTION NUMBER - 4 */
Bool trans_num3(Number *n, int *isConst)
{
    if(*tokenPtr == '(') {
        getToken();

        if(!trans_num(n, isConst)) return False;

        if(*tokenPtr != ')') return setError(E_SYNTAX, 11.4, "expected ')'");

        getToken();
    }
    else
        return trans_get_num(n, isConst);

    return True;
}

Bool trans_num2(Number *n, int *isConst)
{
    char op = '\0';

    if(*tokenPtr == '+' || *tokenPtr == '-') {
        op = *tokenPtr;
        getToken();
    }

    if(!trans_num3(n, isConst)) return False;

    if(op == '-') {
        if(*isConst)
            numInvert(n);
        else
            /* invert sign in accumulator */
            setCommand(FNC_INV, 0, 0);
    }

    return True;
}

/* ERROR FUNCTION NUMBER - 12 */
Bool trans_num1(Number *n, int* isConst)
{
    Number tmpVal, numVal = constOne;
    char op, flag = True;
    int vPos = -1, cPos;

    if(!trans_num2(&numVal, isConst)) return False;
    /* prepare data for continue */
    if(*tokenPtr == '*' || *tokenPtr == '/') {
        /* correct flag */
        flag = *isConst;
        /* get variable */
        if((vPos = setTemporary()) == -1)
            return setError(E_MEM_OUT, 11.12, "var");
        /* set 1 - prepare possible data */
        if(*isConst)
            setCommand(TMP_SET, (u_char) vPos, 1);
        else
            /* move from accumulator to memory */
            setCommand(SET_NUM, IND_VAR, vPos);
    }

    while((op = *tokenPtr) == '*' || op == '/') {
        /* eval */
        getToken();
        if(!trans_num2(&tmpVal, isConst)) return False;
        /* add */
        switch(op) {
        case '*':
            if(*isConst)
                numVal = nProd(numVal, tmpVal);
            else
                setCommand(TMP_MLT, (u_char) vPos, 0);
            break;
        case '/':
            if(*isConst)
                numVal = nRat(numVal, tmpVal);
            else
                setCommand(TMP_DIV, (u_char) vPos, 0);
            break;
        }
        /* is still const */
        flag = flag && *isConst;
    }

    /* if get code and operations with product */
    if(!flag) {
        /* get from memory */
        setCommand(GET_NUM, IND_VAR, vPos);
        /* set marker */
        *isConst = flag;
        /* mult to constant (!= 1) */
        if(nCompare(numVal, constOne, NE)) {
            if((cPos = setConstant(numVal)) == -1)
                return setError(E_MEM_OUT, 11.12, "const");
            /* add constant */
            setCommand(FNC_MLT, IND_CONST, cPos);
        }
    }
    /* if not constant don't change number */
    if(*isConst) *n = numVal;

    freeTemporary(vPos);

    return True;
}
/* ERROR FUNCTION NUMBER - 13 */
Bool trans_num(Number *n, int *isConst)
{
    Number tmpVal, numVal = constZero;
    char op, flag = True;
    int vPos = -1, cPos;

    if(!trans_num1(&numVal, isConst)) return False;

    /* prepare data for continue */
    if(*tokenPtr == '+' || *tokenPtr == '-') {
        /* correct flag */
        flag = *isConst;
        /* get variable */
        if((vPos = setTemporary()) == -1)
            return setError(E_MEM_OUT, 11.13, "var");
        /* set 0 - prepare possible data */
        if(*isConst)
            setCommand(TMP_SET, (u_char) vPos, 0);
        else
            /* move from accumulator to memory */
            setCommand(SET_NUM, IND_VAR, vPos);
    }

    /* read and evaluate */
    while((op = *tokenPtr) == '+' || op == '-') {
        /* eval */
        getToken();
        if(!trans_num1(&tmpVal, isConst)) return False;
        /* add */
        switch(op) {
        case '+':
            if(*isConst)
                numVal = nSum(numVal, tmpVal);
            else
                setCommand(TMP_SUM, (u_char) vPos, 0);
            break;
        case '-':
            if(*isConst)
                numVal = nSub(numVal, tmpVal);
            else
                setCommand(TMP_SUB, (u_char) vPos, 0);
            break;
        }
        /* is still const */
        flag = flag && *isConst;
    }
    /* if get code and summation */
    if(!flag) {
        /* get from memory */
        setCommand(GET_NUM, IND_VAR, vPos);
        /* set marker */
        *isConst = flag;
        /* add constant (!= 0) */
        if(nCompare(numVal, constZero, NE)) {
            if((cPos = setConstant(numVal)) == -1)
                return setError(E_MEM_OUT, 11.13, "const");
            /* add constant */
            setCommand(FNC_SUM, IND_CONST, cPos);
        }
    }
    /* if constant set value */
    if(*isConst) *n = numVal;

    freeTemporary(vPos);

    return True;
}

Bool trans_num_arg(Number *n, int* isConst, int *size)
{
    int i, cPos;
    u_char pos = 0, flag = 1;

    do {
        getToken();
        if(!trans_num(n + pos, isConst)) return False;

        if(!*isConst) {
            setCommand(SET_ARG, pos, 0);
            if(flag == 1) {
                for(i = 0; i < (int) pos; i ++) {
                    if((cPos = setConstant(n[i])) == -1) return False;

                    setCommand(CST_ARG, (u_char) i, cPos);
                }
                flag = 0;
            } }
        else if(!flag) {
            if((cPos = setConstant(n[pos])) == -1) return False;

            setCommand(CST_ARG, pos, cPos);
        }
        pos ++;

    } while(*tokenPtr == ',' && pos < MAX_ARG_NUMBER);

    *isConst = flag;
    *size = pos;

    return True;
}

/* ERROR FUNCTION NUMBER - 19 */
Bool trans_func(Number *n, int* isConst, int fn)
{
    Number arg[MAX_ARG_NUMBER];
    int size;

    getToken();
    if(*tokenPtr != '(') return setError(E_SYNTAX, 11.19, "expected '('");
    /* evaluate arguments */
    if(!trans_num_arg(arg, isConst, &size)) return False;

    if(*tokenPtr == ')') {
        if(size == (int) getFuncArg(fn)) {
            if(*isConst) {
                if(!evalFunc(fn, arg, n)) return False;
            }
            else {
                setCommand(FNC_ARG, 0, fn);
            }
        }
        else
            return setError(E_WRONG_ARG, 11.19, "wrong number");
    }
    else
        return setError(E_SYNTAX, 11.19, "expected ')'");
    getToken();

    return True;
}

/* ERROR FUNCTION NUMBER - 5 */
Bool trans_get_ind(Number *n, int *isConst)
{
    Number arg[MAX_ARG_NUMBER];
    Data *datPtr = currentData;
    int sz;
    int indx = 0;

    if(!trans_num_arg(arg, isConst, &sz)) return False;

    /* what if sz == 0 ? */

    if(*tokenPtr != ']') return setError(E_SYNTAX, 11.5, "index");

    if(!*isConst) {
        if(!parseEltIndex(datPtr, arg, sz, &indx)) return False;

        transformInt(n, (int) indx);
    }

    return True;
}

/* Logical OR */
/* ERROR FUNCTION NUMBER - 15 */
Bool trans_log(Number *n, int* isConst)
{
    Number numVal = constZero, tmpVal;
    int vPos = -1, flag = True, cPos;

    if(!trans_log1(&numVal, isConst)) return False;

    if(stringEqUp(tokenPtr, "OR")) {
        flag = *isConst;

        if((vPos = setTemporary()) == -1)
            return setError(E_MEM_OUT, 11.15, "var");

        if(*isConst)
            setCommand(TMP_SET, (u_char) vPos, 0);
        else
            /* move from accumulator to memory */
            setCommand(SET_NUM, IND_VAR, vPos);
    }

    while(stringEqUp(tokenPtr, "OR")) {

        getToken();

        if(!trans_log1(&tmpVal, isConst)) return False;
        /* OR */
        if(*isConst)
            numVal = nOr(numVal, tmpVal);
        else
            setCommand(TMP_ORL, (u_char) vPos, 0);

        flag = flag && *isConst;
    }

    if(!flag) {
        /* get from memory */
        setCommand(GET_NUM, IND_VAR, vPos);
        /* set marker */
        *isConst = flag;
        /* add constant (!= 0) */
        if(nCompare(numVal, constZero, NE)) {
            if((cPos = setConstant(numVal)) == -1)
                return setError(E_MEM_OUT, 11.15, "const");
            /* add constant */
            setCommand(FNC_ORL, IND_CONST, cPos);
        }
    }
    /* if constant set value */
    if(*isConst) *n = numVal;

    freeTemporary(vPos);

    return True;
}

/* Logical AND */
/* ERROR FUNCTION NUMBER - 16 */
Bool trans_log1(Number *n, int* isConst)
{
    Number numVal = constOne, tmpVal;
    int vPos = -1, flag = True, cPos;

    if(!trans_log2(&numVal, isConst)) return False;

    if(stringEqUp(tokenPtr, "AND")) {
        flag = *isConst;

        if((vPos = setTemporary()) == -1)
            return setError(E_MEM_OUT, 11.16, "var");

        if(*isConst)
            setCommand(TMP_SET, (u_char) vPos, 1);
        else
            /* move from accumulator to memory */
            setCommand(SET_NUM, IND_VAR, vPos);
    }

    while(stringEqUp(tokenPtr, "AND")) {

        getToken();

        if(!trans_log2(&tmpVal, isConst)) return False;
        /* OR */
        if(*isConst)
            numVal = nAnd(numVal, tmpVal);
        else
            setCommand(TMP_AND, (u_char) vPos, 0);

        flag = flag && *isConst;
    }

    if(!flag) {
        /* get from memory */
        setCommand(GET_NUM, IND_VAR, vPos);
        /* set marker */
        *isConst = flag;
        /* add constant (!= 1) */
        if(nCompare(numVal, constOne, NE)) {
            if((cPos = setConstant(numVal)) == -1)
                return setError(E_MEM_OUT, 11.16, "const");
            /* add constant */
            setCommand(FNC_AND, IND_CONST, cPos);
        }
    }
    /* if constant set value */
    if(*isConst) *n = numVal;

    freeTemporary(vPos);

    return True;
}

/* Logical NOT */
Bool trans_log2(Number *n, int* isConst)
{
    u_char flNot = False;

    if(stringEqUp(tokenPtr, "NOT")) {
        flNot = True;
        getToken();
    }

    if(!trans_log3(n, isConst)) return False;

    if(flNot) {
        if(*isConst)
            *n = nNot(*n);
        else
            setCommand(FNC_NOT, 0, 0);
    }

    return True;
}

/* Complex logical expressions */
/* ERROR FUNCTION NUMBER - 6 */
Bool trans_log3(Number *n, int* isConst)
{
    if(*tokenPtr == '(') {
        getToken();

        if(!trans_log(n, isConst)) return False;

        if(*tokenPtr != ')') return setError(E_SYNTAX, 11.06, "expected )");

        getToken();
    }
    else
        return trans_log4(n, isConst);

    return True;
}

/* evaluate compare */
/* ERROR FUNCTION NUMBER - 17 */
Bool trans_log4(Number *n, int *isConst)
{
    Number tmpVal1, tmpVal2;
    u_char op, flag = True, command, comp;
    int vPos = -1, cPos = -1;

    if(!trans_num(&tmpVal1, isConst)) return False;

    /* get compare */
    if((op = token_spec) >= S_NE && op <= S_GT) {
        /* prepare memory */
        flag = *isConst;

        if((vPos = setTemporary()) == -1)
            return setError(E_MEM_OUT, 11.17, "var");
        /* if not const, move to variables */
        if(!flag) {
            setCommand(SET_NUM, IND_VAR, vPos);
        }

        /* get second */
        getToken();
        if(!trans_num(&tmpVal2, isConst)) return False;

        /* get command */
        switch(op) {
            case S_NE: command = CMP_NEQ; comp = NE; break;
            case S_LT: command = CMP_LTE; comp = LT; break;
            case S_LE: command = CMP_LEQ; comp = LE; break;
            case S_EQ: command = CMP_EQL; comp = EQ; break;
            case S_GE: command = CMP_GEQ; comp = GE; break;
            case S_GT: command = CMP_GTE; comp = GT; break;
            default:
                return setError(E_WTF, 11.17, NULL);
        }

        if(flag && *isConst)
            /* both are constants */
            *n = nCompare(tmpVal1, tmpVal2, comp) ? constOne : constZero;
        else if(flag && !*isConst) {
            /* first constant, second not */
            if((cPos = setConstant(tmpVal1)) == -1)
                return setError(E_MEM_OUT, 11.17, "const");
            /* invert command, because in accumulator second val */
            command = compInvert(command);
            /* add */
            setCommand(command, IND_CONST, cPos);
        }
        else if(!flag && *isConst) {
            /* second constant, first not */
            if((cPos = setConstant(tmpVal2)) == -1)
                return setError(E_MEM_OUT, 11.17, "const");
            /* get from memory to accumulator */
            setCommand(GET_NUM, IND_VAR, vPos);
            setCommand(command, IND_CONST, cPos);
        }
        else {
            /* both are not constant, in accumulator second val */
            command = compInvert(command);
            setCommand(command, IND_VAR, vPos);
        }

        *isConst = flag && *isConst;

        freeTemporary(vPos);
    }
    else {
        if(*isConst) *n = tmpVal1;
    }

    return True;
}

/* set value */
/* ERROR FUNCTION NUMBER - 7 */
Bool trans_set(int elt)
{
    int typ = -1, pos = -1, cnst = 1, cPos, dPos;
    Number n;
    Data *dPtr = NULL;
    Bool numb = False;
    /* comment or empty string */
    if(token_type == T_ENDLINE) return trans_eval(elt);
#if 0
    /* check if directives */
    if(stringEqUp(tokenPtr, "BREAK")) {
        setCommand(BRN_BRK, 0, 0);
        return trans_eval(elt);
    }
    else if(stringEqUp(tokenPtr, "CONTINUE")) {
        setCommand(BRN_CON, 0, 0);
        return trans_eval();
    }
#endif
    /* get or set variable */
    if(!findVariable(tokenPtr, &typ, &pos)) {
        pos = regParameter(elt, tokenPtr, NULL, dataInt(0), False);
        if(pos == -1)
            return False;
        else
            typ = PTR_PAR;
    }

    getToken();
#if 0
    if(*tokenPtr == '[') {
        /* get index */
    }
#endif

    if(*tokenPtr == '=') {
        /* public parameters are not alowed to modify */
        if(isParPablic(elt, pos))
            return setError(E_SYMANTIC, 11.07, "const parameter");
        /* set pointer to data for parameter */
        if(typ == PTR_PAR)
            dPtr = getParameterData(elt, pos);
        /* just for numbers */
        getToken();
        switch(token_type) {
        /* set string type */
        case T_STRING:
            if(dPtr) {
                stringSet(dPtr, tokenPtr);
            }
            /* else have to write */
            break;
        /* bool */
        default:
            if(!trans_log(&n, &cnst)) return False;
            numb = True;
            break;
        }
        /* get number value */
        if(numb) {
            /* if constant and parameter - copy */
            if(dPtr && cnst) {
                dPtr->type = tNumber;
                dPtr->numVal = n;
            }
            else {
                /* set pointer to data */
                if((dPos = setDPointer()) == -1)
                    return setError(E_MEM_OUT, 11.07, NULL);
                setCommand((u_char) typ, pos, dPos);

                /* copy constant to accumulator */
                if(cnst) {
                    if((cPos = setConstant(n)) == -1)
                        return setError(E_MEM_OUT, 11.07, NULL);
                    setCommand(GET_NUM, IND_CONST, cPos);
                }
                /* set from accumulator to variable */
                setCommand(SET_NUM, (u_char) dPos, 0);

                freeDPointer(dPos);
            }
        }
        else
            getToken();
        /* next token */
    }

    if(token_type == T_END)
        return True;
    else if(*tokenPtr == ',')
        return trans_set(elt);

    return trans_eval(elt);
}

/* Translate script to code */
/* ERROR FUNCTION NUMBER - 8 */
Bool trans_code(int elt)
{
    Bool res = True;

    clearLockal();

    currentElt = elt;

    res = trans_eval(elt);

    if(res) {
        if(setCommand(HLT_EVL, 0, 0) == -1)
            return setError(E_MEM_OUT, 11.08, "word");
    }

    return res;
}

/* ERROR FUNCTION NUMBER - 9 */
Bool trans_if(int elt)
{
    int isConst = 0, corrPos1, corrPos2, cPos;
    Number n;
    /* evaluate equation */
    getToken();
    if(!trans_log(&n, &isConst)) return False;
    /* if constant - put to accumulator */
    if(isConst) {
        if((cPos = setConstant(n)) == -1)
            return setError(E_MEM_OUT, 11.09, "const");
        setCommand(GET_NUM, IND_CONST, cPos);
    }
    /* comments or white spaces */
    while(token_type == T_ENDLINE) getToken();
    /* then block */
    if(!stringEqUp(tokenPtr, "THEN"))
        return setError(E_SYNTAX, 11.09, "expected 'then'");

    /* get current command for go to else block */
    corrPos1 = wordPos ++;
    /* evaluate next equations */
    if(!trans_eval(elt)) return False;

    if(stringEqUp(tokenPtr, "ELSE")) {
        /* get current word for go to end of block */
        corrPos2 = wordPos ++;
        /* correct first command */
        code[corrPos1].command = BRN_ZER;
        code[corrPos1].value = 0;
        code[corrPos1].position = wordPos;
        /* evaluate second part */
        if(!trans_eval(elt)) return False;
        /* correct second command */
        code[corrPos2].command = BRN_POS;
        code[corrPos2].value = 0;
        code[corrPos2].position = wordPos;
    }
    else {
        /* first command correct */
        code[corrPos1].command = BRN_ZER;
        code[corrPos1].value = 0;
        code[corrPos1].position = wordPos;
    }

    return trans_eval(elt);
}

/* ERROR FUNCTION NUMBER - 10 */
Bool trans_while(int elt)
{
    int isConst = 0, corrPos1, corrPos2, cPos;
    Number n;
    /* remember current position */
    corrPos1 = wordPos;
    /* evaluate equation */
    getToken();
    if(!trans_log(&n, &isConst)) return False;
    /* set constant to accumulator */
    if(isConst) {
        if((cPos = setConstant(n)) == -1)
            return setError(E_MEM_OUT, 11.10, "const");
        setCommand(GET_NUM, IND_CONST, cPos);
    }
    /* remember begin of block */
    corrPos2 = wordPos++;
    /* evaluate */
    if(!trans_eval(elt)) return False;

    if(!stringEqUp(tokenPtr, "END"))
        return setError(E_SYNTAX, 11.10, "expected 'end'");
    /* go to begin of circle */
    setCommand(BRN_POS, 0, corrPos1);
    /* set exit from block */
    code[corrPos2].command = BRN_ZER;
    code[corrPos2].value = 0;
    code[corrPos2].position = wordPos;

    return trans_eval(elt);
}

/* print code */
void dbg_code(Word* w)
{
    int i;
    FILE* f;

    f = fopen("code.tt", "w");
    fprintf(f,"_Code:\n");
    for(i = 0; i < wordPos; i ++) {
        fprintf(f, "%d\t%d\t%d\n", (int) w[i].command, (int) w[i].value, w[i].position);
    }
    fclose(f);
}

void dbg_const(Number *n)
{
    int i;
    FILE* f;

    f = fopen("code.tt", "a");

    fprintf(f, "_Constants:\n");
    for(i = 0; i < constPos; i ++) {
        fprintf(f, "%d\t%s\n", i, numAsText(n[i]));
    }

    fclose(f);
}
/* ERROR FUNCTION NUMBER - 14 */
Bool transWriteCode(int elt)
{
    Data *ptr;
    Word *w;
    int i;

    /* write code */
    ptr = getParameterData(elt, 0);
    if(!ptr) return setError(E_WRONG_ARG, 11.14, NULL);

    w = (Word*) malloc(sizeof(Word)*wordPos);

    for(i = 0; i < wordPos; i ++) {
        w[i] = code[i];
    }
    /* add info */
    w[0].command = INF_WRD;
    w[0].position = wordPos;
    /* set to ticket */
    ptr->freeVal.ptr = (void*) w;
/* debug
    dbg_code(w);
*/
    /* write constants */
    ptr = getParameterData(elt, 1);
    if(!dataInitArray(ptr, constPos)) return False;

    for(i = 0; i < constPos; i ++) {
        ptr->arrVal.numb[i] = numConst[i];
    }
/* debug
    dbg_const(numConst);
*/
    return True;
}

/* code execute */
/* ERROR FUNCTION NUMBER - 11 */
Bool transCodeExec(Element *e, Number t, Number dt)
{

    int pos = 0, i = 0;
    Bool branch = False;
    /* code */
    Word *base = (Word*) _DP(e,0)->freeVal.ptr, *w;
    /* variables */
    Number var[ARRAY_VAR_SIZE];
    /* accumulator */
    Number accumulator = constZero, swp;
    /* argumets/indexes */
    Number arg[MAX_ARG_NUMBER];
    /* pointers to data: input, output, parameter */
    Data *dat[ARRAY_PTR_SIZE] = {NULL};
    Data dVars = dataArrayNull();
    /* prepare temporary variables */
    dVars.arrVal.numb = var;
    dVars.arrVal.length = ARRAY_VAR_SIZE;
    /* copy time */
    var[0] = t; var[1] = dt;
    /* set pointers to data */
    dat[0] = &dVars;    /* temporary variables */
    dat[1] = _DP(e, 1); /* constants */

    w = base;

    while(w->command != HLT_EVL) {

        switch(w->command) {
        /* input data pointer */
        case PTR_CIN:
            /* PTR_..., data_position, array_position */
            dat[w->position] = _DC(e, C_IN, w->value);
            break;
        /* output data pointer */
        case PTR_COT:
            dat[w->position] = _DC(e, C_OUT, w->value);
            break;
        /* parameter data pointer */
        case PTR_PAR:
            dat[w->position] = _DP(e, w->value);
            break;
        /* write number from accumulator to data */
        case SET_NUM:
            /* SET_NUM, data_poition, index */
            *N(dat[w->value], w->position) = accumulator;
            break;
        /* write number from accumulator to argument */
        case SET_ARG:
            /* SET_ARG, position, 0 */
            if(w->value < MAX_ARG_NUMBER)
                arg[w->value] = accumulator;
            else
                return setError(E_WRONG_INDEX, 11.11, "SET_ARG");
            break;
        /* set argument from constants */
        case CST_ARG:
            /* CST_ARG, arg_pos, cst_pos */
            if(w->value < MAX_ARG_NUMBER)
                arg[w->value] = *N(_DP(e,1), w->position);
            else
                return setError(E_WRONG_INDEX, 11.11, "CST_ARG");
            break;
        /* read number from data to accumulator */
        case GET_NUM:
            /* GET_NUM, array_position, [array index] */
            accumulator = *N(dat[w->value], w->position);
            break;
        /* add accumulator to number, write to accumulator */
        case FNC_SUM:
            /* FNC_SUM, data_pos, index */
            accumulator = nSum(accumulator, *N(dat[w->value],w->position));
            break;
        /* substract accumulator from number, write to position */
        case FNC_SUB:
            /* FNC_SUB, data_pos, index */
            accumulator = nSub(accumulator, *N(dat[w->value],w->position));
            break;
        /* multiply accumulator to number, write to position */
        case FNC_MLT:
            /* FNC_MLT, data_pos, index */
            accumulator = nProd(accumulator, *N(dat[w->value],w->position));
            break;
        /* devide number to accumulator, write to position */
        case FNC_DIV:
            /* FNC_DIV, data, index */
            accumulator = nRat(accumulator, *N(dat[w->value],w->position));
            break;
        case TMP_SUM:
            var[w->value] = nSum(accumulator, var[w->value]);
            break;
        case TMP_SUB:
            var[w->value] = nSub(var[w->value], accumulator);
            break;
        case TMP_MLT:
            var[w->value] = nProd(var[w->value], accumulator);
            break;
        case TMP_DIV:
            var[w->value] = nRat(var[w->value], accumulator);
            break;
        /* invert accumulator sign */
        case FNC_INV:
            /* FNC_INV, 0, 0 */
            numInvert(&accumulator);
            break;
        /* write to accumulator logical OR */
        case FNC_ORL:
            /* FNC_ORL, data, index */
            accumulator = nOr(accumulator, *N(dat[w->value], w->position));
            break;
        /* write to accumulator logical AND */
        case FNC_AND:
            /* FNC_AND, data, index */
            accumulator = nAnd(accumulator, *N(dat[w->value], w->position));
            break;
        /* write to accumulator logical NOT */
        case FNC_NOT:
            /* FNC_NOT, 0, 0 */
            accumulator = nNot(accumulator);
            break;
        /* write to variables logical OR */
        case TMP_ORL:
            /* TMP_ORL, index, 0 */
            var[w->value] = nOr(var[w->value], accumulator);
            break;
        /* write to variables logical AND */
        case TMP_AND:
            /* TMP_AND, index, 0 */
            var[w->value] = nAnd(var[w->value], accumulator);
            break;
        /* swap number between accumulator and cell */
        case ACC_SWP:
            /* ACC_SWP, data, index */
            swp = *N(dat[w->value], w->position);
            *N(dat[w->value], w->position) = accumulator;
            accumulator = swp;
            break;
        /* write memporary memory number */
        case TMP_SET:
            /* TMP_SET, index, number */
            var[w->value] = intToNumber(w->position);
            break;
        /* get accumulator == value */
        case CMP_EQL:
            /* CMP_EQL, data, position */
            accumulator = nCompare(accumulator, *N(dat[w->value], w->position), EQ) ?
                            constOne : constZero;
            break;
         /* get accumulator != value */
        case CMP_NEQ:
            /* CMP_NEQ, data, position */
            accumulator = nCompare(accumulator, *N(dat[w->value], w->position), NE) ?
                            constOne : constZero;
            break;
         /* get accumulator < value */
        case CMP_LTE:
            /* CMP_LTE, data, position */
            accumulator = nCompare(accumulator, *N(dat[w->value], w->position), LT) ?
                            constOne : constZero;
            break;
         /* get accumulator <= value */
        case CMP_LEQ:
            /* CMP_LEQ, data, position */
            accumulator = nCompare(accumulator, *N(dat[w->value], w->position), LE) ?
                            constOne : constZero;
            break;
         /* get accumulator >= value */
        case CMP_GEQ:
            /* CMP_GEQ, data, position */
            accumulator = nCompare(accumulator, *N(dat[w->value], w->position), GE) ?
                            constOne : constZero;
            break;
         /* get accumulator > value */
        case CMP_GTE:
            /* CMP_EQL, data, position */
            accumulator = nCompare(accumulator, *N(dat[w->value], w->position), GT) ?
                            constOne : constZero;
            break;
        /* evaluate function with given arguments */
        case FNC_ARG:
            /* FNC_ARG, 0, function */
            if(!evalFunc(w->position, arg, &accumulator)) return False;
            break;
        /* branch to position */
        case BRN_POS:
            /* BRN_POS, 0, position */
            pos = w->position;
            branch = True;
            break;
        /* branch if accumulator 0 */
        case BRN_ZER:
            if(isZero(accumulator)) {
                pos = w->position;
                branch = True;
            }
            break;
        /* break with error */
        case HLT_ERR:
            return False;
        /* info about code */
        case INF_WRD:
            /* INF_WRD, 0, length */
            break;
        default:
            return setError(E_WTF, 11.11, "error code");
        }

        if(branch) {
            w = base + pos;
            branch = False;
        }
        else
            w ++;

        if(++i > 10000) return setError(E_WTF, 11.11, "circle");
    }

    return True;
}

u_char compInvert(u_char op)
{
    switch(op) {
        case CMP_EQL: return CMP_NEQ;
        case CMP_NEQ: return CMP_EQL;
        case CMP_GEQ: return CMP_LTE;
        case CMP_LTE: return CMP_GEQ;
        case CMP_GTE: return CMP_LEQ;
        case CMP_LEQ: return CMP_GTE;
    }

    return HLT_ERR; /* error */
}

int check_direct(void)
{
    int i = 0;

    while(direct[i]) {
        if(stringEqUp(tokenPtr, direct[i]))
            return i;
        i ++;
    }

    return -1;
}

/* ERROR FUNCTION NUMBER - 18 */
Bool trans_eval(int elt)
{
    Bool res = True;

    getToken();

    if(wordPos >= ARRAY_WORD_SIZE)
        return setError(E_MEM_OUT, 11.18, "word");

    if(token_type == T_END)
        return True;
    else if(token_type == T_ENDLINE)
        return trans_eval(elt);

    switch(check_direct()) {
    case DW_IF:
        res = trans_if(elt);
        break;
    case DW_WHILE:
        res = trans_while(elt);
        break;
    case DW_ELSE: case DW_END:   break;
    default:
        res = trans_set(elt);
    }

    return res;
}
