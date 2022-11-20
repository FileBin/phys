
#include "defines.h"
#include <assert.h>
#include <branch_scheme.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void graphToString(BranchScheme *scheme, char *str) {
    str[0] = 0;

    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        char buf[0x40];
        sprintf(buf, "%d: %d -> %d\n", (int)i, (int)b->start_node, (int)b->end_node);
        strcat(str, buf);
    }
}

unum schemeNextNodeIndex(BranchScheme *scheme) {
    unum index = 0;
    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        index = MAX(index, b->start_node);
        index = MAX(index, b->end_node);
    }
    return index + 1;
}

void findSchemeBranchesByNode(const BranchScheme *scheme, unum node_id, Branch ***p_branches_arr, unum *founded_count) {
    unum count = 0;
    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        if (b->start_node == node_id || b->end_node == node_id) {
            count++;
        }
    }
    (*p_branches_arr) = ALLOC_ARR(Branch *, count);
    unum index = 0;
    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        if (b->start_node == node_id || b->end_node == node_id) {
            (*p_branches_arr)[index++] = b;
        }
    }
    if (founded_count) {
        (*founded_count) = count;
    }
}

void findSchemeBranchesIdsByNode(const BranchScheme *scheme, unum node_id, unum **p_branches_ids_arr,
                                 unum *founded_count) {
    unum count = 0;
    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        if (b->start_node == node_id || b->end_node == node_id) {
            count++;
        }
    }
    (*p_branches_ids_arr) = ALLOC_ARR(unum, count);
    unum index = 0;
    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        if (b->start_node == node_id || b->end_node == node_id) {
            (*p_branches_ids_arr)[index++] = i;
        }
    }
    if (founded_count) {
        (*founded_count) = count;
    }
}

unum countNodes(const BranchScheme *scheme, unum **pnodes_map) {
    unum nbranches = scheme->branches_count;
    unum *nodes_map = ALLOC_ARR(unum, nbranches * 2);
    unum nnodes = 0;
    for (unum i = 0; i < nbranches; ++i) {
        unum j;
        unum node = scheme->branches[i].start_node;
        for (j = 0; j < nnodes; ++j) {
            if (nodes_map[j] == node) {
                break;
            }
        }
        nodes_map[j] = node;
        nnodes = MAX(nnodes, j + 1);

        node = scheme->branches[i].end_node;
        for (j = 0; j < nnodes; ++j) {
            if (nodes_map[j] == node) {
                break;
            }
        }
        nodes_map[j] = node;
        nnodes = MAX(nnodes, j + 1);
    }
    if (pnodes_map)
        (*pnodes_map) = nodes_map;
    else
        free(nodes_map);
    return nnodes;
}

