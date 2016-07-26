/** @file vpb_data.c
~~~ Base Data ~~~
Realisation of functions for working with data */

/* ERROR FILE NUMBER - 2 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "vpb_data.h"
#include "vpb_error.h"


#define HASH1 1664525
#define HASH2 1013904223

/* in ASCII 'a' - 'A' = 32 */
#define UPPER(D) ((D) < 'a' ? (D) : (D)-32) /* here not used symbols upper 'z' */

/* Clear memory.
   Data have to be allready initialized !!! */
void dataClear(Data *d)
{
    if(d) switch(d->type) {
        case tArray:
            IFFREE(d->arrVal.numb);
            d->arrVal.numb = NULL;
            d->arrVal.length = 0;
            IFFREE(d->arrVal.grid);
            d->arrVal.dim = 1;
            d->arrVal.grid = NULL;
            break;
        case tText:
            IFFREE(d->txtVal.str);
            d->txtVal.str = NULL;
            d->txtVal.length = 0;
            break;
        case tFree:
            /*  */
            if(d->freeVal.root || d->freeVal.del) {
                if(d->freeVal.key == KEY_FILE) {
                    if(d->freeVal.ptr)  fclose((FILE*) d->freeVal.ptr);
                }
                else {
                    IFFREE(d->freeVal.ptr);
                }
            }
            d->freeVal.ptr = NULL;
            d->freeVal.size = 0;
            break;
        default:
            break;
    }
}

/* Create array with given length, fill 0 */
Bool dataInitArray(Data *d, int size)
{
    /* ERROR: 02 */
    Number* ptr = NULL;

    d->type = tArray;
    d->arrVal.length = size;
    d->arrVal.numb = NULL;
    d->arrVal.dim = 1;
    d->arrVal.grid = NULL;
    /* get memory */
    if(size > 0) {
        ptr = (Number *) calloc(size, sizeof(Number));
        if(!ptr) {
            d->arrVal.length = 0;
            _CRITICAL(NULL)
        }
        d->arrVal.numb = ptr;
    }

    return True;
}


/* Get string copy */
char* stringCopy(const char* str, int *len)
{
    char *copy;
    int length;

    if(!str) return NULL;

    length = (int) strlen(str) + 1;

    _CRITICAL(copy = (char*) malloc( length ))

    strcpy(copy, str);
    if(len) *len = length;

    return copy;
}

/* Copy string and convert to upper register */
char* stringCopyUp(const char* str, int *len)
{
    char* copy, *c;

    copy = stringCopy(str, len);
    if(copy) {
        c = copy;
        while(*c) { *c = toupper(*c);    c++; }
    }

    return copy;
}

Bool stringSet(Data* d, const char* s)
{
    /* ERROR: 22 */
    int len;
    char* ptr;

    if(!d || !s || d->type != tText)
        return setError(E_WRONG_ARG, 2.22, NULL);

    len = (int) strlen(s) + 1;

    if(len > d->txtVal.length) {
        _CRITICAL(ptr = (char*) malloc(len))

        free(d->txtVal.str);
        d->txtVal.str = ptr;
        d->txtVal.length = len;
    }

    return strcpy(d->txtVal.str, s) != NULL;
}

Bool stringAdd(Data* d, const char *s)
{
    /* ERROR: 23 */
    int len;
    char* ptr;

    if(!d || !s || d->type != tText)
        return setError(E_WRONG_ARG, 2.23, NULL);

    len = (int) (strlen(s) + 1 + (d->txtVal.str ? strlen(d->txtVal.str) : 0));

    if(d->txtVal.length < len) {
        _CRITICAL(ptr = (char*) realloc(d->txtVal.str, len))
        d->txtVal.str = ptr;
        d->txtVal.length = len;
    }

    return strcat(d->txtVal.str, s) != NULL;
}

