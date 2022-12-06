/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03.12.2022 09:35:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Кирилл Крашенинников,
 *   Organization:  АО ВНИИРА
 *
 * =====================================================================================
 */
#include "field.h"

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <regex>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;
using namespace std;

boost::system::error_code ec;

//Таблица с позициями
char field[4][4] =
{{' ', '1', '2', '3' },
 {'1', ' ', ' ', ' '},
 {'2', ' ', ' ', ' '},
 {'3', ' ', ' ', ' '}};


io_service service;
string* address;
int port = 8001;

bool TurnGame(Field *game, string coordinate, char simbol, bool& win, bool& overflow){

    //Разделение строки для преобразования
    static const regex rdelim{";"};
    vector<string> str{
        sregex_token_iterator(coordinate.begin(), coordinate.end(), rdelim, -1),
                sregex_token_iterator()
    };

    if(str.size() < 3){
        //Преобразование в числа значений
        int y = atoi(str.begin()->c_str());
        int x = atoi((str.end()-1)->c_str());

        //Проверка введеных позиций
        if((x < game->GetSize() && x > 0) && (y < game->GetSize() && y > 0) &&
                ((game->GetElement(x,y) != game->GetPlayerChar()) && (game->GetElement(x,y) != game->GetEnemyChar()))){

            //Установка символа в позицию
            game->SetSimbol(x, y, simbol);
            //Обновление таблицы
            game->RefrestField();

            //Проверка победы
            win = game->CheckWin(x, y, simbol);
            overflow = game->CheckOverflow();

            return true;
        }
        else{
            cout << "Позиции введены не правильно. Повторите ввод." << endl;
            return false;
        }
    }
    else{
        cout << "Позиции введены не правильно. Повторите ввод." << endl;
        return false;
    }
}

size_t read_complete(char * buff, const error_code & err, size_t bytes) {
    if (err){
        cerr << "Read failed: \n" << err.message()<< endl;
        return false;
    }
    bool found = std::find(buff, buff + bytes, '\n') < buff + bytes;
    // we read one-by-one until we get to enter, no buffering
    return found ? 0 : 1;
}

void handle_connections(Field *game) {
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(),port));
    char buff[1024];

    bool win = false;
    bool overflow = false;

    do {
        ip::tcp::socket sock(service);
        acceptor.accept(sock, ec);
        if (ec){
            cerr << "Accept failed: \n" << ec.message()<< endl;
        }

        //Обновление таблицы
        game->RefrestField();

        int bytes = read(sock, buffer(buff),
                         boost::bind(read_complete,buff,_1,_2));
        string msg(buff, bytes - 1);

        TurnGame(game, msg, game->GetEnemyChar(), win, overflow);

        //Обновление таблицы
        game->RefrestField();

        if(win ^ !overflow){
            string coordinate;
            do{
                cout << "Введите позицию для символа (пример: горизонталь;вертикаль ): " << endl;
                cin >> coordinate;
            }
            while(!TurnGame(game, coordinate, game->GetPlayerChar(), win, overflow));

            sock.write_some(buffer(coordinate+"\n"), ec);
            if (ec){
                cerr << "Write failed: \n" << ec.message()<< endl;
            }

            if(win){
                cout << "Ты победил" << endl;
            }else{
                if(overflow)
                    cout << "Ничья" << endl;
            }
        }
        else{
            if(win){
                cout << "Победил противник" << endl;
            }else{
                if(overflow)
                    cout << "Ничья" << endl;
            }
        }

        sock.close();
    }
    while(win ^ !overflow);
}

string sync_echo(ip::tcp::endpoint ep, std::string msg, Field *game, bool& win, bool& overflow) {
    msg += "\n";
    ip::tcp::socket sock(service);

    sock.connect(ep, ec);
    if (ec){
        cerr << "Connection failed: \n" << ec.message()<< endl;
    }

    sock.write_some(buffer(msg), ec);
    if (ec){
        cerr << "Write failed: \n" << ec.message()<< endl;
    }

    string copy;

    if(win ^ !overflow){
        char buff[1024];
        int bytes = read(sock, buffer(buff),
                         boost::bind(read_complete,buff,_1,_2));

        copy = string(buff, bytes - 1);
    }
    if(win){
        cout << "Ты победил" << endl;
    }else{
        if(overflow)
            cout << "Ничья" << endl;
    }

    sock.close();

    return copy;
}

int main(){
    Field *game = new Field (field);

    address = new string("192.168.0.105");

    cout << "Выберите режим игры (\x1b[4;1mС\x1b[0mоздать игру, \x1b[4;1mП\x1b[0mоиск игры): ";
    string choice;
    cin >> choice;

    bool win = false;
    bool overflow = false;

    if (choice == "С" || choice == "с"){
        handle_connections(game);
    }else{
        if (choice == "П" || choice == "п"){
            string newGame;
            ip::tcp::endpoint ep(ip::address::from_string(*address), port);

            do{
                game->RefrestField();

                string coordinate;

                do{
                    cout << "Введите позицию для символа (пример: горизонталь;вертикаль ): " << endl;
                    cin >> coordinate;
                }
                while(!TurnGame(game, coordinate, game->GetPlayerChar(), win, overflow));

                string result = sync_echo(ep, coordinate, game, win, overflow);

                if(win ^ !overflow){
                    TurnGame(game, result, game->GetEnemyChar(), win, overflow);
                    if(win ^ !overflow){
                        if(win){
                            cout << "Ты победил" << endl;
                        }else{
                            if(overflow)
                                cout << "Ничья" << endl;
                        }
                    }
                }
            }
            while(win ^ !overflow);
        }
    }
    return 0;
}
