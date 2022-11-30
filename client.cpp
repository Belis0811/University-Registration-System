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
#include <iostream>
#include <string>
#include <vector>

using namespace std;

/**
 * client.cpp
 * client sends username, password, course and category to serverM
 *
*/

/**
 * Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host IP address
#define ServerM_Client_TCP_PORT 25247 // serverM port number
#define MAXDATASIZE 1024 
#define FAIL -1 


/**
 * Defined global variables
 */
int count; //store login failed times
int localPort; //client TCP port number(dynamic)
int sockfd_client_TCP; // client socket
struct sockaddr_in serverM_addr; // serverM  address
struct sockaddr_in loc_addr; //client address
socklen_t len = 0;
char write_buf[MAXDATASIZE]; //send to serverM
char write_result[MAXDATASIZE]; // result from serverM
char name[50]; //username
char pass[50]; //password
char detail[50]; //category 
char course[50]; //course code

/**
 * Function define:
 */
// 1. Create TCP socket
void create_client_socket_TCP();

// 2. Initialize TCP connection with serverM
void init_serverM_connection();

// 3. Send connection request to serverM
void request_serverM_connection();

/**
 * Step 1: Create client TCP socket 
 */
void create_client_socket_TCP() {
    sockfd_client_TCP = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (sockfd_client_TCP == FAIL) {
        perror("[ERROR] client: can not open client socket ");
        exit(1);
    }

}

/**
 * Step 2: Initial TCP connection info
 */
void init_serverM_connection() {
    // *** Beej’s guide to network programming - 9.24
    // Initialize TCP connection between client and serverM using given IP address and port number
    memset(&serverM_addr, 0, sizeof(serverM_addr)); //  make sure the struct is empty
    serverM_addr.sin_family = AF_INET; // Use IPv4 address family
    serverM_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Source address
    serverM_addr.sin_port = htons(ServerM_Client_TCP_PORT); // serverM port number
}

/**
 * Step 3: Send connection request to serverM
 */
void request_serverM_connection() {

    if (connect(sockfd_client_TCP, (struct sockaddr *) &serverM_addr, sizeof(serverM_addr)) == FAIL) {
        perror("[ERROR] client: fail to connect with serverM server");
        close(sockfd_client_TCP);
        exit(1); 
    }
    printf("The client is up and running \n");
}


int main(void) {

    create_client_socket_TCP();
    init_serverM_connection();
    request_serverM_connection();
 
    	 /****** get local port number ******/
    len = sizeof(sizeof(loc_addr));
    memset(&loc_addr,0,len);
    getsockname(sockfd_client_TCP,(struct sockaddr *)&loc_addr,&len);
    if(loc_addr.sin_family == AF_INET){
            localPort = loc_addr.sin_port;
    }

   /****** get input username and password ******/
while(true){
        printf("Please enter the username:");
       
        memset(name,0,sizeof(name));
        cin.getline(name,50);

        printf("Please enter the password:");

        memset(pass,0,sizeof(pass));
        cin.getline(pass,50);

        memset(write_buf,0,sizeof(write_buf));//clear before store
        strcpy(write_buf,name);
        strcat(write_buf,",");
        strcat(write_buf,pass); // send data in the format of "username,password"

        /******    Step 4:  Send data to serverM    *******/
        if (send(sockfd_client_TCP, write_buf, sizeof(write_buf), 0) == FAIL) {
            perror("[ERROR] client: fail to send input data");
            close(sockfd_client_TCP);
            exit(1);
        }
        printf("<%s> sent an authentication request to the main server \n",name);
       //cout<<write_buf;

        /******    Step 5:  Get result back from ServerM    *******/
        memset(write_result,0,sizeof(write_result));
        if (recv(sockfd_client_TCP, write_result, sizeof(write_result), 0) == FAIL) {
            perror("[ERROR] client: fail to receive write result from serverM");
            close(sockfd_client_TCP);
            exit(1);
        }
        char result = write_result[0];
        
       /******  Step 6: Process the result and present on the screen ******/
        if (result == 's') { // If all info are correct
            printf("<%s> received the result of authentication using TCP over port <%u>. Authentication is successful.\n", name, localPort);
            memset(write_result,0,sizeof(write_result));
            break;
        }else if(result == 'u'){ // If username is incorrect
            printf("<%s> received the result of authentication using TCP over port <%u>. Authentication failed: Username Does not exist.\n", name, localPort);
            count++;//record failed attempts 
            if(count==3){ //attempted times over 3
                printf("Authentication Failed for 3 attempts. Client will shut down");
                close(sockfd_client_TCP);
                exit(1);
            }
            memset(write_result,0,sizeof(write_result));
            printf("Attempts remaining:<%d>", 3-count); //print remaining attempts
            continue;
        }else if(result == 'p'){ // If username is correct but password is incorrect
            printf("<%s> received the result of authentication using TCP over port <%u>. Authentication failed: Password does not match.\n", name, localPort);
            count++;
             if(count==3){
                printf("Authentication Failed for 3 attempts. Client will shut down");
                close(sockfd_client_TCP);
                exit(1);
            }
            memset(write_result,0,sizeof(write_result));
            printf("Attempts remaining:<%d>", 3-count);
            continue;
        }
}

       /******  Step 7: Process the course info and present on the screen ******/
while(1){
        printf("Please enter the course code to query: \n");
     
        memset(course,0,sizeof(course));
        cin.getline(course,50);

        printf("Please enter the category(Credit/Professor/Days/CourseName):\n");

        memset(detail,0,sizeof(detail));
        cin.getline(detail,50);
        
        memset(write_buf,0,sizeof(write_buf));
        strcpy(write_buf,course);
        strcat(write_buf,",");
        strcat(write_buf,detail);

        if (send(sockfd_client_TCP, write_buf, sizeof(write_buf), 0) == FAIL) {
            perror("[ERROR] client: fail to send input data");
            close(sockfd_client_TCP);
            exit(1);
        }
        printf("<%s> sent a request to the main server.\n",name);

         /******   Get result back from Server    *******/
        memset(write_result,0,sizeof(write_result));
        if (recv(sockfd_client_TCP, write_result, sizeof(write_result), 0) == FAIL) {
            perror("[ERROR] client: fail to receive write result from serverM");
            close(sockfd_client_TCP);
            exit(1);
        }
        printf("The client received the response from the Main server using TCP over port <%u>.\n",localPort);
         char result = write_result[0];
         char info[500];
         /*for(int i=1; i<sizeof(write_result);i++){
             info[i-1] = write_result[i];
         }*/
         strcpy(info,write_result+1);
     if(result=='y'){
            printf("The <%s> of <%s> is <%s>.\n",detail,course,info);
     }else{
         printf("Didn't find the course: <%s>.",course);
     }
}
    // Close the socket and tear down the connection after we are done using the socket
    close(sockfd_client_TCP);
    return 0;
}
