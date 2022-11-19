#include "branch_scheme.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define SCALE 8

int main() {
    char buffer[0x1000];
    char doc[0x10000];
    doc[0] = 0;
    buffer[0] = 0;
    BranchScheme scheme;
    ZERO_TYPE(&scheme, BranchScheme);
    puts("Input number of branches in scheme:");
    scanf("%d", (int *)&scheme.branches_count);
    scheme.branches = ALLOC_ARR(Branch, scheme.branches_count);
    ZERO_ARR(scheme.branches, Branch, scheme.branches_count);
    puts("NOTE: Branch input format (start, end, R, E, J)");
    for (unum i = 0; i < scheme.branches_count; i++) {
        Branch *b = scheme.branches + i;
        float R, E, J;
        // printf("Input branch %d:\n", (int)i);
        scanf("%d %d %f %f %f", (int *)&b->start_node, (int *)&b->end_node, &R, &E, &J);
        b->resistance = R;
        b->voltage = E;
        b->ampertage = J;
    }
    unum triangle_branches[3];
    ZERO_ARR(triangle_branches, unum, 3);

    schemeValuesToLatex(&scheme, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    schemeToLatex(&scheme, buffer, SCALE);
    strcat(doc, buffer);
    buffer[0] = 0;

    puts("Input branches to convert to trinagle:");
    scanf("%d %d %d", (int *)&triangle_branches[0], (int *)&triangle_branches[1], (int *)&triangle_branches[2]);

    transformTriangleToStar(&scheme, triangle_branches, buffer);

    schemeValuesToLatex(&scheme, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    schemeToLatex(&scheme, buffer, SCALE);
    strcat(doc, buffer);
    buffer[0] = 0;

    simplifyScheme(&scheme, buffer);

    schemeValuesToLatex(&scheme, buffer);
    strcat(doc, buffer);
    buffer[0] = 0;

    schemeToLatex(&scheme, buffer, SCALE);
    strcat(doc, buffer);
    buffer[0] = 0;

    puts("\nLatex doc:");
    printf(LATEX_DOC_TEMPL, doc);

    return 0;
}