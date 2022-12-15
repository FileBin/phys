#include "branch_scheme.h"
#include "config.h"
#include "defines.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Args {
    FILE *in_file;
    FILE *out_file;
} Args;

int main(int argc, char *argv[]) {
    char doc[0x2000];
    char buffer[0x1000];
    unum triangle_branches[3];

    Args args;
    args.in_file = stdin;
    args.out_file = stdout;
    for (size_t i = 1; i < argc; ++i) {
        char *arg = argv[i];
        char buf[0x80];
        memcpy(buf, arg, 3);
        buf[3] = 0;

        if (strcmp(buf, "if=") == 0) {
            stpcpy(buf, arg + 3);
            args.in_file = fopen(buf, "r");
        } else if (strcmp(buf, "of=") == 0) {
            stpcpy(buf, arg + 3);
            args.out_file = fopen(buf, "w");
        }
    }
    doc[0] = 0;
    buffer[0] = 0;
    BranchScheme scheme;
    ZERO_TYPE(&scheme, BranchScheme);
    puts("Input number of branches in scheme:");
    fscanf(args.in_file, "%d", (int *)&scheme.branches_count);

    unum *loop_buf = ALLOC_ARR(unum, scheme.branches_count);

    scheme.branches = ALLOC_ARR(Branch, scheme.branches_count);
    ZERO_ARR(scheme.branches, Branch, scheme.branches_count);

    puts("NOTE: Branch input format (start, end, R, E, J)");
    for (unum i = 0; i < scheme.branches_count; i++) {
        Branch *b = scheme.branches + i;
        float R, E, J;
        // printf("Input branch %d:\n", (int)i);
        fscanf(args.in_file, "%d %d %f %f %f", (int *)&b->start_node, (int *)&b->end_node, &R, &E, &J);
        sprintf(b->name, "%d", i + 1);
        b->resistance = R;
        b->voltage = E;
        b->ampertage = J;
    }

    triangle_branches[2] = triangle_branches[1] = triangle_branches[0] = 0;

    CalculatedScheme calculated = calculateScheme(&scheme, METHOD_NODE, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    checkPowerBalance(&calculated, buffer);

    strcat(doc, buffer);
    buffer[0] = 0;

    calculated.scheme = 0;
    SAFE_FREE(calculated.branch_currencies);

    schemeValuesToLatex(&scheme, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    unum loop_size = findBiggestLoop(&scheme, loop_buf);
    schemeToLatex(&scheme, buffer, SCALE, loop_buf, loop_size);
    strcat(doc, buffer);
    buffer[0] = 0;

    strcat(doc, "\n\\newpage\n\n");
    puts("Input branches to convert to trinagle:");

    fscanf(args.in_file, "%d %d %d", (int *)&triangle_branches[0], (int *)&triangle_branches[1],
           (int *)&triangle_branches[2]);

    transformTriangleToStar(&scheme, triangle_branches, buffer);

    strcat(doc, buffer);
    buffer[0] = 0;

    strcat(doc, "\n\\newpage\n\n");

    schemeValuesToLatex(&scheme, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    loop_size = findBiggestLoop(&scheme, loop_buf);
    schemeToLatex(&scheme, buffer, SCALE, loop_buf, loop_size);
    strcat(doc, buffer);
    buffer[0] = 0;

    strcat(doc, "\n\\newpage\n\n");

    simplifyScheme(&scheme, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    calculated = calculateScheme(&scheme, METHOD_NODE, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    checkPowerBalance(&calculated, buffer);

    strcat(doc, buffer);
    buffer[0] = 0;

    calculated.scheme = 0;
    SAFE_FREE(calculated.branch_currencies);

    schemeValuesToLatex(&scheme, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    loop_size = findBiggestLoop(&scheme, loop_buf);
    schemeToLatex(&scheme, buffer, SCALE, loop_buf, loop_size);
    strcat(doc, buffer);
    buffer[0] = 0;

    strcat(doc, "\n\\newpage\n\n");

    if (args.out_file)
        fprintf(args.out_file, LATEX_DOC_TEMPL, doc);

    return 0;
}