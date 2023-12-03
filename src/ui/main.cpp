// UI
#include <gtkmm.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

class ChatWindow : public Gtk::Window {
public:
    ChatWindow() {
        set_title("C live chat");
        set_default_size(400, 300);

        // UI 요소 초기화
        chat_view.set_editable(false);
        message_entry.set_max_length(100);

        // 레이아웃 구성
        add(main_box);
        main_box.pack_start(chat_view, Gtk::PACK_EXPAND_WIDGET);
        main_box.pack_start(message_entry, Gtk::PACK_SHRINK);
        main_box.pack_start(send_button, Gtk::PACK_SHRINK);

        // 버튼 클릭 시 이벤트 핸들러 등록
        send_button.signal_clicked().connect(sigc::mem_fun(*this, &ChatWindow::on_send_button_clicked));

        // 윈도우가 닫힐 때의 이벤트 핸들러 등록
        signal_hide().connect(sigc::mem_fun(*this, &ChatWindow::close_window));

        // 서버 연결 및 채팅방 접속
        server_connect();
        room_access();

        receive_thread = std::thread(&ChatWindow::handleClientData, this);

        // 메인 루프 시작
        show_all_children();
    }

    void server_connect() {
        // 서버와 연결
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            std::cerr << "Error creating socket\n";
            return;
        }
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
            std::cerr << "Error connecting to server\n";
            ::close(clientSocket);
            return;
        }
        std::cout << clientSocket << "\n";
    }

    void handleClientData() {
        char buffer[BUFFER_SIZE];
        int bytesRead;

        while (true) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

            if (bytesRead == -1) {
                // 오류 처리
                std::cerr << "Error receiving data\n";
                break;
            } else if (bytesRead == 0) {
                // 연결이 끊겼음을 나타냄
                std::cerr << "Connection closed by the server\n";
                break;
            }

            buffer[bytesRead] = '\0';

            if (bytesRead > 0) {
                Glib::signal_idle().connect_once([this, message = std::string(buffer)] {
                    chat_view.get_buffer()->insert_at_cursor(message + "\n");
                    message_entry.set_text("");
                });
            }
        }
    }


    void room_access() {

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
            room_number  = entry_name.get_text();
            user_name = entry_room.get_text();
        } else {
            hide();
        }

        // Example: You need to replace this with your own data, size, and flags
        std::string data= user_name + " "+ room_number;
        send(clientSocket,data.c_str(),data.length(),0);
    }
    

    void on_send_button_clicked() {
    std::string message = message_entry.get_text();
    if (!message.empty()) {
        ssize_t sentBytes = send(clientSocket, message.c_str(), message.length() + 1, 0);

        if (sentBytes == -1) {
            // 에러 처리
            std::cerr << "Error sending data\n";
        }

        message_entry.set_text("");
    }
}


    void close_window() {
        std::cout << "Chat window closed.\n";
        // 소켓 닫기
        if (clientSocket != -1) {
            ::close(clientSocket);
            clientSocket = -1; // 소켓을 닫은 후에는 -1로 초기화
        }
    }

private:
    Gtk::VBox main_box;
    Gtk::TextView chat_view;
    Gtk::Entry user_entry;
    Gtk::Entry message_entry;
    Gtk::Button send_button{"Send"};

    std::thread receive_thread;
    std::string user_name;
    std::string room_number;
    int clientSocket;
};

int main(int argc, char** argv) {
   

    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.example", Gio::APPLICATION_NON_UNIQUE);

    ChatWindow chat_window1;

    chat_window1.set_position(Gtk::WIN_POS_CENTER);

    app->run(chat_window1);

    return 0;
}
