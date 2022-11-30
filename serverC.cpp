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
 *  Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host address
#define ServerC_UDP_PORT 21247 // Server C port number
#define MAXDATASIZE 1024 
#define FAIL -1
#define MAX_FILE_SIZE 1000

/**
 * Defined global variables
 */

string username;
string pass;
string temp;
ifstream fp;
int sockfd_serverC_UDP; // ServerC datagram socket
struct sockaddr_in serverC_addr, serverM_addr; // serverC address 
char recv_buf[MAXDATASIZE]; // Data sent by serverM
char write_result[MAXDATASIZE]; // Send to serverM
vector<string> new_name(500); //store registered name
vector<string> new_pass(500); //store registerd password
string result="";

/**
 * functions
 */

// 1. Create UDP socket
void create_serverC_socket();

// 2. Initialize connection with serverM
void init_serverM_connection();

// 3. Bind a socket
void bind_socket();

/**
 * Step 1: Create server C UDP sockect
 */
void create_serverC_socket() {
    sockfd_serverC_UDP = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket
    if (sockfd_serverC_UDP == FAIL) {
        perror("[ERROR] server C: can not open socket");
        exit(1);
    }
}

/**
 * Step 2: Create sockaddr_in struct
 */
void init_serverM_connection() {

    // Server C side information
    // Initialize server C IP address, port number
    memset(&serverC_addr, 0, sizeof(serverC_addr)); //  make sure the struct is empty
    serverC_addr.sin_family = AF_INET; // Use IPv4 address family
    serverC_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    serverC_addr.sin_port = htons(ServerC_UDP_PORT); // Server C port number
}

/**
 * Step 3: Bind socket with specified IP address and port number
 */
void bind_socket() {
    if (::bind(sockfd_serverC_UDP, (struct sockaddr *) &serverC_addr, sizeof(serverC_addr)) == FAIL) {
        perror("[ERROR] Server C: fail to bind serverM UDP socket");
        exit(1);
    }

    printf("The Server C is up and running using UDP on port <%d>. \n", ServerC_UDP_PORT);
}

int main() {

    /******    Step 1: Create server C socket (UDP)  ******/
    create_serverC_socket();

    /******    Step 2: Create sockaddr_in struct  ******/
    init_serverM_connection();

    /******    Step 3: Bind socket with specified IP address and port number  ******/
    bind_socket();

    /******    Step 4: Read registered data ******/
    fp.open("cred.txt");
         //fp.open("test.txt");
    if(!fp){
              perror("fail to open the file");
              exit(1);
          }
    int idx=0;
        while(getline(fp,temp)){
             // new_name[idx]=strtok(write_buf,",");
             int pos = temp.find(",");
              new_name[idx] = temp.substr(0,pos);
              new_pass[idx] = temp.substr(pos+1);
              idx++;
        }
    
    // Part of codes is from http://c.biancheng.net
    while (true) {

        /******    Step 5: Receive data from serverM  ******/
        socklen_t serverM_addr_size = sizeof(serverM_addr);
        memset(recv_buf,0,sizeof(recv_buf));
        if (::recvfrom(sockfd_serverC_UDP, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &serverM_addr,&serverM_addr_size) == FAIL) {
            perror("[ERROR] ServerC: fail to receive data from serverM");
            exit(1);
        }

        username = strtok(recv_buf, ",");
        pass = strtok(NULL,",");
        printf("The Server C received an authentication request from the Main Server\n");
        int len=0;
        for(int i=0; i<sizeof(pass);i++){
            if((pass[i]>='a'&&pass[i]<='z')
                    ||(pass[i]>='A'&&pass[i]<='Z')
                    	||(pass[i]>='0'&&pass[i]<='9')
                   			 ||pass[i]=='('||pass[i]==')'
                   				 ||pass[i]=='@'||pass[i]=='#'
                    				||pass[i]=='!'||pass[i]=='$'
                  						  ||pass[i]=='%'||pass[i]=='^'
                   								 ||pass[i]=='&'||pass[i]=='*'){
                    len++;
            }
        }
         /****** Compare received data to registerd ones ******/
          for(int i=0; i<sizeof(new_name);i++){
              result = "u";
              if(username == new_name[i]){
                  result = "s";
                  for(int j=0; j<len;j++){
                      if(pass[j]!=new_pass[i][j]){
                          result = "p";
                          break;
                      }
                  }
                  break;
              }
          }
            memset(write_result,0,sizeof(write_result));
            strncpy(write_result, result.c_str(), 1);
            if (sendto(sockfd_serverC_UDP, write_result, MAXDATASIZE, 0, (struct sockaddr *) &serverM_addr,
                       serverM_addr_size) < 0) {
                perror("[ERROR] server C: fail to send write result to serverM server");
                exit(1);
            }
            printf("The serverC finished sending the response to the Main Server \n");
            fp.close();
        }

    close(sockfd_serverC_UDP);
    return 0;
}