void transformTriangleToStar(BranchScheme *scheme, unum triangle_branches[3], char *doc) {
    Branch *triangle[3];
    // star to triangle mapping:
    // star index 0 is between triangle 0 and 1,
    // star index 1 is between triangle 1 and 2,
    // star index 2 is between triangle 2 and 0,
    decimal star_resistances[3];
    decimal triangle_ampertages[3];

    decimal sum_resistance = 0;

    // calculate sum resistance and transform voltage to ampertage
    for (unum i = 0; i < 3; i++) {
        triangle[i] = &scheme->branches[triangle_branches[i]];
        triangle_ampertages[i] = triangle[i]->ampertage + triangle[i]->voltage / triangle[i]->resistance;
        sum_resistance += triangle[i]->resistance;
    }

    // calculate triangle nodes
    unum triangle_nodes[3];

    triangle_nodes[0] = triangle[0]->end_node;
    triangle_nodes[1] = triangle[1]->end_node;
    triangle_nodes[2] = triangle[2]->end_node;

    // if 1st triangle branch is inversed
    if (triangle_nodes[1] == triangle_nodes[0]) {
        // inverse 1st triangle branch values
        triangle_nodes[1] = triangle[1]->start_node;
        triangle_ampertages[1] *= -1;
    }
    // if 2nd triangle branch is inversed
    if (triangle_nodes[2] == triangle_nodes[1] || triangle_nodes[2] == triangle_nodes[0]) {
        // inverse 2nd triangle branch values
        triangle_nodes[2] = triangle[2]->start_node;
        triangle_ampertages[2] *= -1;
    }

    decimal star_voltages[3];

    for (unum i = 0; i < 3; i++) {
        unum j = (i + 1) % 3;
        Branch *a = triangle[i];
        Branch *b = triangle[j];
        star_resistances[i] = a->resistance * b->resistance / sum_resistance;
        star_voltages[i] = (triangle_ampertages[j] - triangle_ampertages[i]) * star_resistances[i];
    }

    Branch *insert_branches[3];
    for (unum i = 0; i < 3; i++) {
        Branch **other_branches;
        unum other_branches_count;
        findSchemeBranchesByNode(scheme, triangle_nodes[i], &other_branches, &other_branches_count);
        if (other_branches_count == 3) {
            Branch *b = 0;
            for (unum i = 0; i < 3; i++) {
                char unique = 1;
                for (unum j = 0; j < 3; j++) {
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
        free(other_branches);
    }

    unum new_node_id = schemeNextNodeIndex(scheme);
    char added_size = 0;
    Branch star_branches[3];
    for (unum i = 0; i < 3; i++) {
        Branch *star_branch = star_branches + i;
        sprintf(star_branch->name, "%s-%s", triangle[i]->name, triangle[(i + 1) % 3]->name);
        star_branch->start_node = triangle_nodes[i];
        star_branch->end_node = new_node_id;
        star_branch->ampertage = 0;
        star_branch->voltage = star_voltages[i];
        star_branch->resistance = star_resistances[i];

        if (insert_branches[i]) {
            branchConvertAmpertageToVotage(insert_branches[i]);
            mergeBranches(insert_branches[i], star_branch);
            insert_branches[i] = 0;
        } else {
            insert_branches[i] = ALLOC(Branch);
            memcpy(insert_branches[i], star_branch, sizeof(Branch));
            added_size++;
        }
    }

    if (doc) {
        BranchScheme star;
        star.parent = 0;
        star.branches = ALLOC_ARR(Branch, 6);
        star.branches_count = 6;

        memcpy(star.branches, star_branches, 3 * sizeof(Branch));
        memcpy(star.branches + 3, triangle[0], sizeof(Branch));
        memcpy(star.branches + 4, triangle[1], sizeof(Branch));
        memcpy(star.branches + 5, triangle[2], sizeof(Branch));

        star.branches[3].ampertage = star.branches[4].ampertage = star.branches[5].ampertage = 0;

        doc[0] = 0;
        unum loop[3];
        loop[0] = 3;
        loop[1] = 4;
        loop[2] = 5;
        char buffer[0x1000];

        buffer[0] = 0;
        schemeValuesToLatex(&star, buffer);
        strcat(doc, buffer);

        buffer[0] = 0;
        schemeToLatex(&star, buffer, SCALE, loop, 3);
        strcat(doc, buffer);
    }

    unum new_branches_count = scheme->branches_count + added_size - 3;

    Branch *new_arr = ALLOC_ARR(Branch, new_branches_count);
    // insert 3 ranges
    Branch *src = scheme->branches;
    Branch *dst = new_arr;

    unum range = triangle_branches[0];
    unum start = 0;

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
    for (unum i = 0; i < 3; i++) {
        Branch *ibranch = insert_branches[i];
        if (ibranch) {
            memcpy(dst++, ibranch, sizeof(Branch));
            free(ibranch);
        }
    }

    void *old_arr = scheme->branches;
    scheme->branches = new_arr;
    scheme->branches_count = new_branches_count;
    free(old_arr);
}

unum findBiggestLoop(const BranchScheme *scheme, unum *loop_id_buffer) {
    unum nbranches = scheme->branches_count;

    decimal *direct_weights = ALLOC_ARR(decimal, nbranches);
    decimal *indirect_weights = ALLOC_ARR(decimal, nbranches);

    for (unum i = 0; i < nbranches; ++i) {
        direct_weights[i] = nbranches;
        indirect_weights[i] = nbranches;
    }

    unum start_node = scheme->branches[0].start_node;

    decimal *in_weights = ALLOC_ARR(decimal, nbranches);
    decimal *next_weights = ALLOC_ARR(decimal, nbranches);
    unum *nodes_to_visit = ALLOC_ARR(unum, nbranches);
    unum *next_visit = ALLOC_ARR(unum, nbranches);
    unum nvisit = 1;

    nodes_to_visit[0] = start_node;
    in_weights[0] = 0;

    unum next_nvisit = 0;
    while (nvisit > 0) {
        // loop trough next visit nodes and caclulate weight
        for (unum i = 0; i < nvisit; ++i) {
            decimal prev_weight = in_weights[i];
            unum node = nodes_to_visit[i];
            unum *branches;
            unum branches_count;
            findSchemeBranchesIdsByNode(scheme, node, &branches, &branches_count);

            for (unum i = 0; i < branches_count; ++i) {
                decimal w = prev_weight + ((decimal)1) / ((decimal)branches_count);
                unum next = scheme->branches[branches[i]].start_node;
                unum branch_id = branches[i];

                decimal *weights = indirect_weights;
                if (next == node) {
                    weights = direct_weights;
                    next = scheme->branches[branch_id].end_node;
                }

                if (weights[branch_id] > w) {
                    weights[branch_id] = w;
                    if (next != start_node) {
                        unum idx = next_nvisit++;
                        next_weights[idx] = w;
                        next_visit[idx] = next;
                    }
                }
            }
            free(branches);
        }

        // swap arrays
        void *temp = nodes_to_visit;
        nodes_to_visit = next_visit;
        next_visit = temp;

        temp = in_weights;
        in_weights = next_weights;
        next_weights = temp;

        nvisit = next_nvisit;
        next_nvisit = 0;
    }

    SAFE_FREE(in_weights);
    SAFE_FREE(next_weights);
    SAFE_FREE(nodes_to_visit);
    SAFE_FREE(next_visit);

#ifdef TRACE
    puts("weights:");
    puts("ID\tdirect   \tindirect");
    for (unum i = 0; i < nbranches; ++i) {
        printf("%d\t%f\t%f\n", (int)i, (float)direct_weights[i], (float)indirect_weights[i]);
    }
#endif

    unum *branches_id_buffer = ALLOC_ARR(unum, nbranches);
    void *src = branches_id_buffer;

    unum node = start_node;
    unum prev_branch = nbranches;

    unum pos = 0;
    while (1) {
        unum *branches;
        unum bcount;
        findSchemeBranchesIdsByNode(scheme, node, &branches, &bcount);
        unum biggest_id = 0;
        Branch *biggest = 0;
        decimal maxw = -1;
        for (unum i = 0; i < bcount; ++i) {
            unum branch_id = branches[i];
            if (branch_id == prev_branch)
                continue;
            Branch *branch = scheme->branches + branch_id;
            decimal w = direct_weights[branch_id];
            if (branch->start_node == node)
                w = indirect_weights[branch_id];
            if (w > maxw) {
                maxw = w;
                biggest = branch;
                biggest_id = branch_id;
            }
        }
        free(branches);
        assert(biggest);

#ifdef TRACE
        printf("%d\n", (int)biggest_id);
#endif
        byte end_loop = 0;
        for (unum i = 0; i < pos; ++i) {
            if (branches_id_buffer[i] == biggest_id) {
                src = branches_id_buffer + i;
                nbranches = pos - i;
                end_loop = 1;
                break;
            }
        }
        if (end_loop)
            break;

        branches_id_buffer[pos++] = biggest_id;
        prev_branch = biggest_id;

        unum next = biggest->start_node;
        if (next == node) {
            next = biggest->end_node;
        }
        node = next;
    }

    SAFE_FREE(direct_weights);
    SAFE_FREE(indirect_weights);

    memcpy(loop_id_buffer, src, nbranches * sizeof(unum));
    SAFE_FREE(branches_id_buffer);

    return nbranches;
}

void resistorToLatex(char *buf, Point a, Point b, const char *label) {
    sprintf(buf, "(%f,%f) to[%s, l=$R_{%s}$] (%f,%f)\n", (float)a.x, (float)a.y, LATEX_RESISTOR, label, (float)b.x,
            (float)b.y);
}

void voltageSourceToLatex(char *buf, Point a, Point b, const char *label) {
    sprintf(buf, "(%f,%f) to[%s, l=$E_{%s}$] (%f,%f)\n", (float)b.x, (float)b.y, LATEX_VOLTAGE_SRC, label, (float)a.x,
            (float)a.y);
}

void ampertageSourceToLatex(char *buf, Point a, Point b, const char *label) {
    sprintf(buf, "(%f,%f) to[%s, l=$J_{%s}$] (%f,%f)\n", (float)a.x, (float)a.y, LATEX_CURRENT_SRC, label, (float)b.x,
            (float)b.y);
}

void branchValuesToLatex(const Branch *branch, char *dst) {
    char buf[0x80];
    byte volt = fabs(branch->voltage) > EPSILON;
    byte amp = fabs(branch->ampertage) > EPSILON;
    byte res = fabs(branch->resistance) > EPSILON;
    if (res) {
        sprintf(buf, "$R_{%s}=%.3f$\n\n", branch->name, branch->resistance);
        strcat(dst, buf);
    }
    if (volt) {
        sprintf(buf, "$E_{%s}=%.3f$\n\n", branch->name, branch->voltage);
        strcat(dst, buf);
    }
    if (amp) {
        sprintf(buf, "$J_{%s}=%.3f$\n\n", branch->name, branch->ampertage);
        strcat(dst, buf);
    }
}

void branchToLatex(const Branch *branch, Point a, Point b, decimal offset, char *dst) {
    char buf[0x80];
    memset(buf, 0, 0x80);

    byte reversed = 0;

    if (a.y < b.y) {
        Point t = a;
        a = b;
        b = t;
        reversed = 1;
    }

    Point l = b;
    l.x -= a.x;
    l.y -= a.y;

    Point norm;
    norm.x = l.y;
    norm.y = -l.x;
    byte volt = fabs(branch->voltage) > EPSILON;
    byte amp = fabs(branch->ampertage) > EPSILON;
    byte res = fabs(branch->resistance) > EPSILON;

    Point prev_b = b;

    if (fabs(offset) > EPSILON) {
        sprintf(buf, "(%f,%f) to ", (float)a.x, (float)a.y);
        strcat(dst, buf);
        a.x += l.x * 0.05 + norm.x * offset * 0.25;
        a.y += l.y * 0.05 + norm.y * offset * 0.25;
        b.x += -l.x * 0.05 + norm.x * offset * 0.25;
        b.y += -l.y * 0.05 + norm.y * offset * 0.25;
        buf[0] = 0;
        sprintf(buf, "(%f,%f)\n", (float)a.x, (float)a.y);
        strcat(dst, buf);
        buf[0] = 0;
    }

    if (volt && res) {
        Point c = a;
        Point d = b;
        if (reversed) {
            c = b;
            d = a;
        }
        Point mid = a;
        mid.x += l.x * .5;
        mid.y += l.y * .5;
        resistorToLatex(buf, mid, c, branch->name);
        strcat(dst, buf);
        buf[0] = 0;
        voltageSourceToLatex(buf, mid, d, branch->name);
        strcat(dst, buf);
        buf[0] = 0;
    } else if (volt) {
        if (reversed) {
            voltageSourceToLatex(buf, b, a, branch->name);
        } else {
            voltageSourceToLatex(buf, a, b, branch->name);
        }
        strcat(dst, buf);
        buf[0] = 0;
    } else if (res) {
        resistorToLatex(buf, b, a, branch->name);
        strcat(dst, buf);
        buf[0] = 0;
    }
    if (amp) {
        if (volt || res) {
            Point c = a;
            Point d = b;
            decimal nk = -0.15;
            if (volt && res) {
                nk = -nk;
            }
            decimal lk = 0.25;
            c.x += norm.x * nk + l.x * lk;
            c.y += norm.y * nk + l.y * lk;

            d.x += norm.x * nk - l.x * lk;
            d.y += norm.y * nk - l.y * lk;
            sprintf(buf, "(%f,%f) to\n", (float)a.x, (float)a.y);
            strcat(dst, buf);
            buf[0] = 0;
            if (reversed) {
                ampertageSourceToLatex(buf, d, c, branch->name);
            } else {
                ampertageSourceToLatex(buf, c, d, branch->name);
            }
            strcat(dst, buf);
            buf[0] = 0;
            sprintf(buf, "to (%f,%f)\n", (float)b.x, (float)b.y);
            strcat(dst, buf);
            buf[0] = 0;
        } else {
            if (reversed) {
                ampertageSourceToLatex(buf, b, a, branch->name);
            } else {
                ampertageSourceToLatex(buf, a, b, branch->name);
            }
        }
    }

    if (fabs(offset) > EPSILON) {
        sprintf(buf, "(%f,%f) to (%f,%f)\n", (float)b.x, (float)b.y, (float)prev_b.x, (float)prev_b.y);
        strcat(dst, buf);
        buf[0] = 0;
    }
}

void schemeToLatex(const BranchScheme *scheme, char *str, decimal scale, unum *branches_loop, unum nbranches_loop) {
    unum nbranches = scheme->branches_count;

    unum *nodes_map;
    unum nnodes = countNodes(scheme, &nodes_map);

    unum inv_map[MAX_GRAPH_NODES];
    for (unum i = 0; i < nnodes; ++i) {
        inv_map[nodes_map[i]] = i;
    }

    byte *calculated_nodes = ALLOC_ARR(byte, nnodes);
    ZERO_ARR(calculated_nodes, byte, nnodes);
    Point *node_positions = ALLOC_ARR(Point, nnodes);
    ZERO_ARR(node_positions, Point, nnodes);

#ifdef TRACE
    puts("Biggest loop:");
    for (unum i = 0; i < nbranches_loop; ++i) {
        printf("%d -> ", (int)branches_loop[i]);
    }
    printf("%d\n", (int)branches_loop[0]);
#endif

    unum start_node = scheme->branches[branches_loop[0]].start_node;
    unum node = start_node;
    decimal theta = 2 * M_PI / (decimal)nbranches_loop;
    decimal phi = theta * .5;
    unum i = 1;
    while (1) {
        Point p;
        p.x = cos(phi);
        p.y = sin(phi);
        // convert circle to square
        if (fabs(p.x) > fabs(p.y)) {
            p.y = p.y / fabs(p.x);
            p.x /= fabs(p.x);
        } else {
            p.x = p.x / fabs(p.y);
            p.y /= fabs(p.y);
        }
        p.x += 1;
        p.y += 1;
        p.x *= scale * .5;
        p.y *= scale * .5;

#ifdef TRACE
        printf("node %d: (%f, %f)\n", (int)node, (float)p.x, (float)p.y);
#endif

        node_positions[inv_map[node]] = p;
        calculated_nodes[inv_map[node]] = 1;
        unum next = scheme->branches[branches_loop[i]].start_node;
        if (next == node) {
            next = scheme->branches[branches_loop[i]].end_node;
        }
        if (next == start_node) {
            break;
        }
        node = next;

        phi += theta;
        i++;
        if (i >= nbranches_loop) {
            i -= nbranches_loop;
        }
    }
    Point center;
    center.x = center.y = scale * .5;

    const int center_weight = 3;

    for (unum i = 0; i < nnodes; ++i) {
        if (calculated_nodes[i])
            continue;
        Point *point = node_positions + i;
        (*point) = center;
        calculated_nodes[i] = 1;
        Branch **branches;
        unum branches_count;
        findSchemeBranchesByNode(scheme, nodes_map[i], &branches, &branches_count);

        point->x *= (decimal)(center_weight) / (branches_count + center_weight);
        point->y *= (decimal)(center_weight) / (branches_count + center_weight);

        for (unum j = 0; j < branches_count; ++j) {
            unum start_id = branches[j]->start_node;
            unum end_id = branches[j]->end_node;
            num other_id = -1;
            if (start_id == nodes_map[i]) {
                other_id = end_id;
            } else if (end_id == nodes_map[i]) {
                other_id = start_id;
            }
            assert(other_id != -1);

            other_id = inv_map[other_id];

            Point other = node_positions[other_id];

            point->x += other.x / (branches_count + center_weight);
            point->y += other.y / (branches_count + center_weight);
        }

#ifdef TRACE
        printf("node %d: (%f, %f)\n", (int)nodes_map[i], (float)point->x, (float)point->y);
#endif

        free(branches);
    }

    free(nodes_map);

    char buffer[0x1000];
    buffer[0] = 0;

    byte *branch_map = ALLOC_ARR(byte, nnodes * nnodes);
    ZERO_ARR(branch_map, byte, nnodes * nnodes);

    for (unum i = 0; i < nbranches; ++i) {
        Branch *branch = scheme->branches + i;

        unum j = inv_map[branch->start_node];
        unum k = inv_map[branch->end_node];
        decimal offset = 0;
        if (branch_map[j + k * nnodes] > 0) {
            char c = branch_map[j + k * nnodes];
            if ((c % 2) == 0) {
                c = -c + 1;
            }
            offset = c;
        }

        branch_map[k + j * nnodes] = ++branch_map[j + k * nnodes];

        Point a = node_positions[j];
        Point b = node_positions[k];

        branchToLatex(branch, a, b, offset, buffer);
    }
    free(branch_map);

    sprintf(str, LATEX_CIRCUIT_TEMPL, buffer);
}

void schemeValuesToLatex(const BranchScheme *scheme, char *doc) {
    for (size_t i = 0; i < scheme->branches_count; ++i) {
        branchValuesToLatex(scheme->branches + i, doc);
    }
}

void branchConvertAmpertageToVotage(Branch *branch) {
    branch->voltage += branch->ampertage * branch->resistance;
    branch->ampertage = 0;
}

void mergeBranches(Branch *merge_to, const Branch *merge_from) {
    byte opposite = 0xff;
    unum merge_node;
    if (merge_to->end_node == merge_from->end_node) {
        opposite = 1;
        merge_node = merge_to->end_node;
        merge_to->end_node = merge_from->start_node;
    } else if (merge_to->start_node == merge_from->start_node) {
        opposite = 1;
        merge_node = merge_to->start_node;
        merge_to->start_node = merge_from->end_node;
    } else if (merge_to->start_node == merge_from->end_node) {
        opposite = 0;
        merge_node = merge_to->start_node;
        merge_to->start_node = merge_from->start_node;
    } else if (merge_to->end_node == merge_from->start_node) {
        opposite = 0;
        merge_node = merge_to->end_node;
        merge_to->end_node = merge_from->end_node;
    }

    assert(opposite != 0xff);

    merge_to->resistance += merge_from->resistance;
    merge_to->voltage += (opposite ? -1 : 1) * merge_from->voltage;
}

void simplifyScheme(BranchScheme *scheme, char *doc) {
#ifdef TRACE
    {
        puts("simplifying scheme:");
        puts("before:");
        char buf[0x800];
        buf[0] = 0;
        graphToString(scheme, buf);
        puts(buf);
    }
#endif
    unum nbranches = scheme->branches_count;
    Branch **branch_buf = ALLOC_ARR(Branch *, nbranches);
    ZERO_ARR(branch_buf, Branch *, nbranches);

    for (size_t i = 0; i < nbranches; ++i) {
        Branch *branch = scheme->branches + i;
        branchConvertAmpertageToVotage(branch);
        branch_buf[i] = branch;
    }

    unum *nodes_map;
    unum nnodes = countNodes(scheme, &nodes_map);
    unum new_count = nbranches;
    for (size_t i = 0; i < nnodes; ++i) {
        unum real_node = nodes_map[i];
        unum *branch_ids;
        unum nfounded;
        findSchemeBranchesIdsByNode(scheme, real_node, &branch_ids, &nfounded);

        if (nfounded == 2) {
            Branch *merge_to = scheme->branches + branch_ids[0];
            Branch *merge_from = scheme->branches + branch_ids[1];

            mergeBranches(merge_to, merge_from);

            Branch **target2 = branch_buf + branch_ids[1];

            if (*target2) {
                new_count--;
                (*target2) = 0;
            }
        }

        free(branch_ids);
    }
    Branch *new_array = ALLOC_ARR(Branch, new_count);
    size_t pos = 0;
    for (size_t i = 0; i < nbranches; ++i) {
        Branch *b = branch_buf[i];
        if (!b)
            continue;

        memcpy(new_array + pos, b, sizeof(Branch));
        pos++;
    }

    free(scheme->branches);
    scheme->branches = new_array;
    scheme->branches_count = new_count;

    free(nodes_map);
    free(branch_buf);
#ifdef TRACE
    {
        puts("after:");
        char buf[0x800];
        buf[0] = 0;
        graphToString(scheme, buf);
        puts(buf);
    }
#endif
}

unum hashBranch(const Branch *branch) {
    unum l = strlen(branch->name);
    unum *arr = (unum *)branch->name;
    size_t n = l / sizeof(unum);
    unum hash;
    memcpy(&hash, arr, l - n * sizeof(num));
    hash ^= l;

    for (size_t i = 0; i < n; ++i) {
        hash ^= arr[i];
    }
    return hash;
}