/*
 * file: config.h
 * author: filebin
 * date: 2022-11-17 20:38:23
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#ifndef NDEBUG
#define TRACE
#endif

#define MAX_GRAPH_NODES 0xffff
#define EPSILON 0.0000000001

#define LATEX_DOC_TEMPL                                                                                                \
    "\\documentclass{article}\n"                                                                                       \
    "\\usepackage{circuitikz}\n"                                                                                       \
    "\\begin{document}\n"                                                                                              \
    "%s"                                                                                                               \
    "\\end{document}\n\n"

#define LATEX_CIRCUIT_TEMPL                                                                                            \
    "\\begin{circuitikz} \\draw\n"                                                                                     \
    "%s"                                                                                                               \
    ";\n"                                                                                                              \
    "\\end{circuitikz}\n\n"

#endif