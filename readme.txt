pl0pcomp v0.1

Eric Straub
er831564
Systems Software, Fall 21
Euripedes Montagne
Project 4

To compile the compiler's source code:
	- Navigate to the directory containing the following required files:
		compiler.h
		driver.c
		lex.c
		Makefile
		parser.c
		vm.c
		
	- Run the makefile with "make"
	- The makefile forces linkage of the C math library and compiles the driver, lexer, parser/code generator, and vm together
	- The output is a.out by default
	
To compile and run a PL/0+ program using the compiler:
	- Navigate to the directory contaning the following required files:
		a.out (output from Makefile)
		text file containing PL/0+ source code
	
	- Use "./a.out" followed by the name of the text file containing the source code you would like to compile
	- Example with using options included below
	
Options:
	- When executing a.out, there are 4 options that can be used to view the output during each stage of compilation and execution
	- These are used by adding "-l", "-s", "-a", and/or "-v" as arguments to the a.out call after giving the name of your source code file
		"-l" prints the lexeme table and token list generated by the lexical analyzer
		"-s" prints the the symbol table generated by the parser/code generator
		"-a" prints the code generated by the parser/code generator
		"-v" prints the execution trace of the virtual machine
	
Example (using all options):
	- you@yourmachine:~/compilerDirectory$ make
	- you@yourmachine:~/compilerDirectory$ ./a.out source_code.txt -l -s -a -v


