
/* ERROR FILE NUMBER - 17 */
#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
#include <stdarg.h>
#include <string.h>

#include "vpb_array.h"
#include "vpb_number.h"
#include "vpb_error.h"

#include "vpb_data.h"


#define matInd(i,j,gr) (gr[0].coeff*(i)+gr[1].coeff*(j))
#define matPtr(i,j,ar) ((ar)->numb + (ar)->grid[0].coeff*(i) + (ar)->grid[1].coeff*(j))

const Array arrayZero = {0, NULL, NULL, 1};

Bool arrOpAdd(Array *dst, Array *src, Number (*f)(Number, Number));

Bool arrayDimExpand(Array *a);

Bool arrOpMlt(Array *dst, Number n, Number (*f)(Number, Number));

Bool arrayOne(Array *a, int sz);

Bool arrayCopy(Array *dst, Array* src);

Bool arrayLUTransform(Array *lu, Array *src);

Bool arrayEulerDown(Array *lft, Array *rht);

Data* dotLength(Data*, Data*, u_char), *dotDimention(Data*, Data*, u_char),
*dotDeterm(Data*, Data*, u_char);

Data* dotAdd(Data*, Data*, u_char), *dotSub(Data*, Data*, u_char),
*dotProd(Data*, Data*, u_char), *dotInv(Data*, Data*, u_char),
*dotTrans(Data*, Data*, u_char), *dotFn(Data*, Data*, u_char);

struct ArrDotFunk
{
    char* name;
    Data* (*func)(Data*, Data*, u_char);
}
arrDotList[] =
{/* -- Return number -- */
{"len", dotLength}, {"dim", dotDimention}, {"det", dotDeterm},
 /* -- Modify array -- */
{"add", dotAdd}, {"sub", dotSub}, {"prod", dotProd}, {"inv", dotInv},
{"trans", dotTrans},
/* end */ {"", NULL}
};


/* Compare 2 arrays */
Bool dataArrayEq(Array a1, Array a2)
{
    int i;

    if(a1.length == a2.length && a1.dim == a2.dim) {

        if(a1.dim == 1) return True;
        /* compare each dimention */
        for(i = 0; i < (int) a1.dim; i ++) {
            if(a1.grid[i].size != a2.grid[i].size)  return False;
        }
    }
    else
        return False;

    return True;
}

/* Resize array to new length */
/* ERROR FUNCTION NUMBER - 1 */
Bool resizeArray(Array* a, int size)
{
    Number* ptr;

    if(a->length != size) {
        /* get memory */
        _CRITICAL(ptr = (Number*) realloc(a->numb, sizeof(Number) * size))

        a->numb = ptr;
        a->length = size;
        /* set dimention to 1 */
        if(a->grid) {
            free(a->grid);
            a->grid = NULL;
        }
        a->dim = 1;
    }

    return True;
}
#if 0
/* ERROR FUNCTION NUMBER - 7 */
Bool clearArray(Array *a, int size)
{
    Number *ptr;

    if(a->length != size) {
        ptr = (Number*) calloc(size, sizeof(Number));
        if(!ptr) return setError(E_NO_MEMORY, 17.07, NULL);

        IFFREE(a->numb);
        a->numb = ptr;
    }
    else {
        memset(a->numb, 0, sizeof(Number)*size);
    }

    if(a->grid) {
        free(a->grid);
        a->grid = NULL;
    }
    a->dim = 1;

    return True;
}
#endif
/* get array sum -- ? */

Bool arrSum(Array *a, Array *val)
{
    return arrOpAdd(a, val, nSum);
}

Bool arrSub(Array *a, Array *val)
{
    return arrOpAdd(a, val, nSub);
}

/* ERROR FUNCTION NUMBER - 2 */
Bool arrOpAdd(Array *dst, Array *src, Number (*f)(Number, Number))
{
    int i;

    if(dst->dim == 1) arrayDimExpand(dst);
    if(src->dim == 1) arrayDimExpand(dst);

    if(dataArrayEq(*dst, *src)) {
        for(i = 0; i < src->length; i ++) {
            dst->numb[i] = f(dst->numb[i], src->numb[i]);
        }
    }
    else
        return setError(E_DIFF_SIZE, 17.02, NULL);

    return True;
}

