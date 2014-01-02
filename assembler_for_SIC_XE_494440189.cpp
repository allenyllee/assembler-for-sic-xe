#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
OS:windows XP
language:C
compiler: Visual C++ 2008
測試檔案：test.txt
輸出檔案：object_program.txt
*/

struct lable{
	char name[20];
	char instruction[20];
	char operand[100];
	char objcode[200];
	int Loc;
	int TA;
	int directive;
	int opcode;
	bool isImmediate;
	bool isIndirect;
	bool isIndexed;
};


typedef struct lable Lable;

enum directives{START, BASE, EXTEND, BYTE, RESW, RESB, END};
enum registers{A, X, L, B, S, T, F,PC=8, SW};
enum format2{CLEAR, COMPR, TIXR};
enum instructions{STL, LDB, JSUB, LDA, COMP, JEQ, J, STA, LDT, TD, RD, STCH, JLT, STX, RSUB, LDCH, WD};

int readLable(Lable *lables, int *table);
void assignLoc(Lable *lables, int totalLine);
void assignObjcode(Lable *lables, int *table, int totalLine);
void outputfile(Lable *lables,int totalLine);

int getDirective(char *str);
int getFormat2(char *str);
int getRegister(char *str);
int getInstruction(char *str);

int setBYTE(Lable *lables);
int setFormat(Lable *lables);
void setFormat2(Lable *lables,int format2);
void setOpcode(Lable *lables);
void setObjcode(Lable *lables, int base, int pc);

bool isIndirect(char *str);
bool isImmediate(char *str);
bool isIndexed(char *str);

int main(void){

	Lable lables[1024];
	int table[1024];
	int i, totalLine;
	char address[10];

	totalLine = readLable(lables, table);
	assignLoc(lables, totalLine);
	assignObjcode(lables, table, totalLine);
	outputfile(lables,totalLine);

	for(i=0; i<totalLine; i++){
		sprintf(address,"%.4X",lables[i].Loc);
		printf("%s %s\n", address, lables[i].objcode);
		//printf("%s %s %s %s %s\n", address, lables[i].name, lables[i].instruction, lables[i].operand, lables[i].objcode);	
		
	}
	
	for(i=0; table[i] != -1; i++){
		//sprintf(address,"%.4X",lables[table[i]].Loc);
		//printf("%s %s\n", address, lables[table[i]].name);
	}
	

	
	system("PAUSE");

	return 0;
}



int readLable(Lable *lables, int *table){ //讀取檔案
	FILE *fp = NULL; //檔案 pointer
	char line[1024]; //暫存一行1024個字元 
	char tokens[3][20]; //暫存三個tokens
	char *tokenPtr = NULL; //token 的pointer
	int i, j=0, k=0; //counter
	
	fp = fopen("test3.txt","r");
	
	if(fp != NULL){

		do{
			fgets(line,1024,fp); //將一行讀到line 中
			if(line[0] == '\n') break;

			if( strncmp(line,".",1) != 0 ){ //跳過有"."的註解
				
				tokenPtr = strtok(line," \t\n"); //將line分割token
				
				for(i=0; i<3 && tokenPtr != NULL; i++){ 
					strcpy(tokens[i],tokenPtr); //將切開的token 複製到toekn[]中
					tokenPtr = strtok(NULL," \t\n"); //下一個token
				}
				
				if( (getDirective(tokens[0]) != -1 || getInstruction(tokens[0]) != -1 || getFormat2(tokens[0]) != -1) && i==3) i--;
				//如果第1個讀到的token是directives, instructions, format2, 而且讀到i==3個token，則最後一個token必然是註解，故將i減1
				if( getInstruction(tokens[0]) == RSUB && i != 1) i=1;
				//如果第一個token是RSUB，由於它沒有operand，所以i只能等於1

				switch(i){
					case 3:
						strcpy(lables[k].name, tokens[0]);
						strcpy(lables[k].instruction, tokens[1]);
						strcpy(lables[k].operand, tokens[2]);
						table[j++] = k; //記錄lable 的位置
						break;
					case 2:
						strcpy(lables[k].name, "");
						strcpy(lables[k].instruction, tokens[0]);
						strcpy(lables[k].operand, tokens[1]);
						break;
					case 1:
						strcpy(lables[k].name, "");
						strcpy(lables[k].instruction, tokens[0]);
						strcpy(lables[k].operand, "");
						break;
					default:
						printf("error");
						system("PAUSE");
				}
				
				strcpy(lables[k].objcode, "");
				lables[k].isImmediate = isImmediate(lables[k].operand);
				lables[k].directive = getDirective(lables[k].instruction);
				lables[k].isIndirect = isIndirect(lables[k].operand);
				lables[k].isIndexed = isIndexed(lables[k].operand);

				k++;

			}
			
		}while(!feof(fp));
		
		table[j] = -1; //-1作為table的結尾
		fclose(fp); //關閉檔案

	}
	
	return k; //回傳總共讀取行數
}


