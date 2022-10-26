#pragma once
#include "winsock2.h"
#include <stdio.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")
void sendHead(char* filename, char* extname, SOCKET s);
void sendData(char* filename, SOCKET s);

void main() {
	WSADATA wsaData;
	bool first_connection = true;

	int nRc = WSAStartup(0x0202, &wsaData); //初始化Winsock
	if (nRc) {
		printf("Winsock初始化失败!\n");
		WSAGetLastError();
	}
	else if (wsaData.wVersion != 0x0202) {
		printf("Winsock版本不正确!\n");
		WSAGetLastError();
	}
	else printf("Winsock初始化成功!\n");

	SOCKET srvSocket;
	sockaddr_in srvAddr;
	srvSocket = socket(AF_INET, SOCK_STREAM, 0);  //创建一个监听socket
	if (srvSocket != INVALID_SOCKET) printf("Socket创建成功!\n");
	else {
		printf("Socket创建失败!\n");
		WSAGetLastError();
	}
	printf("----------------------------------------------\n");

	int srvPort;
	char srvIP[20];
	char fileAddr[50], fileName[50]; //fileName为输入主目录的副本
	printf("请输入监听端口号："); //置IP和端口号，以及主目录路径
	scanf("%d", &srvPort);
	printf("请输入监听IP地址：");
	scanf("%s", srvIP);
	printf("请输入主目录：");
	scanf("%s", fileAddr);
	printf("----------------------------------------------\n");

	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(srvPort);
	srvAddr.sin_addr.s_addr = inet_addr(srvIP);

	nRc = bind(srvSocket, (LPSOCKADDR)&srvAddr, sizeof(srvAddr));  //监听绑定
	if (nRc != SOCKET_ERROR)
		printf("Socket绑定成功!\n");
	else {
		printf("Socket绑定失败!\n");
		WSAGetLastError();
	}
	nRc = listen(srvSocket, 5);  //设置等待连接状态
	if (nRc != SOCKET_ERROR)
		printf("设置等待连接状态成功!\n");
	else {
		printf("设置等待连接状态失败!\n");
		WSAGetLastError();
	}

	sockaddr_in cltAddr;
	cltAddr.sin_family = AF_INET;
	int addrLen = sizeof(cltAddr);
	while (true) {
		SOCKET cltSocket = accept(srvSocket, (LPSOCKADDR)&cltAddr, &addrLen);
		if (cltSocket != INVALID_SOCKET)
			printf("Socket与客户连接成功!\n");
		else {
			printf("Socket与客户连接失败!\n");
			WSAGetLastError();
		}
		printf("----------------------------------------------\n");
		printf("客户端IP地址：%s\n", inet_ntoa(cltAddr.sin_addr));
		printf("客户端端口号：%u\n", htons(cltAddr.sin_port));

		char rcvdata[2000] = "";
		nRc = recv(cltSocket, rcvdata, 2000, 0); //接收数据
		if (nRc != SOCKET_ERROR)
			printf("接收数据成功!\n");
		else {
			printf("接收数据失败!\n");
			WSAGetLastError();
		}
		printf("从客户端收到%d字节数据:\n %s\n", nRc, rcvdata);

		char rqtname[20] = "";  //在请求行（即第一行）获取请求的文件名
		char extname[10] = "";  //请求文件的后缀
		for (int i = 0; i < nRc; i++) {
			if (rcvdata[i] == '/') {
				for (int j = 0; j < nRc - i; j++) {
					if (rcvdata[i] != ' ') {
						rqtname[j] = rcvdata[i];
						i++;
					}
					else {
						rqtname[j + 1] = '\0';
						break;
					}
				}
				break;
			}
		}
		for (int k = 0; k < nRc; k++) {
			if (rcvdata[k] == '.') {
				for (int j = 0; j < nRc - k; j++) {
					if (rcvdata[k + 1] != ' ') {
						extname[j] = rcvdata[k + 1];
						k++;
					}
					else {
						extname[j + 1] = '\0';
						break;
					}
				}
				break;
			}
		}
		printf("----------------------------------------------\n");

		strcpy(fileName, fileAddr);
		printf("请求文件名：%s\n", rqtname);
		strcat(fileName, rqtname);
		printf("完整路径：%s\n", fileName);
		sendHead(fileName, extname, cltSocket); //发送首部
		sendData(fileName, cltSocket); //发送实体
		printf("----------------------------------------------\n\n\n");

		closesocket(cltSocket);
	}

	closesocket(srvSocket);
	WSACleanup();
	return;
}

void sendHead(char* filename, char* extname, SOCKET s) {
	char content_Type[20] = ""; //设置content-type

	if (strcmp(extname, "html") == 0) strcpy(content_Type, "text/html");
	if (strcmp(extname, "gif") == 0) strcpy(content_Type, "image/gif");
	if (strcmp(extname, "jpg") == 0) strcpy(content_Type, "image/jpeg");
	if (strcmp(extname, "jpeg") == 0) strcpy(content_Type, "image/jpeg");
	if (strcmp(extname, "png") == 0) strcpy(content_Type, "image/png");

	char sendContent_Type[40] = "Content-Type: ";
	strcat(sendContent_Type, content_Type);
	strcat(sendContent_Type, "\r\n");

	const char* ok_find = "HTTP/1.1 200 OK\r\n"; //200 OK
	const char* not_find = "HTTP/1.1 404 NOT FOUND\r\n"; //404 Not Found
	const char* forbidden = "HTTP/1.1 403 FORBIDDEN\r\n"; //404 Forbidden

	if (strcmp(filename, "C:/index/private.png") == 0) {  //403 禁止访问
		if (send(s, forbidden, strlen(forbidden), 0) == SOCKET_ERROR) {
			printf("发送失败！\n");
			return;
		}
		if (send(s, sendContent_Type, strlen(sendContent_Type), 0) == SOCKET_ERROR) {
			printf("发送失败！\n");
			return;
		}
		printf("403 客户端请求禁止访问文件！\n");
		return;
	}

	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {                    //404 无法查找到文件
		if (send(s, not_find, strlen(not_find), 0) == SOCKET_ERROR) {
			printf("发送失败！\n");
			return;
		}
		printf("404 客户端请求文件查找失败！\n");
	}
	else {                                  //200 文件查找成功
		if (send(s, ok_find, strlen(ok_find), 0) == SOCKET_ERROR) {
			printf("发送失败！\n");
			return;
		}
		printf("200 客户端请求文件查找成功！\n");
	}
	if (content_Type) {
		if (send(s, sendContent_Type, strlen(sendContent_Type), 0) == SOCKET_ERROR) {
			printf("发送失败！\n");
			return;
		}
	}
	if (send(s, "\r\n", 2, 0) == SOCKET_ERROR) {
		printf("发送失败！\n");
		return;
	}
	return;
}



void sendData(char* filename, SOCKET s) {
	FILE* fp_Data = fopen(filename, "rb");
	if (fp_Data == NULL) return;
	fseek(fp_Data, 0L, SEEK_END);
	int dataLen = ftell(fp_Data);  //计算文件字节数
	char* p = (char*)malloc(dataLen + 1);
	fseek(fp_Data, 0L, SEEK_SET);
	fread(p, dataLen, 1, fp_Data);
	if (send(s, p, dataLen, 0) == SOCKET_ERROR) {
		printf("发送失败！\n");
	}
	return;
}