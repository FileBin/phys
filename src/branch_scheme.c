
#include <branch_scheme.h>
#include <stdio.h>

unum schemeNextNodeIndex(BranchScheme *scheme) {
    unum index = 0;
    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        index = MAX(index, b->start_node);
        index = MAX(index, b->end_node);
    }
    return index + 1;
}

void findSchemeBranchesByNode(BranchScheme *scheme, unum node_id, Branch ***p_branches_arr, unum *founded_count) {
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

void findSchemeBranchesIdsByNode(BranchScheme *scheme, unum node_id, unum **p_branches_ids_arr, unum *founded_count) {
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

unum countNodes(BranchScheme *scheme, unum **pnodes_map) {
    unum nbranches = scheme->branches_count;
    unum *nodes_map = ALLOC_ARR(unum, nbranches);
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

void transformTriangleToStar(BranchScheme *scheme, unum triangle_branches[3]) {
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

    for (unum i = 0; i < 3; i++) {
        unum j = (i + 1) % 3;
        Branch *a = triangle[i];
        Branch *b = triangle[j];
        star_resistances[i] = a->resistance * b->resistance / sum_resistance;
        star_voltages[i] = (triangle_ampertages[i] + triangle_ampertages[j]) * star_resistances[i];
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
    for (unum i = 0; i < 3; i++) {
        Branch *ibranch = insert_branches[i];
        if (ibranch) {
            unum *node_id = &ibranch->start_node;
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

void findBiggestLoop(BranchScheme *scheme, unum **branches_id_arr, unum *arr_size) {
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
        decimal maxw = 0;
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

    (*branches_id_arr) = ALLOC_ARR(unum, nbranches);
    memcpy(*branches_id_arr, src, nbranches * sizeof(unum));
    SAFE_FREE(branches_id_buffer);

    SAFE_ASSIGN(arr_size, nbranches);
}

void getNodesLocationInScheme(BranchScheme *scheme, Point **points_arr, unum *points_size) {}

void insertBranch(Branch *branch, Point a, Point b, char *dst) {
    char buf[0x80];
    memset(buf, 0, 0x80);
    Point l = b;
    l.x -= a.x;
    l.y -= a.y;

    Point norm;
    norm.x = l.y;
    norm.y = -l.x;
    byte volt = fabs(branch->voltage) > 0.00000001;
    byte amp = fabs(branch->ampertage) > 0.00000001;
    byte res = fabs(branch->resistance) > 0.00000001;

    if (volt && res) {
        Point mid = a;
        mid.x += l.x * .5;
        mid.y += l.y * .5;
        sprintf(buf, "(%f,%f) to[%s]\n", (float)a.x, (float)a.y, "american resistor");
        strcat(dst, buf);
        buf[0] = 0;
        sprintf(buf, "(%f,%f) to[%s] (%f,%f)\n", (float)mid.x, (float)mid.y, "american voltage source", (float)b.x,
                (float)b.y);
        strcat(dst, buf);
        buf[0] = 0;
    } else if (volt) {
        sprintf(buf, "(%f,%f) to[%s] (%f,%f)\n", (float)a.x, (float)a.y, "american voltage source", (float)b.x,
                (float)b.y);
        strcat(dst, buf);
        buf[0] = 0;
    } else if (res) {
        sprintf(buf, "(%f,%f) to[%s] (%f,%f)\n", (float)a.x, (float)a.y, "american resistor", (float)b.x, (float)b.y);
        strcat(dst, buf);
        buf[0] = 0;
    }
    if (amp) {
        if (volt || res) {
            Point c = a;
            Point d = b;
            decimal nk = 0.15;
            decimal lk = 0.25;
            c.x += norm.x * nk + l.x * lk;
            c.y += norm.y * nk + l.y * lk;

            d.x += norm.x * nk - l.x * lk;
            d.y += norm.y * nk - l.y * lk;
            sprintf(buf, "(%f,%f) to\n", (float)a.x, (float)a.y);
            strcat(dst, buf);
            buf[0] = 0;
            sprintf(buf, "(%f,%f) to[%s] (%f,%f)\n", (float)c.x, (float)c.y, "american current source", (float)d.x,
                    (float)d.y);
            strcat(dst, buf);
            buf[0] = 0;
            sprintf(buf, "to (%f,%f)\n", (float)b.x, (float)b.y);
            strcat(dst, buf);
            buf[0] = 0;
        } else {
            sprintf(buf, "(%f,%f) to[%s] (%f,%f)\n", (float)a.x, (float)a.y, "american current source", (float)b.x,
                    (float)b.y);
        }
    }
}

void schemeToGraph(BranchScheme *scheme, char *str, decimal scale) {
    unum nbranches = scheme->branches_count;

    unum *nodes_map;
    unum nnodes = countNodes(scheme, &nodes_map);

    unum inv_map[0x1000];
    for (unum i = 0; i < nnodes; ++i) {
        inv_map[nodes_map[i]] = i;
    }

    byte *calculated_nodes = ALLOC_ARR(byte, nnodes);
    ZERO_ARR(calculated_nodes, byte, nnodes);
    Point *node_positions = ALLOC_ARR(Point, nnodes);
    ZERO_ARR(node_positions, Point, nnodes);

    unum *branches_loop;
    unum loop_size;
    findBiggestLoop(scheme, &branches_loop, &loop_size);

    unum start_node = scheme->branches[branches_loop[0]].start_node;
    unum node = start_node;
    decimal theta = 2. * 3.14159 / (decimal)loop_size;
    decimal phi = theta * .5;
    for (unum i = 0; i < loop_size; ++i) {
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
    }

    for (unum i = 0; i < nnodes; ++i) {
        if (calculated_nodes[i])
            continue;
        Point *point = node_positions + i;

        Branch **branches;
        unum branches_count;
        findSchemeBranchesByNode(scheme, nodes_map[i], &branches, &branches_count);
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

            point->x += other.x / branches_count;
            point->y += other.y / branches_count;
        }

#ifdef TRACE
        printf("node %d: (%f, %f)\n", (int)i, (float)point->x, (float)point->y);
#endif

        free(branches);
    }

    free(nodes_map);

    char buffer[0x1000];
    buffer[0] = 0;

    for (unum i = 0; i < nbranches; ++i) {
        Branch *branch = scheme->branches + i;
        Point a = node_positions[inv_map[branch->start_node]];
        Point b = node_positions[inv_map[branch->end_node]];

        insertBranch(branch, a, b, buffer);
    }

    sprintf(str, LATEX_CIRCUIT_TEMPL, buffer);
}

void graphToString(BranchScheme *scheme, char *str) {
    char buffer[0x1000];
    memset(buffer, 0, 0x1000);

    for (unum i = 0; i < scheme->branches_count; i++) {
        Branch *b = scheme->branches + i;
        char buf[0x40];
        sprintf(buf, "%d: %d -> %d\n", (int)i, (int)b->start_node, (int)b->end_node);
        strcat(buffer, buf);
    }

    strcpy(str, buffer);
}