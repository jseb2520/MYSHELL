#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


#define PROMPT ">>#"
#define MSJ_ERROR "Error de sintaxis \n"
#define MAX 51

int CadIguales(char cadena1[MAX],char cadena2[MAX]);

void Comando(char cadena[MAX], int plano);

void pipes(char **argumento1, char **argumento2);

void CrearProceso(char* argumento[MAX], int plano);

void redirEntrada(char *cadena);

void redirSalida(char *cadena);



/* Con esta funci�n compararemos la cadena que introduce el usuario, con exit,
en caso de ser iguales, saldra del programa, de lo contrario devolvera 0  y el programa
seguira en ejecucion */

int CadIguales(char cadena1[],char cadena2[]){
    int i=0;
    int igual=0;

    while ((cadena1[i]==cadena2[i])&&(cadena1[i]!='\0')&&(cadena2[i]!='\0')){
        i++;
    }

    if ((cadena1[i]==cadena2[i])&&(cadena1[i]=='\0')&&(cadena2[i]=='\0')){
        exit(0);
    }

    return(igual);
}

void Comando(char cadena[MAX], int plano){

    int i,j,k,flag_pipe;
    char argumentoInd[MAX][MAX];
    char argumentoInd2[MAX][MAX];
    char redirec[MAX];
    char entrada[MAX];
    char *argumento[MAX];
    char *argumento2[MAX];
    int ejecutar=0;
    argumento[0] = NULL;
    argumento2[0] = NULL;
    flag_pipe = 0;

    k = 0;
    i = 0;
    while(cadena[i] != '\0' && cadena[i] != '|' && cadena[i] != '>'){
        /* Con este for recorremos la cadena por completo, y la vamos
        almacenando en una variable que consiste en un array bidimensional de tipo char,
        es decir almacena cadenas de cadenas de caracteres */
        for(j=0; cadena[i] != ' ' && cadena[i] != '\0' && cadena[i] != '|'  && cadena[i] != '>' && cadena[i] != '<';j++) {
            argumentoInd[k][j] = cadena[i];
            i++;
        }

        /* Comprobamos si la cadena sali� del bucle por encontrar un espacio...
        en tal caso se incrementa la i, ya que sino entraria en un bucle
        infinito*/

        if (cadena[i] == ' ') i++;

        /* Asignamos el terminador de cadena a cada comando que es leido */
        argumentoInd[k][j] = '\0';

        /* Y finalmente una vez creada la cadena, se la pasamos al puntero
        argumento que sera el que se ejecute con la funcion execvp */

        argumento[k] = argumentoInd[k];
        k++;
        if (cadena[i] == '<') { /* INICIO DEL IF */
            i++;
            if (cadena[i] != ' ') ejecutar=1;
            else { i++;
                for(j=0; cadena[i] != '\0' && cadena[i] != ' ' && cadena [i] != '|' && cadena [i] != '>'; j++){
                    entrada[j] = cadena[i];
                    i++;
                }
                entrada[j] = '\0';
                if (cadena[i] != '\0') i++;
                redirEntrada(entrada);
            }
        }

    }

    argumento[k] = NULL; // Asignamos NULL al �ltimo comando a ejecutar para el EXECVP

    /* Si encuentra un pipe, entrará en el IF y separará el segundo comando de la
    misma forma que se hizo con el primero, y finalmente llamará a la función pipe,
    pasándole los 2 argumentos como par�metros */

    if (cadena[i] == '|') {
        k=0;
        i++;
        if (cadena[i] != ' ') ejecutar=1;
        else {
            i++;
            flag_pipe = 1;
            while(cadena[i] != '\0' && cadena[i] != '>'){
                for(j=0; cadena[i] != ' ' && cadena[i] != '\0' && cadena[i] != '>';j++) {
                    argumentoInd2[k][j] = cadena[i];
                    i++;}
                if (cadena[i] == ' ' ) i++;

                argumentoInd2[k][j] = '\0';
                argumento2[k] = argumentoInd2[k];
                k++;
            }
            argumento2[k] = NULL;
        }
    }


    /* Si encuentra un > cortara la cadena que sera el fichero que quiere utilizar
    para la salida */
    //
    if (cadena[i] == '>'){
        i++;
        if (cadena[i] != ' ') ejecutar=1;
        else {
            i++;
            for(j=0; cadena[i] != '\0';j++) {
                redirec[j] = cadena[i];
                i++;}
            redirec[j] = '\0';
            redirSalida(redirec);
        }
    }/* FIN DE IF */

    /* Comprobamos si la variable ejecutar tiene valor 0 o 1, si tiene valor 0
    el programa se ejecutara correctamente, si tiene valor 1 significa que
    mientras analizaba alguna de las cadenas ha encontrado un error de sintaxis
    y mostrara en pantalla un mensaje de error. */

    if(ejecutar == 0) {
        if (flag_pipe==0) CrearProceso(argumento,plano);

        else pipes(argumento, argumento2);
    }
    else printf( MSJ_ERROR );

}/* FIN DE LA FUNCION COMANDO */

