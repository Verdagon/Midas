/** Main program file
 * @file
 *
 * This source file is part of the Cone Programming Language C compiler
 * See Copyright Notice in conec.h
*/

#include "coneopts.h"
#include "shared/fileio.h"
#include "genllvm/genllvm.h"


int main(int argc, char **argv) {
    ConeOptions coneopt;
    GenState gen;
    ModuleNode *modnode = NULL;
    int ok;

    // Get compiler's options from passed arguments
    ok = coneOptSet(&coneopt, &argc, argv);
    if (ok <= 0)
        exit(ok == 0 ? 0 : ExitOpts);
    if (argc < 2)
        errorExit(ExitOpts, "Specify a Cone program to compile.");
    coneopt.srcpath = argv[1];
    coneopt.srcname = fileName(coneopt.srcpath);

    // We set up generation early because we need target info, e.g.: pointer size
    genSetup(&gen, &coneopt);

    // Parse source file, do semantic analysis, and generate code
    //modnode = parsePgm(&coneopt);
    if (errors == 0) {
        //doAnalysis(&modnode);
        if (errors == 0) {
            //if (coneopt.print_ir)
            //    inodePrint(coneopt.output, coneopt.srcpath, (INode*)modnode);
            genmod(&gen, modnode);
            genClose(&gen);
        }
    }

    errorSummary();
#ifdef _DEBUG
    getchar();    // Hack for VS debugging
#endif
}
