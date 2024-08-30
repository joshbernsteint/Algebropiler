#include "fileParser.h"

int inStringArray(stringArray *arr, string s)
{
    for (int i = 0; i < arr->length; i++)
    {
        if (strcmp(arr->elems[i], s) == 0)
        {
            return i;
        }
    }
    return -1;
}

int findColIndex(string col, tableStructure *t)
{
    for (size_t i = 0; i < t->numCols; i++)
    {
        if (strcmp(t->cols[i].name, col) == 0)
        {
            return i;
        }
    }
    return -1;
}

void freeStringArray(stringArray *s)
{
    for (size_t i = 0; i < s->length; i++)
    {
        free(s->elems[i]);
    }
    if (s->elems != NULL)
        free(s->elems);
}

void printArray(stringArray *s)
{
    for (size_t i = 0; i < s->length; i++)
    {
        printf("Elem %ld: `%s`\n", i, s->elems[i]);
    }
}

void arrayAdd(stringArray *a, string s)
{
    a->length++;
    a->elems = (string *)realloc(a->elems, sizeof(string) * a->length);
    a->elems[a->length - 1] = s;
}

string trim(string s)
{
    if (s == NULL)
        return NULL;
    while (*s != '\0' && *s == ' ' && *s != '#')
    {
        s++;
    }
    int endIndex = strlen(s) - 1;
    while (endIndex > 0 && s[endIndex] == ' ' || s[endIndex] == '\n')
    {
        s[endIndex] = '\0';
        endIndex--;
    }
    return s;
}

int indexOf(string s, char c, int n)
{
    int i = 0;
    while (i < n && s[i] != '\0')
    {
        if (s[i] == c)
        {
            return i;
        }
        else
        {
            i++;
        }
    }
    return -1;
}

stringArray parseList(string curLine)
{
    stringArray res = {.length = 0, .elems = NULL};
    string trimmed = trim(curLine);
    const size_t length = strlen(trimmed) - 1;
    if (length > 0)
    {
        if (trimmed[0] != '{' || trimmed[length] != '}')
        {
            printExit("Error: Invalid List Syntax!\n");
        }
        int index = 0;
        int blockSize;
        string cur = trimmed + 1;
        while (index < length)
        {
            blockSize = length - index;
            cur = trim(cur);
            int commaIndex = indexOf(cur, ',', blockSize);
            int bracketIndex = indexOf(cur, '}', blockSize);
            if (commaIndex == -1 || bracketIndex < commaIndex)
            {
                string curItem = (string)malloc(sizeof(char) * (bracketIndex + 1));
                curItem[bracketIndex] = '\0';
                strncpy(curItem, cur, bracketIndex);
                if (strlen(curItem) > 0)
                {
                    arrayAdd(&res, curItem);
                }
                break;
            }
            else
            {
                string curItem = (string)malloc(sizeof(char) * (commaIndex + 1));
                curItem[commaIndex] = '\0';
                strncpy(curItem, cur, commaIndex);
                arrayAdd(&res, curItem);
            }
            index += commaIndex;
            cur += commaIndex + 1;
        }
    }
    return res;
}

void freePhi(Phi *p)
{
    freeStringArray(&p->S);
    freeStringArray(&p->V);
    for (size_t i = 0; i <= p->N; i++)
    {
        freeStringArray(&p->F[i]);
    }
    free(p->F);

    freeStringArray(&p->O);
    if (p->G != NULL)
        free(p->G);
}

void assignPhi(Phi *p, int index, stringArray *value)
{
    switch (index)
    {
    case 0:
        p->S = *value;
        break;
    case 2:
        p->V = *value;
        break;
    case 4:
        p->O = *value;
        break;
    default:
        break;
    }
}

bool isNumber(string t)
{
    string c = t;
    if (*c == '-' && *(c + 1) != '\0')
        c++;
    while (*c != '\0')
    {
        if (*c < '0' || *c > '9')
        {
            return false;
        }
        c++;
    }
    return true;
}

type getType(string s)
{
    if (s[0] == '"')
    {
        return STRING;
    }
    else if (isNumber(s))
    {
        return INT;
    }
    else
    {
        printExit("Unknown type Found!");
    }
}

