#include <stdio.h>
#include <stdlib.h>
#include "../include/parser.h"

TCB** parser_parse_file(const char *filename, int *out_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo de entrada");
        *out_count = 0;
        return NULL;
    }

    // TODO: Implementar lectura ignorando líneas con '#'[cite: 2]
    // Por ahora retorna un esqueleto vacío funcional para compilar
    fclose(file);
    *out_count = 0;
    return NULL;
}

void parser_free_tasks(TCB **tasks, int count) {
    if (!tasks) return;
    for (int i = 0; i < count; i++) {
        tcb_destroy(tasks[i]);
    }
    free(tasks);
}