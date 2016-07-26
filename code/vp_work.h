#ifndef VP_WORK_H
#define VP_WORK_H

#include "vpb_abstract.h"
#include "vpb_data.h"

/** Init constants for using in project
    @fn numInitConst */
void numInitConst(void);

int base_elements(void);

/** Create element with given type and name
    @fn eltCreate
    @param type - element type
    @param name - element name
    @return pointer to element or NULL */
Element* eltCreate(const char* type, const char* name);

Graph* newGraph(const char* name, const char* descr);

int eltToGraph(Graph *g, Element *e);



int connect(Element *e1, char *c1, Element *e2, char *c2);

int disconnect(Element *e1, char *c1, Element *e2, char *c2);

int disconnectAll(Element *e);

int setParameter(Element *e, const char* param, Data val);

Chain* prepExecChain(Graph *gr);

int engine(Chain *c, Number start, Number stop, Number step);

/** Execute functions from chain for one time
    @fn singleExecute
    @param c - pointer to chain
    @param t - current time
    @param dt - time step from last iteration
    @return execution time in takts */
int singleExecute(Chain *c, Number t, Number dt);



void help_graph(Graph *gr);

void help_type(char* tp);

void help_element(Element* e);


/** Clear memory
    @fn memFree */
void memFree(void);

char* isFinish(char *line);



#endif // VP_WORK_H
