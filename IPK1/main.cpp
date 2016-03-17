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
	//cout << str << endl;
    if((counter = str.find("http://")) != -1) str.erase(counter, 7);
	else if((counter = str.find("https://")) != -1) str.erase(counter, 8);
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
	if((counter = str.find(" ")) != -1) str.replace(counter, 1, "%20");
    	if((counter = str.rfind("/")) != -1){
        	myUrl->path = str;
        	str.erase(0, counter+1);
    	}
	else myUrl->path = "/";
	
	if((counter = str.find("%20")) != -1) str.replace(counter, 3, " ");
    	if(str.length() != 0) myUrl->nameFile = str;
    	else myUrl->nameFile = "index.html";
}

int connecting(Url myUrl, int *sock){

	
	struct hostent *server;
	struct sockaddr_in server_address;

	if((server = gethostbyname(myUrl.name.c_str())) == NULL){
		fprintf(stderr, "ERROR: no such host as %s\n", myUrl.name.c_str());
		exit(EXIT_FAILURE);
	}

	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = PF_INET;

	bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(myUrl.port);

	if((*sock = socket(PF_INET, SOCK_STREAM, 0)) <= 0){
		perror("ERROR: connect");
		exit(EXIT_FAILURE);
	}

	if(connect(*sock, (const struct sockaddr *)&server_address, sizeof(server_address)) != 0){
		perror("ERROR: connect\n");
		exit(EXIT_FAILURE);
	}
	return 0;

}

int sndAndRcv(Url myUrl, string *tmp, int sock){
	char buf[1024];
	bzero(buf, 1024);
	int bytestx, bytesrx;
	string msg(
		"GET " +  myUrl.path + " HTTP/1.1\r\n"
		"Host: " + myUrl.name + "\r\n"
		"Connection: close\r\n\r\n"
	);
	//cout<<myUrl.path<<endl<<endl;

	bytestx = send(sock, msg.c_str(), msg.size(), 0);
	if(bytestx < 0) perror("ERROR in sendto\n");

	while((bytesrx = recv(sock, buf, 1024, 0)) > 0){ 
		tmp->append(buf, bytesrx);
	}
	if(bytesrx < 0) perror("ERROR in recvfrom\n");
	return 0;

}

int response(string tmp){

	int index;
	string response;
	tmp.erase(0, 9);
	index = tmp.find("\n");
	response = tmp.substr(0, index);
	//cout << response << endl;
	if(response.find("200") != -1) return 0;
	else if(response.find("301") != -1) return 300;
	else if(response.find("302") != -1) return 300;
	else return 400;
}

void findLocation(string *tmp){
	int index;
	index = (*tmp).find("Location:");
	(*tmp).erase(0, index + 10);
	index = (*tmp).find("\n");
	(*tmp).erase(index);
}

int main(int argc, char **argv)
{
    	Url myUrl;
	int sock, Response, index;
	string tmp = "";
    	char* url = argv[1];

    	parser(url, &myUrl);
	connecting(myUrl, &sock);
	sndAndRcv(myUrl, &tmp, sock);
	Response = response(tmp);

	if(Response == 300){
		int i;
		for(i = 0; i < 4; i++){
			findLocation(&tmp);
			url = (char *)tmp.c_str();
			
			Url myUrl;

			parser(url, &myUrl);
			//cout << url << endl;
			//cout << myUrl.name << endl;

			connecting(myUrl,&sock);
			tmp.clear();
			sndAndRcv(myUrl, &tmp, sock);
			Response = response(tmp);
			//cout << Response << endl;
			if(Response == 0) break;
			else if(Response == 400) {
				fprintf(stderr, "pristup odmitnut\n");
				exit(400);
			}
		}
		if(i == 4) {
			fprintf(stderr, "webclient se nestihl presmerovat na spravnou adresu do 5-ti pokusu\n");
			exit(1);
		}
	}
	else if(Response == 0){
		
	}
	else if(Response == -1) return 1;
	
	ofstream OutF;
	cout<<myUrl.nameFile<<endl;
	index = tmp.find("\r\n\r\n");
	tmp.erase(0, index + 4);
	OutF.open(myUrl.nameFile.c_str(), ios::out | ios::binary);
	OutF.write(tmp.c_str(), tmp.length());
	OutF.close();
	close(sock);
    return 0;

}
