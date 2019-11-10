#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "55555"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "listlib.h"

#pragma comment (lib, "Ws2_32.lib")

DWORD WINAPI thread_fun(void* argument);

HANDLE ghMutex;
struct list* clients;

int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;
	clients = newList();
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET* ClientSocket = NULL;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	ghMutex = CreateMutex(NULL, FALSE, NULL);
	if (ghMutex == NULL)
		return 1;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);
	iResult = listen(ListenSocket, SOMAXCONN);
	struct list* threads = newList();
	DWORD id;
	HANDLE thread;
	while (1) 
	{
		ClientSocket = (SOCKET*)malloc(sizeof(SOCKET));
		*ClientSocket = accept(ListenSocket, NULL, NULL);
		WaitForSingleObject(ghMutex, INFINITE);
		addToList(clients, ClientSocket);
		thread = CreateThread(
			NULL, // atrybuty bezpieczenstwa
			0, // inicjalna wielkosc stosu
			thread_fun, // funkcja watku
			ClientSocket,// dane dla funkcji watku
			0, // flagi utworzenia
			&id);
		if (thread != INVALID_HANDLE_VALUE && thread != NULL)
		{
			printf(" Utworzylem watek o id %x\n", id);
			// ustawienie priorytetu
			SetThreadPriority(thread, THREAD_PRIORITY_NORMAL);
			addToList(threads, &thread);
		}
		ReleaseMutex(ghMutex);
	}
	closesocket(ListenSocket);
	CloseHandle(ghMutex);
	deleteList(clients);
	deleteList(threads);
	WSACleanup();

	return 0;
}

DWORD WINAPI thread_fun(void* argument)
{
	int iResult;
	char username[64];
	char recvbuf[512];
	char message[580];
	int recvbuflen = sizeof(recvbuf);
	int gotUserName = NULL;
	DWORD dwCount = 0, dwWaitResult;
	SOCKET* thisThreadClient = argument;
	struct list* clientIterator;
	SOCKET* clientSocketIterator;
	do {
		iResult = recv(*thisThreadClient, recvbuf, recvbuflen, 0);
		WaitForSingleObject(ghMutex, INFINITE);
		if (iResult > 0) {
			if (gotUserName == NULL) {
				strcpy(username, recvbuf);
				gotUserName = 1;
			}
			else {
				strcpy(message, username);
				strcat(message, ": ");
				strcat(message, recvbuf);
				clientIterator = clients;
				printf("%s", message);
				while (clientIterator->next != NULL) {
					clientSocketIterator = clientIterator->next->value;
					send(*clientSocketIterator, message, strlen(message) + 1, 0);
					clientIterator = clientIterator->next;
				}
			}
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
			closesocket(*thisThreadClient);
			//WSACleanup();
			return 1;
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(*thisThreadClient);
			//WSACleanup();
			return 1;
		}
		ReleaseMutex(ghMutex);
	} while (iResult > 0);

}
