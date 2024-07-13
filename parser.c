/*
	Eric Straub
	er831564
	Systems Software, Fall 21
	Euripedes Montagne
	Project 4 PL/0+ Parser/Code Generator

	This is the skeleton parser.c for the UCF Fall 2021 Systems Software 
	Project, HW4. Add whatever you want, but make sure to leave the
	print functions as they are to make sure you get your grade as
	quickly as possible. There is a line commented out just before
	the return statement in parse(). Make sure to uncomment it
	after you implement the function, your vm.c may malfunction
	without it.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 1000
#define MAX_SYMBOL_COUNT 100

typedef enum opcode{
	LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SYS
} opcode;
typedef enum opr{
	RTN = 0, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ
}opr;

instruction *code;
int cIndex;
symbol *table;
int tIndex;
int lIndex;		// points to the next lexeme to be loaded into curLex
lexeme curLex;
int level;		// tracks the current lexicographical level
int error;

void emit(int opname, int level, int mvalue);
void addToSymbolTable(int k, char n[], int v, int l, int a, int m);
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();
void program(lexeme *list);
void block(lexeme *list);
void constDec(lexeme *list);
int varDec(lexeme *list);
void procDec(lexeme *list);
void statement(lexeme *list);
void condition(lexeme *list);
void expression(lexeme *list);
void term(lexeme *list);
void factor(lexeme *list);
lexeme getLex(lexeme *list);
int multDecCheck(lexeme test);
int findSymbol(lexeme test, int kind);
int mark(symbol *table);

instruction *parse(lexeme *list, int printTable, int printCode)
{
	code = malloc(sizeof(instruction) * MAX_CODE_LENGTH);
	table = malloc(sizeof(symbol) * MAX_SYMBOL_COUNT);
	cIndex = 0;
	tIndex = 0;
	lIndex = 0;
	level = 0;
	error = 0;
	program(list);
	if (error == 1) return NULL;
	
	if (printTable)
		printsymboltable();
	if (printCode)
		printassemblycode();
	
	code[cIndex].opcode = -1;
	return code;
}

int mark(symbol *table){
	int i = tIndex;
	while(1){
		if(table[i].mark == 0 && table[i].level < level || i < 0) {
			break;
		}

		if (table[i].mark != 1){
			table[i].mark = 1;
		}
		i--;
	}
}

void program(lexeme *list){
	// printf("program\n");

	emit(JMP, 0, 0);
	addToSymbolTable(3, "main", 0, 0, 0, 0);
	level = -1;
	getLex(list);
	block(list);
	if (error == 1) return;
	if (curLex.type != periodsym){
		printparseerror(1); // ERROR 1
	}
	emit(SYS, 0, 3);
	for(int i = 0; i < cIndex; i++){
		if (code[i].opcode == CAL){
			code[i].m = table[code[i].m].addr;
		}
	}
	code[0].m = table[0].addr;
	return;
}

void block(lexeme *list){
	// printf("block");

	if (error == 1) return;

	level++;
	int procedure_idx = tIndex - 1;
	constDec(list);
	if (error == 1) return;
	int numVars = varDec(list);
	procDec(list);
	if (error == 1) return;
	table[procedure_idx].addr = cIndex * 3;

	// increment the SP past the AR and data
	// if (level == 0) emit(INC, 0, numVars - 1);
	// else emit(INC, 0, numVars + 2);

	if (level == 0) emit(INC, 0, numVars);
	else emit(INC, 0, numVars + 4);

	statement(list);
	if (error == 1) return;
	mark(table);
	if (error == 1) return;
	level--;
}

void constDec(lexeme *list){
	// printf("constDec\n");

	// see 'const'
	if (curLex.type == constsym){
		do {
			getLex(list);
			// see identifier symbol 14
			if (curLex.type != identsym){
				printparseerror(2);
			}

			int symidx = multDecCheck(curLex);
			if (symidx != -1){
				printparseerror(18);
			}
			
			// For constants, you must store kind, name, value, level, and mark.
			// do this once we have the value, for now store the name

			char identName[12];
			strcpy(identName, curLex.name);

			getLex(list);
			// see ':='
			if (curLex.type != assignsym){
				printparseerror(2);
			}

			getLex(list);
			// see a number
			if (curLex.type != numbersym){
				printparseerror(2);
			}

			// create symbol kind const = 1, stored name, value from curlex, level, address NULL, and unmarked = 0
			addToSymbolTable(1, identName, curLex.value, level, 0, 0);
			getLex(list);


		} while (curLex.type == commasym);
		if (curLex.type != semicolonsym){
			// check if theres no comma between identifiers or if they just fogor the semicolon
			if (curLex.type == identsym){
				printparseerror(13);
			} else {
				printparseerror(14);
			}
		}
		getLex(list);
	}
}

int varDec(lexeme *list){
	// printf("varDec\n");

	if (error == 1) return -1;

	int numvars = 0;
	if (curLex.type == varsym){
		do {
			numvars++;
			getLex(list);

			if (curLex.type != identsym){
				printparseerror(3);
			}

			int symidx = multDecCheck(curLex);
			if (symidx != -1){
				printparseerror(18);
			}

			// THIS MAYBE THE PROBLEM
			if (level == 0){
				addToSymbolTable(2, curLex.name, 0, level, numvars - 1, 0);
			} else {
				addToSymbolTable(2, curLex.name, 0, level, numvars + 3, 0);
			}

			getLex(list);

		} while (curLex.type == commasym);
		if (curLex.type != semicolonsym){
			// check if theres no comma between identifiers or if they just fogor the semicolon
			if (curLex.type == identsym){
				printparseerror(13);
			} else {
				printparseerror(14);
			}
		}
		getLex(list);
	}
	return numvars;
}

void procDec(lexeme *list){
	// printf("procDec\n");

	if (error == 1) return;

	while (curLex.type == procsym){
		getLex(list);
		if (curLex.type != identsym){
			printparseerror(4);
		}
		int symidx = multDecCheck(curLex);
		if (symidx != -1){
			printparseerror(18);
		}
		addToSymbolTable(3, curLex.name, 0, level, 0, 0);
		getLex(list);
		if (curLex.type != semicolonsym){
			printparseerror(14);
		}
		getLex(list);	// advance past semicolon before calling block
		block(list);
		if (curLex.type != semicolonsym){
			printparseerror(14);
		}
		getLex(list);
		emit(OPR, 0, RTN);
	}
}

void statement(lexeme *list){
	// printf("statement\n");

	if (error == 1) return;

	// assignment case
	if (curLex.type == identsym){
		int symidx = findSymbol(curLex, 2);
		if (symidx == -1){ // symbol not found
			// These are both errors
			if (findSymbol(curLex, 1) != findSymbol(curLex, 3)){
				printparseerror(18);
				return;
			} else {
				printparseerror(19);
				return;
			}
		}
		getLex(list);
		if (curLex.type != assignsym){
			printparseerror(5);
			return;
		}
		getLex(list);
		expression(list);
		if (error == 1) return;
		emit(STO, level - table[symidx].level, table[symidx].addr);
		return;
	}

	if (curLex.type == dosym){
		do {
			getLex(list);
			statement(list);
			if (error == 1) return;
		} while (curLex.type == semicolonsym);

		if (curLex.type != odsym && curLex.type){
			if (curLex.type != identsym && curLex.type != dosym && curLex.type != whensym && curLex.type != whilesym && curLex.type != readsym && curLex.type != writesym && curLex.type != callsym){
				printparseerror(16);
				return;
			} else {
				printparseerror(15);
				return;
			}
		}
		getLex(list);
		return;
	}

	if (curLex.type == whensym){
		getLex(list);
		condition(list);
		if (error == 1) return;
		int jpcidx = cIndex;
		emit(JPC, 0, 0);
		if (curLex.type != dosym){
			printparseerror(8);
			return;
		}
		getLex(list);
		statement(list);
		if (error == 1) return;
		if (curLex.type == elsedosym){
			int jmpidx = cIndex;
			emit(JMP, 0, 0);
			code[jpcidx].m = cIndex * 3;
			getLex(list);
			statement(list);
			if (error == 1) return;
			code[jmpidx].m = cIndex * 3;
		} else {
			code[jpcidx].m = cIndex * 3;
		}
		return;
	}

	if (curLex.type == whilesym){
		getLex(list);
		int loopidx = cIndex;
		condition(list);
		if (error == 1) return;
		if (curLex.type != dosym){
			printparseerror(9);
			return;
		}
		getLex(list);
		int jpcidx = cIndex;
		emit(JPC, 0, 0);
		statement(list);
		if (error == 1) return;
		emit(JMP, 0, loopidx * 3);
		code[jpcidx].m = cIndex * 3;
		return;
	}

	if (curLex.type == readsym){
		getLex(list);
		if (curLex.type != identsym){
			printparseerror(6);
			return;
		}
		int symidx = findSymbol(curLex, 2);
		if (symidx == -1){
			if (findSymbol(curLex, 1) != findSymbol(curLex, 3)){
				printparseerror(18);
				return;
			} else {
				printparseerror(19);
				return;
			}
		}
		getLex(list);
		emit(SYS, 0, 2);
		emit(STO, level-table[symidx].level, table[symidx].addr);
		return;
	}

	if (curLex.type == writesym){
		getLex(list);
		expression(list);
		if (error == 1) return;
		emit(SYS, 0, 1);
		return;
	}

	if (curLex.type == callsym){
		getLex(list);
		int symidx = findSymbol(curLex, 3);
		if (symidx == -1){
			if (findSymbol(curLex, 1) != findSymbol(curLex, 2)){
				printparseerror(18);
				return;
			} else {
				printparseerror(19);
				return;
			}
		}
		getLex(list);
		emit(CAL, level - table[symidx].level, symidx);
	}
}

void condition(lexeme *list){
	// printf("condition\n");

	if (error == 1) return;

	if (curLex.type == oddsym){
		getLex(list);
		expression(list);
		if (error == 1) return;
		emit(OPR, 0, ODD);
	} else {
		expression(list);
		if (curLex.type == eqlsym){
			getLex(list);
			expression(list);
			if (error == 1) return;
			emit(OPR, 0, EQL);
		} else if (curLex.type == neqsym){
			getLex(list);
			expression(list);
			if (error == 1) return;
			emit(OPR, 0, NEQ);
		} else if (curLex.type == lsssym){
			getLex(list);
			expression(list);
			if (error == 1) return;
			emit(OPR, 0, LSS);
		} else if (curLex.type == leqsym){
			getLex(list);
			expression(list);
			if (error == 1) return;
			emit(OPR, 0, LEQ);
		} else if (curLex.type == gtrsym){
			getLex(list);
			expression(list);
			if (error == 1) return;
			emit(OPR, 0, GTR);
		} else if (curLex.type == geqsym){
			getLex(list);
			expression(list);
			if (error == 1) return;
			emit(OPR, 0, GEQ);
		} else {
			printparseerror(10);
			return;
		}
	}
}

void expression(lexeme *list){
	// printf("expression\n");

	if (error == 1) return;

	if (curLex.type == subsym){
		getLex(list);
		term(list);
		if (error == 1) return;
		emit(OPR, 0, NEG);
		while (curLex.type == addsym || curLex.type == subsym){
			if (curLex.type == addsym){
				getLex(list);
				term(list);
				if (error == 1) return;
				emit(OPR, 0, ADD);

			} else {
				getLex(list);
				term(list);
				if (error == 1) return;
				emit(OPR, 0, SUB);
			}
		}
	} else {
		if (curLex.type == addsym){
			getLex(list);
		}
		term(list);
		if (error == 1) return;
		while (curLex.type == addsym || curLex.type == subsym){
			if (curLex.type == addsym){
				getLex(list);
				term(list);
				if (error == 1) return;
				emit(OPR, 0, ADD);

			} else {
				getLex(list);
				term(list);
				if (error == 1) return;
				emit(OPR, 0, SUB);
			}
		}
	}
	if (curLex.type == lparensym || curLex.type == identsym || curLex.type == numbersym || curLex.type == oddsym){	
		printparseerror(17);
		return;
	}
}

void term(lexeme *list){
	// printf("term\n");

	if (error == 1) return;

	factor(list);
	if (error == 1) return;
	while(curLex.type == multsym || curLex.type == divsym || curLex.type == modsym){
		if(curLex.type == multsym){
			getLex(list);
			factor(list);
			if (error == 1) return;
			emit(OPR, 0, MUL);
		} else if (curLex.type == divsym){
			getLex(list);
			factor(list);
			if (error == 1) return;
			emit(OPR, 0, DIV);
		} else {
			getLex(list);
			factor(list);
			if (error == 1) return;
			emit(OPR, 0, MOD);
		}
	}
}

void factor(lexeme *list){
	// printf("factor\n");

	if (error == 1) return;

	if (curLex.type == identsym){
		int symidx_var = findSymbol(curLex, 2);
		int symidx_const = findSymbol(curLex, 1);
		if (symidx_var == -1 && symidx_const == -1){
			if (findSymbol(curLex, 3) != -1){
				printparseerror(11);
				return;
			} else {
				printparseerror(19);
				return;
			}
		}
		if (symidx_var == -1){	//const
			emit(LIT, 0, table[symidx_const].val);
		} else if (symidx_const == -1 || table[symidx_var].level > table[symidx_const].level){	//var
			emit(LOD, level - table[symidx_var].level, table[symidx_var].addr);
		} else {
			emit(LIT, 0, table[symidx_const].val);
		}
		getLex(list);
	} else if (curLex.type == numbersym){
		emit(LIT, 0, curLex.value);
		getLex(list);
	} else if (curLex.type == lparensym){
		getLex(list);
		expression(list);
		if (error == 1) return;
		if (curLex.type != rparensym){
			printparseerror(12);
			return;
		}
		getLex(list);
	} else {
		printparseerror(11);
		return;
	}
}

lexeme getLex(lexeme *list){
	// printf("Parser getting lex #%d: %s\n", lIndex, list[lIndex].name);
	curLex = list[lIndex];
	lIndex++;
}

int multDecCheck(lexeme test){
	for (int i = 0; i < tIndex; i++){
		// if name is the same, symbol is unmarked, and at the same level, return it's index
		if (!strcmp(table[i].name, test.name) && !table[i].mark && table[i].level == level){
			return i;
		} else return -1; // else -1
	}
}

// returns index of symbol or -1 if not found
int findSymbol(lexeme test, int kind){
	int highestLevel = -1;
	for (int i = 0; i < tIndex; i++){
		// if names are the same, kind is the same, symbol is unmarked,
		// and (no prev highest or the level is higher than previous highest)
		if (!strcmp(table[i].name, test.name) && table[i].kind == kind && table[i].mark == 0 && (highestLevel == -1 || table[i].level > table[highestLevel].level)){
			highestLevel = i;
		}
	}
	return highestLevel;
}

void emit(int opname, int level, int mvalue)
{
	code[cIndex].opcode = opname;
	code[cIndex].l = level;
	code[cIndex].m = mvalue;
	cIndex++;
}

void addToSymbolTable(int k, char n[], int v, int l, int a, int m)
{
	table[tIndex].kind = k;
	strcpy(table[tIndex].name, n);
	table[tIndex].val = v;
	table[tIndex].level = l;
	table[tIndex].addr = a;
	table[tIndex].mark = m;
	tIndex++;
}

void printparseerror(int err_code)
{
	switch (err_code)
	{
		case 1:
			printf("Parser Error: Program must be closed by a period\n");
			break;
		case 2:
			printf("Parser Error: Constant declarations should follow the pattern 'ident := number {, ident := number}'\n");
			break;
		case 3:
			printf("Parser Error: Variable declarations should follow the pattern 'ident {, ident}'\n");
			break;
		case 4:
			printf("Parser Error: Procedure declarations should follow the pattern 'ident ;'\n");
			break;
		case 5:
			printf("Parser Error: Variables must be assigned using :=\n");
			break;
		case 6:
			printf("Parser Error: Only variables may be assigned to or read\n");
			break;
		case 7:
			printf("Parser Error: call must be followed by a procedure identifier\n");
			break;
		case 8:
			printf("Parser Error: when must be followed by do\n");
			break;
		case 9:
			printf("Parser Error: while must be followed by do\n");
			break;
		case 10:
			printf("Parser Error: Relational operator missing from condition\n");
			break;
		case 11:
			printf("Parser Error: Arithmetic expressions may only contain arithmetic operators, numbers, parentheses, constants, and variables\n");
			break;
		case 12:
			printf("Parser Error: ( must be followed by )\n");
			break;
		case 13:
			printf("Parser Error: Multiple symbols in variable and constant declarations must be separated by commas\n");
			break;
		case 14:
			printf("Parser Error: Symbol declarations should close with a semicolon\n");
			break;
		case 15:
			printf("Parser Error: Statements within do-od must be separated by a semicolon\n");
			break;
		case 16:
			printf("Parser Error: do must be followed by od\n");
			break;
		case 17:
			printf("Parser Error: Bad arithmetic\n");
			break;
		case 18:
			printf("Parser Error: Confliciting symbol declarations\n");
			break;
		case 19:
			printf("Parser Error: Undeclared identifier\n");
			break;
		default:
			printf("Implementation Error: unrecognized error code\n");
			break;
	}
	
	free(code);
	free(table);
}

void printsymboltable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < tIndex; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr, table[i].mark); 
	
	free(table);
	table = NULL;
}

void printassemblycode()
{
	int i;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < cIndex; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
			case 1:
				printf("LIT\t");
				break;
			case 2:
				switch (code[i].m)
				{
					case 0:
						printf("RTN\t");
						break;
					case 1:
						printf("NEG\t");
						break;
					case 2:
						printf("ADD\t");
						break;
					case 3:
						printf("SUB\t");
						break;
					case 4:
						printf("MUL\t");
						break;
					case 5:
						printf("DIV\t");
						break;
					case 6:
						printf("ODD\t");
						break;
					case 7:
						printf("MOD\t");
						break;
					case 8:
						printf("EQL\t");
						break;
					case 9:
						printf("NEQ\t");
						break;
					case 10:
						printf("LSS\t");
						break;
					case 11:
						printf("LEQ\t");
						break;
					case 12:
						printf("GTR\t");
						break;
					case 13:
						printf("GEQ\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			case 3:
				printf("LOD\t");
				break;
			case 4:
				printf("STO\t");
				break;
			case 5:
				printf("CAL\t");
				break;
			case 6:
				printf("INC\t");
				break;
			case 7:
				printf("JMP\t");
				break;
			case 8:
				printf("JPC\t");
				break;
			case 9:
				switch (code[i].m)
				{
					case 1:
						printf("WRT\t");
						break;
					case 2:
						printf("RED\t");
						break;
					case 3:
						printf("HAL\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			default:
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	if (table != NULL)
		free(table);
}