#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <err.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 512

#define VALID_PORT(x) ((x > 0 && x < 65535) ? 1 : 0) 

/**
 * function: receive and analize the answer from the server
 * sd: socket descriptor
 * code: three leter numerical code to check if received
 * text: normally NULL but if a pointer if received as parameter
 *       then a copy of the optional message from the response
 *       is copied
 * return: result of code checking
 **/
// code es el codigo que se espera recibir del servidor
// si a text le paso NULL descarto la respuesta (mensaje de caracteres)
// si le paso a text una variable extraigo el mensaje de respuesta
bool recv_msg(int sd, int code, char *text) {
    /** 
     * buffer: donde se recibe el buffer
    */
    char buffer[BUFSIZE], message[BUFSIZE];
    /** 
     * recv_s: longitud del mensaje recibido
    */
    int recv_s, recv_code;

    // receive the answer
    // recv y send
    recv_s = read(sd, buffer, BUFSIZE);

    // error checking
    // warn manda al stderr un mensaje de warning pero no llama a exit
    if (recv_s < 0) warn("error receiving data");
    if (recv_s == 0) errx(1, "connection closed by host");

    // parsing the code and message receive from the answer
    /** 
     * toma de buffer un entero y luego aplicamos expresiones regulares para extraer el mensaje que esta despues del codigo
    */
    //    entrada formateo regex        codigo    mensaje 
    sscanf(buffer, "%d %[^\r\n]\r\n", &recv_code, message);
    printf("%d %s\n", recv_code, message);
    // optional copy of parameters
    if(text) strcpy(text, message);
    // boolean test for the code
    return (code == recv_code) ? true : false;
}

/**
 * function: send command formated to the server
 * sd: socket descriptor
 * operation: four letters command
 * param: command parameters
 **/

// param podria usar strtok para dividir en palabras

void send_msg(int sd, char *operation, char *param) {
    char buffer[BUFSIZE] = "";

    // command formating
    if (param != NULL)
        sprintf(buffer, "%s %s\r\n", operation, param);
    else
        sprintf(buffer, "%s\r\n", operation);

    // send command and check for errors
    if(write(sd, buffer, strlen(buffer)+1) < 0){
        // arrojo un warning
        warn("Error enviando el comando.");
    }

}

/**
 * function: simple input from keyboard
 * return: input without ENTER key
 **/
char * read_input() {
    char *input = malloc(BUFSIZE);
    if (fgets(input, BUFSIZE, stdin)) {
        // barre input y devuelve una string donde encuentre el caracter \n
        return strtok(input, "\n");
    }
    return NULL;
}

/**
 * function: login process from the client side
 * sd: socket descriptor
 **/
void authenticate(int sd) {
    char *input, desc[100];
    int code;

    // aca si la contrasena o el usuario es incorrecto hago un exit

    // ask for user
    printf("username: ");
    input = read_input();

    // send the command to the server
    //send_msg
    send_msg(sd, "USER", input);

    // relese memory
    free(input);

    // wait to receive password requirement and check for errors
    // espero una respuesta 331, si es distinto entonces cierro

    // ask for password
    // podriamos intentar meter la password hasta 3 veces
    printf("passwd: ");
    input = read_input();

    // send the command to the server
    // envio el comando PASS

    // release memory
    free(input);

    // wait for answer and process it and check for errors
    // 230 USER username logged, la password es correcta

    // podria intentar reenviar el commando, user se podria enviar tantas veces como se quisiera


}

/**
 * function: operation get
 * sd: socket descriptor
 * file_name: file name to get from the server
 **/
void get(int sd, char *file_name) {
    char desc[BUFSIZE], buffer[BUFSIZE];
    int f_size, recv_s, r_size = BUFSIZE;
    FILE *file;

    // send the RETR command to the server

    // check for the response

    // parsing the file size from the answer received
    // "File %s size %ld bytes"
    sscanf(buffer, "File %*s size %d bytes", &f_size);

    // open the file to write
    file = fopen(file_name, "w");

    //receive the file



    // close the file
    fclose(file);

    // receive the OK from the server

}

/**
 * function: operation quit
 * sd: socket descriptor
 **/
void quit(int sd) {
    // send command QUIT to the client

    // receive the answer from the server

}

/**
 * function: make all operations (get|quit)
 * sd: socket descriptor
 **/
void operate(int sd) {
    char *input, *op, *param;

    while (true) {
        printf("Operation: ");
        input = read_input();
        if (input == NULL)
            continue; // avoid empty input
        op = strtok(input, " ");
        // free(input);
        if (strcmp(op, "get") == 0) {
            param = strtok(NULL, " ");
            get(sd, param);
        }
        else if (strcmp(op, "quit") == 0) {
            quit(sd);
            break;
        }
        else {
            // new operations in the future
            printf("TODO: unexpected command\n");
        }
        free(input);
    }
    free(input);
}

/**
 * Run with
 *         ./myftp <SERVER_IP> <SERVER_PORT>
 **/
int main (int argc, char *argv[]) {
    int sd, connection_port;
    struct sockaddr_in addr;

    // arguments checking
    if(argc == 3){
        
        connection_port = atoi(argv[2]);

        if(VALID_PORT(connection_port)){
            
            //addr.sin_addr = inet(argv[2]);

            // create socket and check for errors
            sd = socket(AF_INET, SOCK_STREAM, 0);
            if(sd < 0){ 
                errx(3, "Error en creacion del socket"); 
            }

            // set socket data    
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet(argv[1]);
            addr.sin_port = htons(connection_port);

            // connect and check for errors
            if(connect(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
                errx(4, "Error en la coneccion del socket");
            }


            // if receive hello proceed with authenticate and operate if not warning
            // recv_msg(sd, 220, text)
            // el servidor manda 220 si el cliente se pudo conectar con exito
            // el cliente siempre consulta al server con comandos de texto
            if(recv_msg(sd, 220, NULL)){
                // Comienza el procesamiento del programa

                // enviar comando de autenticacion
                authenticate(sd);

            } else {
                warn("Fallo en obtener respuesta hello del server.");
            }

            // close socket

        } else {
            // errx
            errx(2,"Puerto invalido.");
        }
    } else {
        // errx
        errx(1,"Cantidad de argumentos incorrecta\nUso: ./myftp <SERVER_IP> <PORT>");
    }
    

    return 0;
}

// resuelto main hasta autenticate
// send_msg 
// recv_msg