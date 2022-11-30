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
 * serverC.cpp
 *
*/

/**
 * Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host address
#define ServerCS_UDP_PORT 22247 // Server CS port number
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
int sockfd_serverCS_UDP; // ServerCS datagram socket
struct sockaddr_in serverCS_addr, serverM_addr; // serverCS address
char recv_buf[MAXDATASIZE]; // Data sent by serverM
char write_result[MAXDATASIZE]; //  Send to serverM
char write_buf[MAXDATASIZE]; //Read registered data
vector<string> new_course(500);
vector<string> new_credit(500);
vector<string> new_professor(500);
vector<string> new_date(500);
vector<string> new_name(500);
string result="";

/**
 * functions
 */
// 1. Create UDP socket
void create_serverCS_socket();

// 2. Initialize connection with serverM
void init_serverM_connection();

// 3. Bind a socket
void bind_socket();

/**
 * Step 1: Create server CS UDP sockect
 */
void create_serverCS_socket() {
    sockfd_serverCS_UDP = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket
    if (sockfd_serverCS_UDP == FAIL) {
        perror("[ERROR] server C: can not open socket");
        exit(1);
    }
}

/**
 * Step 2: Create sockaddr_in struct
 */
void init_serverM_connection() {

    // Server CS side information
    // Initialize server C IP address, port number
    memset(&serverCS_addr, 0, sizeof(serverCS_addr)); //  make sure the struct is empty
    serverCS_addr.sin_family = AF_INET; // Use IPv4 address family
    serverCS_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    serverCS_addr.sin_port = htons(ServerCS_UDP_PORT); // Server C port number
}

/**
 * Step 3: Bind socket with specified IP address and port number
 */
void bind_socket() {
    if (::bind(sockfd_serverCS_UDP, (struct sockaddr *) &serverCS_addr, sizeof(serverCS_addr)) == FAIL) {
        perror("[ERROR] Server CS: fail to bind serverM UDP socket");
        exit(1);
    }

    printf("The Server <CS> is up and running using UDP on port <%d>. \n", ServerCS_UDP_PORT);
}

int main() {

    /******    Step 1: Create server C socket (UDP)  ******/
    create_serverCS_socket();
    /******    Step 2: Create sockaddr_in struct  ******/
    init_serverM_connection();
    /******    Step 3: Bind socket with specified IP address and port number  ******/
    bind_socket();

    fp.open("cs.txt");
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
        if (::recvfrom(sockfd_serverCS_UDP, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &serverM_addr,&serverM_addr_size) == FAIL) {
            perror("[ERROR] ServerCS: fail to receive data from serverM");
            exit(1);
        }

        course = strtok(recv_buf, ",");
        detail = strtok(NULL,",");
        printf("The Server <CS> received a request from the Main Server about the <%s> of <%s>\n",detail.c_str(),course.c_str());
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
            if (sendto(sockfd_serverCS_UDP, write_result, MAXDATASIZE, 0, (struct sockaddr *) &serverM_addr,
                       serverM_addr_size) < 0) {
                perror("[ERROR] server CS: fail to send write result to serverM server");
                exit(1);
            }
            if(flag=="n"){
                printf("Didn't find the course: <%s>.\n",course.c_str());
            }else{
                printf("The course information has been found: THe <%s> of <%s> is <%s>.\n",detail.c_str(),course.c_str(),result.c_str());
            }
            printf("The server <CS> finished sending the response to the Main Server \n");
            fp.close();
        }

               
    close(sockfd_serverCS_UDP);
    return 0;
}
