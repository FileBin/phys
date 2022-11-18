/*
 * file: BranchScheme.h
 * author: filebin
 * date: 2022-11-17 21:53:52
 */

#ifndef SRC_BRANCH_SCHEME_H_
#define SRC_BRANCH_SCHEME_H_
#include "defines.h"

#ifndef __cplusplus
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <bits/stdc++.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Point {
    decimal x, y;
} Point;

typedef struct Branch {
    size_t start_node;
    size_t end_node;
    decimal voltage;
    decimal resistance;
    decimal ampertage;
} Branch;

typedef struct BranchScheme {
    Branch *branches;
    size_t branches_count;
} BranchScheme;

size_t schemeNextNodeIndex(BranchScheme *scheme);

void findSchemeBranchesByNode(BranchScheme *scheme, size_t node_id, Branch ***p_branches_arr, size_t *founded_count);

void findSchemeBranchesIdsByNode(BranchScheme *scheme, size_t node_id, size_t **p_branches_ids_arr,
                                 size_t *founded_count);

size_t countNodes(BranchScheme *scheme, size_t **pnodes_map);

void transformTriangleToStar(BranchScheme *scheme, size_t triangle_branches[3]);

void schemeToGraph(BranchScheme *scheme, char *str, decimal scale);

#ifdef __cplusplus
}
#endif
#endif