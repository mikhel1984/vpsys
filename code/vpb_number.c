/** @file vpb_number.c
~~~ Base Number ~~~
Functions for working with numbers */

/* ERROR FILE NUMBER - 7 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "vpb_number.h"
#include "vpb_error.h"
#include "vpb_data.h"
#include "vpb_memory.h"

#define C99_MACRO /* for isfinite() */
#define LESS(X,T) (X > -(T) && X < (T))
#define SQ(X)     ((X).cVal.re*(X).cVal.re + (X).cVal.im*(X).cVal.im)
#define ISOVER(X) (!((X)==(X))) /* for float point numbers */

unsigned int toll = 4;

Bool ratPrint = False;
/* Number constants */
Number constZero = {nINT,{0}}, constOne = {nINT, {1}}, constIm;
/*constPi = {nDBL,{3.1416}}, constE = {nDBL, {2.7183}},*/



/** Get product of complex numbers
    @fn cmpProd
    @param a - multiplier
    @param b - multiplier
    @return Complex a*b */
Number cmpProd(Number a, Number b);
/** Get ration of complex numbers
    @fn cmpRat
    @param a - dividend
    @param b - divisor
    @return Complex a/b */
Number cmpRat(Number a, Number b);
/** Correct type for mathematical operations
    @fn nCorrectType
    @param a - pointer to first number
    @param b - pointer to second number */
void nCorrectType(Number *a, Number *b);

double nDblRand(double x, unsigned int seed);

Number cmp2trig(Number c);
Number trig2cmp(Number c);



Number powInt(Number n, inumber i);
Number powDbl(Number n, dnumber d);
Number powCmp(Number n, Number c);

Bool isOverSum(long int a, long int b);
Bool isOverProd(long int a, long int b);

rnumber nGCD(rnumber a, rnumber b);

void nRSimp(Number *n);
void nRToDenom(Number *a, Number *b);

/* Representation of standard functions for evaluation if interpretator */
Number _nSin(Number*), _nExp(Number*), _nAbs(Number*), _nRand(Number *),
       _nPow(Number *), _nPow2(Number *n), _nLn(Number *n), _nCos(Number *),
       _nSqrt(Number *), _nTan(Number *);

/** @struct FuncLst
List of functions for evaluation in interpretator */
struct FuncLst
{
    char* name;               /**< Function name */
    u_char narg;              /**< Number of arguments */
    Number (*func)(Number *); /**< Pointer to internal representation */
}
fList[] =
{{fSIN,1,_nSin}, {fEXP,1,_nExp}, {fABS,1,_nAbs}, {fRND, 1, _nRand},
 {fPOW,2,_nPow}, {fPOW2,1,_nPow2}, {fLN,1,_nLn}, {fCOS, 1, _nCos},
 {fSQRT, 1, _nSqrt}, {fTAN, 1, _nTan},
 /* end */ {"",0,NULL}};

/* n1+n2 */
Number nSum(Number n1, Number n2)
{
    nCorrectType(&n1, &n2);

    switch(n1.type) {
    case nDBL:
        n1.dVal += n2.dVal;
        break;
    case nCMP:
        n1.cVal.re += n2.cVal.re;
        n1.cVal.im += n2.cVal.im;
        break;
    case nINT:
        if(isOverSum(n1.iVal, n2.iVal)) {
            /* convert to double, add */
            transformDbl(&n1, (dnumber) n1.iVal);
            n1.dVal += (dnumber) n2.iVal;
        }
        else
            n1.iVal += n2.iVal;
        break;
    case nRAT:
        nRToDenom(&n1, &n2);
        n1.rVal.num += n2.rVal.num;
        nRSimp(&n1);
        if(n1.rVal.denom == 1)
            transformInt(&n1, (inumber) n1.rVal.num);
        break;
    }

    return n1;
}

