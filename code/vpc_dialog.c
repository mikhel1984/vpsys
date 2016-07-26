/** @file vpc_dialog.c
~~~ Code Dialog ~~~
Sintax analize and code execution */

/* FILE ERROR: 9 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>

#include "vpc_dialog.h"
#include "vp_work.h"
#include "vp_construct.h"
#include "vpb_memory.h"
#include "vpb_element.h"
#include "vpb_ticket.h"
#include "vpb_connectors.h"
#include "vpc_parser.h"
#include "vpb_data.h"
#include "vpb_error.h"
#include "vpe_user.h"
#include "vpe_block.h"
#include "vpb_lang.h"
#include "vpb_vars.h"

#define COMM_SIZE 512
#define VAR_LIST -10
#define MAX_IN_LINE 5

#define OP_NAME "eval"
#define BLK_SHFT "  "

/* point to jump */
jmp_buf execErr;
/* pointer to graph working with */
Graph *thisGraph = NULL;
/* pointer to elements for delete */
Chain *delChain = NULL;
Graph *delGraph = NULL;
/* flag for new code file */
u_char flNew = False;

/* array for command string */
char nextCommand[COMM_SIZE];
/* special words */
char *keyWords[] = {"END", "RUN", "CHAIN", "HELP",
                    "FREE", "DEL", "LOAD",  "CONST", NULL};
/* ? */
int nLine = 0;
/* print operation results */
Bool messagePrint = True;

/* list of main syntax errors */
char* dlgErr[] =
{/*0*/  "expected '?'", /*1*/   "expected type", /*2*/  "expected string",
 /*3*/  "expected name", /*4*/  "expected end", /*5*/   "expected parameter",
 /*6*/  "expected ']'", /*7*/   "expected '='", /*8*/   "expected ' ",
 /*9*/  "expected '->'", /*10*/ "expected ','", /*11*/  "expected \" ",
 /*12*/ "constant",     /*13*/  "expected ':='", /*14*/ "expected '['",
 /*15*/ "expected '%'", /*16*/  "expected ')'", /*17*/  "expected '('"};



Bool dlg_graph(void);
Bool dlg_type(void);
Bool dlg_element(void);
int dlg_keyword(int f);
Bool dlg_elt_settings(Element *e);

Element* dlg_elt_find_block(Element* head);

Bool dlg_elt_connect(Element *e);
Bool dlg_elt_linking(Element *from);

//Bool dlg_eval_math(void);
Bool dlg_eval_line(void);
Element* dlg_elt_insert(void);

Bool dlg_key_free(void);

Bool dlg_key_del(void);

Bool dlg_key_load(void);

Element* dlg_elt_find(void);

int isKeyword(void);

Bool dlg_key_run(void);
Bool dlg_key_chain(void);

int isEmpty(char* str);

Bool readNext(char *src);

void dlg_print(char* u8_text, char* name);

Bool dlg_elt_copy(void);

Bool dlg_block(Graph* g);

int dlg_var_left(Number **pos);

Bool dlg_var_rigth(Data* d, Number *nPtr, u_char changeType);

Bool dlg_var_rem(void (* f)(int ));

Connector* dlg_out_find(Element *elt);
Connector* dlg_in_find(Element **elt);

void help_connection(Element* e, char* shift);
void help_block(Element *b);


/* check evaluation function */
int dlg_exec_code(char *c)
{
    /* ERROR: 10 */
    /* return 0 if OK, -1 if error and 1 if exit */
    int res = 0;
    char go = 1, key = -1;
    extern Bool execMode;

    setProgramm(c);
    nLine = 0;

    if(setjmp(execErr)) {
        res = -1; go = 0;
    }

    while(go && res != -1) {

        switch(getToken()) {
        case T_ENDLINE:
            break;
        case T_WORD:
              /* function */
            if(getFunc(tokenPtr) != -1) {
                goto calc;
            } /* command */
            else if((key = (char) isKeyword()) != -1) {
                res = dlg_keyword(key) - 1;
            } /* element */
            else
                res = dlg_element() - 1;
            break;
        case T_KEYCHAR:
            if(*tokenPtr == '@')
                res = dlg_graph() - 1;
            else if(*tokenPtr == '&')
                res = dlg_type() - 1;
            else if(*tokenPtr == '%')
                goto calc;
            break;
        case T_END: go = 0; break;
        default:
calc:
            res = dlg_eval_line() - 1;
            break;
        }
        nLine ++;
    }

    if(res == -1) {
        if(execMode) printf("\nAt line %d\n", nLine);
        printError();
    }

    return res;
}