void assignLoc(Lable *lables, int totalLine){//計算位置
	int i, Loc;

	for(i=0;i<totalLine;i++){
		
		switch( lables[i].directive ){
			case START:
				lables[i].Loc = -1;
				Loc = strtol(lables->operand,NULL,16);
				break;
			case BASE:
				lables[i].Loc = -1;
				break;
			case EXTEND:
				lables[i].Loc = Loc;
				Loc += 4;
				break;
			case BYTE:
				lables[i].Loc = Loc;
				Loc += setBYTE(&lables[i]);
				break;
			case RESW:
				lables[i].Loc = Loc;
				Loc += ( 3 * atoi(lables[i].operand) );
				break;
			case RESB:
				lables[i].Loc = Loc;
				Loc += atoi( lables[i].operand );
				break;
			case END:
				lables[i].Loc = Loc;
				break;
			case -1:
				lables[i].Loc = Loc;
				Loc += setFormat(&lables[i]);
				break;
			default:
				printf("cannot resolution");
				system("PAUSE");
		
		}
	
	}

}

void assignObjcode(Lable *lables, int *table, int totalLine){//得到objcode
	int i, j, k, base=0, pc=0;

 	for(j=0; j<totalLine; j++){
		
		if( lables[j].objcode[0] == 0 ){ //排除掉已處理過的Format2
			
			if(lables[j].operand[0] != 0){ //排除沒有operand的指令，如RSUB
				
				k=1;
				do{
					pc = lables[j+k].Loc;//設定pc值為下一行指令的起始位置
					k++;
				}while( pc < 0);

				for(i=0; table[i] != -1; i++){ //table中記錄著lable 的位置
					
					if( strstr( lables[j].operand, lables[table[i]].name ) != NULL ){ //將當前指令的operand 與table 中的lable 做比較
						if(lables[j].directive == BASE){ //如果當前指令為BASE
							base = lables[table[i]].Loc; //更新assembler 的base 值
						}else if(lables[j].directive == -1 || lables[j].directive == EXTEND){ //如果是Format4或format3，包含有lable的立即值與indirect addessing
							lables[j].TA = lables[table[i]].Loc; //將lable 的位置存入TA
							setOpcode(&lables[j]); //取得指令的opcode
							setObjcode(&lables[j], base, pc);
						}
						break;
					}
					
				}
				
				if( table[i] == -1 && (lables[j].directive == -1 || lables[j].directive == EXTEND) ){//format3 or format4且可能為數字立即值
					if(lables[j].isImmediate){ //如果是立即值
						lables[j].TA = strtol(lables[j].operand, NULL, 10); //將立即值轉成10進位整數，存入TA
						setOpcode(&lables[j]); //取得指令的opcode
						setObjcode(&lables[j], -1, -1);
					}else{ //都不是的話，就是錯誤的lable
						printf("lable is not defined");
						system("PAUSE");
					}
				}
			
			}else{ //沒有operand 的指令，如RSUB
				lables[j].TA = -1;
				setOpcode(&lables[j]);
				setObjcode(&lables[j], base, pc);
			}
		
		}
		
	}

}

void outputfile(Lable *lables, int totalLine){
	FILE *fp = NULL;
	char string[100],location[10], *remainderPtr=NULL;
	int start,j,k,length=0, x;
	fp = fopen("object_program.txt","w");

	if(fp != NULL){
		fprintf(fp,"%s","H");
		start = 0;
		if( lables[start].directive == START ){
			
			//H records
			strcpy(string, lables[start].name);
			while( strlen(string) != 6 ){
				strcat(string," ");
			}
			fprintf(fp,"%s",string);
			sprintf(string,"%.6X",lables[start+1].Loc);
			fprintf(fp,"%s",string);
			sprintf(string, "%.6X", lables[totalLine-1].Loc);
			fprintf(fp,"%s\n",string);
			
			//T records
			j = start + 1;
			while(j<totalLine){
				
				while(lables[j].directive == RESW || lables[j].directive == RESB){
					j++;
				}

				strcpy(string,"");
				fprintf(fp,"%s","T");
				sprintf(location,"%.6X",lables[j].Loc);
				fprintf(fp,"%s",location);
				
				
				
				while(j<totalLine && length < 30){
					strcat(string,lables[j].objcode);
					j++;
					length = (strlen(string) + strlen(lables[j].objcode))/2;
					if(lables[j].directive == RESW || lables[j].directive == RESB) break;
				}
				
				
				length = length - strlen(lables[j].objcode)/2;
				
				sprintf(location,"%.2X",length);
				fprintf(fp,"%s",location);
				fprintf(fp,"%s\n",string);
				
				while(lables[j].directive == RESW || lables[j].directive == RESB){
					j++;
				}
			}
			
			//M records
			for(k=0; k<totalLine; k++){
				strtol(lables[k].operand, &remainderPtr, 10);
				if( lables[k].directive == EXTEND && strcmp(remainderPtr,"") != 0 ){
					fprintf(fp,"%s","M");
					sprintf(location,"%.6X",lables[k].Loc + 1);
					fprintf(fp,"%s%s\n",location,"05");
				}

			}

			//end record
			if( lables[totalLine-1].directive == END){
				sprintf(location,"%.6X",lables[start + 1].Loc);
				fprintf(fp,"%s%s\n","E",location);
			}else{
				printf("no end records");
			}
			

		}else{
			printf("format error");
		}

	
	}else{
		printf("file cannot open");
	}

	fclose(fp);

}




