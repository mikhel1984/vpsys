#ifndef VPB_VARS_H
#define VPB_VARS_H

#include "vpb_abstract.h"

typedef struct vpVars
{
    Names* nm;
    //char name[MAXNAME];
    u_char isConst;
    u_char isPtr;
    Data*  dat;
    //unsigned int hash;
} Var;

int variableFind(char* nm);

int variableAdd(char* nm);

Data* variableSet(int pos, Data* d, u_char ref);

Data* variableGet(int pos);

char* variableName(int pos);

Bool variableFill(int pos);

void variableSetConst(int pos);

u_char variableIsConst(int pos);

void variableFree(int pos);

void variableDelelte(int pos);

void variablePrint(void);


#endif // VPB_VARS_H
