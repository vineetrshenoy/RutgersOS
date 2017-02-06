// Author: John-Austen Francisco
// Date: 9 September 2015
//
// Preconditions: Appropriate C libraries
// Postconditions: Generates Segmentation Fault for
//                               signal handler self-hack

// Student name:   Vineet Shenoy
// Ilab machine used: template.cs.rutgers.edu

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void segment_fault_handler(int signum)
{
	printf("I am slain!\n");
	*((char*) &signum + 60) += 4;

	//Use the signnum to construct a pointer to flag on stored stack
	//Increment pointer down to the stored PC
	//Increment value at pointer by length of bad instruction

	
	
}


int main()
{
	int r2 = 0;

	signal(SIGSEGV, segment_fault_handler);

	r2 = *( (int *) 0 );
	
	printf("I live again!\n");

	return 0;
}


/*
NOTES:

	1. (01/29/17): My own machiine (ThinkPad T440s with Ubuntu 14.04 LTS) did
	not compile with the the 32-bit flag (-m32). To resolve this, I had to
	install a package to do this

		sudo apt-get install gcc-multilib g++-multilib

	2. (01/29/17): Compiling in assembly. THis is done by adding the -S flag
	to the compiling statement. For example, to compile the code into
	assembely in 32-bit mode, we would do the following:

		gcc -m32 -S -o test undead_code.c

		We can then just open "test" in a text editor to see assembly code


	3. (01/29/17): I created a pointer to the signal number currently at line
	17 (int * address = &signum), and when printing in the terminal, we find
	that the address of the signum is 0xff948930.	


	4. (01/29/17): It appears that the assembly language produced by the -S
	flag is the x86 assembly language. THere are two types of syntax: AT&T and
	Intel syntax. The assembly code produced appears to have AT&T syntax.

		Also, according to the CMU OS Supplement, the stack pointer is defined
		by the term %esp
	


*/