/* Find hash code of string */
unsigned int stringHash(const char* str)
{
    /* Ly */
    register unsigned int hash = 0;

    for(;*str; str++) {
        hash = (hash * HASH1) + (u_char) (*str) + HASH2;
    }

    return hash;
}

/* Find hash code for upper register */
unsigned int stringHashUp(const char* str)
{
    register unsigned int hash = 0;

    for(;*str; str++) {
        hash = (hash * HASH1) + (u_char) toupper(*str) + HASH2;
    }

    return hash;
}

/* Initialuze integer */
Data dataInt(long a)
{
    Data res;

    res.type = tNumber;
    res.numVal.type = nINT;
    res.numVal.iVal = (inumber) a;

    return res;
}

/* Initialize float point */
Data dataDbl(double a)
{
    Data res;

    res.type = tNumber;
    res.numVal.type = nDBL;
    res.numVal.dVal = (dnumber) a;

    return res;
}

/* Initialize complex number */
Data dataCmp(float re, float im)
{
    Data res;

    res.type = tNumber;
    res.numVal.type = nCMP;
    res.numVal.cVal.re = (cnumber) re;
    res.numVal.cVal.im = (cnumber) im;

    return res;
}

/* Initialize bool value */
Data dataBool(u_char a)
{
    Data res;

    res.type = tNumber;
    res.numVal = (a == 0) ? constZero : constOne;
    return res;
}

/* Initialize with string (only pointer) */
Data regDataText(char* txt)
{
    Data res;

    res.type = tText;
    res.txtVal.str = txt;
    //res.txtVal.length = strlen(txt)+1;
    //res.txtVal.arr[0] = '\0';

    return res;
}

/* Initialize structure with free type */
Data dataFree(u_char k, u_char useCopy, u_char useDel)
{
    Data res;

    res.type = tFree;
    res.freeVal.key = k;
    res.freeVal.root = 1;
    res.freeVal.copy = useCopy;
    res.freeVal.del = useDel;
    res.freeVal.ptr = NULL;

    return res;
}

/* Initialize structure with zero length array */
Data dataArrayNull(void)
{
    Data res;

    res.type = tArray;
    res.arrVal.length = 0;
    res.arrVal.numb = NULL;
    res.arrVal.grid = NULL;
    res.arrVal.dim = 1;

    return res;
}

Data dataStringNull(void)
{
    Data res;

    res.type = tText;
    res.txtVal.length = 0;
    res.txtVal.str = NULL;
    res.txtVal.arr[0] = '\0';

    return res;
}


/* Change size according given data */
Bool resizeData(Data *d, Data src)
{
    /* ERROR: 06 */
    axe* ptr = NULL;
    int i;

    if(!d || d->type != tArray || src.type != tArray)
        return setError(E_WRONG_ARG, 2.06, NULL);

    if(!dataArrayEq(d->arrVal, src.arrVal)) {
        /* resize array, new dim = 1 */
        if(!resizeArray(&(d->arrVal), src.arrVal.length))
            return False;
        /* new grid */
        if(src.arrVal.dim > 1) {
            _CRITICAL(ptr = (axe*) malloc(sizeof(axe) * src.arrVal.dim))

            for(i = 0; i < src.arrVal.dim; i ++) {
                ptr[i] = src.arrVal.grid[i];
            }
        }
        /* correct dimmentio */
        d->arrVal.dim = src.arrVal.dim;
        d->arrVal.grid = ptr;
    }

    return True;
}