string getArrayByPartial(stringArray *p, string pre, string post)
{
    char buffer[STR_LENGTH] = {0};
    sprintf(buffer, "%s %s", pre, post);
    int index = inStringArray(p, buffer);
    if (index != -1)
    {
        return pre;
    }
    return post;
}

tableStructure parseCSV(const string fileName, Phi *p)
{
    /*
        Steps:
        1. Read column names from first line
        2. Read second line to get types of each column
    */
    tableStructure result = {.numCols = 0, .cols = NULL, .maxFinalRows = 1};
    stringArray *sp = &p->V;
    result.blockMap = (int *)malloc(sizeof(int) * sp->length);
    int *orgIndexes = (int *)malloc(sizeof(int) * sp->length);
    stringArray *optionSet = (stringArray *)malloc(sizeof(stringArray) * sp->length);
    // Zero out array
    for (size_t i = 0; i < sp->length; i++)
    {
        result.blockMap[i] = 0;
        orgIndexes[i] = -1;
        optionSet[i].elems = NULL;
        optionSet[i].length = 0;
    }

    FILE *ptr = fopen(fileName, "r");
    char curLine[LINE_LENGTH] = {0};
    char temp[LINE_LENGTH] = {0};

    fgets(curLine, LINE_LENGTH, ptr);
    string colNames = trim(curLine) + 1;
    int quoteIndex = -1;
    int _colIndex = 0;
    while ((quoteIndex = indexOf(colNames, '"', LINE_LENGTH)) != -1)
    {
        string c = malloc(sizeof(char) * quoteIndex + 1);
        strncpy(c, colNames, quoteIndex);
        c[quoteIndex] = '\0';

        for (size_t i = 0; i < sp->length; i++)
        {
            // if(orgIndexes[i] < 0 && strcmp(c,sp->elems[i]) == 0){
            // printf("%d\n",getArrayByPartial(&p->S,c,sp->elems[i]));
            if (orgIndexes[i] < 0 && (strcmp(c, sp->elems[i]) == 0 || strcmp(c, getArrayByPartial(&p->S, c, sp->elems[i])) == 0))
            {
                orgIndexes[i] = _colIndex;
            }
        }
        _colIndex++;

        // Allocate space
        if (result.cols)
        {
            result.numCols++;
            result.cols = (column *)realloc(result.cols, sizeof(column) * result.numCols);
        }
        else
        {
            result.numCols++;
            result.cols = (column *)malloc(sizeof(column));
        }
        result.cols[result.numCols - 1].name = c;
        colNames += quoteIndex + 3;
    }

    // Reading second line
    quoteIndex = -1;
    fgets(curLine, LINE_LENGTH, ptr);
    size_t colIndex = 0;
    string lineRef = curLine;
    while ((quoteIndex = indexOf(lineRef, ',', LINE_LENGTH)) != -1)
    {
        strncpy(temp, lineRef, quoteIndex);
        temp[quoteIndex] = '\0';
        result.cols[colIndex].c_type = getType(temp);
        lineRef += quoteIndex + 1;
        colIndex++;
    }
    result.cols[colIndex].c_type = getType(lineRef);

    fseek(ptr, SEEK_SET, 0);
    fgets(curLine, LINE_LENGTH, ptr);
    int i;
    int numLines = 0;
    while (fgets(curLine, LINE_LENGTH, ptr) != NULL)
    {
        numLines++;
        lineRef = curLine;
        quoteIndex = 0;
        colIndex = 0;
        while ((quoteIndex = indexOf(lineRef, ',', LINE_LENGTH)) != -1)
        {
            strncpy(temp, lineRef, quoteIndex);
            temp[quoteIndex] = '\0';
            lineRef += quoteIndex + 1;
            // Check if its in the set
            for (i = 0; i < sp->length; i++)
            {
                if (orgIndexes[i] == colIndex)
                {
                    // Check if in set
                    if (inStringArray(&optionSet[i], temp) == -1)
                    { // Not in
                        string copy = (string)malloc(STR_LENGTH);
                        strcpy(copy, temp);
                        arrayAdd(&optionSet[i], copy);
                    }
                    break;
                }
            }
            colIndex++;
        }
        strcpy(temp, trim(lineRef));
        temp[strlen(lineRef) + 1] = '\0';
        for (i = 0; i < sp->length; i++)
        {
            if (orgIndexes[i] == colIndex)
            {
                // Check if in set
                if (inStringArray(&optionSet[i], temp) == -1)
                { // Not in
                    string copy = (string)malloc(STR_LENGTH);
                    strcpy(copy, temp);
                    arrayAdd(&optionSet[i], copy);
                }
                break;
            }
        }
    }
    result.numRows = numLines;
    // Free set
    for (size_t i = 0; i < sp->length; i++)
    {
        result.blockMap[i] = optionSet[i].length;
        result.maxFinalRows *= optionSet[i].length;
        freeStringArray(&optionSet[i]);
    }
    free(optionSet);
    free(orgIndexes);
    fclose(ptr);
    return result;
}