bool isIndirect(char *str){//判斷indirect
	if( strncmp(str,"@",1) == 0 ) {
		strcpy(str,str+1);
		return true;
	}
	return false;
}

bool isImmediate(char *str){//判斷立即值
	if( strncmp(str,"#",1) == 0 ) {
		strcpy(str, str+1);
		return true;
	}
	return false;
}

bool isIndexed(char *str){//判斷index
	char *string = NULL;
	string = strstr(str, ",X");
	if( string != NULL){ 
		strtok(str,",");
		return true;
	}
	return false;
}

void setObjcode(Lable *lables, int base, int pc){//計算objcode
	int n,i,x,b,p,e,xbpe,disp,opcode;
	char string[10];

	if(lables->isImmediate){ //是否為立即值
		n=0;
		i=1;
		x=0;
	}else if(lables->isIndirect){ //是否為indirect
		n=1*2;
		i=0;
		x=0;
	}else{ //若都不是，則ni的bit值為11
		n=1*2;
		i=1;
		if(lables->isIndexed){ //是否為indexed
			x=1*8;
		}else{
			x=0;
		}
	}
	
	opcode = lables->opcode +n +i;

	if(lables->TA != -1){ //有operand時
		
		if(lables->directive == EXTEND){//format4
			b=0;
			p=0;
			e=1;
			xbpe = x+b+p+e;
			
			sprintf(string,"%.2X",opcode);
			strcpy(lables->objcode,string);
			sprintf(string,"%.1X",xbpe);
			strcat(lables->objcode,string);
			sprintf(string,"%.5X",lables->TA);
			strcat(lables->objcode,string);
	
		}else if(pc != -1 && base != -1){//format3 且非數字立即值
			e=0;
			
			if( (disp = lables->TA - pc) <= 2047 && disp >= -2048){ //檢查pc relative 的disp是否在-2048 ~ +2047 間
				b = 0;
				p = 1*2;
			}else if( (disp = lables->TA - base) >= 0 && disp <= 4095){ //檢查base relative 的disp是否在0~4095 間
				b = 1*4;
				p = 0;
			}else{ //若都不在此範圍內，則程式錯誤
				printf("disp error\n");
				system("PAUSE");
			}
			
			xbpe = x+b+p+e;
	
			sprintf(string,"%.2X",opcode);
			strcpy(lables->objcode,string);
			sprintf(string,"%.1X",xbpe);
			strcat(lables->objcode,string);
			sprintf(string,"%.3hX",disp);
			if(strlen(string) == 4) strcpy(string,string+1);
			strcat(lables->objcode,string);
	
		}else{ //format3為數字立即值
			b=0;
			p=0;
			e=0;
			xbpe = x+b+p+e;
			disp = lables->TA;
			
			sprintf(string,"%.2X",opcode);
			strcpy(lables->objcode,string);
			sprintf(string,"%.1X",xbpe);
			strcat(lables->objcode,string);
			sprintf(string,"%.3X",disp);
			strcat(lables->objcode,string);
		}
	
	}else{ //沒有operand
		sprintf(string,"%.2X",opcode);
		strcpy(lables->objcode,string);
		strcat(lables->objcode,"0000");
	}


}


int getFormat2(char *str){//判斷format2
	char string[3][10]={"CLEAR","COMPR","TIXR"};
	int i;

	for(i=CLEAR;i<=TIXR;i++){
		if( strcmp(string[i], str) == 0 ) return i;
	}
	return -1;
}