/* ERROR FUNCTION NUMBER - 3 */
Bool arrayDimExpand(Array *a)
{
    if(a->dim == 1) {
        IFFREE(a->grid);
        _CRITICAL(a->grid = (axe*) calloc(2, sizeof(axe)))

        a->grid[0].size = a->grid[1].coeff = a->length;
        a->grid[0].coeff = a->grid[1].size = 1;

        a->dim = 2;
    }

    return True;
}

/* ERROR FUNCTION NUMBER - 4 */
Bool arrTranspose(Array *a)
{
    axe tmp;

    if(a->dim == 1) arrayDimExpand(a);

    if(a->dim == 2) {
        tmp = a->grid[0];
        a->grid[0] = a->grid[1];
        a->grid[1] = tmp;
    }
    else
        return setError(E_NOT_DEF, 17.04, NULL);

    return True;
}

/* ERROR FUNCTION NUMBER - 4 */
Bool arrProd(Array *a, Array *val)
{
    int i, j, k;
    Number *newArr = NULL, *ptr, *na, *nb;
    axe tmpGrid[2];

    if(a->dim == 1) arrayDimExpand(a);
    if(val->dim == 1) arrayDimExpand(val);

    if(a->dim != 2 || val->dim != 2)
        return setError(E_NOT_DEF, 17.04, NULL);

    if(a->grid[1].size != val->grid[0].size)
        return setError(E_DIFF_SIZE, 17.04, NULL);

    _CRITICAL(newArr = (Number*) calloc(a->grid[0].size*val->grid[1].size, sizeof(Number)))

    tmpGrid[0].size = tmpGrid[1].coeff = a->grid[0].size;
    tmpGrid[0].coeff = 1;
    tmpGrid[1].size = val->grid[1].size;

    for(i = 0; i < a->grid[0].size; i++) {
        for(j = 0; j < val->grid[1].size; j ++) {
            ptr = newArr + matInd(i,j,tmpGrid);
            na = a->numb+i*a->grid[0].coeff;
            nb = val->numb+j*val->grid[1].coeff;
            for(k = 0; k < a->grid[1].size;
                k ++, na += a->grid[1].coeff, nb += val->grid[0].coeff)
                {
                    *ptr = nSum(*ptr, nProd(*na, *nb));
                    /*    nProd(a->numb[matInd(i,k,a->grid)],
                                val->numb[matInd(k,j,val->grid)])); */
                }
    } }

    free(a->numb);
    a->numb = newArr;
    a->grid[0] = tmpGrid[0];
    a->grid[1] = tmpGrid[1];

    return True;
}

Bool arrOpMlt(Array* dst, Number n, Number (*f)(Number, Number))
{
    int i;

    for(i = 0; i < dst->length; i ++) {
        dst->numb[i] = f(dst->numb[i], n);
    }

    return True;
}

Bool arrNumMult(Array *a, Number n)
{
    return arrOpMlt(a, n, nProd);
}

Bool arrNumDiv(Array *a, Number n)
{
    return arrOpMlt(a, n, nRat);
}

/* ERROR FUNCTION NUMBER - 5 */
Bool arrayOne(Array *a, int sz)
{
    int i = sz*sz;
    if(!a || sz <= 0)
        return setError(E_WRONG_ARG, 17.05, NULL);

    if(!resizeArray(a, i) || !resizeDim(a, 2, sz, sz))
        return False;

    memset(a->numb, 0, i*sizeof(Number));

    sz += 1;
    for(i = 0; i < a->length; i += sz) {
        a->numb[i] = constOne;
    }

    return True;
}

/* ERROR FUNCTION NUMBER - 6 */
Bool resizeDim(Array *a, int dim, ...)
{
    va_list ap;
    axe *newAxe;
    int i, ind, k = 1;

    if(!a || dim <= 1)
        return setError(E_WRONG_ARG, 17.06, NULL);

    _CRITICAL(newAxe = (axe*) calloc(dim, sizeof(axe)))

    va_start(ap, dim);
    for(i = 0; i < dim; i ++) {
        ind = va_arg(ap, int);
        if(ind <= 0) {
            free(newAxe);
            va_end(ap);
            return setError(E_WRONG_INDEX, 17.06, NULL);
        }
        newAxe[i].size = ind;
        newAxe[i].coeff = k;
        k *= ind;  /* last value is array size */
    }

    IFFREE(a->grid);
    a->grid = newAxe;
    a->dim = dim;

    va_end(ap);

    return True;
}

