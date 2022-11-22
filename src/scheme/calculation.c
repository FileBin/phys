#include "defines.h"
#include "vecmath.h"
#include <assert.h>
#include <branch_scheme.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

CalculatedScheme nodeMethodImpl(const BranchScheme *scheme, char *doc) {
    BranchScheme copy;
    copy.branches_count = scheme->branches_count;
    copy.parent = scheme;
    copy.branches = ALLOC_ARR(Branch, copy.branches_count);
    memcpy(copy.branches, scheme->branches, copy.branches_count * sizeof(Branch));
    simplifyScheme(&copy, 0);
    unum *nodes_map;
    unum nnodes = countNodes(&copy, &nodes_map);

    num inv_map[MAX_GRAPH_NODES];

    memset(inv_map, -1, MAX_GRAPH_NODES * sizeof(unum));

    for (size_t i = 0; i < nnodes; ++i) {
        inv_map[nodes_map[i]] = i;
    }

    unum n_equasions = nnodes - 1;

    Vector b;
    initVector(&b, n_equasions);

    SqMatrix matrix;
    initSqMatrix(&matrix, n_equasions);

    const unum ground_node_id = nodes_map[0];

    for (size_t i = 1; i < nnodes; ++i) {
        unum node_id = nodes_map[i];
        unum *branch_ids;
        unum nfounded;
        findSchemeBranchesIdsByNode(&copy, node_id, &branch_ids, &nfounded);

        decimal inv_resistance_sum = 0;

        decimal bi = 0;

        for (size_t j = 0; j < nfounded; ++j) {
            Branch *b = copy.branches + branch_ids[j];
            unum end_id = b->start_node;
            bool reversed = false;
            if (end_id == node_id) {
                reversed = true;
                end_id = b->end_node;
            }
            inv_resistance_sum += 1 / b->resistance;

            bi += (reversed ? -1 : 1) * (b->voltage / b->resistance + b->ampertage);

            if (end_id == ground_node_id) {
                if (fabs(b->resistance) < EPSILON) {
                    inv_resistance_sum = 1;
                    bi = (reversed ? -1 : 1) * b->voltage;
                    break;
                }
                continue;
            }
            setSqMatrixElement(&matrix, inv_map[end_id], i - 1, -1 / b->resistance);
        }

        setVectorElement(&b, i - 1, bi);

        setSqMatrixElement(&matrix, i - 1, i - 1, inv_resistance_sum);

        free(branch_ids);
    }

    Vector potentials;
    initVector(&potentials, n_equasions);

#ifdef TRACE
    puts("matrix:");
    for (size_t i = 0; i < n_equasions; ++i) {
        for (size_t j = 0; j < n_equasions; ++j) {
            printf("%.6f ", getSqMatrixElement(&matrix, j, i));
        }
        puts(";");
    }
#endif

    invertMatrix(&matrix);

#ifdef TRACE
    puts("inv matrix:");
    for (size_t i = 0; i < n_equasions; ++i) {
        for (size_t j = 0; j < n_equasions; ++j) {
            printf("%.6f ", getSqMatrixElement(&matrix, j, i));
        }
        puts(";");
    }
#endif

    multiplyVectorByMatrix(&potentials, &b, &matrix);

    CalculatedScheme calc_expanded;
    calc_expanded.branch_currencies = ALLOC_ARR(decimal, scheme->branches_count);
    calc_expanded.scheme = scheme;
    bool *calculated = ALLOC_ARR(bool, scheme->branches_count);
    ZERO_ARR(calculated, bool, scheme->branches_count);

    for (size_t i = 0; i < scheme->branches_count; ++i) {
        Branch *b = scheme->branches + i;

        num end_idx = inv_map[b->end_node];
        num start_idx = inv_map[b->start_node];

        num *founded_idx = 0;
        num *p_idx = 0;
        if (end_idx < 0) {
            p_idx = &end_idx;
            founded_idx = &start_idx;
        }
        if (start_idx < 0) {
            if (p_idx)
                continue;
            p_idx = &start_idx;
            founded_idx = &end_idx;
        }

        if (p_idx) {
            for (size_t j = 0; j < copy.branches_count; ++j) {
                Branch *other = copy.branches + j;
                if (hashBranch(other) == hashBranch(b)) {
                    num o_end_idx = inv_map[other->end_node];
                    num o_start_idx = inv_map[other->start_node];
                    if (o_start_idx == (*founded_idx)) {
                        (*p_idx) = o_end_idx;
                    } else if (o_end_idx == (*founded_idx)) {
                        (*p_idx) = o_start_idx;
                    }
                    break;
                }
            }
        }

        if (end_idx < 0 || start_idx < 0)
            continue;

        decimal u = potentials.data[end_idx] - potentials.data[start_idx];
        calc_expanded.branch_currencies[i] = (b->voltage + u) / b->resistance + b->ampertage;

        calculated[i] = true;

        char buf[0x80];
        sprintf(buf, "$I_{%s}=%.3f$\n\n", b->name, (float)calc_expanded.branch_currencies[i]);
        strcat(doc, buf);

#ifdef TRACE
        printf("%s currency is %.3f\n", b->name, (float)calc_expanded.branch_currencies[i]);

#endif
    }

    for (size_t i = 0; i < scheme->branches_count; ++i) {
        if (calculated[i])
            continue;

        Branch *b = scheme->branches + i;

        unum *branches_ids;
        unum nfounded;

        findSchemeBranchesIdsByNode(scheme, b->start_node, &branches_ids, &nfounded);

        decimal *I = calc_expanded.branch_currencies + i;

        calculated[i] = true;
        for (size_t j = 0; j < nfounded; ++j) {
            unum idx = branches_ids[j];
            if (i == idx)
                continue;
            if (!calculated[idx]) {
                calculated[i] = false;
                (*I) = 0;
                break;
            }

            Branch *other = scheme->branches + branches_ids[idx];
            bool inverted = other->end_node != b->start_node;
            (*I) += (inverted ? -1 : 1) * calc_expanded.branch_currencies[idx];
        }

        free(branches_ids);
        if (calculated[i]) {
            char buf[0x80];
            sprintf(buf, "$I_{%s}=%.3f$\n\n", b->name, (float)(*I));
            strcat(doc, buf);

#ifdef TRACE
            printf("%s currency is %.3f\n", b->name, (float)(*I));

#endif
            continue;
        }

        calculated[i] = true;

        findSchemeBranchesIdsByNode(scheme, b->end_node, &branches_ids, &nfounded);

        for (size_t j = 0; j < nfounded; ++j) {
            unum idx = branches_ids[j];
            if (i == idx)
                continue;
            if (!calculated[idx]) {
                calculated[i] = false;
                (*I) = 0;
                break;
            }

            Branch *other = scheme->branches + branches_ids[idx];
            bool inverted = other->end_node != b->start_node;
            (*I) -= (inverted ? -1 : 1) * calc_expanded.branch_currencies[idx];
        }

        assert(calculated[i]);

        free(branches_ids);

        char buf[0x80];
        sprintf(buf, "$I_{%s}=%.3f$\n\n", b->name, (float)(*I));
        strcat(doc, buf);

#ifdef TRACE
        printf("%s currency is %.3f\n", b->name, (float)(*I));

#endif
    }
    free(calculated);

    destroyVector(&b);
    destroyVector(&potentials);
    destroySqMatrix(&matrix);
    free(nodes_map);

    return calc_expanded;
}