int getInstruction(char *str){//判斷format3, foramt4 的instruction
	char string[20][10] = {"STL", "LDB", "JSUB", "LDA", "COMP", "JEQ", "J", "STA", "LDT", "TD", "RD", "STCH", "JLT", "STX", "RSUB", "LDCH", "WD"};
	int i;
	if( strncmp(str,"+",1) == 0 ) str++;
	for(i=STL;i<=WD;i++){
		if( strcmp(str,string[i]) == 0 ) return i;
	}
	return -1;
}

int getDirective(char *str){//判斷assembly directive
	char string[10][10] = {"START", "BASE", "+", "BYTE", "RESW", "RESB", "END"};
	int i;
	for(i=START;i<=END;i++){
		if( strcmp(str,string[i]) == 0 ) return i;
	}
	if( strncmp(str,"+",1) == 0 ) return EXTEND;
	return -1;
}

int getRegister(char *str){//判斷register
	char string[20][3] = {"A", "X", "L", "B", "S", "T", "F"," ","PC", "SW"};
	int i;
	if(str != NULL){
		for(i=A;i<=SW;i++){
			if(strcmp(str,string[i]) == 0) return i;
		}
	}
	return -1;

}

void setOpcode(Lable *lables){ //取得format3,4 之opcode
	int instruction;

	instruction = getInstruction(lables->instruction);

	switch(instruction){
		case STL:
			lables->opcode = strtol("14", NULL, 16);
			break;
		case LDB:
			lables->opcode = strtol("68", NULL, 16);
			break;
		case JSUB:
			lables->opcode = strtol("48", NULL, 16);
			break;
		case LDA:
			lables->opcode = strtol("00", NULL, 16);
			break;
		case COMP:
			lables->opcode = strtol("28", NULL, 16);
			break;
		case JEQ:
			lables->opcode = strtol("30", NULL, 16);
			break;
		case J:
			lables->opcode = strtol("3C", NULL, 16);
			break;
		case STA:
			lables->opcode = strtol("0C", NULL, 16);
			break;
		case LDT:
			lables->opcode = strtol("74", NULL, 16);
			break;
		case TD:
			lables->opcode = strtol("E0", NULL, 16);
			break;
		case RD:
			lables->opcode = strtol("D8", NULL, 16);
			break;
		case STCH:
			lables->opcode = strtol("54", NULL, 16);
			break;
		case JLT:
			lables->opcode = strtol("38", NULL, 16);
			break;
		case STX:
			lables->opcode = strtol("10", NULL, 16);
			break;
		case RSUB:
			lables->opcode = strtol("4C", NULL, 16);
			break;
		case LDCH:
			lables->opcode = strtol("50", NULL, 16);
			break;
		case WD:
			lables->opcode = strtol("DC", NULL, 16);
			break;
		default:
			printf("not defined");
			break;
	}
}

int setFormat(Lable *lables){
	int format2;
	format2 = getFormat2(lables->instruction);

	if(format2 != -1){
		setFormat2(lables, format2);
		return 2;
	}else{
		return 3;
	}
}

void setFormat2(Lable *lables, int format2){
	char string[20], string2[2][3];
	char *tokenPtr;
	int i=0;

	strcpy(string, lables->operand);
	tokenPtr = strtok(string, ",");
	
	while(tokenPtr != NULL){
		sprintf(string2[i], "%d", getRegister(tokenPtr) );
		tokenPtr = strtok(NULL, ",");
		i++;
	}

	if(i==1){
		sprintf(string2[i], "%d", 0);
	}

	switch(format2){
		case CLEAR:
			strcpy(lables->objcode, "B4");
			break;
		case COMPR:
			strcpy(lables->objcode, "A0");
			break;
		case TIXR:
			strcpy(lables->objcode, "B8");
			break;
		default:
			printf("error");
	}
	
	for(i=0;i<2;i++){
		strcat(lables->objcode, string2[i]);
	}

}

int setBYTE(Lable *lables){
	char string[20],string2[200];
	char *tokenPtr;
	int i, length;
	
	if(lables->operand != NULL){
		strcpy(string, lables->operand);
		tokenPtr = strtok(string, "'");
		tokenPtr = strtok(NULL,"'");
	}
	
	if( strncmp(lables->operand, "C", 1) == 0 ){
		length = strlen(tokenPtr);
		for(i=0; i<length; i++){
			sprintf(&string2[2*i], "%X", tokenPtr[i]);
		}
		strcpy(lables->objcode, string2);
		return length;
	}else if( strncmp(lables->operand, "X", 1) == 0 ){
		strcpy(lables->objcode, tokenPtr);
		return strlen(tokenPtr)/2;
	}
	return 0;
}