void freeTable(tableStructure *t)
{
    for (size_t i = 0; i < t->numCols; i++)
    {
        free(t->cols[i].name);
    }
    free(t->cols);
}

/*
Parses the given file for the Phi operator arguments
## Phi Arguments
S = List of projected attributes for the query output
n = Number of grouping variables
V = List of grouping attributes
[F] = {F0, F1, ..., Fn}, list of sets of aggregate functions. Fi represents a list of
aggregate functions for each grouping variable.
[σ] = {σ0, σ1, ..., σn}, list of predicates to define the ranges for the grouping
variables.
G = Predicate for the having clause
*/
Phi readFile(const string fileName)
{
    Phi result;
    FILE *ptr = fopen(fileName, "r");
    string curLine = (string)malloc(sizeof(char) * LINE_LENGTH);
    stringArray line;
    stringArray *F_ptr;
    int argIndex = 0;
    while (fgets(curLine, LINE_LENGTH, ptr) != NULL)
    {
        string trimmed = trim(curLine);
        int commentIndex = indexOf(trimmed, '#', LINE_LENGTH);
        if (trimmed[0] == '#')
            continue;
        else if (commentIndex != -1)
        {
            trimmed[commentIndex] = '\0';
            trimmed = trim(trimmed);
        }
        if (strlen(trimmed) > 0)
        {
            switch (argIndex)
            {
            case 0:
            case 2:
            case 4:
                line = parseList(trimmed);
                assignPhi(&result, argIndex, &line);
                break;
            case 3:
                int brackIndex;
                int sIndex = 0;
                while ((brackIndex = indexOf(trimmed, '}', LINE_LENGTH)) != -1 && trimmed[0] != '}')
                {
                    line = parseList(trimmed + 1);
                    F_ptr[sIndex] = line;
                    sIndex++;
                    trimmed += brackIndex + 1;
                }
                result.F = F_ptr;

                break;
            case 1:
                result.N = atoi(trimmed);
                F_ptr = (stringArray *)malloc(sizeof(stringArray) * (result.N + 1));
                break;
            case 5:
                string copy = (string)malloc(sizeof(char) * strlen(trimmed) + 1);
                strcpy(copy, trimmed);
                result.G = copy;
            default:
                break;
            }
            argIndex++;
        }
    }

    if (argIndex < 6)
    { // If there is no having clause
        result.G = NULL;
    }
    free(curLine);
    fclose(ptr);
    return result;
}

/*
    Table functions
*/

