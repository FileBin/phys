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
    unum start_node;
    unum end_node;
    decimal voltage;
    decimal resistance;
    decimal ampertage;
} Branch;

typedef struct BranchScheme {
    struct BranchScheme *parent;
    Branch *branches;
    unum branches_count;
} BranchScheme;

typedef struct CalculatedScheme {
    decimal *branch_currencies;
    unum branches_count;
} CalculatedScheme;

unum schemeNextNodeIndex(BranchScheme *scheme);

void findSchemeBranchesByNode(BranchScheme *scheme, unum node_id, Branch ***p_branches_ref_arr, unum *founded_count);

void findSchemeBranchesIdsByNode(BranchScheme *scheme, unum node_id, unum **p_branches_ids_arr, unum *founded_count);

unum countNodes(BranchScheme *scheme, unum **pnodes_map);

void transformTriangleToStar(BranchScheme *scheme, unum triangle_branches[3], char *doc);

void schemeToLatex(BranchScheme *scheme, char *doc, decimal scale);

void simplifyScheme(BranchScheme *scheme, char *doc);

void branchConvertAmpertageToVotage(Branch *branch);

#ifdef __cplusplus
}
#endif
#endif