/* ERROR FUNCTION NUMBER - 7 */
Bool arrayLUTransform(Array* lu, Array* src)
{

    int i, j, k;
    Number *nik, *nij;

    if(src->dim != 2 || src->grid[0].size != src->grid[1].size)
            return setError(E_NOT_DEF, 17.07, NULL);

    if(!arrayCopy(lu, src)) return False;

    for(k = 0; k < src->grid[0].size; k ++) {
        for(i = k + 1; i < src->grid[0].size; i ++) {
            nik = lu->numb + matInd(i,k,lu->grid);
            *nik = nRat(*nik, lu->numb[matInd(k,k,lu->grid)]);
            for(j = k + 1; j < src->grid[0].size; j ++) {
                nij = lu->numb + matInd(i,j,lu->grid);
                *nij = nSub(*nij, nProd(*nik, lu->numb[matInd(k,j,lu->grid)]));
            }
        }
    }

    return True;
}

/* ERROR FUNCTION NUMBER - 8 */
Bool arrayCopy(Array *dst, Array* src)
{
    int i;
    Number *nPtr = NULL;
    axe* aPtr = NULL;

    if(!dst || !src) return setError(E_WRONG_ARG, 17.08, NULL);

    if(src->length > 0) {
        _CRITICAL(nPtr = (Number*) malloc(sizeof(Number) * src->length))

        for(i = 0; i < src->length; i ++)
            nPtr[i] = src->numb[i];
    }

    if(src->dim > 1) {
        _CRITICAL(aPtr = (axe*) malloc(sizeof(axe) * src->dim))

        for(i = 0; i < (int) src->dim; i ++)
            aPtr[i] = src->grid[i];
    }

    IFFREE(dst->numb);
    IFFREE(dst->grid);

    dst->numb = nPtr;
    dst->length = src->length;
    dst->grid = aPtr;
    dst->dim = src->dim;

    return True;
}

/* ERROR FUNCTION NUMBER - 7 */
Bool arrDet(Array *a, Number *res)
{
    Array tmp = arrayZero;
    int r;

    if(a->dim != 2 || a->grid[0].size != a->grid[1].size)
        return setError(E_NOT_DEF, 17.07, NULL);

    if(!arrayCopy(&tmp, a)) return False;

    r = a->grid[0].size-1;

    if(arrayEulerDown(&tmp, NULL)) {
        *res = *matPtr(r, r, &tmp);
        r = True;
    }
    else
        r = False;

    ARRFREE(&tmp)

    return r;
}

/* ERROR FUNCTION NUMBER - 9 */
Bool arrayEulerDown(Array* lft, Array* rht)
{

    Number n, *ptr, *ptr2;
    int i, j, k, step, swp = 1;

    if(lft->grid[0].size > lft->grid[1].size)
        return False;
    if(rht && rht->grid[0].size != lft->grid[0].size)
        return False;
    step = lft->grid[0].size;
    /* last have to be not 1 */
    for(i = 0; i < step-1; i ++) {
        /* check if zero */
        n = *matPtr(i,i,lft);
        if(isZero(n)) {
            k = 0; j = i+1;
            ptr = matPtr(j,i,lft);
            for(; j < step; j ++, ptr += lft->grid[0].coeff) {
                if(!isZero(*ptr)) {
                    k = j;
                    break;
            } }
            if(k == 0) {
                if(i)
                    continue;
                else
                    return False;
            }
            /* add */
            ptr = matPtr(i,i,lft);
            ptr2 = matPtr(i+k,i,lft);
            swp *= ((k-i) % 2) ? -1 : 1;

            for(j = i; j < lft->grid[1].size; j ++) {
                *ptr = nSum(*ptr, *ptr2);
                ptr += step;
                ptr2 += step;
            }
            if(rht) {
                ptr = matPtr(i,0,rht);
                ptr2 = matPtr(i+k,0,rht);

                for(j = 0; j < rht->grid[1].size; j ++) {
                    *ptr = nSum(*ptr, *ptr2);
                    ptr += step;
                    ptr2 += step;
            } }
            n = *matPtr(i,i,lft);
        }
        /* normalize to first element */
        ptr = matPtr(i,i,lft);
        *ptr = constOne;
        ptr += step;
        for(j = i+1; j < lft->grid[1].size; j ++) {
            *ptr = nRat(*ptr, n);
            ptr += step;
        }
        if(rht) {
            ptr = matPtr(i,0,rht);
            for(j = 0; j < rht->grid[1].size; j ++) {
                *ptr = nRat(*ptr, n);
                ptr += step;
        } }
        /* substract */
        for(j = i+1; j < lft->grid[0].size; j ++) {
            ptr = matPtr(i,i,lft);
            ptr2 = matPtr(j,i,lft);
            n = *ptr2;
            *ptr2 = constZero;
            ptr += step; ptr2 += step;
            for(k = i+1; k < lft->grid[1].size; k ++) {
                *ptr2 = nSub(*ptr2, nProd(*ptr, n));
                ptr += step;  ptr2 += step;
            }

            if(rht) {
                ptr = matPtr(i,0,rht);
                ptr2 = matPtr(j,0,rht);
                for(k = 0; k < rht->grid[1].size; k ++) {
                    *ptr2 = nSub(*ptr2, nProd(*ptr, n));
                    ptr += step;
                    ptr2 += step;
            } }
    } }
    /* correct sign of last line*/
    if(swp == -1) {
        j = step-1;
        ptr = matPtr(j, j, lft);
        for(i = j; i < lft->grid[1].size; i ++, ptr += step)
            numInvert(ptr);

        if(rht) {
            ptr = matPtr(j, 0, rht);
            for(i = 0; i < rht->grid[1].size; i++, ptr += step)
                numInvert(ptr);
    } }

    return True;
}