/* n1-n2 */
Number nSub(Number n1, Number n2)
{
    nCorrectType(&n1, &n2);

    switch(n1.type) {
    case nDBL:
        n1.dVal -= n2.dVal;
        break;
    case nCMP:
        n1.cVal.re -= n2.cVal.re;
        n1.cVal.im -= n2.cVal.im;
        break;
    case nINT:
        if(isOverSum(n1.iVal, -n2.iVal)) {
            transformDbl(&n1, (dnumber) n1.iVal);
            n1.dVal -= (dnumber) n2.iVal;
        }
        else
            n1.iVal -= n2.iVal;
        break;
    case nRAT:
        nRToDenom(&n1, &n2);
        n1.rVal.num -= n2.rVal.num;
        nRSimp(&n1);
        if(n1.rVal.denom == 1)
            transformInt(&n1, (inumber) n1.rVal.num);
        break;
    }

    return n1;
}

/* n1*n2 */
/* ERROR FUNCTION NUMBER - 3 */
Number nProd(Number n1, Number n2)
{
    nCorrectType(&n1, &n2);

    switch(n1.type) {
    case nDBL:
        n1.dVal *= n2.dVal;
        if(ISOVER(n1.dVal)) fatalError(E_NOVERFLOW, 7.03, NULL, F_ARITHMETIC);
        break;
    case nINT:
        if(isOverProd(n1.iVal, n2.iVal)) {
            transformDbl(&n1, (dnumber) n1.iVal);
            n1.dVal *= (dnumber) n2.iVal;
        }
        else
            n1.iVal *= n2.iVal;
        break;
    case nCMP:
        n1 = cmpProd(n1, n2);
        if(ISOVER(n1.cVal.re) || ISOVER(n1.cVal.im))
            fatalError(E_NOVERFLOW, 7.03, NULL, F_ARITHMETIC);
        break;
    case nRAT:
        /* check overvlow !!! */
        n1.rVal.num *= n2.rVal.num;
        n1.rVal.denom *= n2.rVal.denom;
        nRSimp(&n1);
        if(n1.rVal.denom == 1)
            transformInt(&n1, (inumber) n1.rVal.num);
        break;
    }

    return n1;
}

/* n1/n2 */
/* ERROR FUNCTION NUMBER - 1 */
Number nRat(Number n1, Number n2)
{
    nCorrectType(&n1, &n2);

    switch(n1.type) {
    case nDBL:
        if(n2.dVal == 0) fatalError(E_ZERO_DEVIDE, 7.01, NULL, F_ARITHMETIC);
        n1.dVal /= n2.dVal;
        if(ISOVER(n1.dVal)) fatalError(E_NOVERFLOW, 7.01, NULL, F_ARITHMETIC);
        break;
    case nINT:
        if(n2.iVal == 0) fatalError(E_ZERO_DEVIDE, 7.01, NULL, F_ARITHMETIC);
        /* if possible set result as integer */
        if(n1.iVal % n2.iVal == 0)
            n1.iVal /= n2.iVal;
        else {
            /* transform to rational number */
            transformRat(&n1, (rnumber) n1.iVal, (rnumber) n2.iVal);
            nRSimp(&n1);
            /* if too big - ? */
        }
        break;
    case nCMP:
        if(n2.cVal.re == 0 && n2.cVal.im == 0) fatalError(E_ZERO_DEVIDE,
                                                    7.01, NULL, F_ARITHMETIC);
        n1 = cmpRat(n1, n2);
        if(ISOVER(n1.cVal.re) || ISOVER(n1.cVal.im))
            fatalError(E_NOVERFLOW, 7.01, NULL, F_ARITHMETIC);
        break;
    case nRAT:
        if(n2.rVal.num == 0) fatalError(E_ZERO_DEVIDE, 7.01, NULL, F_ARITHMETIC);
        n1.rVal.num *= n2.rVal.denom;
        n1.rVal.denom *= n2.rVal.num;
        nRSimp(&n1);
        if(n1.rVal.denom == 1)
            transformInt(&n1, (inumber) n1.rVal.num);
        break;
    }

    return n1;
}