/* New graph */
Bool dlg_graph(void)
{
    /* ERROR: 01 */
    Graph *ptr = NULL;
    u_char coma = 0;
    char tmpName[MAXNAME];

    getToken();
    /* @ ? - current graph */
    if(*tokenPtr == '?') {
        printf("@%s", thisGraph->nm->name);
        return dlg_end(9.01);
    }
    /* @...? - list of all graphs */
    if(token_spec == S_RANGE) {
        getToken();
        if(*tokenPtr == '?') {
            dlg_print(lngText("i_gr", "Graphs:"),NULL);
            /* print list */
            while((ptr = memNextGraph(ptr)) != NULL) {
                if(coma)
                    printf(", ");
                else
                    coma = 1;
                printf("%s", ptr->nm->name);
        } }
        else
            return setError(E_SYNTAX, 9.01, dlgErr[0]);
        /* end line */
        return dlg_end(9.01);
    }

    if(token_type != T_WORD)
        return setError(E_SYNTAX, 9.01, dlgErr[1]);
    /* find graph */
    ptr = findGraph(tokenPtr);
    /* copy graph name to create new */
    if(!ptr) strncpy(tmpName, tokenPtr, MAXNAME-1);

    getToken();
    /* set or create */
    if(ENDLINE) {
        if(ptr) {
            /* set graph as current */
            thisGraph = ptr;
            dlg_print(lngText("i_setgr", "Set graph"), ptr->nm->name);
        }
        else {
            /* create new graph */
            thisGraph = newGraph(tmpName, NULL);
            dlg_print(lngText("i_crgr", "Created graph"), thisGraph->nm->name);
    } }
    /* help */
    else if(*tokenPtr == '?') {
        if(ptr)
            help_graph(ptr);
        else
            dlg_print(lngText("i_nogr", "Graph not found."),NULL);

        return dlg_end(9.01);
    }
    /* add description */
    else if(*tokenPtr == ':') {
        if(getToken() != T_STRING)
            return setError(E_SYNTAX, 9.01, dlgErr[2]);

        if(ptr) { /* add */
            ptr->u8_descript = memDescript(tokenPtr);
            if(ptr->u8_descript)
                dlg_print(lngText("i_done", "Done."), NULL);
        }
        else { /* create new */
            thisGraph = newGraph(tmpName, tokenPtr);
            dlg_print(lngText("i_crgr", "Created graph"), thisGraph->nm->name);

        }
        return dlg_end(9.01);
    }
    /* create block */
    else if(token_type == T_WORD) {
        if(!ptr) {
            dlg_print(lngText("i_nogr", "Graph not found."),NULL);
            return False;
        }
        return dlg_block(ptr);
    }
    else
        return setError(E_SYNTAX, 9.01, tokenPtr);

    return True;
}

/* New type */
Bool dlg_type(void)
{
    /* ERROR: 02 */
    Ticket* tic = NULL;
    int i = 0;
    char tmpName[MAXNAME];
    LastState ls;

    getToken();
    /* & ... ? - list of types */
    if(token_spec == S_RANGE) {
        getToken();
        /* print list */
        if(*tokenPtr == '?') {
            dlg_print(lngText("i_typ","Types: "),NULL);

            while((tic = memGetTicket(i)) != NULL) {
                if(i++) printf(", ");
                printf("%s", tic->type->name);
        } }
        else
            return setError(E_SYNTAX, 9.02, dlgErr[0]);

        return dlg_end(9.02);
    }
    /* create copy */
    if(*tokenPtr == '&')
        return dlg_elt_copy();

    if(token_type != T_WORD)
        return setError(E_SYNTAX, 9.02, dlgErr[1]);
    /* save type name */
    strncpy(tmpName, tokenPtr, MAXNAME - 1);
    /* set point to reread code in future */
    ls = setPoint();
    /* check existing, add */
    if(findTicket(tmpName) == -1) {
        /* register new */
        if(!regUserType(tmpName)) return False;
        dlg_print(lngText("i_addtp", "Add type"), tmpName);
        /* back to this script */
        goToPoint(&ls);
    }
    getToken();
    /* help */
    if(*tokenPtr == '?') {
        help_type(tmpName);

        return dlg_end(9.02);
    }
    /* create element */
    else if(token_type == T_WORD) {
        /* point is at the begining of current element name */
        /* create, add to graph */
        if(!eltToGraph(thisGraph, eltCreate(tmpName, tokenPtr)))
                return False;
        /* message */
        dlg_print(lngText("i_addelt", "Add element"), tokenPtr);
        getToken();
        /* many elements */
        while(*tokenPtr == ',')  {
            ls = setPoint();
            getToken();
            /* break lines */
            if(token_type == T_ENDLINE) {
                ls = setPoint();
                getToken();
            }
            /* name */
            if(token_type != T_WORD)
                return setError(E_SYNTAX, 9.02, dlgErr[3]);
            /* create elt, add to graph */
            if(!eltToGraph(thisGraph, eltCreate(tmpName, tokenPtr)))
                return False;
            /* message */
            dlg_print(lngText("i_addelt", "Add element"), tokenPtr);
            getToken();
        }
        /* reread code */
        goToPoint(&ls);
        getToken();
        /* settings */
        return dlg_element();
    }
    /* check or load type */
    else if(ENDLINE) {
        dlg_print(lngText("i_done", "Done."), NULL);
        return True;
    }
    else
        setError(E_SYNTAX, 9.02, tokenPtr);

    return False;
}

/* Find element, begin to work */
Bool dlg_element(void)
{
    /* ERROR: 03 */
    Element *elt;
    /* find element with given token (in memory) */
    elt = dlg_elt_find(); /* <- get token here */
    if(!elt) return setError(E_NOT_FOUND, 9.03, tokenPtr);

    /* help */
    if(*tokenPtr == '?') {
        help_element(elt);
        return dlg_end(9.03);
    }
    /* set parameters or description */
    if(*tokenPtr == ':') {
        return dlg_elt_settings(elt);
    }
    /* connection */
    if(*tokenPtr == '\'' || token_spec == S_LINK) {
        return dlg_elt_connect(elt);
    }
    if(ENDLINE) {
        return True;
    }

    return setError(E_SYNTAX, 9.03, tokenPtr);
}