CalculatedScheme loopMethodImpl(const BranchScheme *scheme) {}
CalculatedScheme overlayMethodImpl(const BranchScheme *scheme) {}

CalculatedScheme calculateScheme(const BranchScheme *scheme, Method method, char *doc) {
    switch (method) {
    case METHOD_NODE:
        return nodeMethodImpl(scheme, doc);
    case METHOD_LOOP:
        return loopMethodImpl(scheme);
    case METHOD_OVERLAY:
        return overlayMethodImpl(scheme);
    }
}

void checkPowerBalance(const CalculatedScheme *calc, char *doc) {
    char buf[0x80];
    char buffer_src[0x200];
    char buffer_dst[0x200];
    char buffer_src_f[0x1000];
    char buffer_dst_f[0x1000];

    buffer_src[0] = 0;
    buffer_dst[0] = 0;

    decimal P_src = 0, P_dst = 0;

    strcpy(buffer_src_f, "$P_{src}=");
    strcpy(buffer_dst_f, "$P_{dst}=");

    for (size_t i = 0; i < calc->scheme->branches_count; ++i) {
        Branch *b = calc->scheme->branches + i;
        decimal I = calc->branch_currencies[i];

        bool vol = fabs(b->voltage) > EPSILON;
        bool res = fabs(b->resistance) > EPSILON;
        bool amp = fabs(b->ampertage) > EPSILON;

        decimal P_src_i = I * (b->voltage + b->ampertage * b->resistance);
        if (fabs(P_src_i) > EPSILON) {
            sprintf(buf, "%s%.3f", P_src_i < 0 ? "" : "+", P_src_i);
            strcat(buffer_src, buf);
            sprintf(buf, "I_{%s}*(", b->name);
            strcat(buffer_src_f, buf);

            if (vol) {
                sprintf(buf, "E_{%s}", b->name);
                strcat(buffer_src_f, buf);
            }
            if (res && amp) {
                sprintf(buf, "%sJ_{%s}*R_{%s}", vol ? "+" : "", b->name, b->name);
                strcat(buffer_src_f, buf);
            }
            strcat(buffer_src_f, ")+");
        }

        decimal P_dst_i = I * I * b->resistance;
        if (fabs(P_src_i) > EPSILON) {
            sprintf(buf, "%s%.3f", P_dst_i < 0 ? "" : "+", P_dst_i);
            strcat(buffer_dst, buf);

            if (res) {
                sprintf(buf, "I_{%s}^2 * R_{%s}+", b->name, b->name);
                strcat(buffer_dst_f, buf);
            }
        }
        P_src += P_src_i;
        P_dst += P_dst_i;
    }

    sprintf(buf, "=%.3f$", P_src);
    strcat(buffer_src, buf);
    sprintf(buf, "=%.3f$", P_src);
    strcat(buffer_dst, buf);

    buffer_dst_f[strlen(buffer_dst_f) - 1] = '=';
    buffer_src_f[strlen(buffer_src_f) - 1] = '=';

    sprintf(doc, "%s%s\n\n%s%s\n\n", buffer_src_f, buffer_src, buffer_dst_f, buffer_dst);
}
