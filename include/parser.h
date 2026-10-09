#ifndef PARSER_H
#define PARSER_H

#include "tcb.h"

// Lee el archivo de escenario y retorna un arreglo dinámico de TCBs
TCB** parser_parse_file(const char *filename, int *out_count);
void parser_free_tasks(TCB **tasks, int count);

#endif