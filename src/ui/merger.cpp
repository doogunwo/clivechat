#include <gtkmm.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>

using namespace std;

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

// 클라이언트 동작을 처리하는 클래스
class ChatClient {
public:
    ChatClient() {
        // 소켓 생성 및 서버 연결
        // 서버와 연결이 실패하면 에러 메시지 출력 후 종료
        // 성공하면 서버의 환영 메시지 출력
        // 사용자로부터 이름과 채팅방 정보를 입력받아 서버로 전송
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            cerr << "Error creating socket\n";
            return;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
            cerr << "Error connecting to server\n";
            close(clientSocket);
            return;
        }

        char buffer[BUFFER_SIZE];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            cerr << "Error receiving welcome message\n";
            close(clientSocket);
            return;
        }

        buffer[bytesRead] = '\0';
        cout << "Server: " << buffer;

        cout << "Enter your username and chat room (e.g., username room_name):\n";
        string input;
        getline(cin, input);

        // 클라이언트에서 서버로 사용자 이름과 채팅방 정보 전송
        send(clientSocket, input.c_str(), input.length(), 0);

        cout << "Welcome to the chat room! You can start chatting.\n";

        // 서버 데이터 처리 및 클라이언트 데이터 입력을 위한 스레드 시작
        serverDataThread = thread(&ChatClient::handleServerData, this);
        clientDataThread = thread(&ChatClient::handleClientData, this);
    }

    // 소멸자에서 스레드 종료 및 소켓 닫기
    ~ChatClient() {
        serverDataThread.join();
        clientDataThread.join();
        close(clientSocket);
    }

private:
    int clientSocket;
    thread serverDataThread;
    thread clientDataThread;

    // 서버에서 데이터를 받아 화면에 출력하는 함수
    void handleServerData() {
        char buffer[BUFFER_SIZE];
        int bytesRead;

        while (true) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                cerr << "Server disconnection\n";
                close(clientSocket);
                break;
            } else {
                buffer[bytesRead] = '\0';
                cout << buffer << endl;
            }
        }
    }

    // 사용자로부터 데이터를 입력받아 서버로 전송하는 함수
    void handleClientData() {
        string message;
        while (true) {
            cout << "\nEnter your message:";
            getline(cin, message);
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }
};

// GUI 창을 관리하는 클래스
class ChatWindow : public Gtk::Window {
public:
    ChatWindow() {
        set_title("C live chat");
        set_default_size(400, 300);

        // UI 요소 초기화
        chat_view.set_editable(false);
        user_entry.set_max_length(15);
        message_entry.set_max_length(100);

        // 레이아웃 구성
        add(main_box);
        main_box.pack_start(chat_view, Gtk::PACK_EXPAND_WIDGET);
        main_box.pack_start(user_entry, Gtk::PACK_SHRINK);
        main_box.pack_start(message_entry, Gtk::PACK_SHRINK);
        main_box.pack_start(send_button, Gtk::PACK_SHRINK);

        // 버튼 클릭 시 이벤트 핸들러 등록
        send_button.signal_clicked().connect(sigc::mem_fun(*this, &ChatWindow::on_send_button_clicked));

        // 윈도우가 닫힐 때의 이벤트 핸들러 등록
        signal_hide().connect(sigc::mem_fun(*this, &ChatWindow::on_hide));

        // 사용자 이름 입력 다이얼로그 표시
        get_user_name();

        // 메인 루프 시작
        show_all_children();
    }

    // 사용자로부터 이름과 채팅방 정보를 입력받는 함수
    void get_user_name() {
        Gtk::MessageDialog dialog(*this, "Enter Your Information", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
        dialog.set_secondary_text("Please enter your name and room number:");

        Gtk::Label label_name("Name:");
        Gtk::Entry entry_name;
        entry_name.set_max_length(15);
        entry_name.set_text(user_name);
        entry_name.show();
        dialog.get_content_area()->pack_start(label_name, Gtk::PACK_SHRINK);
        dialog.get_content_area()->pack_end(entry_name, Gtk::PACK_SHRINK);

        Gtk::Label label_room("Room Number:");
        Gtk::Entry entry_room;
        entry_room.set_max_length(10);
        entry_room.set_text(room_number);
        entry_room.show();
        dialog.get_content_area()->pack_start(label_room, Gtk::PACK_SHRINK);
        dialog.get_content_area()->pack_end(entry_room, Gtk::PACK_SHRINK);

        int result = dialog.run();
        if (result == Gtk::RESPONSE_OK) {
            user_name = entry_name.get_text();
            room_number = entry_room.get_text();
            // ChatClient 클래스의 인스턴스를 생성하여 클라이언트 동작 시작
            chatClient = std::make_unique<ChatClient>();
        } else {
            hide();  // 취소 버튼을 누르면 창을 닫습니다.
        }
    }

    // 메시지 전송 버튼 클릭 이벤트 핸들러
    void on_send_button_clicked() {
        std::string message = message_entry.get_text();
        if (!message.empty()) {
            chat_view.get_buffer()->insert_at_cursor(user_name + ": " + message + "\n");
            message_entry.set_text("");
        }
    }

    // 윈도우가 닫힐 때의 이벤트 핸들러
    void on_hide() {
        std::cout << "Chat window closed.\n";
    }

private:
    Gtk::VBox main_box;
    Gtk::TextView chat_view;
    Gtk::Entry user_entry;
    Gtk::Entry message_entry;
    Gtk::Button send_button{"Send"};

    std::string user_name;
    std::string room_number;

    // ChatClient 클래스의 포인터
    std::unique_ptr<ChatClient> chatClient;
};

int main(int argc, char** argv) {
    // Gtkmm 애플리케이션 초기화
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

    // ChatWindow 클래스의 인스턴스 생성
    ChatWindow chat_window;

    // 윈도우를 화면 가운데에 표시
    chat_window.set_position(Gtk::WIN_POS_CENTER);

    // Gtkmm 애플리케이션 실행 및 메인 루프 진입
    return app->run(chat_window);
}

