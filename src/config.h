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
#define SCALE 8

#define LATEX_RESISTOR "american resistor"
#define LATEX_CURRENT_SRC "american current source"
#define LATEX_VOLTAGE_SRC "american voltage source"

#define LATEX_DOC_TEMPL                                                                                                \
    "\\documentclass[12pt]{article}\n"                                                                                 \
    "\\usepackage{tex/circuitikz}\n"                                                                                   \
    "\\begin{document}\n"                                                                                              \
    "%s"                                                                                                               \
    "\\end{document}\n\n"

#define LATEX_CIRCUIT_TEMPL                                                                                            \
    "\\begin{circuitikz} \\draw\n"                                                                                     \
    "%s"                                                                                                               \
    ";\n"                                                                                                              \
    "\\end{circuitikz}\n\n"

#endif