// Generates the structure of the final table based on the Phi operator (what columns)
columns generateFinalTable(Phi *p, tableStructure *base)
{
    columns result = {.numCols = p->S.length, .cols = NULL};
    result.cols = (colValue *)malloc(sizeof(colValue) * result.numCols);

    string cur;
    char orgName[STR_LENGTH] = {0};
    int curColIndex = -1;
    int spaceIndex = -1;
    for (size_t i = 0; i < p->S.length; i++)
    {
        cur = p->S.elems[i];
        spaceIndex = indexOf(cur, ' ', STR_LENGTH);
        if (spaceIndex == -1)
        {
            strcpy(orgName, cur);
            result.cols[i].label[0] = '\0';
        }
        else
        {
            strncpy(orgName, cur, spaceIndex);
            orgName[spaceIndex] = '\0';
            strcpy(result.cols[i].label, cur + spaceIndex + 1);
        }
        result.cols[i].baseColIndex = findColIndex(orgName, base);
        if (result.cols[i].baseColIndex == -1)
        { // It's an aggregate function
            // TODO: GET GROUPING DATA
            if (strncmp("sum(", orgName, 4) == 0)
            {
                result.cols[i].operation = SUM;
            }
            else if (strncmp("count(", orgName, 6) == 0)
            {
                result.cols[i].operation = COUNT;
            }
            else if (strncmp("avg(", orgName, 4) == 0)
            {
                result.cols[i].operation = AVG;
            }
            else if (strncmp("max(", orgName, 4) == 0)
            {
                result.cols[i].operation = MAX;
            }
            else if (strncmp("min(", orgName, 4) == 0)
            {
                result.cols[i].operation = MIN;
            }
            else
            {
                printExit("Line 1 Column %ld: Invalid aggregate operator: '%s'\n", i, orgName);
            }
            result.cols[i].colType = result.cols[i].operation == AVG ? FLOAT : INT;

            // Get grouping variable data
            const int paramIndex = indexOf(orgName, ')', STR_LENGTH);
            const int startParamIndex = indexOf(orgName, '(', STR_LENGTH);
            int dotIndex = indexOf(orgName, '.', LINE_LENGTH) + 1;
            memset(result.cols[i].orgColName, 0, STR_LENGTH);
            if (dotIndex > 0)
            {
                if (result.cols[i].operation == COUNT)
                {
                    strncpy(result.cols[i].grouping_var, orgName + 6, paramIndex - 6);
                }
                else
                {
                    strncpy(result.cols[i].grouping_var, orgName + 4, dotIndex - startParamIndex);
                }
                strncpy(result.cols[i].orgColName, orgName + dotIndex, paramIndex - dotIndex);
                result.cols[i].orgColName[paramIndex - dotIndex] = '\0';
                result.cols[i].grouping_var[dotIndex - startParamIndex - 2] = '\0';
            }
            else
            {
                strncpy(result.cols[i].orgColName, orgName + startParamIndex + 1, paramIndex - startParamIndex - 1);
                result.cols[i].orgColName[paramIndex - startParamIndex - 1] = '\0';
                result.cols[i].grouping_var[0] = '\0';
            }
        }
        else
        {
            result.cols[i].grouping_var[0] = '\0';
            result.cols[i].operation = ORG;
            strcpy(result.cols[i].orgColName, orgName);
            result.cols[i].colType = base->cols[result.cols[i].baseColIndex].c_type;
        }
    }
    return result;
}

string replaceAll(string src, char of, char with)
{
    string begin = src;
    while (*src != '\0')
    {
        if (*src == of)
        {
            *src = with;
        }
        src++;
    }
    return begin;
}

int replacePredot(string src, string with)
{
    if (isNumber(src) || src[0] == '\'')
        return 0;
    int dotIndex = indexOf(src, '.', STR_LENGTH);
    char temp[STR_LENGTH] = {0};
    if (dotIndex == -1)
    {
        sprintf(temp, "%s.%s", with, src);
    }
    else
    {
        sprintf(temp, "%s%s", with, src + dotIndex);
    }
    strcpy(src, temp);
    return 0;
}

