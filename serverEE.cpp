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
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>

using namespace std;

/**
 * serverEE.cpp
 *
*/

/**
 * Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host address
#define ServerEE_UDP_PORT 23247 // Server EE port number
#define MAXDATASIZE 1024 
#define FAIL -1
#define MAX_FILE_SIZE 1000

/**
 * Defined global variables
 */
string course;
string detail;
string temp;
ifstream fp;
int sockfd_serverEE_UDP; // ServerEE datagram socket
struct sockaddr_in serverEE_addr, serverM_addr; // serverEE address
char recv_buf[MAXDATASIZE]; // Data sent by client
char write_result[MAXDATASIZE]; // Send to serverEE
char write_buf[MAXDATASIZE]; //Read registered data
vector<string> new_course(500);
vector<string> new_credit(500);
vector<string> new_professor(500);
vector<string> new_date(500);
vector<string> new_name(500);
string result="";

/**
 * Defined functions
 */

// 1. Create UDP socket
void create_serverEE_socket();

// 2. Initialize connection with serverM
void init_serverM_connection();

// 3. Bind a socket
void bind_socket();

// 4. Receive data from serverM

// 5. process data

/**
 * Step 1: Create server CS UDP sockect
 */
void create_serverEE_socket() {
    sockfd_serverEE_UDP = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket
    if (sockfd_serverEE_UDP == FAIL) {
        perror("[ERROR] server EE: can not open socket");
        exit(1);
    }
}

/**
 * Step 2: Create sockaddr_in struct
 */

void init_serverM_connection() {

    // Server CS side information
    // Initialize server C IP address, port number
    memset(&serverEE_addr, 0, sizeof(serverEE_addr)); //  make sure the struct is empty
    serverEE_addr.sin_family = AF_INET; // Use IPv4 address family
    serverEE_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    serverEE_addr.sin_port = htons(ServerEE_UDP_PORT); // Server C port number
}


/**
 * Step 3: Bind socket with specified IP address and port number
 */
void bind_socket() {
    if (::bind(sockfd_serverEE_UDP, (struct sockaddr *) &serverEE_addr, sizeof(serverEE_addr)) == FAIL) {
        perror("[ERROR] Server EE: fail to bind serverM UDP socket");
        exit(1);
    }

    printf("The Server <EE> is up and running using UDP on port <%d>. \n", ServerEE_UDP_PORT);
}

int main() {

    /******    Step 1: Create server C socket (UDP)  ******/
    create_serverEE_socket();
    /******    Step 2: Create sockaddr_in struct  ******/
    init_serverM_connection();
    /******    Step 3: Bind socket with specified IP address and port number  ******/
    bind_socket();

    fp.open("ee.txt");
         //fp.open("test.txt");
    if(!fp){
              perror("fail to open the file");
              exit(1);
          }
    int idx=0;
        while(getline(fp,temp)){
              memset(write_buf,0,sizeof(write_buf));
              strcpy(write_buf,temp.c_str());
              new_course[idx]=strtok(write_buf,",");
              new_credit[idx] = strtok(NULL,",");
              new_professor[idx] =strtok(NULL,",");
              new_date[idx] = strtok(NULL,",");
              new_name[idx] = strtok(NULL,",");
                idx++;
              //stringstream ss(temp);
              
        }
    
    // Part of codes is from http://c.biancheng.net
    while (true) {

        /******    Step 4: Receive data from serverM  ******/
        socklen_t serverM_addr_size = sizeof(serverM_addr);
        memset(recv_buf,0,sizeof(recv_buf));
        if (::recvfrom(sockfd_serverEE_UDP, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &serverM_addr,&serverM_addr_size) == FAIL) {
            perror("[ERROR] ServerCS: fail to receive data from serverM");
            exit(1);
        }

        course = strtok(recv_buf, ",");
        detail = strtok(NULL,",");
        printf("The Server <EE> received a request from the Main Server about the <%s> of <%s>\n",detail.c_str(),course.c_str());
        string flag;
        for(int i=0;i<sizeof(new_course);i++){
             flag="n";
        
             //if(strcmp(course.c_str(),new_course[i].c_str())==0){
                 if(course == new_course[i]){
                    flag="y";
                    if(detail=="Credit"){
                        result = new_credit[i];
                        break;
                    }else if(detail=="Professor"){
                        result = new_professor[i];
                        break;
                    }else if(detail=="Days"){
                        result = new_date[i];
                        break;
                    }else{
                        result = new_name[i];
                        break;
                    }
             }
        }     
            memset(write_result,0,sizeof(write_result));
            strcpy(write_result, flag.c_str());
            strcat(write_result,result.c_str());
            if (sendto(sockfd_serverEE_UDP, write_result, MAXDATASIZE, 0, (struct sockaddr *) &serverM_addr,
                       serverM_addr_size) < 0) {
                perror("[ERROR] server EE: fail to send write result to serverM server");
                exit(1);
            }
            if(flag=="n"){
                printf("Didn't find the course: <%s>.\n",course.c_str());
            }else{
                printf("The course information has been found: THe <%s> of <%s> is <%s>.\n",detail.c_str(),course.c_str(),result.c_str());
            }
            printf("The server <EE> finished sending the response to the Main Server \n");
            fp.close();
        }

               
    close(sockfd_serverEE_UDP);
    return 0;
}