/* Complex a*b */
Number cmpProd(Number a, Number b)
{
    Number res;

    res.type = nCMP;

    res.cVal.re = a.cVal.re*b.cVal.re - a.cVal.im*b.cVal.im;
    res.cVal.im = a.cVal.re*b.cVal.im + a.cVal.im*b.cVal.re;

    return res;
}

/* Complex a/b */
Number cmpRat(Number a, Number b)
{
    Number res;
    cnumber denom = SQ(b) /* b.cVal.re*b.cVal.re + b.cVal.im * b.cVal.im */;

    res.type = nCMP;

    res.cVal.re = (a.cVal.re*b.cVal.re + a.cVal.im*b.cVal.im) / denom;
    res.cVal.im = (b.cVal.re*a.cVal.im - a.cVal.re*b.cVal.im) / denom;

    return res;
}

/* Transform number to complex */
void transformCmp(Number* x, cnumber re, cnumber im)
{
    x->type = nCMP;
    x->cVal.re = re;
    x->cVal.im = im;
}

/* Transform number to float point */
void transformDbl(Number* x, dnumber a)
{
    x->type = nDBL;
    x->dVal = a;
}

/* Transform number to integer */
void transformInt(Number *x, inumber a)
{
    x->type = nINT;
    x->iVal = a;
}

void transformRat(Number *x, rnumber n, rnumber d)
{
    x->type = nRAT;
    x->rVal.num = (d > 0) ? n : -n;
    x->rVal.denom = (d > 0) ? d : -d;
}

/* Return char string with number */
char* numAsText(Number a)
{
    char* ptr = (a.type == nCMP) ? memTmpText(2*(TMP_NUMBER_SIZE+toll)) :
                                    memTmpText(TMP_NUMBER_SIZE+toll);

    switch(a.type) {
    case nDBL:
        sprintf(ptr, "%.*g", toll, a.dVal);
        break;
    case nINT:
        if(a.iVal > 999999 || a.iVal < -999999)
            sprintf(ptr, "%.*e", toll, (double) a.iVal);
        else
            sprintf(ptr, "%ld", a.iVal);
        break;
    case nCMP:
        if(a.cVal.im >= 0)
            sprintf(ptr, "%.*g+i*%.*g", toll, a.cVal.re, toll, a.cVal.im);
        else
            sprintf(ptr, "%.*g-i*%.*g", toll, a.cVal.re, toll, -a.cVal.im);
        break;
    case nRAT:
        sprintf(ptr, "%.*g", toll, ((double)a.rVal.num )/ a.rVal.denom);
        break;
    }

    return ptr;
}

char* numAsTextFull(Number a)
{
    char* ptr = memTmpText(3*TMP_NUMBER_SIZE);

    switch(a.type) {
    case nDBL:
        sprintf(ptr, "%g", a.dVal);
        break;
    case nINT:
        sprintf(ptr, "%ld", a.iVal);
        break;
    case nCMP:
        if(a.cVal.im >= 0)
            sprintf(ptr, "%g+i*%g" , a.cVal.re, a.cVal.im);
        else
            sprintf(ptr, "%g-i*%g" , a.cVal.re, -a.cVal.im);
        break;
    case nRAT:
        sprintf(ptr, "%d/%d", a.rVal.num, a.rVal.denom);
        break;
    }

    return ptr;
}


/* Get number from integer */
Number intToNumber(long a)
{
    Number res;

    res.type = nINT;
    res.iVal = (inumber) a;

    return res;
}

/* Get number from float point */
Number dblToNumber(double a)
{
    Number res;

    res.type = nDBL;
    res.dVal = (dnumber) a;

    return res;
}

/* Get number with real and imaginary */
Number cmpToNumber(float r, float i)
{
    Number res;

    res.type = nCMP;
    res.cVal.re = (cnumber) r;
    res.cVal.im = (cnumber) i;

    return res;
}

