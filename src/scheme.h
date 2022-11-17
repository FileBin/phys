/*
 * file: branch.h
 * author: filebin
 * date: 2022-11-17 20:41:02
 */

#ifndef SRC_BRANCH_H_
#define SRC_BRANCH_H_

#include "pch.h"
#include <stddef.h>
#include <stdlib.h>

struct Element;

typedef struct {
    struct Element **array;
} Node;

struct Element {
    Node *from;
    Node *to;
    void *derived;
};

typedef struct Element Element;

typedef struct {
    Element elem;
    decimal resistance;
} Resistor;

typedef struct {
    Element elem;
    decimal voltage;
} VoltageGenerator;

typedef struct {
    Element elem;
    decimal amperage;
} AmperageGenerator;

typedef struct {
    Element *elem;
    size_t begin_node;
    size_t end_node;
} InputElement;

typedef struct {
    InputElement *elements;
    size_t n;
} InputList;

typedef struct {
    Node *nodes;
    size_t nodes_count;
} Scheme;

typedef struct Branch {};

Scheme *createScheme(InputList *list) {
    size_t nodes_count = 0;
    // find max node id
    for (size_t i = 0; i < list->n; ++i) {
        InputElement *elem = list->elements + i;
        nodes_count = MAX(nodes_count, elem->begin_node);
        nodes_count = MAX(nodes_count, elem->end_node);
    }

    ++nodes_count;

    Scheme *scheme = ALLOC(Scheme);
    scheme->nodes_count = nodes_count;
    scheme->nodes = ALLOC_ARR(Node, nodes_count);

    size_t *nodes_arr_counts = ALLOC_ARR(size_t, nodes_count);
    ZERO_ARR(nodes_arr_counts, size_t, nodes_count);

    // calculate space for arrays
    for (size_t i = 0; i < list->n; ++i) {
        InputElement *elem = list->elements + i;
        nodes_arr_counts[elem->begin_node]++;
        nodes_arr_counts[elem->end_node]++;
    }

    // allocate arrays
    for (size_t i = 0; i < nodes_count; ++i) {
        scheme->nodes[i].array = ALLOC_ARR(Element *, nodes_arr_counts[i]);
        ZERO_ARR(scheme->nodes[i].array, Element *, nodes_arr_counts[i]);
    }
    free(nodes_arr_counts);

    for (size_t i = 0; i < list->n; ++i) {
        InputElement *ielem = list->elements + i;
        scheme->nodes[i].array[ielem->begin_node] = ielem->elem;
        scheme->nodes[i].array[ielem->end_node] = ielem->elem;
        ielem->elem = NULL;
    }
    return scheme;
}

void transformTriangleToStar(Scheme *scheme, Node *p_nodes[3]) { Resistor in_resistors[3]; }

#endif