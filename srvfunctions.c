#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <signal.h>
#include <netinet/in.h>

#include "srvheader.h"

#define BUFSIZE 512
#define CMDSIZE 5
#define PARSIZE 100

#define MSG_220 "220 srvFtp version 1.0\r\n"
#define MSG_331 "331 Password required for %s\r\n"
#define MSG_230 "230 User %s logged in\r\n"
#define MSG_530 "530 Login incorrect\r\n"
#define MSG_221 "221 Goodbye\r\n"
#define MSG_550 "550 %s: no such file or directory\r\n"
#define MSG_299 "299 File %s size %ld bytes\r\n"
#define MSG_226 "226 Transfer complete\r\n"

/**
 * function: receive the commands from the client
 * sd: socket descriptor
 * operation: \0 if you want to know the operation received
 *            OP if you want to check an especific operation
 *            ex: recv_cmd(sd, "USER", param)
 * param: parameters for the operation involve
 * return: only usefull if you want to check an operation
 *         ex: for login you need the seq USER PASS
 *             you can check if you receive first USER
 *             and then check if you receive PASS
 **/
bool recv_cmd(int sd, char *operation, char *param) {
    char buffer[BUFSIZE], *token;
    int recv_s;
    
    // receive the command in the buffer and check for errors
    if((recv_s = read(sd, buffer, BUFSIZE)) <= 0){
        warn("Error leyendo del socket.");
        return false;
    }

    // expunge the terminator characters from the buffer
    buffer[strcspn(buffer, "\r\n")] = 0;

    // complex parsing of the buffer
    // extract command receive in operation if not set \0
    // extract parameters of the operation in param if it needed
    token = strtok(buffer, " ");

    if (token == NULL || strlen(token) < 4) {
        warn("not valid ftp command");
        return false;
    } else {
        if (operation[0] == '\0') strcpy(operation, token);
        if (strcmp(operation, token)) {
            warn("abnormal client flow: did not send %s command", operation);
            return false;
        }
        token = strtok(NULL, " ");
        if (token != NULL) strcpy(param, token);
    }

    return true;
}

/**
 * function: send answer to the client
 * sd: file descriptor
 * message: formatting string in printf format
 * ...: variable arguments for economics of formats
 * return: true if not problem arise or else
 * notes: the MSG_x have preformated for these use
 **/
// argumentos variables
bool send_ans(int sd, char *message, ...){
    char buffer[BUFSIZE];

    va_list args;
    va_start(args, message);

    vsprintf(buffer, message, args);
    va_end(args);

    // send answer preformated and check errors
    if(write(sd, buffer, strlen(message)) < 0) {
        warn("Error al escribir en el socket");
        return false;
    }

    return true;

}

/**
 * function: RETR operation
 * sd: socket descriptor
 * file_path: name of the RETR file
 **/

void retr(int sd, char *file_path) {
    FILE *file;    
    int bread;
    long fsize;
    char buffer[BUFSIZE];
    int tsd;
    struct sockaddr_in taddr;
    int tlen;

    // check if file exists if not inform error to client
    file = fopen(file_path, "r");
    if(file == NULL){
        send_ans(sd, MSG_550, file_path);
        return;
    }

    // send a success message with the file length
    // me desplazo hasta el final del archivo
    fseek(file, 0L, SEEK_END);
    // obtengo la posicion actual (final)
    fsize = ftell(file);
    rewind(file);
    send_ans(sd, MSG_299, file_path, fsize);

    // important delay for avoid problems with buffer size
    sleep(1);

    // send the file

    // close the file

    // send a completed transfer message
}

/**
 * funcion: check valid credentials in ftpusers file
 * user: login user name
 * pass: user password
 * return: true if found or false if not
 **/
bool check_credentials(char *user, char *pass) {
    FILE *file = NULL;
    char *path = "./ftpusers", *line = NULL, cred[100];
    size_t len = 0;
    bool found = false;

    // make the credential string
    strncat(cred, user, strlen(user));
    strncat(cred, ":", 2);
    strncat(cred, pass, strlen(pass));

    #ifdef DEBUG
    printf("\nUsuario intentando conectarse: [%s]\n", cred);
    #endif

    // check if ftpusers file it's present
    file = fopen(path, "r+");
    if(! (file == NULL)){
        
        // search for credential string
        line = (char*)malloc(sizeof(cred));
        while(fgets(line, sizeof(cred), file)){
            len = strcspn(line, "\n");
            line[len] = 0;
            if(strcmp(line, cred) == 0){
                found = true;
                break;
            }
        }

        // close file and release any pointes if necessary
        fclose(file);
        free(line);
        
    } else {
        warnx("Credentials file missing");
    }

    // return search status
    // provisional
    return found;
}

/**
 * function: login process management
 * sd: socket descriptor
 * return: true if login is succesfully, false if not
 **/
bool authenticate(int sd) {
    char user[PARSIZE], pass[PARSIZE];

    // wait to receive USER action
    if(!recv_cmd(sd, "USER", user)){
        warn("Error en recepcion del nombre de usuario (comando USER).");
        return false;
    }

    // ask for password
    if(!send_ans(sd, MSG_331, user)) {
        warn("Error solicitando la password");
    }

    // wait to receive PASS action
    if(!recv_cmd(sd, "PASS", pass)){
        warn("Error en recepcion de la password de usuario (comando PASS).");
        return false;
    }

    // if credentials don't check denied login
    if(check_credentials(user, pass)) {
        if(!send_ans(sd, MSG_230, user)) {
            warn("Error enviando confirmacion de login");
            return false;
        }
        return true;
    } else {
        if(!send_ans(sd, MSG_530)) {
            warn("Error enviando confirmacion de login");
        }
        return false;
    }

    // confirm login
}

/**
 *  function: execute all commands (RETR|QUIT)
 *  sd: socket descriptor
 **/

void operate(int sd) {
    char op[CMDSIZE], param[PARSIZE];

    while (true) {
        op[0] = param[0] = '\0';
        // check for commands send by the client if not inform and exit
        if(!recv_cmd(sd, op, param)){
            //warn("Flujo anormal en operate.");
            continue;
        }

        getchar();
        getchar();

        if (strcmp(op, "RETR") == 0) {
            //retr(sd, param);
        } else if (strcmp(op, "QUIT") == 0) {
            // send goodbye and close connection
            send_ans(sd, MSG_221);
            close(sd);
            #ifdef _DEBUG_
            printf("Conexion cerrada\n");
            #endif
            break;
        } else {
            // invalid command
            // furute use
        }
    }
}