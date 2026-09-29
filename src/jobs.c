#include "jobs.h"
#include "myshell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

// Lista de procesos en background y contador
static background_job jobs[1024];
static int job_count = 0;
static int jobs_id = 1;

void actualizar_lista_jobs() {
    for (int i = 0; i < job_count; i++) {
        if(jobs[i].status != 1) {
            int status;
            pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);

            if (result == -1) {
                fprintf(stderr, "Error al esperar el proceso.");
            } else if (result == 0) {
                jobs[i].status = 0; // El proceso sigue en ejecución
            } else if (WIFSTOPPED(status)) {
                jobs[i].status = 2; // El proceso ha sido detenido
            } else if (WIFCONTINUED(status)) {
                jobs[i].status = 0; // El proceso ha sido continuado
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                jobs[i].status = 1; // El proceso ha terminado normalmente o por señal
            }
        }
    }

    // Eliminar trabajos terminados
    for (int i = 0; i < job_count; ) {
        if(jobs[i].status == 1) {
            // Eliminar de la lista si el proceso ha terminado
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1]; // Desplazar los trabajos hacia la izquierda
            }
            job_count--; // Reducir el contador de trabajos
        } else {
            i++; // Solo incrementar si no se eliminó un trabajo
        }
    }
}

// Agregar un proceso a la lista de background
void add_job(pid_t pid, const char *command) {
    if(job_count >= 1024) {
        fprintf(stderr, "Error: no se pueden agregar más procesos en background.\n");
        return;
    }

    // Actualizar la lista de trabajos antes de agregar uno nuevo
    actualizar_lista_jobs();

    // Agregar el nuevo trabajo
    jobs[job_count].pid = pid; // Guardar el PID del proceso
    jobs[job_count].job_id = jobs_id; // Asignar un ID único al trabajo
    if (command == NULL) {
        fprintf(stderr, "Error: comando nulo.\n");
        return;
    }
    snprintf(jobs[job_count].command, sizeof(jobs[job_count].command) - 1, "%s", command); // Copiar el comando a la estructura
    jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0'; // Asegurar terminación
    jobs[job_count].status = 0; // Estado 0: en ejecución
    
    printf("[%d] %d", jobs[job_count].job_id, pid);
    job_count++; // Incrementar el contador de trabajos
    jobs_id++; // Incrementar el ID del trabajo para el siguiente
}

// Listar todos los procesos en background
void job() {
    // Actualizar la lista de trabajos antes de listar
    actualizar_lista_jobs();

    printf("Lista de trabajos en background:\n");

    if(job_count == 0) {
        printf("No hay trabajos en background.");
        return;
    }
    
    for (int i = 0; i < job_count; i++) {
        if(i == job_count - 2) { // Penúltimo trabajo
            printf("[%d]-\t%s\t\t%s\n",
                jobs[i].job_id,
                (jobs[i].status == 0) ? "En ejecución" : (jobs[i].status == 1) ? "Terminado" : "Detenido",
                jobs[i].command
                );
        } else if(i == job_count - 1) { // Último trabajo
            printf("[%d]+\t%s\t\t%s",
                jobs[i].job_id,
                (jobs[i].status == 0) ? "En ejecución" : (jobs[i].status == 1) ? "Terminado" : "Detenido",
                jobs[i].command
                );
        } else { // Resto de trabajos
            printf("[%d] \t%s\t\t%s\n",
                jobs[i].job_id,
                (jobs[i].status == 0) ? "En ejecución" : (jobs[i].status == 1) ? "Terminado" : "Detenido",
                jobs[i].command
                );
        }
    }
}

// Traer un proceso al foreground
void fg(int job_id) {
    // Buscar el proceso en la lista de background
    int encontrado = -1;

    if(job_id == -2) { // Cuando no se pasan argumentos con el comando fg
        encontrado = job_count - 1; // Traer el último proceso al foreground
    } else {
        for(int i = 0; i < job_count; i++) {
            if (jobs[i].job_id == job_id) {
                encontrado = i;
                break;
            }
        }
    }
    
    if (encontrado == -1) {
        fprintf(stderr, "Error: proceso no encontrado en background.\n");
        return;
    }

    // Traer el proceso al foreground
    pid_t fg_pid = jobs[encontrado].pid;
    printf("%s\n", jobs[encontrado].command);
    fflush(stdout);
    
    // Transferir el control del terminal al grupo de procesos
    if (tcsetpgrp(STDIN_FILENO, fg_pid) == -1) {
        fprintf(stderr, "Error al asignar el control del terminal: %s\n", strerror(errno));
        return;
    }

    // Si el proceso está detenido, reanudarlo (enviar SIGCONT)
    if (jobs[encontrado].status == 2) {
        if (kill(-fg_pid, SIGCONT) == -1) {
            fprintf(stderr, "Error al enviar la señal SIGCONT al proceso %d: %s\n", fg_pid, strerror(errno));
            return;
        }
    }

    // Ignorar señales de terminal para evitar que el shell reciba señales de control
    signal(SIGTTOU, SIG_IGN);   // Ignorar señales de terminal
    signal(SIGTTIN, SIG_IGN);
    signal(SIGINT, SIG_DFL);    // Restaurar el manejador de interrupción
    signal(SIGQUIT, SIG_DFL);   // Restaurar el manejador de salida
    signal(SIGTSTP, SIG_DFL);   // Restaurar el manejador de parada
        
    // Esperar al proceso en foreground
    int status;
    waitpid(fg_pid, &status, WUNTRACED);

    // Restaurar el control del terminal al shell
    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) {
        fprintf(stderr, "Error al restaurar el control del terminal: %s\n", strerror(errno));
    }

    // Restaurar los manejadores de señales del shell
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGINT, SIG_IGN);    // Ignorar señales de interrupción
    signal(SIGQUIT, SIG_IGN);   // Ignorar señales de salida
    signal(SIGTSTP, SIG_IGN);   // Ignorar señales de parada

    // Eliminar el proceso de la lista de background
    if (WIFSTOPPED(status)) {
        jobs[encontrado].status = 2; // Actualizar el estado a detenido
    } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        // Eliminar el proceso de la lista de trabajos si terminó
        for (int i = encontrado; i < job_count - 1; i++) {
            jobs[i] = jobs[i + 1];
        }
        job_count--;
    }
}

void manejador_senales() {
    // Manejar la señal SIGCHLD
    pid_t pid;
    int status;

    // Esperar a que un hijo termine, pero no bloquear la ejecución y también manejar la señal SIGTSTP
    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        for(int i = 0; i < job_count; i++) {
            if(jobs[i].pid == pid) { // Buscar el proceso en la lista de background
                if(WIFEXITED(status) || WIFSIGNALED(status)) {
                    jobs[i].status = 1; // Proceso terminado
                } else if(WIFSTOPPED(status)) {
                    jobs[i].status = 2; // Proceso detenido
                } else if(WIFCONTINUED(status)) {
                    jobs[i].status = 0; // Proceso continuado
                }
                break; // Salir del bucle una vez encontrado el proceso
            }
        }
    }
}