/* Find element and graph */
Element *dlg_elt_find(void)
{
    /* ERROR: 04 */
    Graph *graphPtr = thisGraph;
    Chain* chainPtr;
    char locName[MAXNAME];
    strncpy(locName, tokenPtr, MAXNAME-1);

    getToken();
    /* element in block */
    if(*tokenPtr == '.') {

        for(chainPtr = thisGraph->base; chainPtr; chainPtr = chainPtr->next) {
            if(stringEqUp(chainPtr->elt->nm->name, locName))
                return dlg_elt_find_block(chainPtr->elt);
        }
        return NULL;
    }
    /* choose graph */
    if(*tokenPtr == '@') {

        getToken();

        if((graphPtr = findGraph(tokenPtr)) != NULL)
            getToken();
        else
            return NULL;
    }

    /* for delete*/
    delGraph = graphPtr;
    delChain = NULL;
    /* find element */
    for(chainPtr = graphPtr->base; chainPtr; chainPtr = chainPtr->next) {
        if(stringEqUp(locName, chainPtr->elt->nm->name))
            return chainPtr->elt;
        /* for deletion */
        delChain = chainPtr;
    }

    return NULL;
}

Element* dlg_elt_find_block(Element* head)
{
    Chain* cPtr;
    Element* ePtr = NULL;

    if(head->tick != blockTicket)
        return NULL;

    getToken();

    for(cPtr = blkGraph(head)->base; cPtr; cPtr = cPtr->next) {
        if(stringEqUp(tokenPtr, cPtr->elt->nm->name)) {
            ePtr = cPtr->elt;
            break;
    } }

    if(!ePtr) return NULL;

    getToken();
    if(*tokenPtr == '.')
        return dlg_elt_find_block(ePtr);

    return ePtr;
}

/* Check line end */
Bool dlg_end(float f)
{
    getToken();

    return ENDLINE ? True : setError(E_SYNTAX, f, dlgErr[4]);
}

/* Set parameters */
Bool dlg_elt_settings(Element *e)
{
    /* ERROR: 06 */
    Data *dat;
    Number *nPtr = NULL;
    u_char indSize = 0, flRef = 0;
    Number indLst[MAX_ARG_NUMBER]; /* fill 0 - ? */
    int var = -1;
    char op = '\0';

    /* element's description */
    if(getToken() == T_STRING) {
        e->u8_descript = memDescript(tokenPtr);
        if(!e->u8_descript)
            return setError(E_NO_MEMORY, 9.06, NULL);

        return dlg_end(9.06);
    }
    /* multiline enter */
    else if(token_type == T_ENDLINE) {
        nLine ++; /* ? */
        return dlg_elt_settings(e);
    }
    /* set reference */
    if(*tokenPtr == '%') {
        if(getToken() != T_WORD) return setError(E_SYNTAX, 9.06, dlgErr[3]);
        /* find/add */
        if((var = variableFind(tokenPtr)) == -1) {
            if((var = variableAdd(tokenPtr)) == -1) return False;
        }

        getToken();
        flRef = (token_spec == S_REF);
        if(token_spec != S_REF && *tokenPtr != '=')
            return setError(E_SYNTAX, 9.06, NULL);

        getToken();
    }

    /* parameter name */
    if(token_type != T_WORD)
        return setError(E_SYNTAX, 9.06, dlgErr[5]);
    /* get pointer to data */
    dat = eltParamData(e, tokenPtr);
    if(!dat) return setError(E_NOT_FOUND, 9.06, tokenPtr);
    /* set reference */
    if(var != -1) {
        variableSet(var, dat, flRef);
        getToken();
        goto fEnd;
    }

    getToken();

    /* array index or new size */
    if(*tokenPtr == '[' || *tokenPtr == '(') {
        op = *tokenPtr;
        /* read index list */
        if(!eval_args(indLst, &indSize, MAX_ARG_NUMBER))
            return False;
        /* close arguments */
        if(op == '[' && *tokenPtr != ']')
            return setError(E_SYNTAX, 9.06, dlgErr[6]);
        if(op == '(' && *tokenPtr != ')')
            return setError(E_SYNTAX, 9.06, dlgErr[16]);
        getToken();
    }
    /* resizing */
    if(op == '[') {
        if(!resizeToArray(dat, indLst, indSize)) return False;
        goto fEnd;
    }
    /* get number */
    if(indSize > 0) {
        if((nPtr = arrayNumPos(dat, indLst, indSize)) == NULL)
            return False;
    }
    /* set */
    if(*tokenPtr != '=') return setError(E_SYNTAX, 9.06, dlgErr[7]);

    getToken();

    if(!dlg_var_rigth(dat, nPtr, False)) return False;

fEnd:
    if(*tokenPtr == ',')
        return dlg_elt_settings(e);

    if(ENDLINE) {
        dlg_print(lngText("i_done", "Done."), NULL);
        return True;
    }

    return setError(E_SYNTAX, 9.06, tokenPtr);
}

Bool dlg_elt_connect(Element* e)
{
    /* ERROR: 07 */
    Element *eltFrom;

    /* start connection */
    if(e) return dlg_elt_linking(e);

    if(token_type == T_ENDLINE) {
        nLine ++;
        getToken();
        return dlg_elt_connect(NULL);
    }

    if(token_type != T_WORD) return setError(E_SYNTAX, 9.07, dlgErr[3]);


    eltFrom = dlg_elt_find();
    if(!eltFrom) return setError(E_NOT_FOUND, 9.07, tokenPtr);

    if(*tokenPtr != '\'' && token_spec != S_LINK)
        return setError(E_SYNTAX, 9.07, "expected -> or '");

    return dlg_elt_linking(eltFrom);
}

