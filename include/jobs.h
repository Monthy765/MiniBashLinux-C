#ifndef JOBS_H
#define JOBS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

// Definir la estructura para manejar procesos en background
typedef struct {
    pid_t pid;           // ID del proceso
    int job_id;        // ID del trabajo
    char command[1024];  // Comando ejecutado
    int status;         // Estado del proceso (0: en ejecución, 1: terminado, 2: suspendido)
} background_job;

// Declarar las funciones relacionadas con jobs
void add_job(pid_t pid, const char *command);
void job();
void fg(int job_id);
void manejador_senales();

#endif // JOBS_H