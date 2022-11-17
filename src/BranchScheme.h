/*
 * file: BranchScheme.h
 * author: filebin
 * date: 2022-11-17 21:53:52
 */

#ifndef SRC_BRANCHSCHEME_H_
#define SRC_BRANCHSCHEME_H_
#include "pch.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

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

size_t schemeNextNodeIndex(BranchScheme *scheme) {
    size_t index = 0;
    for (size_t i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        index = MAX(index, b->start_node);
        index = MAX(index, b->end_node);
    }
    return index + 1;
}

void findSchemeBranchesByNode(BranchScheme *scheme, size_t node_id, Branch ***p_branches_arr, size_t *founded_count) {
    size_t count = 0;
    for (size_t i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        if (b->start_node == node_id || b->end_node == node_id) {
            count++;
        }
    }
    (*p_branches_arr) = ALLOC_ARR(Branch *, count);
    size_t index = 0;
    for (size_t i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        if (b->start_node == node_id || b->end_node == node_id) {
            (*p_branches_arr)[index++] = b;
        }
    }
    if (founded_count) {
        (*founded_count) = count;
    }
}

void transformTriangleToStar(BranchScheme *scheme, size_t triangle_branches[3]) {
    Branch *triangle[3];
    // star to triangle mapping:
    // star index 0 is between triangle 0 and 1,
    // star index 1 is between triangle 1 and 2,
    // star index 2 is between triangle 2 and 0,
    decimal star_resistances[3];
    decimal triangle_ampertages[3];

    decimal sum_resistance = 0;

    // calculate sum resistance and transform voltage to ampertage
    for (size_t i = 0; i < 3; i++) {
        triangle[i] = &scheme->branches[triangle_branches[i]];
        triangle_ampertages[i] = triangle[i]->ampertage + triangle[i]->voltage / triangle[i]->resistance;
        sum_resistance += triangle[i]->resistance;
    }

    // calculate triangle nodes
    size_t triangle_nodes[3];

    triangle_nodes[0] = triangle[0]->start_node;
    triangle_nodes[1] = triangle[1]->start_node;
    if (triangle_nodes[1] == triangle_nodes[0]) {
        triangle_nodes[1] = triangle[1]->end_node;
    }
    triangle_nodes[2] = triangle[2]->start_node;
    if (triangle_nodes[2] == triangle_nodes[1] || triangle_nodes[2] == triangle_nodes[0]) {
        triangle_nodes[2] = triangle[2]->end_node;
    }

    decimal star_voltages[3];

    for (size_t i = 0; i < 3; i++) {
        size_t j = (i + 1) % 3;
        Branch *a = triangle[i];
        Branch *b = triangle[j];
        star_resistances[i] = a->resistance * b->resistance / sum_resistance;
        star_voltages[i] = (triangle_ampertages[i] + triangle_ampertages[j]) * star_resistances[i];
    }

    Branch *insert_branches[3];
    for (size_t i = 0; i < 3; i++) {
        Branch **other_branches;
        size_t other_branches_count;
        findSchemeBranchesByNode(scheme, triangle_nodes[i], &other_branches, &other_branches_count);
        if (other_branches_count == 3) {
            Branch *b = 0;
            for (size_t i = 0; i < 3; i++) {
                char unique = 1;
                for (size_t j = 0; j < 3; j++) {
                    if (other_branches[i] == triangle[j]) {
                        unique = 0;
                        break;
                    }
                }
                if (unique) {
                    b = other_branches[i];
                    break;
                }
            }
            insert_branches[i] = b;
        } else {
            insert_branches[i] = 0;
        }
    }

    size_t new_node_id = schemeNextNodeIndex(scheme);
    char added_size = 0;
    for (size_t i = 0; i < 3; i++) {
        Branch *ibranch = insert_branches[i];
        if (ibranch) {
            size_t *node_id = &ibranch->start_node;
            if ((*node_id) != triangle_nodes[i]) {
                node_id = &ibranch->end_node;
            }
            (*node_id) = new_node_id;

            ibranch->voltage += star_voltages[i];
            ibranch->resistance += star_resistances[i];
            insert_branches[i] = 0;
        } else {
            ibranch = insert_branches[i] = ALLOC(Branch);
            ibranch->start_node = triangle_nodes[i];
            ibranch->end_node = new_node_id;
            ibranch->ampertage = 0;
            ibranch->voltage = star_voltages[i];
            ibranch->resistance = star_resistances[i];
            added_size++;
        }
    }

    Branch *new_arr = ALLOC_ARR(Branch, scheme->branches_count + added_size - 3);
    // insert 3 ranges
    Branch *src = scheme->branches;
    Branch *dst = new_arr;

    size_t range = triangle_branches[0];
    size_t start = 0;

    if (range)
        memcpy(dst, src + start, range * sizeof(Branch));
    dst += range;

    start = triangle_branches[0] + 1;
    range = triangle_branches[1] - triangle_branches[0] - 1;

    if (range)
        memcpy(dst, src + start, range * sizeof(Branch));
    dst += range;

    start = triangle_branches[1] + 1;
    range = triangle_branches[2] - triangle_branches[1] - 1;

    if (range)
        memcpy(dst, src + start, range * sizeof(Branch));
    dst += range;

    start = triangle_branches[2] + 1;
    range = scheme->branches_count - triangle_branches[2] - 1;
    if (range)
        memcpy(dst, src + start, range * sizeof(Branch));

    dst += range;
    for (size_t i = 0; i < 3; i++) {
        Branch *ibranch = insert_branches[i];
        if (ibranch) {
            memcpy(dst++, ibranch, sizeof(Branch));
            free(ibranch);
        }
    }

    void *old_arr = scheme->branches;
    scheme->branches = new_arr;
    free(old_arr);
}

void schemeToMermaidGraph() {}

#ifdef __cplusplus
}
#endif
#endif