/* ERROR FUNCTION NUMBER - 10 */
Bool arrayEulerUp(Array *lft, Array *rht)
{
    Number n, *ptr, *ptr2;
    int i, j, k, step;

    if(lft->grid[0].size > lft->grid[1].size)
        return False;
    if(rht && rht->grid[0].size != lft->grid[0].size)
        return False;

    step = lft->grid[0].size;
    i = step - 1;
    ptr = matPtr(i,i,lft);
    n = *ptr;

    if(isZero(n)) return False;

    *ptr = constOne;
    /* normalize */
    for(j = i+1; j < lft->grid[1].size; j ++) {
        *ptr = nRat(*ptr, n);
        ptr += step;
    }
    if(rht) {
        ptr = matPtr(i,0,rht);
        for(j = 0; j < rht->grid[1].size; j ++) {
            *ptr = nRat(*ptr, n);
            ptr += step;
        }
    }

    /* left to E matrix */
    for(j = step-1; j > 0; j--) {
        for(i = j-1; i >= 0; i --) {
            ptr = matPtr(i,j,lft);
            n = *ptr;
            *ptr = constZero;

            if(lft->grid[1].size > step) {
                ptr = matPtr(j,step,lft);
                ptr2 = matPtr(i,step,lft);
                for(k = step; k < lft->grid[1].size; k ++) {
                    *ptr2 = nSub(*ptr2, nProd(n, *ptr));
                    ptr += step; ptr2 += step;
            } }

            if(rht) {
                ptr = matPtr(j, 0, rht);
                ptr2 = matPtr(i, 0, rht);
                for(k = 0; k < rht->grid[1].size; k ++) {
                    *ptr2 = nSub(*ptr2, nProd(n, *ptr));
                    ptr += step; ptr2 += step;
            } }
    } }

    return True;
}

/* ERROR FUNCTION NUMBER - 10 */
Bool arrInv(Array *a)
{
    Array lft = arrayZero, rht = arrayZero;
    int r = False;

    if(a->dim != 2 || a->grid[0].size != a->grid[1].size)
        return setError(E_NOT_DEF, 17.10, NULL);

    if(!arrayCopy(&lft, a)) return False;

    if(!arrayOne(&rht, a->grid[0].size)) {
        ARRFREE(&lft)
        return False;
    }

    if(arrayEulerDown(&lft, &rht) && arrayEulerUp(&lft, &rht)) {
        ARRFREE(a)
        *a = rht;
        r = True;
    }

    ARRFREE(&lft)
    if(!r) { ARRFREE(&rht)}

    return r;
}

int arrDotFind(const char* fn)
{
    int i = 0;

    while(arrDotList[i].func) {
        if(stringEqUp(fn, arrDotList[i].name))
            return i;
        i ++;
    }

    return -1;
}