/* Con esta funci�n si el usuario introduce quiere cambiar la entrada estandar
( el teclado ) por lo que hay en un fichero, podra hacerlo mediante el simbolo
"<", por ejemplo: wc < fichero */

void redirEntrada(char *cadena){

    char *cadenaPtr;
    int fd;
    cadenaPtr = cadena; // Puntero a la cadena
    fd = open (cadenaPtr,O_RDONLY); // Asigno a la salida el fichero
    close (0); // Cierro la salida est�ndar
    dup(fd);


}

/* Con esta funci�n si el usuario introduce una redireccion de salida a un
fichero, en lugar de mostrar el comando en pantalla, lo guardara en fichero
por ejemplo: man -k file > lista.file */

void redirSalida(char *cadena){

    char *cadenaPtr;
    cadenaPtr = cadena; // Puntero a la cadena
    close (1); // Cierro la salida est�ndar
    open (cadenaPtr,O_CREAT | O_WRONLY,0777); // Asigno a la salida el fichero

}


/* Con esta funcion en el caso de que hayan pipes (se ejecuten dos comandos
concatenados) por ejemplo: ls -la | sort , recibira como parametro argumento1 y
argumento2 de tipo puntero a cadena de caracteres y creara un hijo que
sera el encargado de ejecutar los 2 comandos pasados por pipe.*/

void pipes(char **argumento1, char **argumento2){

    int fd[2],estado;
    pid_t hijo;
    hijo=fork();


    if (hijo==-1) printf("ERROR Creacion de proceso");
    else if (hijo==0) {
        pipe(&fd[0]); /* Funcion pipe encargada de crear la tuberia */
        if (fork()!=0) {
            close(fd[0]); /* cerramos el lado de lectura del pipe */
            close(1);
            dup(fd[1]); /* STDOUT = extremo de salida del pipe */
            close(fd[1]);/* cerramos el descriptor de fichero que sobra
								tras el dup(fd[1])*/
            execvp(argumento1[0],argumento1);
        }
        else {
            close(fd[1]);/* cerramos el lado de lectura del pipe */
            close(0);
            dup(fd[0]);/* STDOUT = extremo de salida del pipe */
            close(fd[0]);/* cerramos el descriptor de fichero que sobra
								tras el dup(fd[0])*/
            execvp(argumento2[0],argumento2);

        }
    }
    else  hijo=wait(&estado);
}


/* Con esta función en el caso de que no haya tuber�as y sea un solo comando a ejecutar,
por ejemplo: ls -la , recibirá como parametro el argumento de tipo puntero a cadena
de cadena de caracteres, creará un hijo para que el comando execvp no finalice el
programa y en definitiva ejecutará el comando que le pedimos.*/

void CrearProceso(char* argumento[MAX],int plano) {
    int estado=0;
    pid_t hijo;
    hijo=fork();
/* Comprobamos si el hijo se crea bien */
    if (hijo==-1) printf("ERROR Creacion de proceso");
    else if (hijo==0) {
        /* Y en caso de que el hijo esta bien creado, ejecutamos el proceso,
        si el programa comando a ejecutar no existe, nos da un error.*/
        execvp(argumento[0],argumento);
        perror("MYSHELL");
        exit(estado);
    }
    else  {
        if (plano == 0) hijo=wait(&estado);
    }

}


/* La funcion principal main contiene el bucle que se ejecutara de forma continua hasta que
el usuario escriba exit en el terminal.Recibe una cadena, la analiza, y a partir de las
funciones que hemos creado, hara lo que debe hacer */

int main(int argc, char *argv[]) {
    char cadena[MAX];
    char cadFin[]="exit";
    int fin=0,i,segplano=0, guardaStdout = dup(1), guardaStdin = dup(0);

    while(fin==0)
    {
        close(1); /* Cierro la salida que tenga, caso de haber guardado algo en un fichero
                 sera el fichero*/
        dup(guardaStdout); // Asigno la salida estandar, es decir, la consola.
        close(0); /* Cierro la salida que tenga, caso de haber guardado algo en un fichero
                 sera el fichero*/
        dup(guardaStdin); // Asigno la salida estandar, es decir, la consola.
        printf(PROMPT); // Imprimimos el prompt
        scanf("\n%[^\n]",cadena); // Escaneamos la cadena entera hasta que pulsa intro
        segplano=0;

        for(i=0;cadena[i] != '\0'; i++){
            if(cadena[i] == '&') {
                cadena[i] = '\0';
                segplano = 1;
            }
        }

        fin=CadIguales(cadena,cadFin); // Comparamos si la cadena es exit, en tal caso sale
        Comando(cadena,segplano); // Y si no es exit, entra en la funcion Comando
    }
    exit(0);
    return 0;

}