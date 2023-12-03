#include <gtkmm.h>
#include <iostream>

class chatWindow : public Gtk::Window {
public:
    chatWindow () {
        set_title("Simple Chat Messenger");
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
        get_room_number();

        // 메인 루프 시작
        show_all_children();
    }

    void get_user_name() {

        Gtk::MessageDialog dialog(*this, "Enter Your Name", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
        dialog.set_secondary_text("Please enter your name:");
        dialog.set_modal(true);

        Gtk::Entry username;
        entry.set_max_length(15);
        dialog.get_content_area()->pack_end(entry, true, true);
        entry.show();

        Gtk::Entry roomnumber;
        entry.set_max_length(15);
        dialog.get_content_area()->pack_end(entry, true, true);
        entry.show();

        int result = dialog.run();
        if (result == Gtk::RESPONSE_OK) {
            user_name = username.get_text();
            room_number = roomnuber.get_text();
        } else {
            hide();  // 취소 버튼을 누르면 창을 닫습니다.
        }
    }


    void on_send_button_clicked() {
        std::string message = message_entry.get_text();
        if (!message.empty()) {
            chat_view.get_buffer()->insert_at_cursor(user_name + ": " + message + "\n");
            message_entry.set_text("");
        }
    }

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
};


int main(int argc, char** argv) {
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

    chatWindow chatWindow;
    chatWindow .set_position(Gtk::WIN_POS_CENTER);
    return app->run(chatWindow );
}

//g++ main.cpp -o my_program `pkg-config --cflags --libs gtkmm-3.0`