Data* arrDotEval(int fn, Data *obj, Data *args, u_char narg)
{
    return arrDotList[fn].func(obj, args, narg);
}

Data* dotLength(Data* obj, Data* arg, u_char sz)
{
    Data* ans = getTmpData(1);
    if(!obj || !ans || sz != 0) return NULL;

    *ans = dataInt(obj->arrVal.length);

    return ans;
}

Data* dotDimention(Data* obj, Data* arg, u_char sz)
{
    int i;
    Data* ans = getTmpData(1);
    if(!obj || !ans) return NULL;

    if(sz == 0)
        *ans = dataInt(obj->arrVal.dim);
    else if(sz == 1 && arg->type == tNumber &&
            (i = asInt(arg->numVal)) >= 0 && i < obj->arrVal.dim)
    {
        *ans = dataInt(obj->arrVal.grid[i].size);
    }
    else
        return NULL;

    return ans;
}

Data* dotDeterm(Data* obj, Data* arg, u_char sz)
{
    Data* ans = getTmpData(1);
    if(!obj || !ans || sz != 0) return NULL;

    *ans = dataInt(0);

    return arrDet(&(obj->arrVal), &(ans->numVal)) ? ans : NULL;
}

Data* dotAdd(Data* obj, Data* arg, u_char sz)
{
    if(!obj || !arg || sz != 1 || arg->type != tArray) return NULL;

    return arrOpAdd(&(obj->arrVal), &(arg->arrVal), nSum) ? obj : NULL;
}

Data* dotSub(Data* obj, Data* arg, u_char sz)
{
    if(!obj || !arg || sz != 1 || arg->type != tArray) return NULL;

    return arrOpAdd(&(obj->arrVal), &(arg->arrVal), nSub) ? obj : NULL;
}

Data* dotProd(Data* obj, Data* arg, u_char sz)
{
    if(!obj || !arg || sz != 1) return NULL;

    if(arg->type == tNumber) {
        return arrOpMlt(&(obj->arrVal), arg->numVal, nProd) ? obj : NULL;
    }
    else if(arg->type == tArray) {
        return arrProd(&(obj->arrVal), &(arg->arrVal)) ? obj : NULL;
    }
    /* in other cases */
    return NULL;
}

Data* dotInv(Data* obj, Data* arg, u_char sz)
{
    if(!obj || sz != 0) return NULL;

    return arrInv(&(obj->arrVal)) ? obj : NULL;
}

Data* dotTrans(Data* obj, Data* arg, u_char sz)
{
    if(!obj || sz != 0) return NULL;

    return arrTranspose(&(obj->arrVal)) ? obj : NULL;
}

Data* dotFn(Data* obj, Data* arg, u_char sz)
{
    int pos, i;
    Number *n;
    if(!obj || !arg || arg->type != tText) return NULL;

    if((pos = getFunc(arg->txtVal.str)) == -1 || getFuncArg(pos) > 1)
        return NULL;

    n = obj->arrVal.numb;

    for(i = 0; i < obj->arrVal.length; i ++) {
        evalFunc(pos, n, n);
        n ++;
    }

    return obj;
}

void arrayPrint(Array a, int maxSize)
{
    int i, j, del = 0, k;

    if(a.dim > 2) {
        printf("%s\n", arrayText(a));
        return;
    }

    k = (a.dim == 2) ? a.grid[0].size : 1;

    for(i = 0; i < (a.dim == 2 ? a.grid[0].size : a.length); i ++) {
        if(i == maxSize) {
            printf("...");
            break;
        }
        for(j = 0, del = i; j < (a.dim == 2 ? a.grid[1].size : 1);
                                    j++, del += k) {
            if(j == maxSize) {
                printf("...");
                break;
            }
            printf("%-*s", TMP_NUMBER_SIZE+toll, numAsText(a.numb[del]));
        }
        printf("\n");
    }
}

char* arrayText(Array a)
{
    int i;
    char *p, *res = memTmpText(ARR_TMP_ONE);

    if(a.dim == 1)
        sprintf(res, "array[%d]", a.length);
    else {
        i = 0; p = res;
        strcpy(res, "array[");
        while(i < (int) a.dim) {
            while(*p) p++;
            sprintf(p, "%d%s", a.grid[i].size,
                                    (i == (a.dim-1)) ? "]" : "x");
            i ++;
        }
    }
    return res;
}
