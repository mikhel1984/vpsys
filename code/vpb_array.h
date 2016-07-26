#ifndef VPB_ARRAY_H_INCLUDED
#define VPB_ARRAY_H_INCLUDED

#include "vpb_abstract.h"

#define ARRFREE(X)    free((X)->numb); free((X)->grid);
#define ARR_NUM_FUNC  3
#define ARR_TMP_ONE   20

/** @struct axe_
    Keep data about given dimention */
typedef struct axe_
{
    int size;            /**< Number of items in given dimention */
    int coeff;            /**< Coefficient for easyle recalc index */
} axe;

/** @struct vpDataArray
    Number array representation (vector by default) */
struct vpDataArray
{
    int length;           /**< Array length */
    Number *numb;          /**< Pointer to array data */
    axe *grid;             /**< Length relatively dimentions */
    u_char dim;            /**< Dimention number */
};

Bool dataArrayEq(Array a1, Array a2);

Bool resizeArray(Array *a, int size);

//Bool clearArray(Array *a, int size);

Bool resizeDim(Array *a, int dim, ...);

Bool arrSum(Array *a, Array *val);

Bool arrSub(Array *a, Array *val);

Bool arrTranspose(Array *a);

Bool arrProd(Array *a, Array *val);

//Bool arrRat(Array *a, Array *val);

Bool arrNumMult(Array *a, Number n);

Bool arrNumDiv(Array *a, Number n);

Bool arrDet(Array *a, Number *res);

Bool arrInv(Array *a);

int arrDotFind(const char* fn);

Data* arrDotEval(int fn, Data *obj, Data *args, u_char narg);

void arrayPrint(Array a, int maxSize);

char* arrayText(Array a);





#endif // VPB_ARRAY_H_INCLUDED
