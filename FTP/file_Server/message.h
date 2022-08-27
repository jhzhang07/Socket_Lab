#pragma once
#include <string>
using namespace std;

const int BUF_SIZE = 1024;

class clientMessage { //来自客户端的报文类，可以和char[]互相转换
	//命令：ls（文件列表）、get（下载）
	//可能补充：send
	//类型：0.无效命令 1.ls命令 2.get命令
	//标准报文有长度为1的前缀
private:
	int id = 0;
	char content[100] = { 0 }; //get命令：文件名
public:
	clientMessage() {};
	clientMessage(char message[BUF_SIZE]) {
		char forward;
		int i;
		forward = message[0];
		switch (forward) {
		case'l': //ls
			id = 1;
			break;
		case'g': //get
			id = 2;
			i = 1;
			while ((message[i] != '\0') && (i < 99)) {
				content[i - 1] = message[i];
				i++;
			}
			content[i - 1] = '\0';
			break;
		default:
			id = 0;
		};
	}
	void toString(char result[BUF_SIZE]) {
		int i;
		switch (id) {
		case 0:
			result[0] = '\0';
			break;
		case 1:
			result[0] = 'l';
			result[1] = '\0';
			break;
		case 2:
			result[0] = 'g';
			i = 0;
			while ((content[i] != '\0') && (i < 100)) {
				result[i + 1] = content[i];
				i++;
			}
			result[i + 1] = '\0';
			break;
		default:
			result[0] = '\0';
		};
	}
	int getID() {
		return id;
	}
	void getContent(char content[100]) {
		int i;
		for (i = 0; i < strlen(this->content); i++) {
			content[i] = this->content[i];
		}
		content[i] = '\0';
	}
	void changeID(int id) {
		this->id = id;
	}
	void changeContent(const char* content) {
		int i;
		for (i = 0; i < strlen(content); i++) {
			this->content[i] = content[i];
		}
		this->content[i] = '\0';
	}
};

class serverMessage { //来自服务器端的报文类，可以和char[]互相转换
	//类型：0.命令无效 1.文件列表 2.存在文件，准备发送 3.不存在文件 
private:
	int id = 0;
	char content[701] = { 0 }; //文件列表
public:
	serverMessage() {}
	serverMessage(char message[BUF_SIZE]) {
		char forward;
		int i;
		forward = message[0];
		switch (forward) {
		case'l': //list文件列表
			id = 1;
			i = 1;
			while ((message[i] != '\0') && (i < 700)) {
				content[i - 1] = message[i];
				i++;
			}
			content[i - 1] = '\0';
			break;
		case'e': //exist 文件存在
			id = 2;
			i = 1;
			while ((message[i] != '\0') && (i < 700)) {
				content[i - 1] = message[i];
				i++;
			}
			content[i - 1] = '\0';
			break;
		case'n': //non_exist 文件不存在
			id = 3;
			break;
		default:
			id = 0;
		};
	}
	void toString(char result[BUF_SIZE]) {
		int i;
		switch (id) {
		case 0:
			result[0] = '\0';
			break;
		case 1:
			result[0] = 'l';
			i = 0;
			while ((content[i] != '\0') && (i < 700)) {
				result[i + 1] = content[i];
				i++;
			}
			result[i + 1] = '\0';
			break;
		case 2:
			result[0] = 'e';
			i = 0;
			while ((content[i] != '\0') && (i < 700)) {
				result[i + 1] = content[i];
				i++;
			}
			result[i + 1] = '\0';
			break;
		case 3:
			result[0] = 'n';
			result[1] = '\0';
			break;
		default:
			result[0] = '\0';
		};
	}
	int getID() {
		return id;
	}
	void getContent(char content[701]) {
		int i;
		for (i = 0; i < strlen(this->content); i++) {
			content[i] = this->content[i];
		}
		content[i] = '\0';
	}
	void changeID(int id) {
		this->id = id;
	}
	void changeContent(const char* content) {
		int i;
		for (i = 0; i < strlen(content); i++) {
			this->content[i] = content[i];
		}
		this->content[i] = '\0';
	}
};
