/** @file vpb_data.h
~~~ Base Data ~~~
Define main structure for data exchanging and saving */

#include "vpb_number.h"
#include "vpb_array.h"

#ifndef VPB_DATA_BASE
#define VPB_DATA_BASE

#define dataDelete(ptr)  dataClear(ptr); free(ptr);

/* =============== Base types =================== */
/** @enum DataTypes
    List of allowed data types */
typedef enum DataTypes vpType;
enum DataTypes {tNumber, tArray, tText, tFree};


/** @struct vpDataString
    Get text data */
struct vpDataString
{
    int length;         /**< String length (not using now) */
    char *str;            /**< String */
    char  arr[4];         /**< Array for characters or small strings */
};
typedef struct vpDataString Str;

struct vpDataFree
{
    u_char key;   /**< Key to data type */
    u_char root;   /**< Get original data */
    u_char copy;   /**< Can copy */
    u_char del;    /**< have to create/delete */

    void* ptr;    /**< Pointer to data */
    size_t size; /**< Data size */
};
typedef struct vpDataFree Freetp;

/** @struct vpData
    Main data container */
struct vpData
{
    u_char type;          /**< Data type */
    int iter;             /**< Position for iterations */
    /** @union
       Collect data */
    union {
        Number  numVal;   /**< Number type (for digital and boolean data) */
        Array   arrVal;   /**< Array of numbers */
        Str     txtVal;   /**< String container */
        Freetp  freeVal;
    };
};

/* ================ Data Init ================== */

Data dataInt(long a);
Data dataDbl(double a);
Data dataCmp(float re, float im);
Data dataBool(u_char a);
Data dataFree(u_char k, u_char useCopy, u_char useDel);
Data dataArrayNull(void);
Data dataStringNull(void);



/** Set initial size of array, fill with zero
    @fn dataInitArray
    @param d - pointer to data structure
    @param size - array length
    @return True if successfully */
Bool dataInitArray(Data *d, int size);

/** Clear memory in data structure
    @fn dataClear
    @param d - pointer to data */
void dataClear(Data *d);



Number* N(Data *d, int pos);

/* ================= Data Array =============== */

/** Get array length
    @fn arraySize
    @param d - data structure
    @return number of elements */
int arraySize(Data d);
/** Get dimetion number
    @fn arrayDim
    @param d - data structure
    @return number of dimentions */
int arrayDim(Data d);
/** Get dimetion along axe
    @fn arrayDimSize
    @param d - data structure
    @param i - axe index
    @return dimention value along axe */
//int arrayDimSize(Data d, int i);
/** Get number from array
    @fn arrayNum
    @param d - pointer to data structure
    @param pos - position index
    @return pointer to number */
//Number* arrayNum(Data *d, int pos);

Number* arrayNumPos(Data *d, Number *iLst, u_char sz);
/** Get number with coordinate
    @fn arrayNumInd
    @param d - pointer to data structure
    @param x - begining of coordinate index
    @return pointer to number */

/** Resize array to given dimentions
    @fn resizeIndex
    @param d - pointer to data (array)
    @param dim - dimention number ...
    @return True if resize seccessfully */
//Bool resizeIndex(Data *d, int dim, ...);

Bool resizeToArray(Data *d, Number *indLst, u_char dim);
/** Resize array as in source
    @fn resizeData
    @param d - pointer to data (array)
    @param src -source array
    @return True if resize successfully */
Bool resizeData(Data *d, Data src);
/** Add number to the end of array
    @fn arrayAddNum
    @param d - data structure (array)
    @param n - number
    @return True if done */
//Bool arrayAddNum(Data* d, Number n);

void arrayFill(Data *d, Number src); /* ??? */
/** Iterator for array elements
    @fn getNext
    @param d - pointer to data structure
    @return pointer to current element of NULL */
Number* getNext(Data *d);
/** Reset iterator counter
    @fn iReset
    @param d - pointer to data structure */
void   iReset(Data *d);



char* dataAsText(Data d);

/** Copy data structure
    @fn dataCopy
    @param d - dest data
    @param src - source data
    @return True if copy successfully */
Bool dataCopy(Data *d, Data src);

Bool dataSum(Data *res, Data d);



#endif /* VPB_DATA_BASE */

#ifndef VPB_DATA_H
#define VPB_DATA_H

/** Delete data structure
    @fn dataDelete
    @param dat - pointer to data */
//void  dataDelete(Data* dat);

/** Get element position from index represented as number array
    @fn parseEltIndex
    @param d - pointer to data
    @param n - index (number array)
    @param sz - index length
    @param ndx - result position
    @return True if position have found */
Bool parseEltIndex(Data *d, Number *n, int sz, int *ndx);

/** Copy string
    @fn stringCopy
    @param str - original string
    @return pointer to copy string */
char* stringCopy(const char* str, int* len);
/** Copy string and transform it to upper case
    @fn stringCopyUp
    @param str - original string
    @return pointer to new string */
char* stringCopyUp(const char* str, int *len);

Bool stringSet(Data* d, const char* str);

Bool stringAdd(Data* d, const char *str);

/** Find hash-code for string
    @fn stringHash
    @param str - given string
    @return hash value */
unsigned int stringHash(const char* str);
/** Find hash code for upper version of string
    @fn stringHashUp
    @param str - given string
    @return hash value */
unsigned int stringHashUp(const char* str);
/** Compare upper versions of given strings
    @fn stringEqUp
    @param str1 - first string
    @param str2 - second string
    @return True if strings are equal */
Bool stringEqUp(const char *str1, const char *str2);

Data regDataText(char* txt);

char* memTmpText(int sz);

Data* getTmpData(u_char size);


#endif /* VPB_DATA_H */
