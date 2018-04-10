#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <set>
#include <algorithm>

#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

std::string sLastMessage = "";

inline void printInfo(std::string sText){
	std::cout << "[INFO] " << sText << std::endl;
}

inline void printError(std::string sText){
	std::cerr << "[ERROR] " << sText << std::endl;
}

inline void printSucces(std::string sText){
	std::cout << "[SUCCESS] " << sText << std::endl;
}

inline void printInput(std::string sText){
	std::cout << "[INPUT] " << sText;
}

inline void printLog(std::string sText){
	std::cout << "[LOG] " << sText << std::endl;
}

inline void printBanner(){
	std::cout << "" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "		GGGGG  CCCCC  H   H  AAAAA  TTTTT" << std::endl;
	std::cout << "		G   G  C      H   H  A   A    T  " << std::endl;
	std::cout << "		G      C      HHHHH  AAAAA    T  " << std::endl;
	std::cout << "		G  GG  C      H   H  A   A    T  " << std::endl;
	std::cout << "		GGGGG  CCCCC  H   H  A   A    T  " << std::endl;
	std::cout << "" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "" << std::endl;
}

class Server {
private:
	int m_iPort;
	SOCKET m_sockServer;
	WSAData m_wData;
public:
	static enum Error {
		WSA_STARTUP_ERROR,
		CREATE_SOCKET_ERROR,
		BIND_ERROR,
		LISTEN_ERROR,
		NONBLOCK_ERROR,
		SUCCESS
	};

	Server(int iPort){
		this->m_iPort = iPort;
	}

	Error startServer() {
		if (WSAStartup(MAKEWORD(2, 2), &m_wData) != 0){
			return WSA_STARTUP_ERROR;
		}

		m_sockServer = socket(AF_INET, SOCK_STREAM, 0);
		if (m_sockServer == SOCKET_ERROR){
			return CREATE_SOCKET_ERROR;
		}

		std::set<SOCKET> setClients;

		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(m_iPort);

		if (bind(m_sockServer, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR){
			return BIND_ERROR;
		}

		u_long iMode = 0;
		if (ioctlsocket(m_sockServer, FIONBIO, &iMode) != NO_ERROR){
			return NONBLOCK_ERROR;
		}

		if (listen(m_sockServer, SOMAXCONN) == SOCKET_ERROR){
			return LISTEN_ERROR;
		}

		return SUCCESS;
	}

	void stopServer(){
		closesocket(m_sockServer);
		WSACleanup();
	}

	SOCKET getServerSocket(){
		return m_sockServer;
	};
};

class Chat{
private:
	SOCKET m_sockServer;
	std::set<SOCKET> m_setClients;
public:
	static enum Error {
		SEND_ERROR,
		SUCCESS
	};

	Chat(SOCKET sockServer) {
		this->m_sockServer = sockServer;
	}

	void startChat(){
		while (true){
			fd_set set;
			FD_ZERO(&set);
			FD_SET(m_sockServer, &set);
			for (auto i = m_setClients.begin(); i != m_setClients.end(); i++){
				FD_SET(*i, &set);
			}

			int iMax = std::max<int>(m_sockServer, *std::max_element(m_setClients.begin(), m_setClients.end()));
			select(iMax + 1, &set, NULL, NULL, NULL);
			for (auto i = m_setClients.begin(); i != m_setClients.end(); i++){
				if (FD_ISSET(*i, &set)){
					const int BUFFER_SIZE = 4096;
					char cBuffer[BUFFER_SIZE + 1];
					memset(cBuffer, 0, sizeof(cBuffer));

					int iReceivedSize = recv(*i, cBuffer, BUFFER_SIZE, 0);
					if ((iReceivedSize == 0) || (iReceivedSize == -1)){
						closesocket(*i);
						m_setClients.erase(*i);
					}
					else{
						char cMessage[BUFFER_SIZE + 1];
						memset(cMessage, 0, sizeof(cMessage));
						strncpy(cMessage, cBuffer, iReceivedSize);

						if (!std::string(cMessage).empty()){
							printLog("New message: " + std::string(cMessage));
							for (auto j = m_setClients.begin(); j != m_setClients.end(); j++){
								if (*j != *i){
									send(*j, cMessage, iReceivedSize, 0);
								}
							};
						}
					}
				}
			}
			if (FD_ISSET(m_sockServer, &set)){
				SOCKET sockClient = accept(m_sockServer, 0, 0);
				u_long iMode = 0;
				ioctlsocket(sockClient, FIONBIO, &iMode);
				m_setClients.insert(sockClient);
			}
		}
	}
};

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");

	printBanner();

	Server* server = new Server(1337);
	Server::Error eResult = server->startServer();
	switch (eResult){
	case(Server::WSA_STARTUP_ERROR) :
		printError("Error while function WSAStartUp calling. Try again!");
		delete server;
		return -1;
		break;
	case(Server::CREATE_SOCKET_ERROR) :
		printError("Error while creating a socket. Try again!");
		delete server;
		return -1;
		break;
	case(Server::BIND_ERROR) :
		printError("Error while binding a socket. Try again!");
		delete server;
		return -1;
		break;
	case(Server::LISTEN_ERROR) :
		printError("Error while listening a port. Try again!");
		delete server;
		return -1;
		break;
	case Server::SUCCESS:
		printSucces("Server started!");
		break;
	default:
		printError("Unknown error! Try again!");
		delete server;
		return -1;
	}
	
	Chat* chat = new Chat(server->getServerSocket());
	chat->startChat();

	delete chat;
	delete server;

	return 0;
}