/* Copy data */
Bool dataCopy(Data *d, Data src)
{
    /* ERROR: 07 */
    if(!d) return setError(E_WRONG_ARG, 2.07, NULL);

    switch(src.type) {
    case tArray:

        if(d->type != tArray) {
            dataClear(d);
            *d = dataArrayNull();
        }

        if(!resizeData(d, src)) return False;
        memcpy((void*) d->arrVal.numb, (void*) src.arrVal.numb,
                                    sizeof(Number) * src.arrVal.length);
/*
        for(i = 0; i < src.arrVal.length; i ++) {
            d->arrVal.numb[i] = src.arrVal.numb[i];
        }
*/
        break;
    case tText:
        /* prepare */
        if(d->type != tText)  {
            dataClear(d);
            *d = dataStringNull();
        }

        return stringSet(d, src.txtVal.str);
    case tFree:
        *d = src;
        d->freeVal.root = 0;

        if(src.freeVal.copy) {
            if(src.freeVal.del) {
                if(src.freeVal.size > 0) {
                    _CRITICAL(d->freeVal.ptr = malloc(src.freeVal.size))
                    memcpy(d->freeVal.ptr, src.freeVal.ptr, src.freeVal.size);
                }
                else
                    d->freeVal.ptr = NULL;

                d->freeVal.size = src.freeVal.size;
            }
            else
                d->freeVal.ptr = src.freeVal.ptr;
        }
        else
            d->freeVal.ptr = NULL;

        break;
    default:
        *d = src;
    }

    return True;
}



/* Compare two strings */
Bool stringEqUp(const char* str1, const char* str2)
{
    if(!str1 || !str2) return str1 == str2;
/*    // any coding
    while(*str1 && *str2) {
        if(toupper(*str1++) != toupper(*str2++)) return False;
    }
*/
    /* expected ASCII */
    for(;*str1 && *str2; str1++, str2++) {
        if(UPPER(*str1) != UPPER(*str2))
            return False;
    }

    /* check both string get end */
    return *str1 == *str2;
}


/* Represent data as text */
char* dataAsText(Data d)
{
    char *res = NULL;

    switch(d.type) {
    case tText:
        return d.txtVal.str;
    case tNumber:
        return numAsText(d.numVal);
    case tArray:
        return arrayText(d.arrVal);
    case tFree:
        //res = arrBuf();
        res = memTmpText(ARR_TMP_ONE);
        sprintf(res, "free[%d]", (int) d.freeVal.key);
        break;
    }

    return res;
}

#if 0
/* Get number from indexed position */
/* ERROR FUNCTION NUMBER - 11 */
Number* arrayNumInd(Data* d, int x, ...)
{
    int j, pos, ind;
    va_list ap;

    if(!d) {
        setError(E_WRONG_ARG, 2.11, NULL);
        return NULL;
    }

    if(d->type != tArray) {
        setError(E_WRONG_TYPE, 2.11, NULL);
        return NULL;
    }

    /* get arguments */
    va_start(ap, x);

    pos = x * d->arrVal.grid[0].coeff;

    for(j = 1; j < (int) d->arrVal.dim; j ++) {
        ind = va_arg(ap, int);
        /* check value */
        if(d->arrVal.grid[j].size <= ind || ind < 0) {
            setError(E_WRONG_INDEX, 2.11, NULL);
            va_end(ap);
            return NULL;
        }
        else
            pos += ind * d->arrVal.grid[j].coeff;
    }
    /* exit */
    va_end(ap);

    return d->arrVal.numb + pos;
}
#endif

#if 0
/* Change size according index */
Bool resizeIndex(Data *d, int dim, ...)
{
    /* ERROR: 12 */
    int i, ind, k = 1;
    va_list ap;
    axe* newAxe;

    if(!d || dim <= 0 || d->type != tArray)
        return setError(E_WRONG_ARG, 2.12, NULL);
    /* create new grid */
    newAxe = (axe*) calloc(dim, sizeof(axe));
    if(!newAxe) return setError(E_NO_MEMORY, 2.12, NULL);

    va_start(ap, dim);
    /* get arguments */
    for(i = 0; i < dim; i ++) {
        ind = va_arg(ap, int);
        if(ind <= 0) {
            free(newAxe);
            va_end(ap);
            return setError(E_WRONG_INDEX, 2.12, NULL);
        }
        newAxe[i].size = ind;
        newAxe[i].coeff = k;
        k *= ind;  /* last value is array size */
    }
    /* chenge array length */
    if(!resizeArray(&(d->arrVal), k)) {
        free(newAxe);
        va_end(ap);
        return False;
    }

    d->arrVal.dim = dim;
    d->arrVal.grid = newAxe;

    va_end(ap);

    return True;
}
#endif