Bool dlg_elt_linking(Element *from)
{
    /* ERROR: 08 */
    Element *to;
    Connector *cFrom, *cTo;
    /* output */
    if((cFrom = dlg_out_find(from)) == NULL)
        return False;

    getToken();
    /* add intermediate elements */
    while(*tokenPtr == '[') {
        /* create element, add to graph */
        to = dlg_elt_insert();
        /* input */
        cTo = eltConnector(to, NULL, C_IN);
        /* connect */
        if(!conSetConnection(cFrom, cTo)) return False;
        /* new output connector */
        cFrom = eltConnector(to, NULL, C_OUT);
        getToken();
    }
    /* input */
    if((cTo = dlg_in_find(&to)) == NULL)
        return False;
    /* connect */
    if(!conSetConnection(cFrom, cTo)) return False;
    /* continue */

    if(*tokenPtr == '\'' || token_spec == S_LINK)
        return dlg_elt_linking(to);
    else if(*tokenPtr == ',') {
        getToken();
        return dlg_elt_connect(NULL);
    }
    else if(ENDLINE) {
        dlg_print(lngText("i_done", "Done."), NULL);
        return True;
    }

    return setError(E_SYNTAX, 9.08, tokenPtr);
}


Element* dlg_elt_insert(void)
{
    /* ERROR: 09 */
    Element* elt = NULL;
    Number n;
    char name[MAXNAME], *op = "+-*/\\", use;
    int fn;
    static int ind = 0;

    switch(getToken()) {
    case T_DELIMETER:
        if(strchr(op, *tokenPtr)) {
            use = *tokenPtr;
            /* value */
            getToken();
            if(!eval_num(&n)) return NULL;
            /* element */
            sprintf(name, OP_NAME"%d", ind++);
            if((elt = eltCreate("OP", name)) == NULL)
                return NULL;
            /* parameters */
            _DP(elt, 0)->txtVal.str[0] = use;
            _DP(elt, 1)->numVal = n;
        }
        else
            return (void*) setError(E_SYNTAX, 9.09, "expected '+-*/\\");
        break;
    case T_WORD:
        if((fn = getFunc(tokenPtr)) != -1) {
            if((elt = eltCreate("FUNC", tokenPtr)) == NULL)
                return NULL;
            /* get function name */
            stringSet(_DP(elt,1), tokenPtr);
            getToken();

            /* read other arguments */
            if(getFuncArg(fn) == 2) {
                if(*tokenPtr != ',')
                    return (void*) setError(E_SYNTAX, 9.09, dlgErr[10]);
                getToken();
                if(!eval_num(&n)) return False;
                _DP(elt, 2)->numVal = n;
        } }
        else
            return (void*) setError(E_SYNTAX, 9.09, NULL);
        break;
    default:
        return (void*) setError(E_SYNTAX, 9.09, tokenPtr);
    }

    if(!eltToGraph(thisGraph, elt)) return False;

    if(*tokenPtr != ']')
        return (void*) setError(E_SYNTAX, 9.09, dlgErr[6]);

    return elt;
}


void help_graph(Graph *gr)
{
    Chain* ptr;

    dlg_print(lngText("i_gr2", "Graph"), gr->nm->name);
    if(gr->u8_descript)
        printf(":\n%s\n", gr->u8_descript);
    else
        printf("\n");

    ptr = gr->base;
    if(ptr) {
        dlg_print(lngText("i_elt", "Elements:"), "\n");
        while(ptr) {
            printf("%s", ptr->elt->nm->name);
            ptr = ptr->next;
            printf("%s", ptr ? ", " : " \n");
    } }
}

void help_type(char *tp)
{
    Ticket *ptr;
    ticPar *par;
    ticCon *con;
    int pos, i;
    u_char size;

    pos = findTicket(tp);
    ptr = memGetTicket(pos);

    if(!ptr) {
        dlg_print("i_notp", "No such type");
        return;
    }

    dlg_print(lngText("i_tp2", "Type"), NULL);
    printf(" - %s\n", ptr->type->name);
    if(ptr->u8_descript) {
        u_print(NULL, ptr->u8_descript);
        printf("\n");
    }
    /* parameters */
    if(ptr->parNum > 0) {
        dlg_print(lngText("i_par", "* Parameters:"), "\n");

        for(par = ptr->lstPar; par; par = par->next) {
            if(par->isExternal) {
                printf("  %s", par->nm->name);
                if(par->u8_descript)
                    printf(" - %s\n", par->u8_descript);
                else
                    printf("\n");
    } } }
    /* connectors */
    for(i = C_IN; i <= C_OUT; i ++) {
        size = (i == C_IN) ? ptr->cinNum : ptr->coutNum;
        con = (i == C_IN) ? ptr->lstIn : ptr->lstOut;

        if(size > 0) {
            dlg_print(lngText(i == C_IN ? "i_in" : "i_out",
                              i == C_IN ? "* Input" : "* Output"),
                      "\n");



            for(;con; con = con->next) {
                printf("  %s (%s", con->nm->name, conDataType(con->dat));
                if(con->dat == 0)
                    printf("[])");
                else if(con->dat > 0)
                    printf("[%d])", con->dat);
                else
                    printf(")");

                if(con->u8_descript)
                    printf(" - %s\n", con->u8_descript);
                else
                    printf("\n");
    } } }
}