/* Get number from bool */
Number nBool(Bool val)
{
    Number res;

    res.type = nINT;
    res.iVal = (inumber) val;

    return res;
}

/* Chenge sign of number */
void numInvert(Number* n)
{
    switch(n->type) {
    case nDBL:
        n->dVal = - n->dVal;
        break;
    case nCMP:
        n->cVal.re = - n->cVal.re;
        n->cVal.im = - n->cVal.im;
        break;
    case nINT:
        n->iVal = - n->iVal;
        break;
    case nRAT:
        n->rVal.num = - n->rVal.num;
        break;
    }
}
/* Compare value  */
/* ERROR FUNCTION NUMBER - 2 */
Bool nCompare(Number n1, Number n2, numComp cp)
{
    Bool res = True;

    /* use threshold - ??? */

    switch(cp) {
    case EQ:
        /* find complex numbers */
        if((n1.type == nCMP && n1.cVal.im != 0) ||
            (n2.type == nCMP && n2.cVal.im != 0))
        {
            /* compare complex and not complex */
            if(n1.type != n2.type)
                res = False;
            else /* both are complex */
                res = (n1.cVal.re == n2.cVal.re && n1.cVal.im == n2.cVal.im);
        }
        else
            res = (real_part(n1) == real_part(n2));
        break;
    case NE:
        /* for complex numbers */
        if((n1.type == nCMP && n1.cVal.im != 0) ||
            (n2.type == nCMP && n2.cVal.im != 0))
        {
            /* different types */
            if(n1.type != n2.type)
                res = True;
            else /* both are complex */
                res = (n1.cVal.re != n2.cVal.re || n1.cVal.im != n2.cVal.im);
        }
        else
            res = (real_part(n1) != real_part(n2));
        break;
    case LT: /* lighte */
        if(n1.type == nCMP || n2.type == nCMP)
            res = setError(E_NOT_DEF, 7.02, NULL);
        else
            res = (real_part(n1) < real_part(n2));
        break;
    case LE: /* lighte or equial */
        if(n1.type == nCMP || n2.type == nCMP)
            res = setError(E_NOT_DEF, 7.02, NULL);
        else
            res = (real_part(n1) <= real_part(n2));
        break;
    case GT: /* greate */
        if(n1.type == nCMP || n2.type == nCMP)
            res = setError(E_NOT_DEF, 7.02, NULL);
        else
            res = (real_part(n1) > real_part(n2));
        break;
    case GE: /* greate or equiql */
        if(n1.type == nCMP || n2.type == nCMP)
            res = setError(E_NOT_DEF, 7.02, NULL);
        else
            res = (real_part(n1) >= real_part(n2));
        break;
    }

    return res;
}

/* Chck equivalence to 0 */
Bool isZero(Number n)
{
    return (n.type == nINT && n.iVal == 0)
            || (n.type == nRAT && n.rVal.num == 0)
            || (n.type == nDBL && LESS(n.dVal, 1e-10))
            || (n.type == nCMP && LESS(n.cVal.re, 1e-10)
                                && LESS(n.cVal.im, 1e-10));
}

/* Get real part as double */
double real_part(Number n)
{
    double ans;

    switch(n.type) {
    case nDBL:
        ans = (double) n.dVal;
        break;
    case nCMP:
        ans = (double) n.cVal.re;
        break;
    case nINT:
        ans = (double) n.iVal;
        break;
    case nRAT:
        ans = ((double) n.rVal.num) / n.rVal.denom;
        break;
    }

    return ans;
}

/* Get imaginary part as double */
double imag_part(Number n)
{
    double ans = 0;

    if(n.type == nCMP) ans = (double) n.cVal.im;

    return ans;
}

/* Logical AND */
Number nAnd(Number n1, Number n2)
{
    Number res;

    res.type = nINT;
    /* equial: !isZero(n1) && !isZero(n2) */
    res.iVal = !(isZero(n1) || isZero(n2));

    return res;
}

