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
    char name[0x10];
    unum start_node;
    unum end_node;
    decimal voltage;
    decimal resistance;
    decimal ampertage;
} Branch;

typedef struct BranchScheme {
    const struct BranchScheme *parent;
    Branch *branches;
    unum branches_count;
} BranchScheme;

typedef struct CalculatedScheme {
    decimal *branch_currencies;
    const BranchScheme *scheme;
} CalculatedScheme;

typedef enum {
    METHOD_NODE,
    METHOD_LOOP,
    METHOD_OVERLAY,
} Method;

CalculatedScheme calculateScheme(const BranchScheme *scheme, Method method, char *doc);

void checkPowerBalance(const CalculatedScheme *scheme, char *doc);

unum schemeNextNodeIndex(BranchScheme *scheme);
void findSchemeBranchesByNode(const BranchScheme *scheme, unum node_id, Branch ***p_branches_ref_arr,
                              unum *founded_count);
void findSchemeBranchesIdsByNode(const BranchScheme *scheme, unum node_id, unum **p_branches_ids_arr,
                                 unum *founded_count);
unum countNodes(const BranchScheme *scheme, unum **pnodes_map);
void transformTriangleToStar(BranchScheme *scheme, unum triangle_branches[3], char *doc);
void schemeToLatex(const BranchScheme *scheme, char *doc, decimal scale, unum *branches_loop, unum nbranches_loop);
void schemeValuesToLatex(const BranchScheme *scheme, char *doc);
void simplifyScheme(BranchScheme *scheme, char *doc);
unum findBiggestLoop(const BranchScheme *scheme, unum *loop_id_buffer);

void branchConvertAmpertageToVotage(Branch *branch);
void mergeBranches(Branch *merge_to, const Branch *merge_from);
unum hashBranch(const Branch *branch);

#ifdef __cplusplus
}
#endif
#endif