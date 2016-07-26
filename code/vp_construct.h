/** @file vp_construct.h
Necessary structures and functions for constructing elements.
*/


#ifndef VP_CONSTRUCT_H
#define VP_CONSTRUCT_H

#include "vpb_data.h"



//extern Number constZero, constOne, constPi, constE, constIm;


/* work with element */
Data* getInData(Element* elt, unsigned int pos);
Data* getOutData(Element* elt, unsigned int pos);
Data* getParData(Element* elt, unsigned int pos);

Data* getNameData(Element* elt, const char* var);

int inNum(Element *elt);
int outNum(Element *elt);
int parNum(Element *elt);


/** Get translated string
    @fn lngchar
    @param key - key string
    @param def - default char
    @return pointer to translated string */
char* lngText(char *key, char *def);

void u_print(FILE* out, char* u8_txt);

/* registration type */
int regElement(const char *type, char *descript);
int regParameter(int elt, const char *name, char* descript,
                  Data val, Bool isExternal);
int regConnector(int elt, const char *name, char* descript,
                  int type, int dir);
Bool regExec(int elt, Bool (*execf)(Element*, Number, Number));
Bool regPrep(int elt, Bool (*prepf)(Element*, u_char));

void halt(void);



#endif /* VP_CONSTRUCT_H */