int generateExpression(string buffer, string s, string replaceDot)
{
    char stages[3][STR_LENGTH] = {0, 0, 0};
    buffer[0] = 0;
    char stageFlag = 0;

    char cur;
    char curVal[STR_LENGTH] = {0};
    char tempBuffer[LINE_LENGTH] = {0};
    int curIndex = 0;
    bool inParens = false;
    const int length = strlen(s);
    for (int i = 0; i < length + 1; i++)
    {
        cur = s[i];
        switch (cur)
        {
        case '(':
            strcat(buffer, "(");
            break;
        case ')':
            inParens = true;
        case '\0':
        case ' ':
            curVal[curIndex] = '\0';
            if (strncmp(curVal, "and", 3) == 0)
            {
                strcat(buffer, "&&");
                stageFlag = 0;
            }
            else if (strncmp(curVal, "or", 2) == 0)
            {
                strcat(buffer, "||");
                stageFlag = 0;
            }
            else
            {
                if (stageFlag == 2)
                {
                    strcpy(stages[stageFlag], curVal);
                    const bool stringFlags[2] = {indexOf(stages[0], '\'', STR_LENGTH) != -1, indexOf(stages[2], '\'', STR_LENGTH) != -1};
                    replacePredot(stages[0], replaceDot);
                    replacePredot(stages[2], replaceDot);
                    if (stringFlags[0] || stringFlags[1])
                    {
                        // String comparison
                        replaceAll(stages[0], '\'', '"');
                        replaceAll(stages[2], '\'', '"');
                        sprintf(tempBuffer, "strcmp(%s,%s)%s0", stages[0], stages[2], stages[1]);
                        strcat(buffer, tempBuffer);
                    }
                    else
                    {
                        sprintf(tempBuffer, "%s%s%s", stages[0], stages[1], stages[2]);
                        strcat(buffer, tempBuffer);
                    }
                    if (inParens)
                    {
                        strcat(buffer, ")");
                    }
                }
                else
                {
                    strcpy(stages[stageFlag], curVal);
                }
                stageFlag = (stageFlag + 1) % 3;
                if (s[i - 1] == ')')
                    stageFlag++;
            }
            curIndex = 0;
            break;
        default:
            curVal[curIndex] = cur;
            curIndex++;
            break;
        }
    }
    return 0;
}

void getColFancyName(string inputName, columns *cols, aggregateList *aggs)
{
    char temp[LINE_LENGTH] = {0};
    size_t i;
    aggregateItem *t1;
    for (i = 0; i < cols->numCols; i++)
    {
        if (strcmp(inputName, cols->cols[i].label) == 0)
        {
            if (cols->cols[i].operation == ORG)
            {
                sprintf(temp, "finalTable[i].%s", cols->cols[i].orgColName);
            }
            else
            {
                for (size_t j = 0; j < aggs->length; j++)
                {
                    t1 = &aggs->list[j];
                    if (t1->agg == cols->cols[i].operation && strcmp(t1->groupingVar, cols->cols[i].grouping_var) == 0 && strcmp(t1->attribute, cols->cols[i].orgColName) == 0)
                    {
                        switch (t1->agg)
                        {
                        case AVG:
                            sprintf(temp, "(finalTable[i].%s.total/finalTable[i].%s.count)", t1->label, t1->label);
                            break;
                        default:
                            sprintf(temp, "finalTable[i].%s", t1->label);
                            break;
                        }
                        break;
                    }
                }
            }
            strcpy(inputName, temp);
            return;
        }
    }

    for (i = 0; i < aggs->length; i++)
    {
        if (strcmp(inputName, aggs->list[i].label) == 0)
        {
            switch (aggs->list[i].agg)
            {
            case AVG:
                sprintf(temp, "(finalTable[i].%s.total/finalTable[i].%s.count)", aggs->list[i].label, aggs->list[i].label);
                break;
            default:
                sprintf(temp, "finalTable[i].%s", aggs->list[i].label);
                break;
            }
            strcpy(inputName, temp);
            return;
        }
    }
}