void help_element(Element *e)
{
    int i;

    printf("%s %s\n", e->tick->type->name, e->nm->name);
    if(e->u8_descript) printf("%s\n", e->u8_descript);

    if(e->tick == blockTicket) {
        help_block(e);
        return;
    }

    /* statistic */
    if(e->circle > 0) {
        dlg_print(lngText("i_stat", "Average work time (ms):"), NULL);
        printf(" %.3lf\n", e->workTime *(1000.0/CLOCKS_PER_SEC)/e->circle);
    }

    if(e->state.size > 0) {
        dlg_print(lngText("i_par", "* Parameters:"), "\n");
        for(i = 0; i < (int) e->state.size; i ++) {
            if(e->state.group[i].ticket->isExternal) {
                printf("  %s = %s\n", e->state.group[i].ticket->nm->name,
                        dataAsText(e->state.group[i].dat));
    } } }

    help_connection(e, "");

}

void help_connection(Element* e, char* shift)
{
    int i, j, k;
    Connector *ptr, *eCon;

    for(j = C_IN; j <= C_OUT; j ++) {
        if(e->out[j].size > 0) {
            printf("%s", shift);
            dlg_print(lngText(j == C_IN ? "i_in" : "i_out",
                              j == C_IN ? "* Input" : "* Output"),
                      "\n");

            for(i = 0; i < (int) e->out[j].size; i ++) {
                k = 0;
                eCon = e->out[j].group + i;

                printf("%s  '%s'\n", shift, eCon->ticket->nm->name);

                while((ptr = getConnection(eCon, k)) != NULL) {
                    printf("%s", shift);
                    if(j == C_IN)
                        printf("  <- %s'%s'\n", ptr->master->nm->name,
                                                ptr->ticket->nm->name);
                    else
                        printf("  -> '%s'%s\n", ptr->ticket->nm->name,
                                                ptr->master->nm->name);
                    k++;
            } }
    } }
}

void help_block(Element* b)
{
    int i, j;
    Chain* cPtr;
    Element* elt;
    Connector* con;

    dlg_print(lngText("i_sblk", "Struct:"), "\n");

    for(cPtr = blkGraph(b)->base; cPtr; cPtr = cPtr->next) {
        elt = cPtr->elt;
        /* print connected */
        printf("%s%s\n", BLK_SHFT, elt->nm ? elt->nm->name : "_");
        help_connection(elt, BLK_SHFT);
    }

    dlg_print(lngText("i_cblk", "Connectors:"), "\n");

    for(i = C_IN; i <= C_OUT; i ++) {
        for(j = 0; j < b->out[i].size; j ++) {
            con = b->out[i].group + j;
            printf("%s", con->blk.name);
            con = conGetBegin(con);

            if(i == C_IN)
                printf(" ('%s'%s)\n", con->ticket->nm->name,
                                        con->master->nm->name);
            else
                printf(" (%s'%s')\n", con->master->nm->name,
                                        con->ticket->nm->name);
    } }
}

Bool dlg_eval_line(void)
{
    /* ERROR: 11 */
    int var, var2;
    Number *nPtr = NULL;
    u_char asConst = False;
    Data *res;
    extern Bool execMode;

    if(token_type == T_ENDLINE) {
        getToken();
        return dlg_eval_line();
    }
    /* constant */
    if(stringEqUp(tokenPtr, keyWords[7])) {
        asConst = True;
        getToken();
    }
    /* get left part of expression */
    if((var = dlg_var_left(&nPtr)) == -1) return False;
    /* list of variables */
    if(var == VAR_LIST) {
        variablePrint();
        return dlg_end(9.11);
    }
    /* evaluate rigth part */
    if(*tokenPtr == '=' || token_spec == S_REF) {
        if(variableIsConst(var))
            return setError(E_SYMANTIC, 9.11, dlgErr[12]);

        if(*tokenPtr == '=')
            variableFill(var); /* add data if need */
        else {
            /* reference */
            getToken();
            if(*tokenPtr != '%')
                return setError(E_SYMANTIC, 9.11, "not variable");
            if(getToken() != T_WORD || (var2 = variableFind(tokenPtr)) == -1)
                return setError(E_SYMANTIC, 9.11, tokenPtr);

            variableSet(var, variableGet(var2), True);
            if(variableIsConst(var2))
                variableSetConst(var);
            getToken();
            goto rest;
        }

        getToken();
    }

    if(!(ENDLINE || *tokenPtr == ',')) {
        if(!dlg_var_rigth(variableGet(var), nPtr, True)) return False;
    }

rest:
    if(asConst) variableSetConst(var);
    /* print result */
    if(nPtr)
        printf("%s  ", numAsTextFull(*nPtr));
    else {
        res = variableGet(var);
        if(res) {
            if(res->type == tArray)
                arrayPrint(res->arrVal, MAX_IN_LINE);
            else if(res->type == tNumber)
                printf("%s ", numAsTextFull(res->numVal));
            else
                printf("%s  ", dataAsText(*res));
        }
        else
            printf("%s  ", variableName(var));
    }

    if(execMode) printf("\n");

    if(ENDLINE)
        return True;
    if(*tokenPtr == ',') {
        getToken();
        return dlg_eval_line();
    }
    else
        setError(E_SYNTAX, 9.11, tokenPtr);

    return False;
}


