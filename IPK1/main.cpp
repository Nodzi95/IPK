#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
using namespace std;

typedef struct Url {
    int port;
    string name;
    string path;
    string nameFile;
};

void parser(char* url, Url *myUrl){
    int counter;
    char* neco = "";
    string str = url;
    if((counter = str.find("http://")) != -1) str.erase(counter, 7);
    else {
        cerr << "nenalezeno http://\n";
        exit(1);
    }

    if((counter = str.find(":")) != -1){
        myUrl->name = str.substr(0, counter);
        str.erase(0, counter);
    }
    else if((counter = str.find("/")) != -1){
        myUrl->name = str.substr(0, counter);
        str.erase(0, counter);
    }
    else {
        myUrl->name = str;
        str.erase(0, counter);
    }
    //cout << myUrl->name;
    if((str.find(":")) != -1){
        str.erase(0, 1);
        if((counter = str.find("/")) != -1){

            myUrl->port = atoi(str.substr(0,counter).c_str());
            str.erase(0, counter);
        }
        else {
            myUrl->port = atoi(str.substr(0,counter).c_str());
            str.erase(0, counter);
        }
    }
    else myUrl->port = 80;
    //cout << myUrl->port;
    //cout << str;
    if((counter = str.rfind("/")) != -1){
        myUrl->path = str;
        str.erase(0, counter+1);
    }
	else myUrl->path = "/";
    if(str.length() != 0) myUrl->nameFile = str;
    else myUrl->nameFile = "index.html";
}


int main(int argc, char **argv)
{
    Url myUrl;
	int sock, bytestx, bytesrx;
	string tmp = "";
	struct hostent *server;
	struct sockaddr_in server_address;
	char buf[1024];
    char* url = argv[1];
    parser(url, &myUrl);
    
	if((server = gethostbyname(myUrl.name.c_str())) == NULL){
		fprintf(stderr, "ERROR: no such host as %s\n", myUrl.name.c_str());
		exit(EXIT_FAILURE);
	}
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = PF_INET;
	bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(myUrl.port);

	printf("INFO: Server socket : %s : %d\n", inet_ntoa(server_address.sin_addr), 
server_address.sin_port);
	
	if((sock = socket(PF_INET, SOCK_STREAM, 0)) <= 0){
		perror("ERROR: connect");
		exit(EXIT_FAILURE);
	}

	bzero(buf, 1024);
	string msg(
		"GET " +  myUrl.path + " HTTP/1.1\r\n"
		"Host: " + myUrl.name + "\r\n"
		"Connection: close\r\n\r\n"
	);
	cout<<myUrl.path<<endl<<endl;

	if(connect(sock, (const struct sockaddr *)&server_address, sizeof(server_address)) != 0){
		perror("ERROR: connect\n");
		exit(EXIT_FAILURE);
	}

	bytestx = send(sock, msg.c_str(), msg.size(), 0);
	if(bytestx < 0) perror("ERROR in sendto\n");

	while((bytesrx = recv(sock, buf, 1024, 0)) > 0){ 
		tmp.append(buf, bytesrx);
	}
	if(bytesrx < 0) perror("ERROR in recvfrom\n");

	ofstream OutF;
	cout<<myUrl.nameFile<<endl;
	OutF.open(myUrl.nameFile.c_str(), ios::out | ios::binary);
	OutF.write(tmp.c_str(), tmp.length());
	OutF.close();
	close(sock);
    return 0;

}
