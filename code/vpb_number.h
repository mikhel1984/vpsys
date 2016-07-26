/** @file vpb_number.h
~~~ Base Number ~~~
Define basical type for working with numbers */

#include "vpb_abstract.h"

#define _PI 3.141592653589793
#define _E  2.718281828459045

#define fSIN "SIN"
#define fEXP "EXP"
#define fABS "ABS"
#define fRND "RAND"
#define fPOW "POW"
#define fPOW2 "POW2"
#define fLN  "LN"
#define fCOS "COS"
#define fSQRT "SQRT"
#define fTAN "TAN"

/* maximum number of function argements and array dimentions */
#define MAX_ARG_NUMBER 10
#define TMP_NUMBER_SIZE 8
extern unsigned int toll;

#ifndef VPB_NUMBERS_BASE
#define VPB_NUMBERS_BASE



/* ============= Base types =================== */

/** @enum NumberType
    List of allowed number types */
enum NumberType {nINT, nDBL, nCMP, nRAT};
typedef enum NumberType nType;
/** @enum NumberCompare
    Marks of logical operations with numbers */
enum NumberCompare { EQ, NE, LT, LE, GE, GT };
typedef enum NumberCompare numComp;

extern Number constZero, constOne;

/** @struct Complex
    Complex data structure */
typedef struct Complex
{
    cnumber re;         /**< Real part */
    cnumber im;         /**< Imaginary part */
} cmp;

typedef struct Rational
{
    rnumber num;
    rnumber denom;
} rat;

/** @struct vpNumber
    Representation of digital (and boolean) data */
struct vpNumber
{
    u_char type;         /**< Number type */
    /** @union
       Collect number types */
    union {
        inumber iVal;   /**< Integer value */
        dnumber dVal;   /**< Float point value */
        cmp     cVal;   /**< Complex value */
        rat     rVal;   /**< Rational value */
    };
};

/* ================ Major functions ================= */

/** Get sum
    @fn nSum
    @param n1 - first item
    @param n2 - second item
    @return n1+n2 */
Number nSum(Number n1, Number n2);
/** Get difference
    @fn nSub
    @param n1 - minuend
    @param n2 - subtrahend
    @return n1-n2 */
Number nSub(Number n1, Number n2);
/** Get product
    @fn nProd
    @param n1 - multiplier
    @param n2 - multiplier
    @return n1*n2 */
Number nProd(Number n1, Number n2);
/** Get ratio
    @fn nRat
    @param n1 - dividend
    @param n2 - divisor
    @return n1/n2 */
Number nRat(Number n1, Number n2);
/** Get module of number
    @fn nAbs
    @param n - number value
    @return |n| */
Number nAbs(Number n);
/** Get exponent
    @fn nExp
    @param n - number value
    @return exp(n) */
Number nExp(Number n);
/** Get sinusoid value
    @fn nSin
    @param n - number value
    @return sin(n) */
Number nSin(Number n);
/** Get random number
    @fn nRand
    @param n - limit for range
    @return random number according limit */
Number nRand(Number n);
/** Get natural logarithm
    @fn nLn
    @param n - argument
    @return logarithm of argument */
Number nLn(Number n);
/** Power of number
    @fn nPow
    @param n - base
    @param p - exponent
    @return n ^ p */
Number nPow(Number n, Number p);
/** Second power
    @fn nPow2
    @param n - base
    @return n ^ 2 */
Number nPow2(Number n);

Number nCos(Number n);

Number nTan(Number n);

Number nSqrt(Number n);

/* ================== Logic ========================= */

/** Compare two numbers (value, not type)
    @fn nCompare
    @param n1 - finst number
    @param n2 - second number
    @param cp - condition
    @return is condition true */
Bool nCompare(Number n1, Number n2, numComp cp);
/** Logical AND
    @fn nAnd
    @param n1 - first item
    @param n2 - second item
    @return n1&&n1 */
Number nAnd(Number n1, Number n2);
/** Logical OR
    @fn nOr
    @param n1 - first item
    @param n2 - second item
    @return n1||n2 */
Number nOr(Number n1, Number n2);
/** Logicl NOT
    @fn nNot
    @param n - number (bool value)
    @return if n == 0 return 1 else 0 */
Number nNot(Number n);
/** Represent boolean value as Number
    @fn nBool
    @param val - boolean value
    @return Number 1 of 0 */
Number nBool(Bool val);

/* ===================== Representation ====================== */

/** Get number from integer value
    @fn intToNumber
    @param a - integer
    @return number type */
Number intToNumber(long a);
/** Get number from float point value
    @fn dblToNumber
    @param a - float point
    @return number type */
Number dblToNumber(double a);
/** Get number with given real and imaginary parts
    @fn cmpToNumber
    @param r - real value
    @param i - imaginary value
    @return number type */
Number cmpToNumber(float r, float i);
/** Get real part as double value
    @fn real_part
    @param n - number type
    @return real part as double */
double real_part(Number n);
/** Get imaginary part as double
    @fn imag_part
    @param n - number type
    @return imaginary part as double */
double imag_part(Number n);
/** Check if number have fractional part
    @fn eqInt
    @param n - number type
    @return True if there is not fractional part */
Bool eqInt(Number n);
/** Get number value as integer
    @fn asInt
    @param n - number type
    @return integer value */
int asInt(Number n);
/** Check number equivalence to zero
    @fn isZero
    @param n - number value
    @return True if n == 0 */
Bool isZero(Number n);

#endif  /* VPB_NUMBERS_BASE */

/* ==================== INTERNAL PART ===================== */

#ifndef VPB_NUMBERS_H
#define VPB_NUMBERS_H




/** Invert sign of given number
    @fn numInvert
    @param n - pointer to number */
void numInvert(Number* n);
/** Write number to string
    @fn numAsText
    @param a - number
    @return pointer to string */
char* numAsText(Number a);
/** Find function in list
    @fn getFunc
    @param nm - function name
    @return index of funcion in list or -1 */
int getFunc(const char* nm);
/** Get number of function arguments
    @fn getFuncArg
    @param pos - index of function in list
    @return number of arguments */
u_char getFuncArg(int pos);
/** Eval function
    @fn evalFunc
    @param pos - index of function to evaluate
    @param args - list of agruments
    @param res - pointer to number for writing result
    @return true if evaluation was successful */
Bool evalFunc(int pos, Number *args, Number *res);
/** Get Number from complex parts
    @fn transformCmp
    @param x - pointer to number
    @param re - real value
    @param im - imaginary value */
void transformCmp(Number *x, cnumber re, cnumber im);
/** Get Number from float point
    @fn transformDbl
    @param x - pointer to number
    @param a - float point value */
void transformDbl(Number *x, dnumber a);
/** Get Number from integer
    @fn transformInt
    @param x - pointer to number
    @param a - integer */
void transformInt(Number *x, inumber a);

void transformRat(Number *x, rnumber n, rnumber d);

Number nMult(Number n, double k);

char* numAsTextFull(Number a);

#endif /* VPB_NUMBERS_H */