/* Logical OR */
Number nOr(Number n1, Number n2)
{
    Number res;

    res.type = nINT;
    /* equial: !isZero(n1) || !isZero(n2) */
    res.iVal = !(isZero(n1) && isZero(n2));

    return res;
}

/* Logical NOT */
Number nNot(Number n)
{
    Number res;

    res.type = nINT;
    res.iVal = isZero(n) ? 1 : 0;

    return res;
}

/* Module */
Number nAbs(Number n)
{
    switch(n.type) {
    case nINT:
        if(n.iVal < 0) n.iVal = -n.iVal;
        break;
    case nDBL:
        if(n.dVal < 0) n.dVal = -n.dVal;
        break;
    case nCMP:
        transformDbl(&n, (dnumber) sqrt(SQ(n)));
        break;
    case nRAT:
        if(n.rVal.num < 0) n.rVal.num = - n.rVal.num;
        if(n.rVal.denom < 0) n.rVal.denom = -n.rVal.denom;
    }

    return n;
}

/* Exponente */
Number nExp(Number n)
{
    switch(n.type) {
    case nDBL:
        n.dVal = (dnumber) exp(n.dVal);
        break;
    case nINT:
        transformDbl(&n, exp(n.iVal));
        break;
    case nCMP:
        transformCmp(&n, (cnumber) exp(n.cVal.re)*cos(n.cVal.im),
                    (cnumber) exp(n.cVal.re)*sin(n.cVal.im));
        /* maybe have to use -i*sin(b), i.e. b is positive always ? */
        break;
    case nRAT:
        transformDbl(&n, exp(((double)n.rVal.num)/n.rVal.denom));
        break;
    }

    return n;
}

/* Sinus */
Number nSin(Number n)
{
    switch(n.type) {
    case nDBL:
        n.dVal = (dnumber) sin(n.dVal);
        break;
    case nINT:
        transformDbl(&n, (dnumber) sin(n.iVal));
        break;
    case nCMP:
        transformCmp(&n, (cnumber) sin(n.cVal.re) * cosh(n.cVal.im),
                        (cnumber) cos(n.cVal.re) * sinh(n.cVal.im));
        break;
    case nRAT:
        transformDbl(&n, (dnumber) sin(((double)n.rVal.num)/n.rVal.denom));
        break;
    }

    return n;
}

Number nCos(Number n)
{
    switch(n.type) {
    case nDBL:
        n.dVal = (dnumber) cos(n.dVal);
        break;
    case nINT:
        transformDbl(&n, (dnumber) cos(n.iVal));
        break;
    case nCMP:
        transformCmp(&n, (cnumber) cos(n.cVal.re) * cosh(n.cVal.im),
                        (cnumber) (-1)*sin(n.cVal.re) * sinh(n.cVal.im));
        break;
    case nRAT:
        transformDbl(&n, (dnumber) cos(((double)n.rVal.num)/n.rVal.denom));
        break;
    }

    return n;
}

Number nSqrt(Number n)
{
    switch(n.type) {
    case nDBL:
        n.dVal = (dnumber) sqrt(n.dVal);
        break;
    case nINT:
        transformDbl(&n, (dnumber) sqrt((double) n.iVal));
        break;
    case nCMP:
        n = cmp2trig(n);
        n.cVal.re = (cnumber) sqrt((double) n.cVal.re);
        n.cVal.im /= 2;
        n = trig2cmp(n);
        break;
    case nRAT:
        transformDbl(&n, (dnumber) sqrt(((double) n.rVal.num)/n.rVal.denom));
        break;
    }

    return n;
}

/* ERROR FUNCTION NUMBER - 6 */
Number nTan(Number n)
{
    Number den = nCos(n);

    if(isZero(den)) fatalError(E_ZERO_DEVIDE, 7.06, NULL, F_ARITHMETIC);

    n = nSin(n);

    return nRat(n, den);
}

