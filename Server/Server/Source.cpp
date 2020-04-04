#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <thread>

#pragma warning(disable: 4996)



const int MaxNumClient = 7;
SOCKET Connections[MaxNumClient];
int Counter = 0;

void What_to_do(int i);
void Menu(int i);
//дл€ чата
void ClientHandler_Rev_Send(int index) {
	char msg[256];
	while (true) {
		recv(Connections[index], msg, sizeof(msg), NULL);
		for (int i = 0; i < Counter; i++) {
			if (i == index) {
				continue;
			}

			send(Connections[i], msg, sizeof(msg), NULL);
		}
	}
}
//дл€ получени€ всех сообщений от 1 клиента (нужен новый поток)
void ClientHandler_Rev(int index) {
	char msg[256];
	while (true) {
		recv(Connections[index], msg, sizeof(msg), NULL);
		std::cout << msg << "\n";
	}
}
//дл€ рассылки всем сообщений (нужен новый поток)
void ClientHandler_Send(const char msg[], const int size_ofmsg) {
	while (true) {
		for (int i = 0; i < Counter; i++) {
			send(Connections[i], msg, size_ofmsg, NULL);
		}
	}
}



//дл€ рассылки
void SMSTC_ALL(std::string MSG_S) {
	for (int i = 0; i < Counter; i++) {
		int msg_size = MSG_S.size();
		char *msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		strcpy(msg, MSG_S.c_str());

		send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[i], msg, msg_size, NULL);
		std::cout << msg << "   (отправлено всем)\n";
		delete[] msg;
	}
}
//дл€ получени€ 1 сообщени€ от 1 клиента
std::string RMSFC(int index) {
	int msg_size = -999;
	//эту фигню убирать нельз€, чтобы сервер не крашилс€ при отключении клиента
	while (msg_size < 1 || msg_size > 1000) {
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
	}
	char *msg = new char[msg_size + 1];
	msg[msg_size] = '\0';
	recv(Connections[index], msg, msg_size, NULL);
	std::cout << msg << "   (получено)\n";

	std::string MSG_S = msg;
	delete[] msg;
	msg_size = -999;
	return MSG_S;
}
//дл€ отправки 1 сообщени€ дл€ 1 клиента
void SMSTC(int index, std::string MSG_S) {
	int msg_size = MSG_S.size();
	char *msg = new char[msg_size + 1];
	msg[msg_size] = '\0';
	//memset(msg, 0, sizeof(msg));
	strcpy(msg, MSG_S.c_str());

	send(Connections[index], (char*)&msg_size, sizeof(int), NULL);
	send(Connections[index], msg, msg_size, NULL);
	std::cout << msg << "   (отправлено)\n";
	delete[] msg;
}

class MyServer
{
public:
	//MyServer();
	void Start_Server(const std::string address, const int port) {
		DLLVersion = MAKEWORD(2, 1);
		if (WSAStartup(DLLVersion, &wsaData) != 0) {
			std::cout << "Error" << "\n";
			exit(1);
		}

		int sizeofaddr = sizeof(addr);
		addr.sin_addr.s_addr = inet_addr(address.c_str());
		addr.sin_port = htons(port);
		addr.sin_family = AF_INET;								//дл€ инетернет протоколов така€ константа

		sListen = socket(AF_INET, SOCK_STREAM, NULL);	//1) семейство интернет протоколов 2)конкретный протокол соединени€
		bind(sListen, (SOCKADDR*)&addr, sizeof(addr));			//прив€зываем адрес сокету
		listen(sListen, SOMAXCONN);								//прослушивание порта в ожидании клиента  2)максимум пришедших запросов от клиента ожидающих обработки

		//максиму numClient клиентов
		for (int i = 0; i < MaxNumClient; ++i) {
			SOCKET newConnection;
			newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);		//2) содержит данные о клиенте. функци€ возвращает указатель на новый сокет дл€ общени€ с клиентом

			//подключилс€ ли клиент
			if (newConnection == 0) {
				std::cout << "Error #2\n";
			}
			else {
				Connections[i] = newConnection;
				Counter++;

				std::cout << "Client Connected! (from Server)\n";
				char m[256] = "Hi:)! (from Server)\n";
				SMSTC(Counter-1, m);

				//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)What_to_do, (LPVOID)(i), NULL, NULL);
				std::thread th1(What_to_do, i);
				th1.detach();
			}
		}
	}

private:
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	SOCKADDR_IN addr;
	SOCKET sListen;
};

void What_to_do(int i) {
	Menu(i);
}

void Menu(int i) {
	SMSTC(i, "ѕривет, путник!\n");
	SMSTC(i, "Menu!\n");
	SMSTC(i, "1) Play\n");
	SMSTC(i, "2) —татистика\n");
	SMSTC(i, "3) ¬ыйти\n");
	SMSTC_ALL("¬сем");

	std::string choose_s;
	int choose_i = -999;
	bool warning = false;
	while (choose_s.size() != 1 || choose_i < 1 || choose_i > 9) {
		if (warning) { SMSTC(i, "¬ведите корректно!\n"); }
		choose_s = RMSFC(i);//можно исп stoi но не нужно
		if (choose_s.size() == 1) {
			choose_i = atoi(choose_s.c_str());
		}
		warning = true;
	}

	std::cout << "¬вели: " << choose_i << "\n";
	switch (choose_i) {
	case 1: SMSTC(i, "¬вели!!!: 1\n"); break;
	case 2: SMSTC(i, "¬вели!!!: 2\n"); break;
	case 3: exit(1); break;
	}
}

int main() {
	setlocale(LC_ALL, "Russian");
	MyServer Ser;
	Ser.Start_Server("127.0.0.1", 1111);

	system("pause");
	return 0;
}