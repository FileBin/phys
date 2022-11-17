#include "BranchScheme.h"

#include <stddef.h>
#include <stdio.h>

int main() {
    BranchScheme scheme;
    ZERO_TYPE(&scheme, BranchScheme);
    puts("Input number of branches in scheme:");
    scanf("%d", (int *)&scheme.branches_count);
    scheme.branches = ALLOC_ARR(Branch, scheme.branches_count);
    ZERO_ARR(scheme.branches, Branch, scheme.branches_count);
    puts("NOTE: Branch input format (start, end, R, E, J)");
    for (size_t i = 0; i < scheme.branches_count; i++) {
        Branch *b = scheme.branches + i;
        float R, E, J;
        // printf("Input branch %d:\n", (int)i);
        scanf("%d %d %f %f %f", (int *)&b->start_node, (int *)&b->end_node, &R, &E, &J);
        b->resistance = R;
        b->voltage = E;
        b->ampertage = J;
    }
    size_t triangle_branches[3];
    ZERO_ARR(triangle_branches, size_t, 3);

    puts("Input branches to convert to trinagle:");
    scanf("%d %d %d", &triangle_branches[0], &triangle_branches[1], &triangle_branches[2]);
    transformTriangleToStar(&scheme, triangle_branches);
    return 0;
}