int generateExpressionDependent(string buffer, string s, string replaceDotA, string replaceDotB, stringArray *groupingVarsCompare, stringArray *selectClause, columns *cols, aggregateList *aggs)
{
    char stages[3][STR_LENGTH] = {0, 0, 0};
    buffer[0] = 0;
    char stageFlag = 0;

    char cur;
    char curVal[STR_LENGTH] = {0};
    char tempBuffer[LINE_LENGTH] = {0};
    int curIndex = 0;
    bool inParens = false;
    const int length = strlen(s);
    for (int k = 0; k < length + 1; k++)
    {
        cur = s[k];
        switch (cur)
        {
        case '(':
            strcat(buffer, "(");
            break;
        case ')':
            inParens = true;
        case '\0':
        case ' ':
            curVal[curIndex] = '\0';
            if (strncmp(curVal, "and", 3) == 0)
            {
                strcat(buffer, "&&");
                stageFlag = 0;
            }
            else if (strncmp(curVal, "or", 2) == 0)
            {
                strcat(buffer, "||");
                stageFlag = 0;
            }
            else
            {
                if (stageFlag == 2)
                {
                    strcpy(stages[stageFlag], curVal);
                    const bool stringFlags[2] = {indexOf(stages[0], '\'', STR_LENGTH) != -1, indexOf(stages[2], '\'', STR_LENGTH) != -1};
                    bool hasSelect = false;
                    for (size_t i = 0; i < groupingVarsCompare->length; i++)
                    {   
                        if (strstr(stages[2], groupingVarsCompare->elems[i]) != NULL)
                        {
                            for (size_t j = 0; j < selectClause->length; j++)
                            {
                                if (strstr(selectClause->elems[j], groupingVarsCompare->elems[i]) != NULL && cols->cols[j].operation == ORG)
                                {
                                    hasSelect = true;
                                    break;
                                }
                            }
                            if (hasSelect)
                                break;
                        }
                    }
                    bool hasAgg = false;
                    for (size_t i = 0; i < cols->numCols; i++)
                    {
                        if (cols->cols[i].operation != ORG)
                        {
                            if (strstr(stages[2], cols->cols[i].orgColName) != NULL)
                            {
                                hasAgg = true;
                                break;
                            }
                        }
                    }

                    if (hasSelect && !hasAgg)
                    {
                        replacePredot(stages[0], replaceDotA);
                    }
                    else
                    {
                        replacePredot(stages[0], replaceDotB);
                    }
                    if(hasAgg){
                        getColFancyName(stages[2], cols, aggs);
                    }else{
                        replacePredot(stages[2], replaceDotB);
                    }
                    if (hasSelect && !hasAgg)
                    {
                        // get column that matches stages[0] and stages[2]
                        char *temp0 = (char *)malloc(sizeof(char) * STR_LENGTH);
                        char *org = temp0;
                        strcpy(temp0, stages[0]);

                        // remove everything before the dot
                        char *dot0 = strchr(temp0, '.');

                        if (dot0 != NULL)
                        {
                            temp0 = dot0 + 1;
                        }

                        // find the column that matches the name
                        int colIndex0 = -1;
                        for (size_t i = 0; i < cols->numCols; i++)
                        {
                            if (strcmp(cols->cols[i].orgColName, temp0) == 0)
                            {
                                colIndex0 = i;
                            }

                        }

                        free(org);

                        if (colIndex0 != -1)
                        {
                            // check the type of the column
                            if (cols->cols[colIndex0].colType == STRING)
                            {
                                sprintf(tempBuffer, "strcmp(%s,%s)%s0", stages[0], stages[2], stages[1]);
                                strcat(buffer, tempBuffer);
                            }
                            else {
                                sprintf(tempBuffer, "%s%s%s", stages[0], stages[1], stages[2]);
                                strcat(buffer, tempBuffer);
                            }
                        }
                        else
                        {
                            sprintf(tempBuffer, "%s%s%s", stages[0], stages[1], stages[2]);
                            strcat(buffer, tempBuffer);
                        }
                    }
                    else if(hasAgg) {
                        // get column that matches stages[0] and stages[2]
                        char *temp2 = (char *)malloc(sizeof(char) * STR_LENGTH);
                        char *org = temp2;
                        strcpy(temp2, stages[2]);

                        // remove everything before the dot
                        char *dot2 = strchr(temp2, '.');

                        if (dot2 != NULL)
                        {
                            temp2 = dot2 + 1;
                        }

                        // find the column that matches the name
                        int colIndex2 = -1;
                        for (size_t i = 0; i < cols->numCols; i++)
                        {
                            if (strcmp(cols->cols[i].orgColName, temp2) == 0)
                            {
                                colIndex2 = i;
                            }
                        }

                        free(org);

                        if (colIndex2 != -1)
                        {
                            // check the type of the column
                            if (cols->cols[colIndex2].colType == STRING)
                            {
                                sprintf(tempBuffer, "strcmp(%s,%s)%s0", stages[0], stages[2], stages[1]);
                                strcat(buffer, tempBuffer);
                            }
                            else {
                                sprintf(tempBuffer, "%s%s%s", stages[0], stages[1], stages[2]);
                                strcat(buffer, tempBuffer);
                            }
                        }
                        else
                        {
                            sprintf(tempBuffer, "%s%s%s", stages[0], stages[1], stages[2]);
                            strcat(buffer, tempBuffer);
                        }
                    }
                    else if (stringFlags[0] || stringFlags[1])
                    {
                        // String comparison
                        replaceAll(stages[0], '\'', '"');
                        replaceAll(stages[2], '\'', '"');
                        sprintf(tempBuffer, "strcmp(%s,%s)%s0", stages[0], stages[2], stages[1]);
                        strcat(buffer, tempBuffer);
                    }
                    else
                    {
                        sprintf(tempBuffer, "%s%s%s", stages[0], stages[1], stages[2]);
                        strcat(buffer, tempBuffer);
                    }
                    if (inParens)
                    {
                        strcat(buffer, ")");
                        inParens = false;
                    }
                }
                else
                {
                    strcpy(stages[stageFlag], curVal);
                }
                stageFlag = (stageFlag + 1) % 3;
                if (s[k - 1] == ')')
                    stageFlag++;
            }
            curIndex = 0;
            break;
        default:
            curVal[curIndex] = cur;
            curIndex++;
            break;
        }
    }
    return 0;
}

