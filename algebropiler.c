#include "algebropiler.h"

/**
 * Gets the column index by its name
 */
int getColIndexByName(string s, columns *t)
{
    int index = -1;
    for (int i = 0; i < t->numCols; i++)
    {
        if (strcmp(s, t->cols[i].label) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

/**
 * Transforms the name if it is still in its aggregate function format.
 * Ex: `avg(z.quant) -> avg_z_quant` or `avg(year) -> avg_year`
 * This function does not check for duplicate names.
 */
string transformName(string to, aggregate op)
{
    if (op == ORG)
    {
        to[0] = '\0';
        return to;
    }
    int parenIndex = 0;
    if ((parenIndex = indexOf(to, '(', LINE_LENGTH)) != -1)
    {
        int endIndex = indexOf(to, '.', LINE_LENGTH);
        int endParenIndex = indexOf(to, ')', LINE_LENGTH);
        char grouping_buffer[STR_LENGTH] = {0};
        char var_buffer[STR_LENGTH] = {0};

        if (endIndex == -1)
        {
            endIndex = endParenIndex;
        }
        else
        {
            strncpy(var_buffer, to + endIndex + 1, endParenIndex - endIndex - 1);
        }

        strncpy(grouping_buffer, to + parenIndex + 1, endIndex - parenIndex - 1);
        switch (op)
        {
        case AVG:
            var_buffer[0] == 0 ? sprintf(to, "avg_%s", grouping_buffer) : sprintf(to, "avg_%s_%s", grouping_buffer, var_buffer);
            break;
        case COUNT:
            var_buffer[0] == 0 ? sprintf(to, "count_%s", grouping_buffer) : sprintf(to, "count_%s_%s", grouping_buffer, var_buffer);
            break;
        case MAX:
            var_buffer[0] == 0 ? sprintf(to, "max_%s", grouping_buffer) : sprintf(to, "max_%s_%s", grouping_buffer, var_buffer);
            break;
        case MIN:
            var_buffer[0] == 0 ? sprintf(to, "min_%s", grouping_buffer) : sprintf(to, "min_%s_%s", grouping_buffer, var_buffer);
            break;
        case SUM:
            var_buffer[0] == 0 ? sprintf(to, "sum_%s", grouping_buffer) : sprintf(to, "sum_%s_%s", grouping_buffer, var_buffer);
            break;
        }
    }
    return to;
}

void transformLabel(aggregateItem *a)
{
    switch (a->agg)
    {
    case ORG:
        strcpy(a->label, a->attribute);
        break;
    case AVG:
        if (a->groupingVar[0])
            sprintf(a->label, "avg_%s_%s", a->groupingVar, a->attribute);
        else
            sprintf(a->label, "avg_%s", a->attribute);
        break;
    case COUNT:
        if (a->groupingVar[0])
            sprintf(a->label, "count_%s_%s", a->groupingVar, a->attribute);
        else
            sprintf(a->label, "count_%s", a->attribute);
        break;
    case MAX:
        if (a->groupingVar[0])
            sprintf(a->label, "max_%s_%s", a->groupingVar, a->attribute);
        else
            sprintf(a->label, "max_%s", a->attribute);
        break;
    case MIN:
        if (a->groupingVar[0])
            sprintf(a->label, "min_%s_%s", a->groupingVar, a->attribute);
        else
            sprintf(a->label, "min_%s", a->attribute);
        break;
    case SUM:
        if (a->groupingVar[0])
            sprintf(a->label, "sum_%s_%s", a->groupingVar, a->attribute);
        else
            sprintf(a->label, "sum_%s", a->attribute);
        break;
    }
}

/**
 * Generates the "default" row struct to be used by the compiled file
 */
void generateRowString(FILE *f, tableStructure *t, table *ta, stringArray *atts)
{
    FPRINT("typedef struct row{\n");
    aggregateList *agg = &ta->aggregates;
    columns *t2 = &ta->S;
    column cur;
    int i;
    char colTypesArray[LINE_LENGTH] = {0};
    strcpy(colTypesArray, "const type colTypes[] = {");

    // For generating the row struct
    for (i = 0; i < t->numCols; i++)
    {
        cur = t->cols[i];
        switch (cur.c_type)
        {
        case STRING:
            FPRINT("\tchar %s[STR_LEN];\n", cur.name);
            strcat(colTypesArray, "STRING,");
            break;
        case INT:
            FPRINT("\tint %s;\n", cur.name);
            strcat(colTypesArray, "INT,");
            break;
        case FLOAT:
            FPRINT("\tfloat %s;\n", cur.name);
            strcat(colTypesArray, "FLOAT,");
            break;
        default:
            break;
        }
    }
    FPRINT("} row;\n\n");

    FPRINT("%s};\n", colTypesArray);

    // Generating the blockMap
    FPRINT("const int blockMap[] = {");
    int multiple = t->maxFinalRows;
    for (i = 0; multiple > 1; i++)
    {
        FPRINT("%d,", t->blockMap[i]);
        multiple /= t->blockMap[i];
    }
    FPRINT("};\n\n");

    // For creating the getRow function
    FPRINT("#define GET_STR(name) copy[nextIndex-1] = '\\0';strcpy(result.name,copy + 1);\n#define GET_INT(var) result.var = atoi(copy); break;\n\nrow getRow(char* line){\n\trow result;int nextIndex = 0;char* segment = line;char copy[STR_LEN] = {0};\n\tfor (size_t colIndex = 0; colIndex < NUM_COLS; colIndex++){\n");
    FPRINT("\t\tnextIndex = indexOf(segment, ',');strncpy(copy, segment, nextIndex);copy[nextIndex] = '\\0';\n\t\tswitch (colIndex){\n");
    for (i = 0; i < t->numCols; i++)
    {
        cur = t->cols[i];
        switch (cur.c_type)
        {
        case STRING:
            if(i == t->numCols - 1){
                FPRINT("\t\tcase %d: GET_STR(%s);result.%s[nextIndex-2] = '\\0';break;\n", i, cur.name, cur.name);
            }
            else{
                FPRINT("\t\tcase %d: GET_STR(%s);break;\n", i, cur.name);
            }
            break;
        case INT:
            FPRINT("\t\tcase %d: GET_INT(%s);\n", i, cur.name);
            break;
        case FLOAT:
            FPRINT("\t\tcase %d: GET_FLOAT(%s);\n", i, cur.name);
            break;
        default:
            break;
        }
    }
    FPRINT("\t\t}\n\t\tsegment += nextIndex + 1;\n\t}\n\treturn result;\n}\n");

    // For generating the selected row struct
    FPRINT("typedef struct selectedRow{\n\tbool active;\n");

    // Getting the aggregate column names
    aggregateItem *a;
    for (i = 0; i < agg->length; i++)
    {
        a = &agg->list[i];
        transformLabel(a);
        switch (a->agg)
        {
        case AVG:
            FPRINT("\tstruct {\n\t\tfloat total;\n\t\tint count;\n\t} %s;\n", a->label);
            break;
        case ORG:
            switch (a->itemType)
            {
            case INT:
                FPRINT("\tint %s;\n", a->attribute);
                break;
            case FLOAT:
                FPRINT("\tfloat %s;\n", a->attribute);
                break;
            case STRING:
                FPRINT("\tchar %s[STR_LEN];\n", a->attribute);
                break;
            default:
                break;
            }
            break;
        default:
            FPRINT("\tint %s;\n", a->label);
            break;
        }
    }
    FPRINT("} selectedRow;\n\n");
}

/**
 * Prints to file f functions to increment the aggregate with name name given by c. The boolean indicates whether or not the aggregate is being initialized
 */
void incrementAggregate(FILE *f, string name, aggregateItem *c, bool initialize, int tabs)
{
    string tabString = (string)malloc(sizeof(char) * (tabs + 1));
    for (size_t i = 0; i < tabs; i++)
    {
        tabString[i] = '\t';
    }
    tabString[tabs] = '\0';
    if (initialize)
    {
        switch (c->agg)
        {
        case AVG:
            FPRINT("%sfinalTable[searchIndex].%s.count = 1;\n%sfinalTable[searchIndex].%s.total = curRow.%s;\n", tabString, name, tabString, name, c->attribute);
            break;
        case COUNT:
            FPRINT("%sfinalTable[searchIndex].%s = 1;\n", tabString, name);
            break;
        case MIN:
            FPRINT("%sif(finalTable[searchIndex].%s == -1 || finalTable[searchIndex].%s > curRow.%s){finalTable[searchIndex].%s = curRow.%s;}\n", tabString, name, name, c->attribute, name, c->attribute);
            break;
        case MAX:
            FPRINT("%sif(finalTable[searchIndex].%s == -1 || finalTable[searchIndex].%s < curRow.%s){finalTable[searchIndex].%s = curRow.%s;}\n", tabString, name, name, c->attribute, name, c->attribute);
            break;
        case SUM:
            FPRINT("%sfinalTable[searchIndex].%s = curRow.%s;\n", tabString, name, c->attribute);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (c->agg)
        {
        case AVG:
            FPRINT("%sfinalTable[searchIndex].%s.count++;\n%sfinalTable[searchIndex].%s.total += curRow.%s;\n", tabString, name, tabString, name, c->attribute);
            break;
        case COUNT:
            FPRINT("%sfinalTable[searchIndex].%s++;\n", tabString, name);
            break;
        case MIN:
            FPRINT("%sif(finalTable[searchIndex].%s == -1 || finalTable[searchIndex].%s > curRow.%s){finalTable[searchIndex].%s = curRow.%s;}\n", tabString, name, name, c->attribute, name, c->attribute);
            break;
        case MAX:
            FPRINT("%sif(finalTable[searchIndex].%s == -1 || finalTable[searchIndex].%s < curRow.%s){finalTable[searchIndex].%s = curRow.%s;}\n", tabString, name, name, c->attribute, name, c->attribute);
            break;
        case SUM:
            FPRINT("%sfinalTable[searchIndex].%s += curRow.%s;\n", tabString, name, c->attribute);
            break;
        default:
            break;
        }
    }
    free(tabString);
}

bool isSelected(columns *c, aggregateItem *a)
{
    for (size_t i = 0; i < c->numCols; i++)
    {
        if (a->agg == c->cols[i].operation && strcmp(a->attribute, c->cols[i].orgColName) == 0)
        {
            return true;
        }
    }
    return false;
}

void getValuesAfterDot(stringArray *sa, string s)
{
    int curIndex = 0;
    while (curIndex < strlen(s) - 1)
    {
        if (s[curIndex] == '.')
        {
            char *temp = (char *)malloc(sizeof(char) * (strlen(s)));
            getValueAfterDot(temp, s + curIndex);
            int i = 0;
            while (temp[i] != ' ')
            {
                i++;
            }
            temp[i] = '\0';
            arrayAdd(sa, temp);
            curIndex += strlen(temp);
        }
        else
        {
            curIndex++;
        }
    }
}

void generateMainHasDependents(FILE *f, table *p, Phi *p2)
{
    const int numChecks = p2->V.length; // Number of grouping variables
    int colIndex = 0;
    aggregateList *aggList = &p->aggregates;
    int i;

    FPRINT("\n\tFILE* f = fopen(FILENAME, \"r\");\n\tfor(int i=0; i<=N; i++){\n\t\tfseek(f, 0, SEEK_SET);\n\t\tfgets(curLine, LINE_LEN, f);\n\t\tif(i==0){\n");
    // In the main while loop
    FPRINT("\t\t\twhile(fgets(curLine, LINE_LEN, f) != NULL){\n\t\t\t\tcurRow=getRow(curLine);\n\t\t\t\tblockIndex=0;\n\t\t\t\tsearchIndex=0;\n\t\t\t\tblockMultiple=blockMap[0];\n");
    // Locate open row
    FPRINT("\t\t\t\twhile(finalTable[searchIndex].active){\n");
    for (i = 0; i < numChecks - 1; i++)
    {
        if (i > 0)
        {
            FPRINT("\t\t\t\t\telse if(blockIndex==%d &&", i);
        }
        else
        {
            FPRINT("\t\t\t\t\tif(blockIndex==0&&");
        }

        switch (aggList->list[i].itemType)
        {
        case STRING:
            FPRINT("STREQ(finalTable[searchIndex].%s, curRow.%s)){\n", aggList->list[i].label, aggList->list[i].attribute);
            break;
        default:
            FPRINT("finalTable[searchIndex].%s == curRow.%s){\n", aggList->list[i].label, aggList->list[i].attribute);
            break;
        }
        FPRINT("\t\t\t\t\tblockIndex++;\n\t\t\t\t\tblockMultiple*=blockMap[blockIndex];\n\t\t\t\t\t}\n");
    }
    if (numChecks > 1)
    { // Print out else if
        FPRINT("\t\t\t\telse if(blockIndex==%d&&", numChecks - 1);
    }
    else
    {
        FPRINT("\t\t\t\tif(blockIndex==%d&&", 0);
    }

    switch (aggList->list[i].itemType)
    {
    case STRING:
        FPRINT("STREQ(finalTable[searchIndex].%s, curRow.%s)){\n", aggList->list[i].label, aggList->list[i].attribute);
        break;
    default:
        FPRINT("finalTable[searchIndex].%s == curRow.%s){\n", aggList->list[i].label, aggList->list[i].attribute);
        break;
    }
    FPRINT("\t\t\t\t\tbreak;\n\t\t\t\t}\n");

    // Print else case
    FPRINT("\t\t\t\telse{\n\t\t\t\t\tsearchIndex += (MAX_FINAL_ROWS / blockMultiple);\n\t\t\t\t}}\n");

    // Print assignment if/else
    colValue *curCol;
    aggregateItem *aggItem;

    /**
     * If the row is already active
     */
    FPRINT("\t\t\t\tif(finalTable[searchIndex].active){\n");
    string name;
    // Non grouping variable aggregates
    for (i = 0; i < p->aggregates.length; i++)
    {
        if (p->aggregates.list[i].groupingVar[0] == '\0')
        {
            incrementAggregate(f, p->aggregates.list[i].label, &p->aggregates.list[i], false, 5);
        }
    }

    FPRINT("\t\t\t\t}\n");
    /**
     * If the row is being initialized
     */
    FPRINT("\t\t\t\telse{\n");
    // Non grouping variable aggregates
    FPRINT("\t\t\t\t\tfinalTable[searchIndex].active = true;\n");
    for (i = 0; i < p->aggregates.length; i++)
    {
        if (!p->aggregates.list[i].groupingVar[0] && p->aggregates.list[i].agg != ORG)
        {
            incrementAggregate(f, p->aggregates.list[i].label, &p->aggregates.list[i], true, 5);
        }
        else if (p->aggregates.list[i].agg == MIN)
        { // Set all minimum aggregates to -1
            FPRINT("\t\t\t\t\tfinalTable[searchIndex].%s = -1;\n", p->aggregates.list[i].label);
        }
        else if (p->aggregates.list[i].agg == ORG)
        {
            switch (p->aggregates.list[i].itemType)
            {
            case STRING:
                FPRINT("\t\t\t\t\tstrcpy(finalTable[searchIndex].%s,curRow.%s);\n", p->aggregates.list[i].label, p->aggregates.list[i].attribute);
                break;
            default:
                FPRINT("\t\t\t\t\tfinalTable[searchIndex].%s = curRow.%s;\n", p->aggregates.list[i].label, p->aggregates.list[i].attribute);
                break;
            }
        }
    }

    FPRINT("\t\t\t\t}\n\t\t\t}\n\t\t}\n");

    char expressionBuffer[LINE_LENGTH] = {0};
    char varBuffer[STR_LENGTH] = {0};
    char afterDotBuffer[STR_LENGTH] = {0};
    FPRINT("\t\telse {\n\t\t\twhile(fgets(curLine, LINE_LEN, f) != NULL){\n\t\t\t\tcurRow=getRow(curLine);\n\n\t\t\t\tfor(searchIndex=0; searchIndex<MAX_FINAL_ROWS; searchIndex++){\n");
    for (i = 0; i < p2->O.length; i++)
    {
        stringArray groupingVarsComp = {0};
        getValuesAfterDot(&groupingVarsComp, p2->O.elems[i]);

        generateExpressionDependent(expressionBuffer, p2->O.elems[i], "finalTable[searchIndex]", "curRow", &groupingVarsComp, &p2->S, &p->S, &p->aggregates);
        getGroupingVar(varBuffer, p2->O.elems[i]);
        FPRINT("\t\t\t\t\tif(i==%d&&%s){\n", i + 1, expressionBuffer);
        FPRINT("\t\t\t\t\t\tif(finalTable[searchIndex].active){\n");
        for (size_t j = 0; j < p->aggregates.length; j++)
        {
            aggItem = &p->aggregates.list[j];
            if (aggItem->groupingVar[0] != '\0' && strcmp(aggItem->groupingVar, varBuffer) == 0)
            {
                incrementAggregate(f, aggItem->label, aggItem, false, 7);
            }
        }
        FPRINT("\t\t\t\t\t\t}\n");
        FPRINT("\t\t\t\t\t\telse{\n");
        FPRINT("\t\t\t\t\t\t\tfinalTable[searchIndex].active = true;\n");
        for (size_t j = 0; j < p->aggregates.length; j++)
        {
            aggItem = &p->aggregates.list[j];
            if (aggItem->groupingVar[0] != '\0' && strcmp(aggItem->groupingVar, varBuffer) == 0)
            {
                incrementAggregate(f, aggItem->label, aggItem, true, 7);
            }
        }
        // free the groupingVars
        for (size_t k = 0; k < groupingVarsComp.length; k++)
        {
            free(groupingVarsComp.elems[k]);
        }
        if (groupingVarsComp.elems)
            free(groupingVarsComp.elems);
        FPRINT("\t\t\t\t\t\t}\n");
        FPRINT("\t\t\t\t\t}\n");
    }

    FPRINT("\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n\n");
}

/**
 * Prints to f all the components required for the `main()` function. This includes scanning through the table, upgrading the aggregates, and printing the final table.
 */
void generateMain(FILE *f, table *p, Phi *p2)
{
    const int numChecks = p2->V.length; // Number of grouping variables
    int colIndex = 0;
    aggregateList *aggList = &p->aggregates;
    int i;
    FPRINT("selectedRow finalTable[MAX_FINAL_ROWS]={0};\n\nint main(){\n");
    // Add variables
    FPRINT("\tchar curLine[LINE_LEN]={0};\n\tint searchIndex=0;\n\tint blockIndex=0;\n\tint blockMultiple=0;\n\trow curRow;\n");

    char expressionBuffer[LINE_LENGTH] = {0};
    char varBuffer[STR_LENGTH] = {0};
    colValue *curCol;
    aggregateItem *aggItem;

    generateMainHasDependents(f, p, p2);

    /**
     * Creating the printing protocol
     */

    // 1. Get largest column
    int colWidth = 0;
    int tempLength = 0;
    for (int i = 0; i < p->aggregates.length; i++)
    {
        if (isSelected(&p->S, &p->aggregates.list[i]))
        {
            tempLength = strlen(p->aggregates.list[i].label);
            if (tempLength > colWidth)
                colWidth = tempLength;
        }
    }
    const int colWidth2 = colWidth + 6;

    // Print header
    FPRINT("\tprintf(\"+%.*s", colWidth2, TABLE_BORDER);
    for (int i = 1; i < p->S.numCols; i++)
    {
        FPRINT("+%.*s", colWidth2, TABLE_BORDER);
    }
    FPRINT("+\\n");
    for (int i = 0; i < p->S.numCols; i++)
    {
        FPRINT("|     ");
        if (p->S.cols[i].label[0])
        {   
            FPRINT("%*s ", colWidth, p->S.cols[i].label);
        }
        else
        {
            // Get the index in the aggregates list
            for (size_t j = 0; j < p->aggregates.length; j++)
            {
                if (p->aggregates.list[j].agg == p->S.cols[i].operation && strcmp(p->aggregates.list[j].attribute, p->S.cols[i].orgColName) == 0 && strcmp(p->aggregates.list[j].groupingVar, p->S.cols[i].grouping_var) == 0)
                {
                    FPRINT("%*s ", colWidth, p->aggregates.list[j].label);
                    break;
                }
            }
        }
    }
    FPRINT("|\\n+%.*s", colWidth2, TABLE_BORDER);
    for (int i = 1; i < p->S.numCols; i++)
    {
        FPRINT("+%.*s", colWidth2, TABLE_BORDER);
    }
    FPRINT("+\\n\");\n");

    if (p2->G != NULL)
    {
        // Need to find the column in selectedTable
        generateHaving(expressionBuffer, p2->G, &p->S, &p->aggregates);
        FPRINT("\tint printedRows = 0;\n\tfor(int i=0;i<MAX_FINAL_ROWS;i++){\n\t\tif(finalTable[i].active && %s){\n\t\t\tprintf(\"|     ", expressionBuffer);
    }
    else
        FPRINT("\tint printedRows = 0;\n\tfor(int i=0;i<MAX_FINAL_ROWS;i++){\n\t\tif(finalTable[i].active){\n\t\t\tprintf(\"|     ");
    for (i = 0; i < p->S.numCols; i++)
    {
        curCol = &p->S.cols[i];
        switch (curCol->colType)
        {
        case INT:
            FPRINT("%%*d");
            break;
        case FLOAT:
            FPRINT("%%*f");
            break;
        default:
            FPRINT("%%*s");
            break;
        }
        FPRINT(" |     ");
    }
    FPRINT("\\n\",");
    int colsPrinted = 0;
    for (i = 0; i < p->S.numCols; i++)
    {
        // Find column index in normal table

        colIndex = -1;
        for (size_t j = 0; j < p->aggregates.length; j++)
        {
            aggItem = &p->aggregates.list[j];
            if (aggItem->agg == p->S.cols[i].operation && strcmp(aggItem->attribute, p->S.cols[i].orgColName) == 0 && strcmp(aggItem->groupingVar, p->S.cols[i].grouping_var) == 0)
            {
                colIndex = j;
                break;
            }
        }
        if (colIndex == -1)
            continue;
        else if (aggItem->agg == AVG)
        {
            FPRINT("%d,finalTable[i].%s.total/finalTable[i].%s.count", colWidth, aggItem->label, aggItem->label);
        }
        else
        {
            FPRINT("%d,finalTable[i].%s", colWidth, aggItem->label);
        }
        if (colsPrinted < p->S.numCols - 1)
        {
            FPRINT(",");
        }
        colsPrinted++;
    }

    FPRINT(");\n\t\t\tprintedRows++;\n\t\t}\n\t}\n");
    // Print bottom line
    FPRINT("\tprintf(\"+%.*s", colWidth2, TABLE_BORDER);
    for (int i = 1; i < p->S.numCols; i++)
    {
        FPRINT("+%.*s", colWidth2, TABLE_BORDER);
    }
    FPRINT("+\\n\");\n");
    FPRINT("\tprintf(\"Total Rows: %%d\\n\",printedRows);\n");
    FPRINT("\tfclose(f);\n}");
}

/**
 * Wrapper function to compile the input from the Phi struct to a C file that can be compiled and run.
 */
void compileAll(string fileName, table *p, Phi *p2)
{
    FILE *f = fopen(fileName, "w+");
    // Write basic headers and type enum
    FPRINT("#include <stdlib.h>\n#include <stdio.h>\n#include <string.h>\n\ntypedef enum type{INT,STRING,FLOAT} type;\n\n#define STR_LEN 64\n#define LINE_LEN 1024\ntypedef char bool;\n#define true 1\n#define false 0\n#define FILENAME \"sales.csv\"\n#define STREQ(x,y) (strcmp(x,y) == 0)\n");
    FPRINT("int indexOf(char* s, char c){int i = 0;while(s[i] != '\\0' && s[i] != '\\n'){if(s[i] == c){return i;}else{i++;}} return i;}\n\n");
    FPRINT("#define NUM_ROWS %ld\n#define NUM_COLS %ld\n#define MAX_FINAL_ROWS %ld\n#define N %d\n", p->structure.numRows, p->structure.numCols, p->structure.maxFinalRows, p2->N);
    generateRowString(f, &p->structure, p, &p2->V);

    generateMain(f, p, p2);

    fclose(f);
}

int main(int argc, char** argv){
    if(argc < 3){
        printExit("Error: Insufficient Arguments!\n\tUsage: <executable> [agc file] <path to csv> <output name>.c\n");
    }
    char inputBuffer[LINE_LENGTH] = {0};
    Phi rawPhi;
    table final;
    string outputFile;

    // Read the AGC file if it is provided, otherwise get user input
    if(argc == 4) {
        rawPhi = readFile(argv[1]);
    } else {
        bool guidedInput = false;
        FILE* temp = fopen("temp.agc", "w+");
        if(temp == NULL) {
            printExit("Error: Could not create temporary file\n");
        }
        CHTXT(CYAN);
        puts("<---- Welcome to the Relational Algebro Kim-piler! ---->");
        CHTXT(YEL);
        puts("You have opted not to include an input file, please select your form of manual input: \n\tg) Guided Input "TXT_GREEN"(Beginner-Friendly)"TXT_YEL"\n\tb) Block (Bulk) input");
        printf(TXT_CYAN"Input type('g' or 'b'): "TXT_DEF);
        fgets(inputBuffer, LINE_LENGTH, stdin);
        if(trim(inputBuffer)[0] == 'g'){
            guidedInput = true;
        }


        if(guidedInput){
            int n;
            puts(TXT_CYAN "You have chosen"TXT_GREEN" Guided Mode"TXT_CYAN", please follow the instructions to create your query.");
            //S Variable
            printf(TXT_GREEN"\tS "TXT_CYAN"--- This argument dictates the projected attributes of your query (select). Projection also allows you to rename columns and specifiy aggregates.\n\t\t Input the S argument here (comma seperated): "TXT_DEF);
            fgets(inputBuffer, LINE_LENGTH, stdin);
            inputBuffer[strlen(inputBuffer)-1] = '\0';
            fprintf(temp, "{%s}\n",inputBuffer);

            //N variable
            printf(TXT_GREEN"\tN "TXT_CYAN"--- This argument specifies the number of grouping variables in your query.\n\t\t Input the N argument here: "TXT_DEF);
            fgets(inputBuffer, LINE_LENGTH, stdin);
            n = atoi(inputBuffer);
            fprintf(temp, "%s",inputBuffer);

            //V Argument
            printf(TXT_GREEN"\tV "TXT_CYAN"--- This argument specifies the group by attributes of your query. These attributes must be present in the select clause.\n\t\t Input the V argument here (comma seperated): "TXT_DEF);
            fgets(inputBuffer, LINE_LENGTH, stdin);
            inputBuffer[strlen(inputBuffer)-1] = '\0';
            fprintf(temp, "{%s}\n",inputBuffer);

            //F argument
            printf(TXT_GREEN"\tF "TXT_CYAN"--- This argument contains a list of aggregate functions for each grouping variable. Implicitly, it also includes aggregates with no grouping variable (Grouping variable 0).\n");
            fprintf(temp, "{");
            for (int i = 0; i <= n; i++){
                printf(TXT_CYAN"\t\tGrouping Variable "TXT_GREEN "#%d"TXT_DEF" (comma seperated): ",i);
                fgets(inputBuffer, LINE_LENGTH, stdin);
                inputBuffer[strlen(inputBuffer)-1] = '\0';
                if(i != n){
                    fprintf(temp, "{%s},",inputBuffer);
                }
                else{
                    fprintf(temp, "{%s}",inputBuffer);
                }
            }
            fprintf(temp, "}\n");

            //Sigma argument
            printf(TXT_GREEN"\tO "TXT_CYAN"--- This argument dictates the 'such that' clause of each grouping variable. Clauses can be dependent of fields of grouping variables that come earlier.\n");
            fprintf(temp, "{");
            for (int i = 0; i < n; i++){
                printf(TXT_CYAN"\t\tSuch that of Grouping Variable "TXT_GREEN "#%d"TXT_DEF": ",i+1);
                fgets(inputBuffer, LINE_LENGTH, stdin);
                inputBuffer[strlen(inputBuffer)-1] = '\0';
                if(i != n - 1){
                    fprintf(temp, "%s,",inputBuffer);
                }
                else{
                    fprintf(temp, "%s",inputBuffer);
                }
            }
            fprintf(temp, "}\n");

            //G argument
            printf(TXT_GREEN"\tG "TXT_CYAN"--- This argument filters out rows that do not meet the specified clause. This is performed before projection.\n\t\t Input the G argument here (comma seperated): "TXT_DEF);
            fgets(inputBuffer, LINE_LENGTH, stdin);
            fprintf(temp, "%s",inputBuffer);
            CHTXT(DEFAULTTXT);
        }
        else{
            printf(TXT_YEL "Enter the AGC file line by line. Enter 'q' to quit."TXT_DEF"\n");
            int i = 0;
            while(i < 6) {
                fgets(inputBuffer, LINE_LENGTH, stdin);
                if(strcmp(inputBuffer, "q\n") == 0){
                    fclose(temp);
                    return 0;
                }
                fprintf(temp, "%s", inputBuffer);
                i++;
            }
        }
        
        fclose(temp);
        rawPhi = readFile("temp.agc");
        remove("temp.agc");
    }

    puts("\n"TXT_YEL"<-- Your file is being generated... -->"TXT_DEF);
    final.structure = parseCSV(argv[argc - 2], &rawPhi);
    final.S = generateFinalTable(&rawPhi, &final.structure);
    final.aggregates = getAggregates(rawPhi.F, rawPhi.N + 1, &rawPhi.V, &final.structure);
    compileAll(argv[argc - 1], &final, &rawPhi);

    PRINT_PERSON;
    printf(TXT_GREEN"\n<-- Generation Complete: See "TXT_CYAN"%s"TXT_GREEN" for the output! -->\n"TXT_DEF, argv[argc - 1]);

    //Free pointers to avoid memory leaks
    free(final.S.cols);
    free(final.structure.blockMap);
    free(final.aggregates.list);
    freeTable(&final.structure);
    freePhi(&rawPhi);
    puts(TXT_YEL"<---- Exiting, Thank you for using the Kim-piler! ---->"TXT_DEF);
    return 0;
}