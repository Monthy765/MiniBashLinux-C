#include "parser.h"
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

pid_t procesos_foreground;
pid_t procesos_background;

int main(void) {
	char buf[1024];
	tline * line;
	int z;

	printf("==> ");

    signal(SIGCHLD, manejador_senales); // Manejar la señal SIGCHLD para procesos en background
    signal(SIGINT, SIG_IGN);    // Ignorar señales de interrupción
    signal(SIGQUIT, SIG_IGN);   // Ignorar señales de salida
    signal(SIGTSTP, SIG_IGN);   // Ignorar señales de parada

    procesos_background = -1; // Inicializar el PID de background a -1
    procesos_foreground = getpid(); // Guardar el PID del proceso principal
    setpgid(0, procesos_foreground); // Establecer el grupo de procesos del shell
    if (tcsetpgrp(STDIN_FILENO, procesos_foreground) == -1) {
        fprintf(stderr, "Error al restaurar el control del terminal: %s\n", strerror(errno));
    }
	while (fgets(buf, 1024, stdin)) {
		line = tokenize(buf);
		if (line==NULL) {
			continue;
		}

        int num_commands = line->ncommands;
        int fd, pipefds[2], prev_pipe[2], status;
        pid_t pid;
        pid_t pids_jobs[num_commands]; // Array para almacenar los PIDs de los trabajos en background

        // Lista de comandos
        char commands[1024] = "";
        for(int i = 0; i < num_commands; i++) {
            if(i > 0) {
                strcat(commands, " | "); // Agregar el separador de comandos
            }

            if(line->commands[i].filename != NULL) {
                strcat(commands, line->commands[i].argv[0]); // Concatenar el nombre del comando
            } else if(line->commands[i].argv[0] != NULL && line->commands[i].argc > 0) {
                strcat(commands, line->commands[i].argv[0]); // Concatenar el nombre del comando
            } else {
                strcat(commands, "/Vacio/"); // Comando vacío
                fprintf(stderr, "Error: Comando no válido.\n");
                continue;
            }

            for (int j = 1; j < line->commands[i].argc; j++) {
                strcat(commands, " "); // Agregar un espacio entre los argumentos
                strcat(commands, line->commands[i].argv[j]); // Concatenar los argumentos
            }

            if(i == num_commands - 1 && line->background) {
                strcat(commands, " ");
                strcat(commands, "&"); // Agregar el símbolo de background al final
            }
        }

        for(z = 0; z < num_commands; z++){
            // Manejo comandos excepciones
            if(line->commands[z].filename == NULL){
                // Manejo comando 'cd'
                if(strcmp(line->commands[z].argv[0], "cd") == 0) {
                    char *dir = NULL;
                    // Sin argumentos: cambiar a directorio HOME
                    if(line->commands[z].argc == 1){
                        dir = getenv("HOME");
                        if(dir == NULL) {
                            fprintf(stderr, "Error: No se pudo obtener el directorio HOME.\n");
                            continue;
                        }                        
                    } else if(line->commands[z].argc == 2) {
                        // Con argumento: cambiar a directorio especificado
                        dir = line->commands[z].argv[1];
                    } else {
                        // Más de un argumento: error
                        fprintf(stderr, "Error: El comando 'cd' solo acepta uno o dos argumentos.\n");
                        continue;
                    }
                    if(chdir(dir) == -1) {
                        fprintf(stderr, "Error: No se pudo cambiar al directorio %s: %s\n", dir, strerror(errno));
                        continue;
                    }
                
                // Manejo comando 'exit'
                } else if(strcmp(line->commands[z].argv[0], "exit") == 0){
                    exit(0); // Terminar el programa inmediatamente
                
                // Manejo comando 'jobs'
                } else if(strcmp(line->commands[z].argv[0], "jobs") == 0){
                    job(); // Listar trabajos en background
                    continue;
                
                // Manejo comando 'fg'
                } else if(strcmp(line->commands[z].argv[0], "fg") == 0){
                    int id = -1; // Inicializar id a -1, porque no existe
                    if(line->commands[z].argc == 1) {
                        id = -2; // Traer último trabajo al foreground
                    } else if(line->commands[z].argc == 2) {
                        // Con argumento: traer trabajo especificado al foreground
                        id = atoi(line->commands[z].argv[1]);
                    } else {
                        fprintf(stderr, "Error: El comando 'fg' solo acepta uno o dos argumentos.\n");
                        continue;
                    }
                    fg(id); // Traer trabajo al foreground
                }
            }

            // Crear pipe para el comando actual si no es el último
            if(z < num_commands - 1) {
                if(pipe(pipefds) == -1){
                    fprintf(stderr, "Falló al crear la pipe.");
                    exit(-1);
                }
            }

            pid = fork();
            
            if(pid < 0){ /* Error */
                fprintf(stderr, "Falló el fork().");
                exit(-1);
            } else if(pid == 0){ /* Proceso Hijo */
                
                signal(SIGINT, SIG_DFL);    // Restaurar el manejador de interrupción
                signal(SIGQUIT, SIG_DFL);   // Restaurar el manejador de salida
                signal(SIGTSTP, SIG_DFL);   // Restaurar el manejador de parada

                if(line->background) {
                    if(z == 0){
                        procesos_background = pid; // Guardar el PID del proceso hijo en la variable global
                        setpgid(0, procesos_background); // Establecer el grupo de procesos del shell
                    } else {
                        setpgid(0, procesos_background); // Establecer el grupo de procesos del shell
                    }
                } else {
                    setpgid(0, procesos_foreground); // Establecer el grupo de procesos del shell
                }

                // Primer hijo y hay redirección de entrada
                if (z == 0 && line->redirect_input != NULL) {
                    fd = open(line->redirect_input, O_RDONLY);
                    if (fd == -1) {
                        fprintf(stderr, "Error al abrir el archivo de entrada: %s.\n", strerror(errno));
                        exit(1);
                    }
                    if(dup2(fd, STDIN_FILENO) == -1){
                        fprintf(stderr, "Error al redirigir la entrada a archivo: %s.\n", strerror(errno));
                        close(fd);
                        exit(1);
                    }
                    close(fd);
                }

                // Comandos intermedios
                if (z > 0) { // No es el primer comando: conectar entrada al pipe anterior
                    if(dup2(prev_pipe[0], STDIN_FILENO) == -1){
                        fprintf(stderr, "Error al redirigir la entrada a comando anterior: %s.\n", strerror(errno));
                        exit(1);
                    }
                }
                if (z < num_commands - 1) { // No es el último comando: conectar salida al siguiente pipe
                    if(dup2(pipefds[1], STDOUT_FILENO) == -1){
                        fprintf(stderr, "Error al redirigir la salida a comando siguiente: %s.\n", strerror(errno));
                        exit(1);
                    }
                }

                // Último comando y redirección de salida
                if(z == num_commands-1 && line->redirect_output != NULL){ // Redirección de salida, si es a archivo
                    fd = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644); // O_WRONLY, solo escritura. O_CREAT, si no existe el archivo lo crea, con los permisos 0644 (Propietario: Lectura, Escritura; Grupo y Otros: Lectura)
                    if (fd == -1) {
                        fprintf(stderr, "Error al abrir el archivo de salida: %s.\n", strerror(errno));
                        exit(1);
                    }
                    if(dup2(fd, STDOUT_FILENO) == -1){
                        fprintf(stderr, "Error al redirigir la salida a archivo: %s.\n", strerror(errno));
                        close(fd);
                        exit(1);
                    }
                    close(fd);
                }
                // Último comando y redirección de error
                if(z == num_commands-1 && line->redirect_error != NULL){ // Redirección de error, si es a archivo
                    fd = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0644); // O_WRONLY, solo escritura. O_CREAT, si no existe el archivo lo crea, con los permisos 0644 (Propietario: Lectura, Escritura; Grupo y Otros: Lectura)
                    if (fd == -1) {
                        fprintf(stderr, "Error al abrir el archivo de salida de error: %s.\n", strerror(errno));
                        exit(1);
                    }
                    if(dup2(fd, STDERR_FILENO) == -1){
                        fprintf(stderr, "Error al redirigir la salida de error: %s.\n", strerror(errno));
                        close(fd);
                        exit(1);
                    }
                    close(fd);
                }

				// Cerrar todos los descriptores de pipe que no se necesiten
                if (z > 0) {
                    close(prev_pipe[0]);
                    close(prev_pipe[1]);
                }
                if (z < num_commands - 1) {
                    close(pipefds[0]);
                    close(pipefds[1]);
                }

                // Ejecutar el comando
                execvp(line->commands[z].filename, &line->commands[z].argv[0]); // El primero es el comando y el segundo un puntero donde coge las opciones de despues
                fprintf(stderr, "Error al ejecutar el comando. \n%s\n", strerror(errno)); // Mostrar el mensaje de error
                exit(1);
            } else if(pid > 0) { /* Proceso Padre */
                signal(SIGINT, SIG_IGN);    // Ignorar señales de interrupción
                signal(SIGQUIT, SIG_IGN);   // Ignorar señales de salida
                signal(SIGTSTP, SIG_IGN);   // Ignorar señales de parada
                pids_jobs[z] = pid; // Guardar el PID del proceso hijo en el array
                
                if(line->background) {
                    if(z == 0) {
                        procesos_background = pid; // Guardar el PID del primer proceso hijo en la variable global
                        setpgid(0, procesos_background); // Establecer el grupo de procesos del shell
                    } else {
                        setpgid(pid, procesos_background); // Establecer el grupo de procesos del shell
                    }
                }
                
                // Restaurar el control del terminal al shell
                if (tcsetpgrp(STDIN_FILENO, procesos_foreground) == -1) {
                    fprintf(stderr, "Error al restaurar el control del terminal: %s\n", strerror(errno));
                }

                if(z > 0) {
                    close(prev_pipe[0]); // Cerrar el pipe de lectura del anterior
                    close(prev_pipe[1]); // Cerrar el pipe de escritura del anterior
                }
                if(z < num_commands - 1) {
                    prev_pipe[0] = pipefds[0]; // Guardar el pipe actual para el siguiente hijo
                    prev_pipe[1] = pipefds[1]; // Guardar el pipe actual para el siguiente hijo
                }
            }
        }

        // Esperar a todos los procesos
        if (!line->background) { // Si se ejecuta en foreground            
            signal(SIGCHLD, SIG_DFL); // Restaurar el manejador de SIGCHLD
            
            for (int i = 0; i < num_commands; i++) {
                waitpid(pids_jobs[i], &status, 0); // Esperar a que todos los procesos terminen
            }

            signal(SIGCHLD, manejador_senales); // Restaurar el manejador de SIGCHLD
        } else { // Si se ejecuta en background
            for(int i = 0; i < num_commands; i++) {
                if (line->commands[i].filename != NULL) {
                    add_job(pids_jobs[num_commands-1], commands); // Agregar el proceso a la lista de background                            
                }
            }
        }

		printf("\n==> ");
        fflush(stdout); // Forzar el vaciado del buffer de salida estándar
	}

	return 0;
}