/* Return array (or number) length */
int arraySize(Data d)
{
    return d.type == tArray ? d.arrVal.length : (d.type == tNumber ? 1 : -1);
}

/* Return array (or number) dimention number */
int arrayDim(Data d)
{
    return d.type == tArray ? d.arrVal.dim : (d.type == tNumber ? 1 : -1);
}
#if 0
/* Return size along given axe (array or number) */
int arrayDimSize(Data d, int i)
{
    /* ERROR: 16 */
    int ans = -1;

    if(i < 0) {
        setError(E_WRONG_ARG, 2.16, NULL);
        return -1;
    }
    /* check type and get size */
    if(d.type == tArray) {
        if(d.arrVal.dim == 1)
            ans = (i == 0) ? d.arrVal.length : 0;
        else
            ans = (i < d.arrVal.dim) ? d.arrVal.grid[i].size : 0;
    }
    else if(d.type == tNumber)
        ans = (i == 0) ? 1 : 0;

    return ans;
}
#endif

/* Find index from array of numbers */
Bool parseEltIndex(Data *d, Number *n, int sz, int* ndx)
{
    int res = 0, pos, i;

    if(!d || sz == 0) return False;

    switch(d->type) {
    case tArray: /* go below */
        break;
    case tNumber:
        /* have had 1 argument equial to 0 */
        if(sz == 1 && asInt(n[0]) == 0) {
            *ndx = 0;
            return True;
        }
        else
            return False;
        break;
    default:
        return False;
    }

    if(sz == 1) {
        /* one dimentin */
        pos = asInt(n[0]);
        if(pos < 0 || pos >= d->arrVal.length)
            return False;
    }
    else if(sz == (int) d->arrVal.dim) {
        /* check index and find position */
        for(i = 0; i < sz; i ++) {
            pos = asInt(n[i]);
            if(pos < d->arrVal.grid[i].size)
                res += pos * d->arrVal.grid[i].coeff;
            else
                return False;
        } }
    else
        return False;
    /* set position */
    *ndx = pos;

    return True;
}

/* Iterator */
Number* getNext(Data* d)
{
    Number *n = NULL;

    switch(d->type) {
    case tArray:
        if(d->iter < d->arrVal.length) {
            n = d->arrVal.numb + d->iter;
            d->iter ++;
        }
        else
            d->iter = 0;
        break;
    case tNumber:
        if(d->iter == 0) {
            n = &(d->numVal);
            d->iter ++;
        }
        else
            d->iter = 0;
        break;
    default:
        break;
    }

    return n;
}

/* Iterator reset */
void iReset(Data *d)
{
    d->iter = 0;
}

#if 0
/* ERROR FUNCTION NUMBER - 17 */
Bool arrayAddNum(Data *d, Number n)
{
    int sz;

    if(d && d->type == tArray) {
        /* initial length (and new index) */
        sz = d->arrVal.length;
        if(resizeArray(&(d->arrVal), sz + 1)) {
            d->arrVal.numb[sz] = n;
        }
        else
            return False;
    }
    else
        return setError(E_WRONG_ARG, 2.17, NULL);

    return True;
}
#endif


