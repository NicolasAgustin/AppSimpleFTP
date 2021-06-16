#include "clientheader.h"

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
    int code, passwordTry = 0;

    // aca si la contrasena o el usuario es incorrecto hago un exit
    /** 
     * aca puede ir un bucle infinito para que el comando USER se pueda enviar todas las veces que se quiera
     * se puede definir un handler para la signal SIGKILL, entonces cuando el usuario presiona ctrl+c la funcion retorna.
    */
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
    if(recv_msg(sd, 331, NULL)){
        while(passwordTry < 3){
            // ask for password
            // podriamos intentar meter la password hasta 3 veces
            printf("passwd: ");
            input = read_input();

            // send the command to the server
            // envio el comando PASS
            send_msg(sd, "PASS", input);

            // release memory
            free(input);

            // wait for answer and process it and check for errors
            // 230 USER username logged, la password es correcta

            if(recv_msg(sd, 230, desc)){
                printf("%s \n", desc);
                break;
            } else {
                passwordTry++;
                printf("Incorrect password, try again.\n");
            }
        }
        if(passwordTry == 3){
            errx(6, "Limite de intentos alcanzado.");
        }
    } else {
        errx(5, "Usuario incorrecto.");
    }
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
    int dsd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    // listen to data channel (default idem port)
    if(listen(sd, 1) < 0) errx(15, "Listen data channel error: line 173");

    // send the RETR command to the server
    send_msg(sd, "RETR", file_name);

    // check for the response
    if(!recv_msg(sd, 299, buffer)) return;

    // accept new connection
    dsd = accept(dsd, (struct sockaddr*)&addr, &len);
    if(dsd < 0){
        errx(16, "Accept data channel error: line 184");
    }

    // parsing the file size from the answer received
    // "File %s size %ld bytes"
    sscanf(buffer, "File %*s size %d bytes", &f_size);

    // open the file to write
    file = fopen(file_name, "w");

    //receive the file from data channel 
    while(true){
        if(f_size < BUFSIZE){ 
            r_size = f_size;
        }
        recv_s = read(dsd, buffer, r_size);
        if(recv_s < 0) warn("receive error: Line 200");
        fwrite(buffer, 1, r_size, file);
        if(f_size < BUFSIZE) break;
        f_size = f_size - BUFSIZE;
    }

    // close data channel 
    close(dsd);


    // close the file
    fclose(file);

    // receive the OK from the server
    if(!recv_msg(sd, 226, NULL)) warn("RETR anormally terminated");

}

/**
 * function: operation quit
 * sd: socket descriptor
 **/
void quit(int sd) {
    // send command QUIT to the client
    send_msg(sd, "QUIT", NULL);
    // receive the answer from the server
    recv_msg(sd, 221, NULL);
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
        if (input == NULL) {
            continue; // avoid empty input
        }
        // retorna un token en cada llamada a strtok, cuando se le envia NULL, trabaja con la string que ya tenia cargada (la variable es static)
        op = strtok(input, " ");
        // free(input);
        
        if (strcmp(op, "get") == 0) {
            param = strtok(NULL, " ");
            #if DEBUG
                printf("[%s]\n", param);
            #endif
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