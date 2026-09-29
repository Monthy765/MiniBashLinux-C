# MiniBashLinux‑C
Mini shell escrita en C para ejecutar comandos básicos en Linux, con soporte para redirecciones, pipes, procesos en background y gestión de trabajos. Proyecto desarrollado y probado en entorno WSL (Ubuntu) y compilado mediante Makefile.

---

## 🚀 Características principales

MiniBashLinux‑C implementa:

### ✔ Ejecución de comandos externos
Ejemplos:
- `ls`
- `cat archivo.txt`
- `grep -i hola archivo.txt`
- `wc -l archivo.txt`

### ✔ Redirecciones
- Entrada: `comando < archivo`
- Salida: `comando > archivo`
- Error: `comando 2> archivo`

### ✔ Pipes
Permite encadenar comandos:
- `cat entrada.txt | grep Hola`
- `cat entrada.txt | grep Hola | wc -l`
- `cat entrada.txt | grep -i Hola | sort | uniq`

### ✔ Comandos internos
- `cd` — cambiar de directorio  
- `cd ..` — subir al directorio padre  
- `pwd` — mostrar directorio actual  

### ✔ Procesos en background
- `sleep 100 &` — ejecuta el proceso en segundo plano

### ✔ Gestión de trabajos
- `jobs` — lista procesos en background  
- `fg` — trae el último proceso al foreground  

---

## 📦 Estructura del proyecto
´´´text
MiniBashLinux-C/
│
├── src/          # Código fuente (.c)
│   ├── myshell.c
│   ├── jobs.c
│   └── ...
│
├── include/      # Cabeceras (.h)
│   ├── myshell.h
│   ├── jobs.h
│   └── parser.h
│
├── lib/          # Librerías externas
│   └── libparser.a
│
├── build/        # Ejecutable generado
│   └── myshell
│
├── test/         # Archivos de prueba
│   ├── entrada.txt
│   ├── salida.txt
│   └── errores.txt
│
├── Makefile      # Compilación automática
└── README.md     # Este documento
´´´
---

## 🔧 Compilación

El proyecto incluye un **Makefile**, por lo que solo necesitas ejecutar:

```bash
make
```

El ejecutable se generará en:
    build/myshell

---

## ▶️ Ejecución
```bash
./build/myshell
```

---

## 🧪 Ejemplos de uso

### Redirecciones
```bash
./build/myshell < test/entrada.txt > test/salida.txt
./build/myshell 2> test/errores.txt
```

### Pipes
```bash
cat < entrada.txt | grep -i Hola > salida.txt
cat < entrada.txt | grep Hola | wc -l > salida.txt
cat < entrada.txt | grep -i Hola | sort | uniq > salida.txt
```

### Comandos internos
```bash
pwd
cd ..
```

### Background y jobs
```bash
sleep 100 &
jobs
fg
```
---

## 🛠 Requisitos
- WSL (Ubuntu recomendado)
- gcc
- make

---

## 📄 Licencia
Este proyecto puede distribuirse bajo licencia MIT (opcional).

---

## 👤 Autor
Desarrollado por Monthy765.