Bool dataSum(Data *res, Data d)
{
    /* ERROR: 18 */
    switch(res->type) {
    case tText:
        return stringAdd(res, dataAsText(d));
    case tArray:
        switch(d.type) {
        case tArray:
            return arrSum(&(res->arrVal), &(d.arrVal));
        case tNumber:
            if(res->arrVal.length == 1) {
                res->arrVal.numb[0] = nSum(res->arrVal.numb[0], d.numVal);
                break;
            }
        default:
            return setError(E_DIFF_TYPE, 2.18, NULL);
        }
        break;
    case tNumber:
        switch(d.type) {
        case tNumber:
            res->numVal = nSum(res->numVal, d.numVal);
            break;
        case tArray:
            if(d.arrVal.length == 1) {
                res->numVal = nSum(res->numVal, d.arrVal.numb[0]);
                break;
            }
        default:
            return setError(E_DIFF_TYPE, 2.18, NULL);
        }
        break;
    default:
        return setError(E_WTF, 2.18, "not defined");
    }

    return True;
}


Number* N(Data* d, int pos)
{
    /* ERROR: 19 */
    Number *res = NULL;

    switch(d->type) {
    case tArray:
        if(pos >= d->arrVal.length)
            fatalError(E_WRONG_INDEX, 2.19, NULL, F_ARITHMETIC);
        res = d->arrVal.numb + pos;
        break;
    case tNumber:
        res = &(d->numVal);
        break;
    default:
        fatalError(E_WRONG_TYPE, 2.19, NULL, F_ARITHMETIC);
    }

    return res;
}

void arrayFill(Data *d, Number src)
{
    int i;
    if(d && d->type == tArray) {
        for(i = 0; i < d->arrVal.length; i ++)
            d->arrVal.numb[i] = src;
    }
}


Bool resizeToArray(Data *d, Number *indLst, u_char dim)
{
    /* ERROR: 20 */
    int newSize = 1, i, val;
    axe *aPtr = NULL;

    if(dim == 0 || !d || !indLst)
        return setError(E_WRONG_ARG, 2.20, NULL);
    /* count new size */
    for(i = 0; i < (int) dim; i ++) {
        if((val = asInt(indLst[i])) <= 0) return setError(E_WRONG_INDEX, 2.20, NULL);
        newSize *= val;
    }
    /* resize */
    if(!resizeArray(&(d->arrVal), newSize)) return False;
    /* axes */
    if(dim > 1) {
        newSize = 1;
        _CRITICAL(aPtr = (axe*) malloc(dim*sizeof(axe)))

        for(i = 0; i < (int) dim; i ++) {
            val = asInt(indLst[i]);
            aPtr[i].size = val;
            aPtr[i].coeff = newSize;
            newSize *= val;
        }

        d->arrVal.dim = dim;
        d->arrVal.grid = aPtr;
    }

    return True;
}


Number* arrayNumPos(Data *d, Number *iLst, u_char sz)
{
    /* ERROR: 21 */
    int pos = 0, i, val;

    if(!d || !iLst || d->type != tArray)
        return (void*) setError(E_WRONG_ARG, 2.21, NULL);

    if(sz == 1) {
        if(d->arrVal.dim == 1 ||
            (d->arrVal.dim == 2 && (d->arrVal.grid[0].size == 1 ||
                                    d->arrVal.grid[1].size == 1))) {

            val = asInt(iLst[0]);
            if(val < 0) val += d->arrVal.length;
            if(val < 0 || val >= d->arrVal.length)
                return (void*) setError(E_WRONG_INDEX, 2.21, NULL);

            return d->arrVal.numb + val;
    } }

    if(sz != d->arrVal.dim)
        return (void*) setError(E_WRONG_ARG, 2.21, NULL);

    for(i = 0; i < (int) sz; i ++) {
        val = asInt(iLst[i]);
        if(val < 0) val += d->arrVal.grid[i].size;

        if(val < 0 || val >= d->arrVal.grid[i].size)
            return (void*) setError(E_WRONG_INDEX, 2.21, NULL);

        pos += val * (sz > 1 ? d->arrVal.grid[i].coeff : 1);
    }

    return d->arrVal.numb+pos;
}
