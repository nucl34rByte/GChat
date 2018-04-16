#include "stdafx.h"
#include <iostream>
#include <string>
#include <Windows.h>
#include <thread>

#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

unsigned int uiCursorOffset = 0;

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

class Client{
private:
	int iPort;
	std::string sHost;
	SOCKET sSocket;
	struct sockaddr_in addr;
	struct hostent* hp;
	WSAData wData;
public:
	static enum Error{
		WSA_STARTUP_ERROR,
		CREATE_SOCKET_ERROR,
		GET_HOST_BY_NAME_ERROR,
		CONNECT_ERROR,
		SUCCESS
	};

	Client(std::string sHost, int iPort){
		this->sHost = sHost;
		this->iPort = iPort;
	}

	Error connectToTheServer(){
		if (WSAStartup(MAKEWORD(2, 2), &wData) != 0){
			return WSA_STARTUP_ERROR;
		}
		
		sSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (sSocket == SOCKET_ERROR){
			return CREATE_SOCKET_ERROR;
		}

		hp = gethostbyname(sHost.c_str());
		if (hp == NULL){
			return GET_HOST_BY_NAME_ERROR;
		}

		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr[0];
		addr.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr[1];
		addr.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr[2];
		addr.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr[3];
		addr.sin_port = htons(iPort);

		if (connect(sSocket, (struct sockaddr *)&addr, sizeof(addr)) != 0){
			return CONNECT_ERROR;
		}
		
		return SUCCESS;
	}

	SOCKET getConnection(){
		return sSocket;
	}
};

class Chat{
private:
	SOCKET sClient;
public:
	static enum Error {
		SEND_ERROR,
		SUCCESS
	};

	Chat(SOCKET sClient) {
		this->sClient = sClient;
	}

	std::string getMessage() {
		const unsigned int BUFFER_LENGTH = 4096;
		char cBuffer[BUFFER_LENGTH + 1];
		memset(cBuffer, 0, sizeof(cBuffer));
		int iBytesReceived;
		do {
			iBytesReceived = recv(sClient, cBuffer, BUFFER_LENGTH, 0);
			if (iBytesReceived == -1) {
				return "";
			}
			else {
				std::string sResponse(cBuffer);
				return sResponse;
			}
		} while (iBytesReceived == BUFFER_LENGTH);
	}

	Error sendMessage(std::string sMessage){
		if (send(sClient, sMessage.c_str(), sMessage.length(), 0) == -1){
			return SEND_ERROR;
		}
		return SUCCESS;
	}
};

void sendMessageThread(Chat *chat){
	while (true){
		CONSOLE_SCREEN_BUFFER_INFO cnslInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cnslInfo);
		SHORT lCurrentX = cnslInfo.dwCursorPosition.X;
		SHORT lCurrentY = cnslInfo.dwCursorPosition.Y;

		COORD crdNewPosition;
		crdNewPosition.X = 0;
		crdNewPosition.Y = lCurrentY + uiCursorOffset;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), crdNewPosition);

		uiCursorOffset = 0;

		std::string sMessage;

		std::cout << "[SENDED] ";
		std::getline(std::cin, sMessage);
		switch (chat->sendMessage(sMessage)){
		case(Chat::SEND_ERROR) :
			printError("Cannot send a message! Try again!");
			break;
		}
	}
}

void getMessageThread(Chat *chat){
	while (true){
		std::string sMessage = chat->getMessage();
		if (!sMessage.empty()){
			uiCursorOffset++;
			CONSOLE_SCREEN_BUFFER_INFO cnslInfo;
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cnslInfo);
			SHORT lCurrentX = cnslInfo.dwCursorPosition.X;
			SHORT lCurrentY = cnslInfo.dwCursorPosition.Y;

			COORD crdNewPosition;
			crdNewPosition.X = 0;
			crdNewPosition.Y = lCurrentY + uiCursorOffset;
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), crdNewPosition);

			std::cout << "[RECEIVED] " << sMessage << std::endl;

			crdNewPosition.X = lCurrentX;
			crdNewPosition.Y = lCurrentY;
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), crdNewPosition);
		}
		else{
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");

	printBanner();

	Client::Error eResult;
	Client* client;
	do{
		std::string sIP;
		printInput("Enter a server ip: ");
		std::getline(std::cin, sIP);

		client = new Client(sIP, 1337);
		eResult = client->connectToTheServer();
		switch (eResult){
		case Client::WSA_STARTUP_ERROR:
			printError("Error while connecting to the server. Try again!");
			delete client;
			break;
		case Client::CREATE_SOCKET_ERROR:
			printError("Error while connecting to the server. Try again!");
			delete client;
			break;
		case Client::GET_HOST_BY_NAME_ERROR:
			printError("Error while connecting to the server. Try again!");
			delete client;
			break;
		case Client::CONNECT_ERROR:
			printError("Error while connecting to the server. Try again!");
			delete client;
			break;
		case Client::SUCCESS:
			printSucces("Connected!");
			break;
		default:
			printError("Unknown error! Try again!");
			delete client;
			return -1;
		}
	} while (eResult != Client::SUCCESS);

	SOCKET sServer = client->getConnection();
	Chat *chat(new Chat(sServer));

	std::thread tSendMessage = std::thread(sendMessageThread, chat);
	std::thread tGetMessage = std::thread(getMessageThread, chat);

	tGetMessage.join();
	tSendMessage.join();

	delete chat;
	return 0;
}

