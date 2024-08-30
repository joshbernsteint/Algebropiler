#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum type{INT,STRING,FLOAT} type;

#define STR_LEN 64
#define LINE_LEN 1024
typedef char bool;
#define true 1
#define false 0
#define FILENAME "sales.csv"
#define STREQ(x,y) (strcmp(x,y) == 0)
int indexOf(char* s, char c){int i = 0;while(s[i] != '\0' && s[i] != '\n'){if(s[i] == c){return i;}else{i++;}} return i;}

#define NUM_ROWS 10000
#define NUM_COLS 8
#define MAX_FINAL_ROWS 90
#define N 1
typedef struct row{
	char cust[STR_LEN];
	char prod[STR_LEN];
	int day;
	int month;
	int year;
	char state[STR_LEN];
	int quant;
	char date[STR_LEN];
} row;

const type colTypes[] = {STRING,STRING,INT,INT,INT,STRING,INT,STRING,};
const int blockMap[] = {9,10,};

#define GET_STR(name) copy[nextIndex-1] = '\0';strcpy(result.name,copy + 1);
#define GET_INT(var) result.var = atoi(copy); break;

row getRow(char* line){
	row result;int nextIndex = 0;char* segment = line;char copy[STR_LEN] = {0};
	for (size_t colIndex = 0; colIndex < NUM_COLS; colIndex++){
		nextIndex = indexOf(segment, ',');strncpy(copy, segment, nextIndex);copy[nextIndex] = '\0';
		switch (colIndex){
		case 0: GET_STR(cust);break;
		case 1: GET_STR(prod);break;
		case 2: GET_INT(day);
		case 3: GET_INT(month);
		case 4: GET_INT(year);
		case 5: GET_STR(state);break;
		case 6: GET_INT(quant);
		case 7: GET_STR(date);result.date[nextIndex-2] = '\0';break;
		}
		segment += nextIndex + 1;
	}
	return result;
}
typedef struct selectedRow{
	bool active;
	char cust[STR_LEN];
	char prod[STR_LEN];
	struct {
		float total;
		int count;
	} avg_x_quant;
} selectedRow;

selectedRow finalTable[MAX_FINAL_ROWS]={0};

int main(){
	char curLine[LINE_LEN]={0};
	int searchIndex=0;
	int blockIndex=0;
	int blockMultiple=0;
	row curRow;

	FILE* f = fopen(FILENAME, "r");
	for(int i=0; i<=N; i++){
		fseek(f, 0, SEEK_SET);
		fgets(curLine, LINE_LEN, f);
		if(i==0){
			while(fgets(curLine, LINE_LEN, f) != NULL){
				curRow=getRow(curLine);
				blockIndex=0;
				searchIndex=0;
				blockMultiple=blockMap[0];
				while(finalTable[searchIndex].active){
					if(blockIndex==0&&STREQ(finalTable[searchIndex].cust, curRow.cust)){
					blockIndex++;
					blockMultiple*=blockMap[blockIndex];
					}
				else if(blockIndex==1&&STREQ(finalTable[searchIndex].prod, curRow.prod)){
					break;
				}
				else{
					searchIndex += (MAX_FINAL_ROWS / blockMultiple);
				}}
				if(finalTable[searchIndex].active){
				}
				else{
					finalTable[searchIndex].active = true;
					strcpy(finalTable[searchIndex].cust,curRow.cust);
					strcpy(finalTable[searchIndex].prod,curRow.prod);
				}
			}
		}
		else {
			while(fgets(curLine, LINE_LEN, f) != NULL){
				curRow=getRow(curLine);

				for(searchIndex=0; searchIndex<MAX_FINAL_ROWS; searchIndex++){
					if(i==1&&strcmp(finalTable[searchIndex].cust,curRow.cust)==0&&strcmp(finalTable[searchIndex].prod,curRow.prod)==0){
						if(finalTable[searchIndex].active){
							finalTable[searchIndex].avg_x_quant.count++;
							finalTable[searchIndex].avg_x_quant.total += curRow.quant;
						}
						else{
							finalTable[searchIndex].active = true;
							finalTable[searchIndex].avg_x_quant.count = 1;
							finalTable[searchIndex].avg_x_quant.total = curRow.quant;
						}
					}
				}
			}
		}
	}

	printf("+-----------------+-----------------+-----------------+\n|            cust |            prod |     avg_x_quant |\n+-----------------+-----------------+-----------------+\n");
	int printedRows = 0;
	for(int i=0;i<MAX_FINAL_ROWS;i++){
		if(finalTable[i].active){
			printf("|     %*s |     %*s |     %*f |     \n",11,finalTable[i].cust,11,finalTable[i].prod,11,finalTable[i].avg_x_quant.total/finalTable[i].avg_x_quant.count);
			printedRows++;
		}
	}
	printf("+-----------------+-----------------+-----------------+\n");
	printf("Total Rows: %d\n",printedRows);
	fclose(f);
}