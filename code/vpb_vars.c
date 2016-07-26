
/* ERROR FILE NUMBER - 18 */
#include <stdio.h>
#include <stdlib.h>

#include "vpb_vars.h"
#include "vpb_memory.h"
#include "vpb_data.h"
#include "vpb_element.h"

extern Memory vars;

Number* memAddConst(char *nm, Number val, Bool isConst)
{
    int v;
    Data d;
    VarPtr* lst;

    d.type = tNumber;
    d.numVal = val;

    v = variableAdd(nm);
    if(v == -1) return NULL;

    if(!variableSet(v, &d, False)) return NULL;

    lst = (VarPtr*) vars.base;

    lst[v]->isConst = isConst;

    return &(lst[v]->dat->numVal);
}


Number* findConst(char *nm, u_char *isConst)
{
    int v;
    VarPtr* lst = (VarPtr*) vars.base;

    v = variableFind(nm);
    if(v == -1) return NULL;

    if(lst[v]->dat->type == tNumber) {
        if(isConst) *isConst = lst[v]->isConst;
        return &(lst[v]->dat->numVal);
    }

    return NULL;
}

Data* variableGet(int pos)
{
    VarPtr* lst = (VarPtr*) vars.base;
    return (pos > -1 && pos < vars.pos && lst[pos]) ?
                    lst[pos]->dat : NULL;
}

char* variableName(int pos)
{
    VarPtr* lst = (VarPtr*) vars.base;
    return (pos >= 0 && pos < vars.pos && lst[pos]) ?
                    lst[pos]->nm->name : NULL;
}

u_char variableIsConst(int pos)
{
    VarPtr* lst = (VarPtr*) vars.base;
    return (pos >= 0 && pos < vars.pos && lst[pos]) ?
                                lst[pos]->isConst : False;
}

void variableSetConst(int pos)
{
    VarPtr* lst = (VarPtr*) vars.base;
    if(pos >= 0 && pos < vars.pos && lst[pos])
        lst[pos]->isConst = True;
}

Data* variableSet(int pos, Data* d, u_char ref)
{
    Var* ptr = NULL;
    VarPtr* lst = (VarPtr*) vars.base;

    if(!d || pos < 0 || pos >= vars.length) return NULL;

    ptr = lst[pos];
    if(!ptr || ptr->isConst) return NULL;

    if(ref) {
        if(!ptr->isPtr) {
            dataClear(ptr->dat);
            free(ptr->dat);
            ptr->isPtr = True;
        }
        ptr->dat = d;
    }
    else {
        if(ptr->isPtr) {
            ptr->dat = NULL;
            ptr->isPtr = False;
        }

        if(!ptr->dat)
            ptr->dat = (Data*) calloc(1, sizeof(Data));
        else
            dataClear(ptr->dat);

        dataCopy(ptr->dat, *d);
    }

    return ptr->dat;
}


void variableFree(int pos)
{
    VarPtr* lst = (VarPtr*) vars.base;
    Var* ptr = lst[pos];

    ptr->isConst = False;

    if(ptr->isPtr)
        ptr->dat = NULL;
    else {
        dataClear(ptr->dat);
        *(ptr->dat) = dataInt(0);
    }
}

void variableDelelte(int pos)
{
    VarPtr* lst = (VarPtr*) vars.base;
    Var* ptr = lst[pos];
    int i;

    if(!ptr->isPtr && ptr->dat) {
        /* clear references */
        for(i = 0; i < vars.pos; i ++) {
            if(lst[i] && lst[i]->dat == ptr->dat && i != pos)
                lst[i]->dat = NULL;
        }
        /* free data */
        dataClear(ptr->dat);
        free(ptr->dat);
    }

    free(ptr);
    lst[pos] = NULL;
}

Bool variableFill(int pos)
{
    Var* ptr;
    VarPtr *lst = (VarPtr*) vars.base;

    if(pos < 0 || pos >= vars.length) return False;

    ptr = lst[pos];

    if(!ptr->dat) {
        if((ptr->dat = (Data*) malloc(sizeof(Data))) == NULL) return False;
        (*ptr->dat) = dataInt(0);
    }

    return True;
}

int variableFind(char* nm)
{
    unsigned int hash;
    int i;
    VarPtr* lst = (VarPtr*) vars.base;

    hash = stringHashUp(nm);
    for(i = 0; i < vars.length; i ++, lst++) {
        if(*lst && (*lst)->nm->name_hash == hash
                        && stringEqUp((*lst)->nm->name, nm))
            return i;
    }

    return -1;
}

void variablePrint(void)
{
    VarPtr* lst = (VarPtr*) vars.base;
    int i;

    if(vars.pos == 0 || !vars.base) {
        return;
    }
    //memSortName(&vars);

    for(i = 0; i < vars.pos; i ++, lst++) {
        if(!(*lst)) continue;

        if((*lst)->isConst) printf("const ");
        printf("%%%s", (*lst)->nm->name);
        if((*lst)->isPtr) printf(" (r)");
        if(i+1 != vars.pos) printf(", ");
        //printf("%s", (i+1 == vars.pos) ? "\n" : ", ");
    }
}



