# MiniBashLinux‑C

Una mini-shell escrita en **C** para ejecutar comandos básicos en Linux, con soporte para redirecciones, pipes, procesos en background y gestión de trabajos. El proyecto a sido desarrollado y probado en un entorno **WSL (Ubuntu)** y compilado mediante **Makefile**.

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

```text
MiniBashLinux-C/
├── src/          # Código fuente (.c)
│   ├── myshell.c
│   └── jobs.c
├── include/      # Cabeceras (.h)
│   ├── myshell.h
│   ├── jobs.h
│   └── parser.h
├── lib/          # Librerías externas
│   └── libparser.a
├── build/        # Ejecutable generado
│   └── myshell
├── test/         # Archivos de prueba
│   ├── entrada.txt
│   ├── salida.txt
│   └── errores.txt
├── Makefile      # Compilación automática
└── README.md     # Documentación principal
```

---

## 🛠 Requisitos del Sistema

Antes de compilar, asegúrate de tener instaladas las herramientas de desarrollo esenciales en tu entorno Linux/WSL:

```bash
sudo apt update
sudo apt install build-essential gcc make
```

---

## 🔧 Compilación

El proyecto incluye un **Makefile**, por lo que solo necesitas ejecutar:

```bash
make
```

El ejecutable se generará en: **build/myshell**.

---

## ▶️ Ejecución

Para iniciar la mini-shell, se ejecuta el binario generado:

```bash
./build/myshell
```

---

## 🧪 Ejemplos de uso

Una vez dentro de la shell (`MiniBashLinux-C>`), puedes probar los siguientes escenarios utilizando los archivos de la carpeta `test/`:

### Redirecciones
```bash
# Redirigir la entrada desde un archivo y guardar el resultado
./build/myshell < test/entrada.txt > test/salida.txt

# Capturar los mensajes de error en un archivo independiente
./build/myshell 2> test/errores.txt
```

### Tuberías (Pipes)
```bash
cat < entrada.txt | grep -i Hola > salida.txt
cat < entrada.txt | grep Hola | wc -l > salida.txt
cat < entrada.txt | grep -i Hola | sort | uniq > salida.txt
```

### Comandos internos
```bash
# Ver el directorio actual
pwd

# Volver al directorio anterior
cd ..
```

### Background y jobs
```bash
# Lanza un proceso largo en segundo plano
sleep 100 &

# Consulta la lista de trabajos activos
jobs

# Recupera el último proceso al primer plano
fg
```
---

## 🛠 Requisitos
- WSL (Ubuntu recomendado)
- gcc
- make

---

## 📄 Licencia
Este proyecto puede distribuirse bajo **Licencia MIT**. Para más detalles, consulta el archivo [LICENSE](LICENSE) adjunto en este repositorio.

---

## 👤 Autor
Desarrollado por **Monthy765** ([@Monthy765](https://github.com/Monthy765)).