int isKeyword(void)
{
    int i = 0;

    while(keyWords[i]) {
        if(stringEqUp(tokenPtr, keyWords[i]))
            return i;
        i ++;
    }

    return -1;
}

int dlg_keyword(int f)
{
    /* ERROR: 12 */
    char* point;

    switch(f) {
    case 0: /* END */
        return 2; /* 2-1 = 1 - exit code */
    case 1: /* RUN */
        return dlg_key_run();
    case 2: /* CHAIN */
        return dlg_key_chain();
    case 3: /* HELP */
        point = setPoint().pos;
        strcat(point, " ?");
        return dlg_exec_code(point) + 1;  /* in dlg_exec_code used res-1 */
    case 4: /* FREE */
        return dlg_key_free();
    case 5: /* DEL */
        return dlg_key_del();
    case 6: /* LOAD */
        return dlg_key_load();
    case 7: /* CONST */
        return dlg_eval_line();
    default:
        setError(E_WTF, 9.12, NULL);
        break;
    }
    return False;
}

Bool dlg_key_run(void)
{
    /* ERROR: 19 */
    extern double tstep;
    Number stop, start = *findConst("TAU0", NULL), step = dblToNumber(tstep);
    int res;

    getToken();
    /* single execution */
    if(ENDLINE) {
        /* rebuild chain */
        if(!prepExecChain(thisGraph)) return False;
        transformDbl(&start, real_part(start));
        /* run */
        return (singleExecute(thisGraph->exec, start, dblToNumber(0)) != -1 );
    }
    /* get range */
    if(!eval_num(&stop)) return False;
    /* range, coma or line end */
    if(token_spec == S_RANGE) {
        start = stop;
        getToken();
        if(!eval_num(&stop)) return False;
    }
    /* step */
    if(*tokenPtr == ',') {
        getToken();
        if(!eval_num(&step)) return False;
    }
    /* run */
    if(ENDLINE) {
        /* rebuild chain */
        if(!prepExecChain(thisGraph)) return False;
        /* execute */
        res = engine(thisGraph->exec, start, stop, step);
        if(res != -1) {
            dlg_print(lngText("i_texec", "Execution time, ms"), NULL);
            printf(": %d\n", res);
        }
        else
            return False;
    }
    else
        return setError(E_SYNTAX, 9.19, tokenPtr);

    return True;
}


Bool dlg_key_chain(void)
{
    /* ERROR: 18 */
    Chain* ptr;
    u_char flag = 0;

    if(!prepExecChain(thisGraph))
            return False;

    getToken();
    if(*tokenPtr == '?') {
        /* print structure */
        for(ptr = thisGraph->exec; ptr; ptr = ptr->next) {
            if(flag)
                printf(" -> ");
            else
                flag = 1;
            printf("%s", ptr->elt->nm->name);
        }
        return dlg_end(9.18);
    }
    else if(ENDLINE)
        dlg_print(lngText("i_done", "Done."), NULL);
    else
        return setError(E_SYNTAX, 9.18, tokenPtr);

    return True;
}

void dlg_print(char* u8_text, char* name)
{
    if(messagePrint) {
        u_print(stdout, u8_text);
        if(name)
            printf(" %s", name);
    }
}


Bool dlg_key_free(void)
{
    /* ERROR: 13 */
    Element *elt1 = NULL;
    Connector *cFrom, *cTo;

    if(getToken() == T_ENDLINE) return dlg_key_free();
    /* variable */
    if(*tokenPtr == '%') {
        if(!dlg_var_rem(variableFree)) return False;
        goto end;
    }

    if(token_type != T_WORD) return setError(E_SYNTAX, 9.13, dlgErr[3]);
    /* current element */
    elt1 = dlg_elt_find();
    if(!elt1) return setError(E_SYMANTIC, 9.13, "wrong name");

    if(ENDLINE || *tokenPtr == ',') {
        disconnectAll(elt1);
    }
    else if(*tokenPtr == '\'' || token_spec == S_LINK) {

        cFrom = dlg_out_find(elt1);
        if(!cFrom) return False;
        /* second part */
        getToken();

        cTo = dlg_in_find(NULL);
        if(!cTo) return False;

        if(!conDisconnect(cFrom, cTo)) return False;
    }
    else
        return setError(E_SYNTAX, 9.13, NULL);

end:
    dlg_print(lngText("i_done", "Done."), NULL);

    if(*tokenPtr == ',')
        return dlg_key_free();
    else if(token_type != T_END && token_type != T_ENDLINE)
        return setError(E_SYNTAX, 9.13, tokenPtr);

    return True;
}


