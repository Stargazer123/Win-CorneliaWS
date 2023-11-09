

#include "../../include/win/wwebs.h"
#include <WS2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void init_tls_server(){

    struct sockaddr_in* pV4Addr;
    struct in_addr ipAddr;
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    DWORD dwThreadId;
    thread_params* params;
    char cip[17];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    printf("Server listening on port 8080...\n");

	while(1){
      params = (thread_params*)malloc(sizeof(thread_params));
		  clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
		  if (clientSocket == INVALID_SOCKET) {
			     printf("Accept failed\n");
			     closesocket(serverSocket);
			     WSACleanup();
           continue;
		  }
      memset(&params->cip[0],0,16);
      pV4Addr = (struct sockaddr_in*)&clientAddr;
      ipAddr = pV4Addr->sin_addr;
      strcpy(&cip[0], inet_ntoa(ipAddr));
      params->clientSocket=clientSocket;
      strcpy(&params->cip[0], &cip[0]);

      CreateThread(
                  NULL,
                  0,
                  server_thread,
                  (LPVOID)params,
                  0,
                  &dwThreadId);
	   }

    closesocket(serverSocket);
    WSACleanup();
}
