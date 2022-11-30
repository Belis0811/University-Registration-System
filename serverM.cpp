#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

/**
 * serverM.cpp
 *
*/

/**
 * Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host address
#define ServerM_UDP_PORT 24247 // serverM UDP port number
#define ServerM_Client_TCP_PORT 25247 // serverM TCP port number
#define ServerC_PORT 21247 //serverC port number
#define ServerCS_PORT 22247 //serverCS port number
#define ServerEE_PORT 23247 //serverEE port number
#define MAXDATASIZE 1024 
#define BACKLOG 10 // max number of connections allowed on the incoming queue
#define FAIL -1

/**
 * Defined global variables
 */
int sockfd_client_TCP, sockfd_UDP; // Parent socket for client &UDP socket
int child_sockfd_client; // Child socket for connection with client 
struct sockaddr_in serverM_client_addr, serverM_UDP_addr;
struct sockaddr_in dest_client_addr, dest_serverC_addr, dest_serverCS_addr,dest_serverEE_addr; 
string username;
char input_buf[MAXDATASIZE]; // Input data from client
char process_buf[MAXDATASIZE]; //Process the name and the password
char C_write_result[MAXDATASIZE]; // result returned from serverC
char CS_return_buf[MAXDATASIZE]; //data returned by server CS
char EE_return_buf[MAXDATASIZE]; // data returned by server EE


/**
 *  functions
 */
// 1. Create TCP socket w/ client & bind socket
void create_TCP_client_socket();

// 2. Create UDP socket
void create_UDP_socket();

// 3. Listen for client
void listen_client();

void init_connection_serverC();

void init_connection_serverCS();

void init_connection_serverEE();

/**
 * Step 1: Create TCP socket for client & bind socket
 */
void create_TCP_client_socket() {
    sockfd_client_TCP = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sockfd_client_TCP == FAIL) {
        perror("[ERROR] serverM: fail to create socket for client");
        exit(1);
    }
    // Initialize IP address, port number
    memset(&serverM_client_addr, 0, sizeof(serverM_client_addr)); //  make sure the struct is empty
    serverM_client_addr.sin_family = AF_INET; // Use IPv4 address family
    serverM_client_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    serverM_client_addr.sin_port = htons(ServerM_Client_TCP_PORT); // Port number for client
    // Bind socket for client with IP address and port number for client
    if (::bind(sockfd_client_TCP, (struct sockaddr *) &serverM_client_addr, sizeof(serverM_client_addr)) == FAIL) {
        perror("[ERROR] serverM server: fail to bind client socket");
        exit(1);
    }
}

/**
 * Step 3: Create UDP socket and bind socket
 */
void create_UDP_socket() {
    sockfd_UDP = socket(AF_INET, SOCK_DGRAM, 0); // UDP datagram socket
    if (sockfd_UDP == FAIL) {
        perror("[ERROR] serverM: fail to create UDP socket");
        exit(1);
    }
    // Initialize IP address, port number
    memset(&serverM_UDP_addr, 0, sizeof(serverM_UDP_addr)); //  make sure the struct is empty
    serverM_UDP_addr.sin_family = AF_INET; // Use IPv4 address family
    serverM_UDP_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    serverM_UDP_addr.sin_port = htons(ServerM_UDP_PORT); // Port number for client
    // Bind socket
    if (::bind(sockfd_UDP, (struct sockaddr *) &serverM_UDP_addr, sizeof(serverM_UDP_addr)) == FAIL) {
        perror("[ERROR] serverM: fail to bind UDP socket");
        exit(1);
    }
}

/**
 * Step 4: Listen for incoming connection from client
 */
void listen_client() {
    if (listen(sockfd_client_TCP, BACKLOG) == FAIL) {
        perror("[ERROR] serverM: fail to listen for client socket");
        exit(1);
    }
}

void init_connection_serverC() {
    // Info about serverC
    memset(&dest_serverC_addr, 0, sizeof(dest_serverC_addr));
    dest_serverC_addr.sin_family = AF_INET;
    dest_serverC_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    dest_serverC_addr.sin_port = htons(ServerC_PORT);
}