Bool dlg_key_del(void)
{
    /* ERROR: 14 */
    Element *e;
    Chain* ch;

    delChain = NULL;
    delGraph = NULL;

    getToken();
    if(token_type == T_ENDLINE) return dlg_key_del();

    if(*tokenPtr == '%') {
        if(!dlg_var_rem(variableDelelte)) return False;
        goto end;
    }

    if(token_type != T_WORD) setError(E_SYNTAX, 9.14, dlgErr[3]);

    e = dlg_elt_find();
    if(!e) return setError(E_SYMANTIC, 9.14, "wrong name");
    /* break links */
    disconnectAll(e);

    /* remove from chain */
    if(delChain) {
        /* remove */
        ch = delChain->next;
        delChain->next = ch->next;
    }
    else {
        ch = delGraph->base;
        delGraph->base = ch->next;
    }
    /* correct last */
    if(ch == delGraph->baseLast)
        delGraph->baseLast = delChain;

    free(ch);
    /* remove from memory */
    memEltDelete(e);

end:
    dlg_print(lngText("i_done", "Done."), NULL);

    if(*tokenPtr == ',')
        return dlg_key_del();
    else if(ENDLINE)
        return True;

    return setError(E_SYNTAX, 9.14, tokenPtr);
}


Bool dlg_key_load(void)
{
    /* ERROR: 15 */
    extern char* progBufPtr;
    u_char useDbg = False, res = True;
    char ans[8] = {'\0'};
    LastState ls;

    if(getToken() == T_WORD) {
        if(stringEqUp(tokenPtr, "DBG"))
            useDbg = True;
        else
            return setError(E_SYNTAX, 9.15, tokenPtr);
        getToken();
    }

    if(token_type != T_STRING)
        return setError(E_SYNTAX, 9.15, "expected filename");

    ls = setPoint();

    if(!loadFile(tokenPtr, progBufPtr, PROG_BUF_SIZE))
            return setError(E_FOPEN_ERR, 9.15, tokenPtr);

    if(useDbg) {
        flNew = True;
        while(readNext(progBufPtr)) {
            printf("\n%s\n- ", nextCommand);
            dlg_print(lngText("i_qexec", "Execute?"), "(../no/break): ");
            fgets(ans, 7, stdin);

            if(stringEqUp(ans, "NO\n"))    /* !!! ACHTUNG KOSTYL' */
                continue;
            else if(stringEqUp(ans, "BREAK\n"))
                break;

            if(dlg_exec_code(nextCommand) == -1) {
                res = False;
                break;
    } } }
    else
        res = (dlg_exec_code(progBufPtr) != -1);

    goToPoint(&ls);

    return res;
}

int isEmpty(char* str)
{
    while(*str && iswhite(*str)) str++;

    if(*str == '\n' || *str == '\0')
        return 1;
    else if(*str == '#')
        return 2;

    return 0;
}

Bool readNext(char *src)
{
    char *pos, *mark;
    u_char empty;
    static char* srcPtr;
    int i = 0;

    if(flNew) {
        srcPtr = src;
        flNew = False;
    }

    if(*srcPtr == '\0') return False;

    mark = pos = nextCommand;

    while(1) {
        while(*srcPtr && *srcPtr != '\n') {
            *pos++ = *srcPtr++;
            if(++i == COMM_SIZE) break;
        }

        *pos = '\0';
        srcPtr++;
        /* end source file or no memory */
        if(*srcPtr == '\0' || i == COMM_SIZE) break;
        /* empty string */
        empty = isEmpty(mark);
        if(empty == 1) {
            pos = mark;
            continue;
        }

        /* not finished */
        if(isFinish(nextCommand) == nextCommand && empty != 2)
            break;

        *pos = '\n';
        mark = ++pos;
    }

    return True;
}


Bool dlg_elt_copy(void)
{
    /* ERROR: 16 */
    char tmpName[MAXNAME];
    Element* elt;

    if(getToken() == T_ENDLINE)
        return dlg_elt_copy();

    if(token_type != T_WORD)
        return setError(E_SYNTAX, 9.16, dlgErr[3]);

    strncpy(tmpName, tokenPtr, MAXNAME-1);

    getToken();
    if(*tokenPtr != '=') return setError(E_SYNTAX, 9.16, dlgErr[7]);
    /* find element */
    getToken();
    elt = dlg_elt_find();
    if(!elt) return setError(E_NOT_FOUND, 9.16, tmpName);
    /* set to graph */
    if(!eltToGraph(thisGraph, eltCopy(tmpName, elt)))
        return False;

    dlg_print(lngText("i_addelt", "Add element"), tmpName);
    /* end */
    if(*tokenPtr == ',')
        return dlg_elt_copy();
    else if(ENDLINE)
        return True;
    else
        setError(E_SYNTAX, 9.16, tokenPtr);

    return False;
}


Bool dlg_block(Graph* g)
{
    /* ERROR: 17 */
    if(g == thisGraph)
        return setError(E_SYMANTIC, 9.17, "this is current graph");

    if(*tokenPtr == T_ENDLINE) {
        getToken();
        return dlg_block(g);
    }

    if(token_type != T_WORD) return setError(E_SYNTAX, 9.17, dlgErr[3]);
    /* create block */
    if(!eltToGraph(thisGraph, blkCreate(tokenPtr, g)))
        return False;

    dlg_print(lngText("i_addelt", "Add element"), tokenPtr);
    /* next */
    getToken();

    if(ENDLINE)
        return True;
    else if(*tokenPtr == ',') {
        getToken();
        return dlg_block(g);
    }
    else
        setError(E_SYNTAX, 9.17, tokenPtr);

    return False;
}

Bool dlg_var_rigth(Data* d, Number *nPtr, u_char changeType)
{
    Data *tmp;

    if(nPtr) {
        tmp = getTmpData(1);
        if(!eval_data(tmp, tNumber)) return False;
        *nPtr = tmp->numVal;
        return True;
    }

    return eval_data(d, changeType ? -1 : d->type);
}