int generateHaving(string buffer, string G, columns *cols, aggregateList *aggs)
{
    char stages[3][STR_LENGTH * 4] = {0, 0, 0};
    buffer[0] = 0;
    char stageFlag = 0;

    char cur;
    char curVal[STR_LENGTH] = {0};
    char tempBuffer[LINE_LENGTH] = {0};
    int curIndex = 0;
    bool inParens = false;
    const int length = strlen(G);
    for (int i = 0; i < length + 1; i++)
    {
        cur = G[i];
        switch (cur)
        {
        case '(':
            strcat(buffer, "(");
            break;
        case ')':
            inParens = true;
        case '\0':
        case ' ':
            curVal[curIndex] = '\0';
            if (strncmp(curVal, "and", 3) == 0)
            {
                strcat(buffer, "&&");
                stageFlag = 0;
            }
            else if (strncmp(curVal, "or", 2) == 0)
            {
                strcat(buffer, "||");
                stageFlag = 0;
            }
            else
            {
                if (stageFlag == 2)
                {
                    strcpy(stages[stageFlag], curVal);
                    const bool stringFlags[2] = {indexOf(stages[0], '\'', STR_LENGTH) != -1, indexOf(stages[2], '\'', STR_LENGTH) != -1};
                    if (!stringFlags[0] && !isNumber(stages[0]))
                    {
                        // Find the name of the column
                        getColFancyName(stages[0], cols, aggs);
                    }
                    if (!stringFlags[1] && !isNumber(stages[2]))
                    {
                        getColFancyName(stages[2], cols, aggs);
                    }

                    if (stringFlags[0] || stringFlags[1])
                    {
                        // String comparison
                        replaceAll(stages[0], '\'', '"');
                        replaceAll(stages[2], '\'', '"');
                        sprintf(tempBuffer, "strcmp(%s,%s)%s0", stages[0], stages[2], stages[1]);
                        strcat(buffer, tempBuffer);
                    }
                    else
                    {
                        sprintf(tempBuffer, "%s%s%s", stages[0], stages[1], stages[2]);
                        strcat(buffer, tempBuffer);
                    }
                    if (inParens)
                    {
                        strcat(buffer, ")");
                        inParens = false;
                    }
                }
                else
                {
                    strcpy(stages[stageFlag], curVal);
                }
                stageFlag = (stageFlag + 1) % 3;
                if (G[i - 1] == ')')
                    stageFlag++;
            }
            curIndex = 0;
            break;
        default:
            curVal[curIndex] = cur;
            curIndex++;
            break;
        }
    }
    return 0;
}