/* Representation of functions for interpretation */
Number _nSin(Number *n) { return nSin(n[0]); }
Number _nExp(Number *n) { return nExp(n[0]); }
Number _nAbs(Number *n) { return nAbs(n[0]); }
Number _nRand(Number *n) { return nRand(n[0]); }
Number _nPow(Number *n) { return nPow(n[0], n[1]); }
Number _nPow2(Number *n) { return nPow2(n[0]); }
Number _nLn(Number *n) { return nLn(n[0]); }
Number _nCos(Number *n) { return nCos(n[0]); }
Number _nSqrt(Number *n) { return nSqrt(n[0]); }
Number _nTan(Number *n) { return nTan(n[0]); }

/* Find function in list */
int getFunc(const char* nm)
{
    int i = 0, pos = -1;

    while(fList[i].func) {
        if(stringEqUp(nm, fList[i].name)) {
            pos = i;
            break;
        }
        i ++;
    }

    return pos;
}

/* Get argument number */
u_char getFuncArg(int pos)  {  return fList[pos].narg;  }

/* Evaluate function */
Bool evalFunc(int pos, Number *args, Number *res)
{
    newError = E_OK;

    *res = fList[pos].func(args);

    return newError == E_OK;
}

/* Set constants */
void numInitConst(void)
{
/*
    transformDbl(&constPi, acos(-1.0));
    transformDbl(&constE, exp(1.0));
*/
    transformCmp(&constIm, 0, 1); /* ? */

    memAddConst("pi", dblToNumber(_PI), True);
    memAddConst("e", dblToNumber(_E), True);
    memAddConst("i", constIm, True);
    /* result of last evaluation */
    memAddConst("ans", constZero, False);
    /* default time start */
    memAddConst("tau0", dblToNumber(0), False);
}

/* Is it fraction? */
Bool eqInt(Number n)
{
    double tmp;

    switch(n.type) {

    case nINT: return True;
    case nDBL: return modf((double) n.dVal, &tmp) == 0;
    case nCMP: return n.cVal.im == 0 && modf((double) n.cVal.re, &tmp) == 0;
    case nRAT: return n.rVal.denom == 1;

    }
    return False;
}

/* Read integer part */
int asInt(Number n)
{
    switch(n.type) {

    case nINT: return (int) n.iVal;
    case nDBL: return (int) n.dVal;
    case nCMP: return (int) n.cVal.re;
    case nRAT: return (int) n.rVal.num / n.rVal.denom;

    }
    return 0;
}

/* Find equivalent number type */
void nCorrectType(Number *a, Number *b)
{
    switch(a->type) {
    case nINT:
        switch(b->type) {
        case nINT: return;
        case nDBL:
            transformDbl(a, (dnumber) a->iVal);
            break;
        case nCMP:
            /* if possible transform to float point */
            if(LESS(b->cVal.im, 1e-12)) {
                transformDbl(a, (dnumber) a->iVal);
                transformDbl(b, (dnumber) b->cVal.re);
            }
            else
                transformCmp(a, (cnumber) a->iVal, 0);
            break;
        case nRAT:
            if(b->rVal.denom == 1)
                transformInt(b, (inumber) b->rVal.num);
            else
                transformRat(a, (rnumber) a->iVal, 1);
            break;
        }
        return;
    case nDBL:
        switch(b->type) {
        case nDBL: return;
        case nINT:
            transformDbl(b, (dnumber) b->iVal);
            break;
        case nCMP:
            /* if possible transform to float point */
            if(LESS(b->cVal.im, 1e-12))
                transformDbl(b, (dnumber) b->cVal.re);
            else
                transformCmp(a, (cnumber) a->dVal, 0);
            break;
        case nRAT:
            transformDbl(b, ((dnumber) b->rVal.num)/b->rVal.denom);
            break;
        }
        return;
    case nRAT:
        switch(b->type) {
        case nRAT: return;
        case nCMP:
            transformCmp(a, ((cnumber) a->rVal.num)/a->rVal.denom, 0);
            break;
        default:
            nCorrectType(b, a);
        }
        return;
    case nCMP:
        if(b->type != nCMP) nCorrectType(b, a);
        return;
    }
}

