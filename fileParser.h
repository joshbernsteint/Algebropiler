#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * String macros and functions
 */
#define string char * // Just a simple rename.
#define LINE_LENGTH 1024
#define STR_LENGTH 64
#define SMALL_LENGTH 32

// Trims space characters from the beginning and the end of the string.
string trim(string s);

// Gets the index of a character in the string, up to the max length `n`.
int indexOf(string s, char c, int n);

int getGroupingVar(string dest, string src);

int getValueAfterDot(string dest, string src);

/**
 * stringArray:
 * This serves as a very simple implementation of string arraylist(or vector) in C.
 */
typedef struct stringArray
{
    string *elems; // Array of char pointers
    size_t length; // Size of the array
} stringArray;

// Adds `s` to the end of the array.
void arrayAdd(stringArray *a, string s);

// Prints the array in human-readable format. Used for debugging.
void printArray(stringArray *s);

// Finds a string in the stringArray, returns its index. Returns -1 if it is not found.
int inStringArray(stringArray *arr, string s);

/**
 * Phi Struct:
 * A container to hold the raw data extracted from the parsed file.
 */
typedef struct Phi
{
    stringArray S;  // Selection
    int N;          // Number of grouping variables
    stringArray V;  // Array of grouping attributes
    stringArray *F; // 2D array of aggregate functions
    stringArray O;  // Sigma
    string G;       // Having clause
} Phi;

/**
 * Bool type:
 * I just think it's neat.
 */
typedef char bool;
#define true 1
#define false 0

// Checks to see if `s` could be interpreted as a number. Returns true if yes or false if no.
bool isNumber(string s);

// Prints to stderr and immediately calls exit. Used for errors.
#define printExit(...)            \
    fprintf(stderr, __VA_ARGS__); \
    exit(EXIT_FAILURE)

// Frees all heap memory associated with the Phi struct.
void freePhi(Phi *);

// Reads the file denoted by path `s` and converts it to a Phi struct. Note: Please call `freePhi()` before program terminates to avoid memory leaks.
Phi readFile(const string s);

// Enum for setting column aggregate operations.
typedef enum aggregate
{
    ORG, // No operation
    AVG,
    COUNT,
    MAX,
    MIN,
    SUM,
} aggregate;

// Enum for setting the type of column.
typedef enum type
{
    STRING,
    INT,
    FLOAT,
} type;

// Holds the column data of the table.
typedef struct column
{
    string name;
    type c_type;
} column;

// Holds the data about the source table. In most cases, this will be `sales.csv`.
typedef struct tableStructure
{
    column *cols;        // Array of columns.
    size_t numCols;      // Number of columns.
    size_t numRows;      // Number of rows.
    size_t maxFinalRows; // Maximum number of rows given the grouping attributes.
    int *blockMap;       // An array corresponding to the number of unique entries for each grouping attribute.
} tableStructure;

typedef struct colValue
{
    char label[STR_LENGTH];
    char grouping_var[STR_LENGTH];
    char orgColName[STR_LENGTH];
    int baseColIndex; // The column index in the base table (without selection) -1 if its an aggregate
    aggregate operation;
    type colType;
} colValue;

tableStructure parseCSV(const string file, Phi *p);
void freeTable(tableStructure *t);
int findColIndex(string col, tableStructure *t);

typedef struct columns
{
    size_t numCols;
    colValue *cols;
} columns;

columns generateFinalTable(Phi *p, tableStructure *base);
int generateExpression(char *buffer, string s, string);

typedef struct aggregateItem
{
    type itemType;
    aggregate agg;
    char label[STR_LENGTH * 2];
    char attribute[SMALL_LENGTH];
    char groupingVar[SMALL_LENGTH]; // Length zero if none
} aggregateItem;

typedef struct aggregateList
{
    aggregateItem *list;
    size_t length;
} aggregateList;

aggregateList getAggregates(stringArray *, int, stringArray *, tableStructure *);

typedef struct table
{
    tableStructure structure;
    aggregateList aggregates;
    columns S;
} table;

int getGroupingVar(string dest, string src);

int generateExpressionDependent(char *buffer, string s, string, string, stringArray *, stringArray *, columns *cols, aggregateList *aggs);

int generateHaving(char *buffer, string G, columns *cols, aggregateList *aggs);