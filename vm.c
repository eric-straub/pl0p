/*
    Eric Straub
	er831564
	Systems Software, Fall 21
	Euripedes Montagne
	Project 4 PL/0+ Virtual Machine

	This is the skeleton vm.c for the UCF Fall 2021 Systems Software 
	Project, HW4. Add whatever you want, but make sure to leave the
	print functions as they are to make sure you get your grade as
	quickly as possible. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#define MAX_PAS_LENGTH 3000

// Holds the contents of the IR
typedef struct instructionRegister{
    int op;
    int l;
    int m;
}instReg;

//void print_execution(int line, char *opname, int *IR, int PC, int BP, int SP, int DP, int *pas, int GP)
void print_execution(int line, char *opname, instReg ir, int PC, int BP, int SP, int DP, int *pas, int GP)
{
	int i;
	// print out instruction and registers
	printf("%2d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t", line, opname, ir.l, ir.m, PC, BP, SP, DP);
	
	// print data section
	for (i = GP; i <= DP; i++)
		printf("%d ", pas[i]);
	printf("\n");
	
	// print stack
	printf("\tstack : ");
	for (i = MAX_PAS_LENGTH - 1; i >= SP; i--)
		printf("%d ", pas[i]);
	printf("\n");
}

int base(int L, int pas[], int BP)
{
	int ctr = L;
	int rtn = BP;
	while (ctr > 0)
	{
		rtn = pas[rtn-1];
		ctr--;
	}
	return rtn;
}

void execute_program(instruction *code, int printFlag)
{
	// variables
	int *pas = calloc(MAX_PAS_LENGTH, sizeof(int));
	int ic, line;
	
	// read in program
	ic = 0;
	line = 0;
	while (code[line].opcode != -1)
	{
		pas[ic] = code[line].opcode;
		ic++;
		pas[ic] = code[line].l;
		ic++;
		pas[ic] = code[line].m;
		ic++;
		line++;
	}

	// Set pointers, create IR, now that we have the text loaded into the PAS
    int bp = ic; // base pointer
    int gp = ic; // global pointer
    int dp = ic - 1; // data pointer
    int free = ic + 40; // heap pointer
    int sp = MAX_PAS_LENGTH; // stack pointer
    int pc = 0; // program counter
    instReg ir; // instruction register struct declaration and variable initialization
    ir.op = -1;
    ir.l = -1;
    ir.m = -1;
    int haltFlag = 1; // set to zero when the end of program is encountered
    char *opname = malloc(sizeof(char) * 4); // holds string of the name of the current operation
	
	// this will print the header in the output, make sure you put your
	//		execution loop after it
	if (printFlag)
	{
		printf("\t\t\t\tPC\tBP\tSP\tDP\tdata\n");
		printf("Initial values:\t\t\t%d\t%d\t%d\t%d\n", pc, bp, sp, dp);
	}

while(haltFlag != 0){
        //Fetch cycle. Next 3 ints in pas[pc] loaded into the ir
        // printf("fetching instruction... pc = %d\n", pc);
        ir.op = pas[pc];
        ir.l = pas[pc+1];
        ir.m = pas[pc+2];
        pc += 3;

        // printf("%d %d %d\n", ir.op, ir.l, ir.m);

        // collect initial values for print at the end of the loop
        int ipc = pc, ibp = bp, isp = sp, idp = dp;

        // this switch looks at the ir.op value (current op code in the instruction register)
        // and executes the operation based on that value

        switch(ir.op){
            case 1: // literal, pushes ir.m onto stack or data
                strcpy(opname, "LIT");
                if(bp == gp){
                    dp++;
                    pas[dp] = ir.m;
                } else {
                    sp--;
                    pas[sp] = ir.m;
                }
                break;

            case 2: 
                // opr implementation uses another switch to determine the operation based on ir.m
                // applicable operations are performed on values in the stack or data
                switch(ir.m){
                    case 0: // returns to the last AR
                        strcpy(opname, "RTN");
                        sp = bp + 1;
                        bp = pas[sp - 3];
                        pc = pas[sp - 4];
                        break;

                    case 1: // neg turns top value negative
                        strcpy(opname, "NEG");
                        if (bp == gp)
                            pas[dp] *= -1;
                        else 
                            pas[sp] *= -1;
                        break;

                    case 2: // add the top 2 values
                        strcpy(opname, "ADD");
                        if (bp == gp){
                            dp--;
                            pas[dp] += pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] += pas[sp-1];
                        }
                        break;

                    case 3: // subtract the top 2 values
                        strcpy(opname, "SUB");
                        if (bp == gp){
                            dp--;
                            pas[dp] -= pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] -= pas[sp-1];
                        }
                        break;

                    case 4: // multiply the top 2 values
                        strcpy(opname, "MUL");
                        if (bp == gp){
                            dp--;
                            pas[dp] *= pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] *= pas[sp-1];
                        }
                        break;

                    case 5: // divied the top 2 values
                        strcpy(opname, "DIV");
                        if (bp == gp){
                            dp--;
                            pas[dp] /= pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] /= pas[sp-1];
                        }
                        break;

                    case 6: // odd, changes the top value to 1 if odd and 0 if even
                        strcpy(opname, "ODD");
                        if (bp == gp){
                            pas[dp] = pas[dp] % 2;
                        } else {
                            pas[sp] = pas[sp] % 2;
                        }
                        break;

                    case 7: // modulus, performs [first] mod [second]
                        strcpy(opname, "MOD");
                        if (bp == gp){
                            dp--;
                            pas[dp] = pas[dp] % pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] = pas[sp] % pas[sp-1];
                        }
                        break;

                    case 8: // changes values to a 1 if they are equal and 0 otherwise
                        strcpy(opname, "EQL");
                        if (bp == gp){
                            dp--;
                            pas[dp] = pas[dp] == pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] = pas[sp] == pas[sp-1];
                        }
                        break;

                    case 9: // changes values to a 1 if they are not equal and 0 otherwise
                        strcpy(opname, "NEQ");
                        if (bp == gp){
                            dp--;
                            pas[dp] = pas[dp] != pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] = pas[sp] != pas[sp-1];
                        }
                        break;

                    case 10: // changes values to 1 if first value is less than second
                        strcpy(opname, "LSS");
                        if (bp == gp){
                            dp--;
                            pas[dp] = pas[dp] < pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] = pas[sp] < pas[sp-1];
                        }
                        break;

                    case 11: // changes value to 1 if first value is less than or equal to second
                        strcpy(opname, "LEQ");
                        if (bp == gp){
                            dp--;
                            pas[dp] = pas[dp] <= pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] = pas[sp] <= pas[sp-1];
                        }
                        break;

                    case 12: // changes values to 1 if first value is greater than second
                        strcpy(opname, "GTR");
                        if (bp == gp){
                            dp--;
                            pas[dp] = pas[dp] > pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] = pas[sp] > pas[sp-1];
                        }
                        break;

                    case 13:// changes values to 1 if first value is greater than or equal to second
                        strcpy(opname, "GEQ");
                        if (bp == gp){
                            dp--;
                            pas[dp] = pas[dp] >= pas[dp+1];
                        } else {
                            sp++;
                            pas[sp] = pas[sp] >= pas[sp-1];
                        }
                        break;

                    default:
                        break;

                } // end of nested switch
                break;
            
            case 3: // loads the value from ir.l lexicographical levels and ir.m offset
                strcpy(opname, "LOD");
                if (bp == gp){
                    dp++;
                    pas[dp] = pas[gp + ir.m];
                } else {
                    if (base(ir.l, pas, bp) == gp){
                        sp--;
                        pas[sp] = pas[gp + ir.m];
                    } else {
                        sp--;
                        pas[sp] = pas[base(ir.l, pas, bp) - ir.m];
                    }
                }
                break;
            
            case 4: // store top value at the location ir.l lexicographical levels up at an offset of ir.m
                strcpy(opname, "STO");
                if (bp == gp){
                    pas[gp + ir.m] = pas[dp];
                    dp--;
                } else {
                    if(base(ir.l, pas, bp) == gp){
                        pas[gp + ir.m] = pas[sp];
                        sp++;
                    } else {
                        pas[base(ir.l, pas, bp) - ir.m] = pas[sp];
                        sp++;
                    }
                }
                break;
            
            case 5: // call creates a new activation record and jumps to the instruction starting at ir.m
                strcpy(opname, "CAL");
                pas[sp-1] = 0; // functional value
                pas[sp-2] = base(ir.l, pas, bp);
                pas[sp-3] = bp;
                pas[sp-4] = pc;
                bp = sp-1;
                pc = ir.m;
                break;
            
            case 6: // increment the data pointer or decrement stack pointer by ir.m
                strcpy(opname, "INC");
                if (bp == gp){
                    dp = dp + ir.m;
                } else {
                    sp = sp - ir.m;
                }
                break;
            
            case 7: // jump moves the program counter to the location indicated by ir.m
                strcpy(opname, "JMP");
                pc = ir.m;
                break;
            
            case 8: // jpc executes jump only if 0 is at the top of the stack/data
            strcpy(opname, "JPC");
                if (bp == gp){
                    if (pas[dp] == 0) pc = ir.m;
                    dp--;
                } else {
                    if (pas[sp] == 0) pc = ir.m;
                    sp++;
                }
                break;
            
            case 9: // 
                strcpy(opname, "SYS");
                // print data from top of stack and pop it off the stack
                if(ir.m == 1){
                    if (bp == gp){
                        printf("Top of Stack Value: %d\n", pas[dp]);
                        dp--;
                    } else {
                        printf("Top of Stack Value: %d\n", pas[sp]);
                        sp++;
                    }
                }

                // scan and push value onto the stack
                if(ir.m == 2){
                    if (bp == gp){
                        dp++;
                        printf("Please Enter an Integer: ");
                        scanf("%d", &pas[dp]);
                    } else {
                        sp--;
                        printf("Please Enter an Integer: ");
                        scanf("%d", &pas[sp]);
                    }
                }
                
                // quit by setting hte halt flag to 0 to break the while loop
                if(ir.m == 3){
                    haltFlag = 0;
                    
                }
                break;

            default: 
                break;

        }

        // print using the given function
        if (printFlag) print_execution(ipc/3 - 1, opname, ir, pc, bp, sp, dp, pas, gp);

    }
	
	// free(pas);
}