/* Get random number */
Number nRand(Number n)
{
    int a, s;
    static unsigned int seed = 0;

    seed += time(NULL);

    srand(seed);

    switch(n.type) {
    case nINT:
        s = n.iVal >= 0 ? 1 : -1;
        if(n.iVal == 0)
            n.iVal = 0;
        else {
            a = abs(n.iVal);
            if(a < RAND_MAX)
                n.iVal = (rand() % a) * s;
            else
                n.iVal = (rand() * a * s)/ RAND_MAX;
        }
        break;
    case nDBL:
        n.dVal = (dnumber) nDblRand(n.dVal, seed);
        break;
    case nCMP:
        n.cVal.re = (cnumber) nDblRand(n.cVal.re, seed);
        n.cVal.im = (cnumber) nDblRand(n.cVal.im, seed);
        break;
    case nRAT:
        n.type = nDBL;
        n.dVal = (dnumber) nDblRand(((double) n.rVal.num)/n.rVal.denom, seed);
        break;
    }

    return n;
}

double nDblRand(double x, unsigned int seed)
{
    srand(seed);

    return x / RAND_MAX * rand();
}

Bool isOverSum(long int a, long int b)
{
    if(a > 0 && b > 0)
        return a > (LONG_MAX - b);
    else if(a < 0 && b < 0)
        return a < (LONG_MIN - b);

    return False;
}

Bool isOverProd(long int a, long int b)
{
    if(a != 0 && b != 0)
        return abs(a) > (LONG_MAX/abs(b));

    return False;
}

Number nPow2(Number n)
{
    switch(n.type) {
    case nINT:
        n.iVal *= n.iVal;
        break;
    case nDBL:
        n.dVal *= n.dVal;
        break;
    case nCMP:
        n = cmpProd(n, n);
        break;
    case nRAT:
        n.rVal.num *= n.rVal.num;
        n.rVal.denom *= n.rVal.denom;
        break;
    }

    return n;
}

Number powInt(Number n, inumber i)
{
    Number res = constOne;
    int p;

    for(p = abs((int) i); p > 0; p >>= 1) {
        /* product res to arg */
        if(p % 2 == 1)    res = nProd(res, n);
        /* square arg */
        if(p > 1)  n = nPow2(n);
    }

    return i > 0 ? res : nRat(constOne, res);
}

Number powCmp(Number n, Number c)
{
    float r, m;
    /* convert to complex */
    switch(n.type) {
    case nINT:
        transformCmp(&n, (cnumber) n.iVal, 0);
        break;
    case nDBL:
        transformCmp(&n, (cnumber) n.dVal, 0);
        break;
    case nRAT:
        transformCmp(&n, ((cnumber) n.rVal.num)/n.rVal.denom, 0);
        break;
    default:
        break;
    }
    /* get trigonometric form */
    n = cmp2trig(n);
    /* correct values */
    if(n.cVal.re < 0) {
        n.cVal.re = - n.cVal.re;
        n.cVal.im += _PI;
    }
    /* get power */
    r = (cnumber) exp(c.cVal.re*log(n.cVal.re) - n.cVal.im*c.cVal.im);
    m = (cnumber) (c.cVal.im*log(n.cVal.re) + n.cVal.im*c.cVal.re);
    /* check r and m !!! */

    n.cVal.re = (cnumber) r*cos(m);
    n.cVal.im = (cnumber) r*sin(m);

    return n;
}

