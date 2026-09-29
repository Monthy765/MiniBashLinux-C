// Para compilar gcc -Wall -Wextra -Iinclude src/myshell.c src/jobs.c lib/libparser.a -o build/myshell -static
// Para ir a la carpeta en WSL: cd /mnt/c/Users/maroa_crfkf22/Downloads/Universidad/VisualStudioCode_Proyectos/C_C++/Practica2
// Redirigir la salida de error a un archivo: ./build/myshell 2> test/errores.txt

// COMANDOS PARA PROBAR:
// cat < entrada.txt > salida.txt : Funciona
//  - cat entrada.txt > salida.txt : Funciona
// cat < entrada.txt | grep -i Hola > salida.txt : Funciona, sin """
//   - grep Hola entrada.txt : Funciona, sin ""
//   - grep -i Hola entrada.txt : Funciona, sin """
// cat < entrada.txt | grep Hola | wc -l > salida.txt : Funciona
//  - wc entrada.txt : Funciona
//  - cat entrada.txt | wc -w : Funciona
//  - cat entrada.txt | wc : Funciona
// cat < entrada.txt | grep -i Hola | sort | uniq > salida.txt : Funciona, sin """
// pwd : Muestra el directorio actual / cd : Cambia de directorio y cd .. : Cambia al directorio padre
// sleep 100 & : Ejecuta el comando en background(&) / jobs : Muestra los trabajos en background / fg : Trae el último trabajo al foreground