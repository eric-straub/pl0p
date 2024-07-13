/*
	Eric Straub
	er831564
	Systems Software, Fall 21
	Euripedes Montagne
	Project 4 PL/0+ Lexical Analyzer

	This is the skeleton lex.c for the UCF Fall 2021 Systems Software 
	Project, HW4. Add whatever you want, but make sure to leave the
	print functions as they are to make sure you get your grade as
	quickly as possible. There is a line commented out just before
	the return statement in lexanalyzer(). Make sure to uncomment it
	after you implement the function, your parser.c may malfunction
	without it.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#define MAX_NUMBER_TOKENS 1000
#define MAX_IDENT_LEN 11
#define MAX_NUMBER_LEN 5


lexeme *list;
int lex_index;

void printlexerror(int type);
void printtokens();
lexeme tokenize(char* str, int type); // type 1, identifier or reserved word, 2, number, 3, special symbol
int issymbol(char c);
int isreserved(char *str);
int getSymType(char *sym);


lexeme *lexanalyzer(char *input, int printFlag)
{
	list = malloc(sizeof(lexeme) * 500);
	char curWord[12];
	int i = 0, j = 0; // i traverses input string, j traverses curWord
	lex_index = 0; // traverses the list of lexemes
	while(input[i] != '\0'){
		// skip whitespace
		if (iscntrl(input[i]) || isspace(input[i])){
			i++;
			continue;
		}

		// hit a letter, take letters and numbers for an identifier
		// stop when nonalphanumeric is reached
		if (isalpha(input[i])){
			while(isalnum(input[i])){
				if (j >= MAX_IDENT_LEN){ // check ident length
					printlexerror(4);
					return NULL;
				}
				curWord[j] = input[i];
				i++;
				j++;
			}
			curWord[j] = '\0';
			// tokenize and reset curword
			list[lex_index] = tokenize(curWord, 1);

			j = 0;
			continue;
		}

		// hit a number, take numbers until nondigit
		if (isdigit(input[i])){
			while(isdigit(input[i])){
				if(j >= MAX_NUMBER_LEN){
					printlexerror(3);
					return NULL;
				}
				curWord[j] = input[i];
				i++;
				j++;
			}
			// if theres a letter immediately after the number its an invalid identifier error
			if(isalpha(input[i])){
				printlexerror(2);
				return NULL;
			}
			curWord[j] = '\0';
			// tokenize and reset
			list[lex_index] = tokenize(curWord, 2);

			j = 0;
			continue;
		}

		// hit a symbol
		if (issymbol(input[i])){
			int done = 0;
			// immediately tokenize if the symbol is a single char special symbol
			// % * / + - ( ) , . ; these chars are tokens on their own
			char singlecharsymbol[11] = {'=', '%', '*', '/', '+', '-', '(', ')', ',', '.', ';'};
			for (int k = 0; k < 11; k++){
				if (input[i] == singlecharsymbol[k]){
					// if the char found was a / see if its a comment
					if (k == 3){
						// if this is true we have a comment
						if (input[i+1] == '/'){
							while (input[i] != '\n'){
								i++;
							}
							done = 1;
							break;
						}
					}
					curWord[j] = input[i];
					curWord[j+1] = '\0';
					list[lex_index] = tokenize(curWord, 3);

					i++;
					j = 0;
					done = 1;
					break;
				}
			}
			// if a single char was tokenized above, continue
			if (done == 1){
				continue;
			}
			
			// now there must be a possibility for a 2 char special symbol
			// != <= >= := are all the multichar special symbols
			// ! and : must be followed by =
			if (input[i] == '!' || input[i] == ':'){
				if (input[i+1] == '='){
					curWord[j] = input[i];
					i++;
					j++;
					curWord[j] = input[i];
					i++;
					j++;
					curWord[j] = '\0';
					list[lex_index] = tokenize(curWord, 3);

					j = 0;
					continue;
				} else // otherwise its an invalid symbol
					printlexerror(1);
					return NULL;

			}
			// < > can either stand alone or be followed by =
			if (input[i] == '<' || input[i] == '>'){
				curWord[j] = input[i];
				i++;
				j++;
				if(input[i] == '='){
					curWord[j] = input[i];
					i++;
					j++;
				}
				curWord[j] = '\0';
				j = 0;
				list[lex_index] = tokenize(curWord, 3);

				continue;
			}

		}
		
	}

	if (printFlag) printtokens();
	list[lex_index++].type = -1;
	return list;
}

lexeme tokenize(char* str, int type){ // type 1, identifier or reserved word, 2, number, 3, special symbol
	lexeme newLex;
	strcpy(newLex.name, str);

	// type 1 identifier or reserved word
	if (type == 1){
		int test = isreserved(str);
		newLex.type = test;
	}

	// type 2 number
	if (type == 2){
		newLex.type = numbersym;
		newLex.value = atoi(str);
	}

	// type 3 special sym
	if (type == 3){
		newLex.type = getSymType(str);
	}

	lex_index++;
	return newLex;

}

int getSymType(char *sym){
	char special[17][3] = {"=", "!=", "<", "<=", ">", ">=", "%", "*", "/", "+", "-", "(", ")", ",", ".", ";", ":="};

	for (int i = 0; i < 17; i++){
		if (!strcmp(sym, special[i])){
			switch(i){
				case 0: return eqlsym;
				case 1: return neqsym;
				case 2: return lsssym;
				case 3: return leqsym;
				case 4: return gtrsym;
				case 5: return geqsym;
				case 6: return modsym;
				case 7: return multsym;
				case 8: return divsym;
				case 9: return addsym;
				case 10: return subsym;
				case 11: return lparensym;
				case 12: return rparensym;
				case 13: return commasym;
				case 14: return periodsym;
				case 15: return semicolonsym;
				case 16: return assignsym;
				
			}
		}
	}
}

// returns the token number of the reserved word, or 0 if thats not what this is
int isreserved(char* str){
	char reserved[14][12] = {"const", "var", "procedure", "call", "do", "od", "when", "elsedo", "while", "read", "write", "odd"};

	for (int i = 0; i < 14; i++){
		if (!strcmp(str, reserved[i])){
			switch (i){
				case 0: return constsym; // const
				case 1: return varsym; // var
				case 2: return procsym; // procedure
				case 3: return callsym; // call
				case 4: return dosym; // do
				case 5: return odsym; // od
				case 6: return whensym; // when
				case 7: return elsedosym; // elsedo
				case 8: return whilesym; // while
				case 9: return readsym; // read
				case 10: return writesym; // write
				case 11: return oddsym; // odd
			}
			// return i;
		}
	}

	return identsym;

}

int issymbol(char c){
	char symbols[15] = {'=', '!', '<', '>', '%', '*', '/', '+', '-', '(', ')', ',', '.', ';', ':'};
	for (int i = 0; i < 15; i++){
		if (c == symbols[i])
			return 1;
	}
	return 0;
}

void printtokens()
{
	int i;
	printf("Lexeme Table:\n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < lex_index; i++)
	{
		switch (list[i].type)
		{
			case oddsym:
				printf("%11s\t%d", "odd", oddsym);
				break;
			case eqlsym:
				printf("%11s\t%d", "=", eqlsym);
				break;
			case neqsym:
				printf("%11s\t%d", "!=", neqsym);
				break;
			case lsssym:
				printf("%11s\t%d", "<", lsssym);
				break;
			case leqsym:
				printf("%11s\t%d", "<=", leqsym);
				break;
			case gtrsym:
				printf("%11s\t%d", ">", gtrsym);
				break;
			case geqsym:
				printf("%11s\t%d", ">=", geqsym);
				break;
			case modsym:
				printf("%11s\t%d", "%", modsym);
				break;
			case multsym:
				printf("%11s\t%d", "*", multsym);
				break;
			case divsym:
				printf("%11s\t%d", "/", divsym);
				break;
			case addsym:
				printf("%11s\t%d", "+", addsym);
				break;
			case subsym:
				printf("%11s\t%d", "-", subsym);
				break;
			case lparensym:
				printf("%11s\t%d", "(", lparensym);
				break;
			case rparensym:
				printf("%11s\t%d", ")", rparensym);
				break;
			case commasym:
				printf("%11s\t%d", ",", commasym);
				break;
			case periodsym:
				printf("%11s\t%d", ".", periodsym);
				break;
			case semicolonsym:
				printf("%11s\t%d", ";", semicolonsym);
				break;
			case assignsym:
				printf("%11s\t%d", ":=", assignsym);
				break;
			case odsym:
				printf("%11s\t%d", "od", odsym);
				break;
			case whensym:
				printf("%11s\t%d", "when", whensym);
				break;
			case elsedosym:
				printf("%11s\t%d", "elsedo", elsedosym);
				break;
			case whilesym:
				printf("%11s\t%d", "while", whilesym);
				break;
			case dosym:
				printf("%11s\t%d", "do", dosym);
				break;
			case callsym:
				printf("%11s\t%d", "call", callsym);
				break;
			case writesym:
				printf("%11s\t%d", "write", writesym);
				break;
			case readsym:
				printf("%11s\t%d", "read", readsym);
				break;
			case constsym:
				printf("%11s\t%d", "const", constsym);
				break;
			case varsym:
				printf("%11s\t%d", "var", varsym);
				break;
			case procsym:
				printf("%11s\t%d", "procedure", procsym);
				break;
			case identsym:
				printf("%11s\t%d", list[i].name, identsym);
				break;
			case numbersym:
				printf("%11d\t%d", list[i].value, numbersym);
				break;
		}
		printf("\n");
	}
	printf("\n");
	printf("Token List:\n");
	for (i = 0; i < lex_index; i++)
	{
		if (list[i].type == numbersym)
			printf("%d %d ", numbersym, list[i].value);
		else if (list[i].type == identsym)
			printf("%d %s ", identsym, list[i].name);
		else
			printf("%d ", list[i].type);
	}
	printf("\n");
}

void printlexerror(int type)
{
	if (type == 1)
		printf("Lexical Analyzer Error: Invalid Symbol\n");
	else if (type == 2)
		printf("Lexical Analyzer Error: Invalid Identifier\n");
	else if (type == 3)
		printf("Lexical Analyzer Error: Excessive Number Length\n");
	else if (type == 4)
		printf("Lexical Analyzer Error: Excessive Identifier Length\n");
	else
		printf("Implementation Error: Unrecognized Error Type\n");
	
	free(list);
	return;
}