Number powDbl(Number n, dnumber d)
{
    double val;
    switch(n.type) {
    case nINT: case nDBL: case nRAT:
        if((val = real_part(n)) > 0)
            transformDbl(&n, pow(val, d));
        else
            n = trig2cmp(cmpToNumber((cnumber) (pow(-val, d)), _PI*d));
        break;
    case nCMP:
        n = cmp2trig(n);
        n.cVal.im *= (cnumber) d;
        n.cVal.re = (cnumber) pow(n.cVal.re > 0 ? n.cVal.re : -n.cVal.re, d);
        n = trig2cmp(n);
        break;
    }

    return n;
}

/* ERROR FUNCTION NUMBER - 5 */
Number nPow(Number n, Number p)
{
    if(isZero(p)) {
        if(isZero(n))
            fatalError(E_WRONG_ARG, 7.05, "0^0", F_ARITHMETIC);
        else
            return constOne;
    }

    switch(p.type) {
        case nINT: n = powInt(n, p.iVal); break;
        case nDBL: n = powDbl(n, p.dVal); break;
        case nCMP: n = powCmp(n, p); break;
        case nRAT: n = powDbl(n, ((dnumber) p.rVal.num)/p.rVal.denom); break;
    }

    return n;
}

/* ERROR FUNCTION NUMBER - 4 */
Number cmp2trig(Number c)
{
    double tmp = sqrt(SQ(c));

    if(tmp == 0.0) fatalError(E_ZERO_DEVIDE, 7.04, NULL, F_ARITHMETIC);

    c.cVal.im = (cnumber) atan2(c.cVal.im/tmp, c.cVal.re/tmp);
    c.cVal.re = (cnumber) tmp;

    return c;
}

Number trig2cmp(Number c)
{
    cnumber tmp = c.cVal.re;

    c.cVal.re *= (cnumber) cos(c.cVal.im);
    c.cVal.im = (cnumber) tmp * sin(c.cVal.im);

    return c;
}

/* ERROR FUNCTION NUMBER - 3 */
Number nLn(Number n)
{
    double val;

    if(isZero(n))
        fatalError(E_WRONG_ARG, 7.03, "ln(0)", F_ARITHMETIC);

    switch(n.type) {
    case nINT: case nDBL: case nRAT:
        if((val = real_part(n)) > 0) {
            transformDbl(&n, log(val));
        }
        else {
            transformCmp(&n, (cnumber) log(-val), _PI);
        }
        break;
    case nCMP:
        n = cmp2trig(n);

        if(n.cVal.re < 0) {
            n.cVal.re = - n.cVal.re;
            n.cVal.im += _PI;
        }
        n.cVal.re = (cnumber) log(n.cVal.re);

        break;
    }

    return n;
}

Number nMult(Number n, double k)
{
    switch(n.type) {
    case nDBL:
        n.dVal *= k;
        break;
    case nINT:
        transformDbl(&n, (dnumber) n.iVal * k);
        break;
    case nCMP:
        n.cVal.re *= k;
        n.cVal.im *= k;
        break;
    case nRAT:
        transformDbl(&n, (k * n.rVal.num) / n.rVal.denom);
        break;
    }
    return n;
}

rnumber nGCD(rnumber a, rnumber b)
{
    rnumber c;

    /* abs */
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    /* swap */
    if(a < b) {
        if(a == 0) return b;
        c = a; a = b; b = c;
    }

    while(1) {
        if((c = a % b) == 0)
            return b;
        a = b; b = c;
    }
}

void nRSimp(Number *n)
{
    rnumber r;

    if(n->type != nRAT) return;

    r = nGCD(n->rVal.num, n->rVal.denom);

    if(r > 1) {
        n->rVal.num /= r;
        n->rVal.denom /= r;
    }
}

void nRToDenom(Number *a, Number *b)
{
    rnumber den;

    if(a->rVal.denom == b->rVal.denom) return;

    den = abs(a->rVal.denom * b->rVal.denom) / nGCD(a->rVal.denom, b->rVal.denom);

    a->rVal.num *= den / a->rVal.denom;
    b->rVal.num *= den / b->rVal.denom;

    a->rVal.denom = b->rVal.denom = den;
}

