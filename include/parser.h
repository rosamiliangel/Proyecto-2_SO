#ifndef PARSER_H
#define PARSER_H

#include "tcb.h"

// Lee el archivo de entrada y retorna un arreglo dinámico
TCB** parser_parse_file(const char *filename, int *out_count);
// Libera la memoria de las tareas
void parser_free_tasks(TCB **tasks, int count);

#endif