/* return -1 if error or index (default - for "ans") */
int dlg_var_left(Number **pos)
{
    /* ERROR: 21 */
    u_char indSize = 0, add = False;
    char par = '\0';
    Number indLst[MAX_ARG_NUMBER];
    Data* dat = NULL, tmpDat;
    int ind = -1;
    LastState ls;

    *pos = NULL;

    if(*tokenPtr != '%')
        return variableFind("ANS");

    ls = setPoint();
    getToken();
    /* get help */
    if(token_spec == S_RANGE) {
        getToken();
        if(*tokenPtr != '?')
            return setError(E_SYNTAX, 9.21, dlgErr[0])-1;
        return VAR_LIST;
    }
    /* variable name */
    if(token_type != T_WORD)
        return setError(E_SYNTAX, 9.21, dlgErr[3]) - 1;

    ind = variableFind(tokenPtr);
    if(ind == -1) {
        if((ind = variableAdd(tokenPtr)) == -1) return -1;
        add = True;
    }

    getToken();

    /* array */
    if(*tokenPtr == '[' || *tokenPtr == '(') {
        par = *tokenPtr;
        if(!eval_args(indLst, &indSize, MAX_ARG_NUMBER)) return -1;
        if(par == '[' && *tokenPtr != ']')
            return setError(E_SYNTAX, 9.21, dlgErr[6]) - 1;
        if(par == '(' && *tokenPtr != ')') return -1;
        getToken();
    }
    else if(*tokenPtr == '.') {  goto Done; }

    /* check end */
    if(*tokenPtr == '=' || *tokenPtr == ',' || token_spec == S_REF || ENDLINE) {

        if(add && par == '(')
            return setError(E_SYNTAX, 9.21, NULL) - 1;
        /* get array */
        if(indSize > 0) {
            if(token_spec == S_REF)
                return setError(E_SYMANTIC, 9.21, "array") - 1;
            /* get / set data */
            if((dat = variableGet(ind)) == NULL) {
                tmpDat = dataArrayNull();
                dat = variableSet(ind, &tmpDat, False);
            }
            if(dat->type != tArray)
                return setError(E_SYMANTIC, 9.21, "not array") - 1;
            /* resize or get number */
            if(par == '[') {
                if(variableIsConst(ind))
                    return setError(E_SYMANTIC, 9.21, dlgErr[12]) - 1;
                if(dat->arrVal.length > 0)
                    return setError(E_SYMANTIC, 9.21, "not empty") - 1;
                if(!resizeToArray(dat, indLst, indSize)) return -1;
            }
            else {
                if(*tokenPtr == '=' && variableIsConst(ind))
                        return setError(E_SYMANTIC, 9.21, dlgErr[12]) - 1;
                if((*pos = arrayNumPos(dat, indLst, indSize)) == NULL)
                    return -1;
            }

            return ind;
        }

        if(variableIsConst(ind) && (*tokenPtr == '=' || token_spec == S_REF))
            return setError(E_SYMANTIC, 9.21, dlgErr[12]) - 1;

        return ind;
    }
Done:
    /* back to begining state */
    //setProgramm(point);
    goToPoint(&ls);
    tokenPtr[0] = '%'; tokenPtr[1] = '\0';
    //token_type = T_KEYCHAR;
    //token_spec = S_NONE;

    return ind == -1 ? variableFind("ANS") : ind;
}


Bool dlg_var_rem(void (*f)(int ))
{
    /* ERROR: 22 */
    int pos;

    if(getToken() != T_WORD)
        return setError(E_SYNTAX, 9.22, dlgErr[3]);

    if((pos = variableFind(tokenPtr)) == -1)
        return setError(E_NOT_FOUND, 9.22, tokenPtr);

    f(pos);
    getToken();

    return True;
}

Connector* dlg_out_find(Element *elt)
{
    /* ERROR: 23 */
    Connector* cOut = NULL;
    if(!elt) return NULL;

    if(*tokenPtr == '\'') {
        if(getToken() != T_WORD)
            return (void*) setError(E_SYNTAX, 9.23, tokenPtr);
        cOut = eltConnector(elt, tokenPtr, C_OUT);
        getToken();
        if(*tokenPtr != '\'')
            return (void*) setError(E_SYNTAX, 9.23, dlgErr[8]);
        getToken();
        if(token_spec != S_LINK)
            return (void*) setError(E_SYNTAX, 9.23, dlgErr[9]);
    }
    else
        cOut = eltConnector(elt, NULL, C_OUT);

    return cOut;
}

Connector* dlg_in_find(Element **elt)
{
    /* ERROR: 24 */
    Element *to;
    char tmpName[MAXNAME] = "";

    if(*tokenPtr == '\'') {
        if(getToken() != T_WORD)
            return (void*) setError(E_SYNTAX, 9.24, dlgErr[3]);
        strncpy(tmpName, tokenPtr, MAXNAME-1);
        getToken();
        if(*tokenPtr != '\'')
            return (void*) setError(E_SYNTAX, 9.24, dlgErr[8]);
        getToken();
    }
    /* find input element */
    to = dlg_elt_find();
    if(!to) return (void*) setError(E_SYMANTIC, 9.24, "wrong name");
    if(elt) *elt = to;
    /* find input connector */
    return eltConnector(to, *tmpName ? tmpName : NULL, C_IN);
}
