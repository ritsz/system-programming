#include <tcl.h>
#include <tclDecls.h>
#include <tclPlatDecls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char **argv) {
	Tcl_Interp *interp;
	int code;
	char *result;

	printf("inside main function \n");
	//    Tcl_InitStubs(interp, "8.5", 0);
	Tcl_FindExecutable(argv[0]);
	interp = Tcl_CreateInterp();
	code = Tcl_Eval(interp, "source ~/PROGRAMMING/Scripting/simple_addition.tcl; add_two_nos");

	/* Retrieve the result... */
	result = Tcl_GetString(Tcl_GetObjResult(interp));

/* Check for error! If an error, message is result. */
	if (code == TCL_ERROR) {
    		fprintf(stderr, "ERROR in script: %s\n", result);
    		exit(1);
	}
	
	/* Print (normal) result if non-empty; we'll skip handling encodings for now */
	if (strlen(result)) {
    		printf("%s\n", result);
	}

	/* Clean up */
	Tcl_DeleteInterp(interp);
	exit(0);
}
