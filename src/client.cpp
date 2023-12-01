#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>

std::mutex outputMutex;

using namespace std;

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;


void handleServerData(int clientsocket){
    char buffer[BUFFER_SIZE];
    int bytesRead;

    while(true){
        bytesRead = recv(clientsocket,buffer, sizeof(buffer),0);
        if(bytesRead <= 0){
            cerr << "server disconnection\n";
            close(clientsocket);
            break;
        }
        else{
            buffer[bytesRead] = '\0';
            cout <<buffer << endl;
        }
    }

}

void handleClientData(int clientsocket){
    string message;
    while(true){
        cout << "\nEnter your message:";
        getline(cin,message);
        send(clientsocket,message.c_str(), message.length(),0);
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error creating socket\n";
        return -1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        cerr << "Error connecting to server\n";
        close(clientSocket);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        cerr << "Error receiving welcome message\n";
        close(clientSocket);
        return -1;
    }

    buffer[bytesRead] = '\0';
    cout << "Server: " << buffer;

    cout << "Enter your username and chat room (e.g., username room_name):\n";
    string input;
    getline(cin, input);

    // 클라이언트에서 서버로 사용자 이름과 채팅방 정보 전송
    send(clientSocket, input.c_str(), input.length(), 0);

    cout << "Welcome to the chat room! You can start chatting.\n";

    thread serverDataThread(handleServerData, clientSocket);
    thread clientDataThread(handleClientData, clientSocket); 
    // 메시지 수신 및 출력 루프

    serverDataThread.join();
    clientDataThread.join();
   
    close(clientSocket);

    return 0;
}