void init_connection_serverCS() {
    // Info about server CS
    memset(&dest_serverCS_addr, 0, sizeof(dest_serverCS_addr));
    dest_serverCS_addr.sin_family = AF_INET;
    dest_serverCS_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    dest_serverCS_addr.sin_port = htons(ServerCS_PORT);
}

void init_connection_serverEE() {
    // Info about server EE
    memset(&dest_serverEE_addr, 0, sizeof(dest_serverEE_addr));
    dest_serverEE_addr.sin_family = AF_INET;
    dest_serverEE_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    dest_serverEE_addr.sin_port = htons(ServerEE_PORT);
}


int main() {

    create_TCP_client_socket();
    listen_client();
    create_UDP_socket();
    printf("The serverM is up and running.\n");

        /******    Step 5: Accept connection from client using child socket   ******/
        socklen_t client_addr_size = sizeof(dest_client_addr);
        // Accept listening socket (parent)
        child_sockfd_client = ::accept(sockfd_client_TCP, (struct sockaddr *) &dest_client_addr, &client_addr_size);
        if (child_sockfd_client == FAIL) {
            perror("[ERROR] serverM: fail to accept connection with client");
            exit(1);
        }
             
while (true) {
        /******    Step 6: Receive input from client  ******/
        /******    Beej's Notes  ******/
        // Receive through child socket
        memset(input_buf,0,sizeof(input_buf));
        int recv_client = recv(child_sockfd_client, input_buf, MAXDATASIZE, 0);
        if (recv_client == FAIL) {
            perror("[ERROR] serverM: fail to receive input data from client");
            exit(1);
        }
        char judge[2]; //judge if message is course inquiry or username
        for(int i=0;i<2;i++){
            judge[i] = input_buf[i];
        }
        if(strcmp(judge,"CS0")==0||strcmp(judge,"EE0")==0){ //course inquiry
            memset(process_buf,0,sizeof(process_buf));
            strcpy(process_buf,input_buf);
            string course = strtok(process_buf,",");
            string detail = strtok(NULL,",");
            printf("The main server received from <%s> to query couse <%s> about <%s> using TCP over port <%d> \n",username.c_str(),course.c_str(),detail.c_str(),ServerM_Client_TCP_PORT);
             // Same data from client, but send to serverCS/EE
            char data_buf[MAXDATASIZE];
            memset(data_buf,0,sizeof(data_buf));
            strncpy(data_buf, input_buf, strlen(input_buf));

	/******* Handling which server to send ******/ 
            if(strcmp(judge,"CS0")==0){
                   init_connection_serverCS();
                //send request to serverCS
            if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *) &dest_serverCS_addr,sizeof(dest_serverCS_addr)) == FAIL) {
                perror("[ERROR] serverM: fail to send input data to serverCS");
                exit(1);
             }
             printf("The main server sent a request to server <CS>.\n");
                // Receive from Server CS
            socklen_t dest_serverCS_size = sizeof(dest_serverCS_addr);
            memset(CS_return_buf,0,sizeof(CS_return_buf));
            if (::recvfrom(sockfd_UDP, CS_return_buf, sizeof(CS_return_buf), 0, (struct sockaddr *) &dest_serverCS_addr,
                           &dest_serverCS_size) == FAIL) {
                perror("[ERROR] serverM: fail to receive result from ServerCS");
                exit(1);
            }
            printf("The main server received the response from Server <CS> using UDP over port <%d> \n",ServerM_UDP_PORT);

            char write_result[MAXDATASIZE];
            memset(write_result,0,sizeof(write_result));
            strncpy(write_result,CS_return_buf, sizeof(CS_return_buf));

            // Send response to client
            if (sendto(child_sockfd_client, write_result, sizeof(write_result), 0, (struct sockaddr *) &dest_client_addr,
                       sizeof(dest_client_addr)) == FAIL) {
                perror("[ERROR] serverM: fail to send write response from ServerC to client");
                exit(1);
            }
            printf("The main server sent the query information to the client. \n");
          }else{
                  //send request to serverEE
                   init_connection_serverEE();
                if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *) &dest_serverEE_addr,sizeof(dest_serverEE_addr)) == FAIL) {
                perror("[ERROR] serverM: fail to send input data to serverEE");
                exit(1);
             }
             printf("The main server sent a request to server <EE>.\n");
                // Receive from Server EE
            socklen_t dest_serverEE_size = sizeof(dest_serverEE_addr);
            memset(EE_return_buf,0,sizeof(EE_return_buf));
            if (::recvfrom(sockfd_UDP, EE_return_buf, sizeof(EE_return_buf), 0, (struct sockaddr *) &dest_serverEE_addr,
                           &dest_serverEE_size) == FAIL) {
                perror("[ERROR] serverM: fail to receive result from ServerEE");
                exit(1);
            }
            printf("The main server received the response from Server <EE> using UDP over port <%d> \n",ServerM_UDP_PORT);

            char write_result[MAXDATASIZE];
            memset(write_result,0,sizeof(write_result));
            strncpy(write_result,EE_return_buf, sizeof(EE_return_buf));

            // Send response to client
            if (sendto(child_sockfd_client, write_result, sizeof(write_result), 0, (struct sockaddr *) &dest_client_addr,
                       sizeof(dest_client_addr)) == FAIL) {
                perror("[ERROR] serverM: fail to send write response from ServerEE to client");
                exit(1);
            }
            printf("The main server sent the query information to the client. \n");
          }
        }else{
        init_connection_serverC();
         memset(process_buf,0,sizeof(process_buf));
         strcpy(process_buf,input_buf);
         username = strtok(process_buf, ",");
        printf("The main server received the authentication for <%s> using TCP over port <%d> \n", username.c_str(),ServerM_Client_TCP_PORT);

        // process client data using caeser code
        char data_buf[MAXDATASIZE];
        memset(data_buf,0,sizeof(data_buf));
        for(int i=0; i<sizeof(input_buf);i++){
            if(input_buf[i]>='a'&&input_buf[i]<='z'){
                     input_buf[i]+=4%26;
                     if(input_buf[i]<'a') input_buf[i]+=26;
                     if(input_buf[i]>'z') input_buf[i]-=26; 
                   
            }else if(input_buf[i]>='A'&&input_buf[i]<='Z'){
                    input_buf[i]+=4%26;
                    if(input_buf[i]<'A') input_buf[i]+=26;
                    if(input_buf[i]>'Z') input_buf[i]-=26; 
                }else if(input_buf[i]>='0'&&input_buf[i]<='9'){
                    input_buf[i]+=4%10;
                    if(input_buf[i]<'0') input_buf[i]+=10;
                     if(input_buf[i]>'9') input_buf[i]-=10; 
            }
        }
        //cout<<input_buf;
        strncpy(data_buf, input_buf, strlen(input_buf));

        // Send to server C
        if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *) &dest_serverC_addr,sizeof(dest_serverC_addr)) == FAIL) {
            perror("[ERROR] serverM: fail to send input data to serverC");
            exit(1);
        }
        printf("The main server sent an authentication request to serverC.\n");

            // Receive from Server C
            socklen_t dest_serverC_size = sizeof(dest_serverC_addr);
            memset(C_write_result,0,sizeof(C_write_result));
            if (::recvfrom(sockfd_UDP, C_write_result, 1, 0, (struct sockaddr *) &dest_serverC_addr,
                           &dest_serverC_size) == FAIL) {
                perror("[ERROR] serverM: fail to receive result from ServerC");
                exit(1);
            }
            printf("The main server received the result of the authetication request from ServerC using UDP over port <%d> \n",ServerM_UDP_PORT);

            char write_result[MAXDATASIZE];
            memset(write_result,0,sizeof(write_result));
            strncpy(write_result, C_write_result, sizeof(C_write_result));

            // Send response to client
            if (sendto(child_sockfd_client, write_result, 1, 0, (struct sockaddr *) &dest_client_addr,
                       sizeof(dest_client_addr)) == FAIL) {
                perror("[ERROR] serverM: fail to send write response from ServerC to client");
                exit(1);
            }
            printf("The main server sent the authentication result to the client. \n");
        }

    }

    // Close parent socket
    close(sockfd_client_TCP);
    close(sockfd_UDP);
    return 0;
}