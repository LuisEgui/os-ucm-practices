## Respuestas del ejercicio 1

- Para generar el ejecutable "Show_file" se debe:
    1. gcc -c -Wall -g show_file.c -o show_file.o
    2. gcc -Wall -g -o show_file show_file.o

    O bien, de manera alternativa, directamente:
    1. gcc -c -Wall -g show_file.c -o show_file

- Los comandos indicados anteriormente cumplen con la generacion del fichero objeto y el enlazado del mismo de forma independiente.