/**
 * Gets the grouping variable from an aggregate function string. Unexpected behavior will happen if string does not contain a grouping variable
 */
int getGroupingVar(string dest, string src)
{
    const int startParenIndex = indexOf(src, '(', LINE_LENGTH);
    const int dotIndex = indexOf(src, '.', LINE_LENGTH);
    strncpy(dest, src + startParenIndex + 1, dotIndex - startParenIndex - 1);
    return 0;
}

int getValueAfterDot(string dest, string src)
{
    const int dotIndex = indexOf(src, '.', LINE_LENGTH);
    int endIndex = dotIndex;
    while (src[endIndex] != ' ')
    {
        endIndex++;
    }
    strncpy(dest, src + dotIndex + 1, endIndex - dotIndex);
    return 0;
}

int getGroupingVarAndAttribute(string varDest, string attDest, string src)
{
    const int startIndex = indexOf(src, '(', LINE_LENGTH) + 1;
    const int endParenIndex = indexOf(src, ')', LINE_LENGTH);
    const int dotIndex = indexOf(src, '.', LINE_LENGTH);
    if (startIndex == -1)
    {
        strcpy(attDest, src);
        varDest[0] = '\0';
        return -1;
    }
    else if (dotIndex == -1)
    { // No grouping variable
        varDest[0] = '\0';
        strcpy(attDest, src + startIndex);
        attDest[endParenIndex - startIndex] = '\0';
    }
    else
    {
        strncpy(varDest, src + startIndex, dotIndex - startIndex);
        varDest[dotIndex - startIndex] = '\0';
        strcpy(attDest, src + dotIndex + 1);
        attDest[endParenIndex - dotIndex - 1] = '\0';
    }
    return 0;
}

type getColType(tableStructure *t, string colName)
{
    for (size_t i = 0; i < t->numCols; i++)
    {
        if (strcmp(t->cols[i].name, colName) == 0)
        {
            return t->cols[i].c_type;
        }
    }
    return INT;
}

aggregateList getAggregates(stringArray *f, int length, stringArray *V, tableStructure *t)
{
    size_t i, j;
    aggregateList result = {.length = V->length, .list = NULL};
    aggregateItem *temp;
    int listIndex = 0;
    int attIndex = 0;
    int endParenIndex = 0;

    // Count the number of aggregates
    for (i = 0; i < length; i++)
    {
        for (j = 0; j < f[i].length; j++)
            result.length++;
    }
    result.list = (aggregateItem *)malloc(sizeof(aggregateItem) * (result.length));
    string cur;
    for (listIndex = 0; listIndex < V->length; listIndex++)
    {
        temp = &result.list[listIndex];
        temp->groupingVar[0] = '\0';
        temp->agg = ORG;
        strcpy(temp->attribute, V->elems[listIndex]);
        temp->itemType = getColType(t, V->elems[listIndex]);
    }

    for (i = 0; i < length; i++)
    {
        for (j = 0; j < f[i].length; j++)
        {
            cur = f[i].elems[j];
            temp = &result.list[listIndex];
            getGroupingVarAndAttribute(temp->groupingVar, temp->attribute, cur);
            // Check the aggregate operation
            if (strncmp("sum(", cur, 4) == 0)
            {
                temp->agg = SUM;
                temp->itemType = INT;
            }
            else if (strncmp("count(", cur, 6) == 0)
            {
                temp->agg = COUNT;
                temp->itemType = INT;
            }
            else if (strncmp("avg(", cur, 4) == 0)
            {
                temp->agg = AVG;
                temp->itemType = FLOAT;
            }
            else if (strncmp("max(", cur, 4) == 0)
            {
                temp->agg = MAX;
                temp->itemType = INT;
            }
            else if (strncmp("min(", cur, 4) == 0)
            {
                temp->agg = MIN;
                temp->itemType = INT;
            }
            else
            {
                printExit("Line 4 Column %d: Invalid aggregate operator: '%s'\n", listIndex, cur);
            }
            listIndex++;
        }
    }
    return result;
}