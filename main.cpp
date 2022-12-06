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
#include <fstream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;
using namespace std;

boost::system::error_code ec;

io_service service;

string* address; //ip адрес для подключения
int port = 0; //Порт для подключения

bool firstMsg = true;

string winnerName; //Имя выигрывшего игрока



//Функция осуществляющая за внесение изменений в таблицу, ее перерисовку и проверку победы
bool TurnGame(Field *game, string coordinate, char simbol, bool& win, bool& overflow)
{
    static const regex rdelim{";"};
    vector<string> str{                                                             //
        sregex_token_iterator(coordinate.begin(), coordinate.end(), rdelim, -1),    //Разделение строки для преобразования в отдельные координаты
                sregex_token_iterator()                                             //
    };                                                                              //

    if(str.size() < 3)  //Условие на выполнение операции чисто с координатами
    {
        //Преобразование в числа значений
SetCoordinate:
        int y = atoi(str.begin()->c_str());     //Преобразование в числа значений
        int x = atoi((str.begin()+1)->c_str()); //Преобразование в числа значений


        if((x < game->GetSize() && x > 0) && (y < game->GetSize() && y > 0) && //Проверка введеных позиций
                ((game->GetElement(x,y) != game->GetPlayerChar()) && (game->GetElement(x,y) != game->GetEnemyChar()))) //Проверка пустоты позиции
        {
            //Установка символа в позицию
            game->SetSimbol(x, y, simbol);

            //Обновление таблицы
            game->RefrestField();

            //Проверка победы
            win = game->CheckWin(x, y, simbol);

            //Проверка на наличие пустых мест
            overflow = game->CheckOverflow();

            return true;
        }
        else
        {
            cout << "Позиции введены не правильно. Повторите ввод." << endl;
            return false;
        }
    }
    else
    {
        if(str.size() == 4 && firstMsg)//Условие для синхронизации символов игры и имен игроков
        {
            //Синхронизация символов
            char newSimbol = (str.begin()+2)->c_str()[0];
            if(newSimbol != game->GetPlayerChar())//Сравнение совпадения символов
            {
                game->SwapSymbols(); //Смена символов
                simbol = game->GetEnemyChar(); //Изменение символа для данной функции
            }

            game->SetEnemyName((str.begin()+3)->c_str());//Установка имени противника

            game->RefrestField(); //Обновление таблицы

            goto SetCoordinate; //Переход к установке символа
        }
        else
        {
            if(str.size() == 3 && firstMsg) //Условие для установки имен игроков
            {
                game->SetEnemyName((str.begin()+2)->c_str());//Установка имени врага
                firstMsg = false;

                goto SetCoordinate;//Переход к установке символа
            }
            cout << "Позиции введены не правильно. Повторите ввод." << endl;
            return false;
        }
    }
}



//Функция для проверки окончания сообщения
size_t ReadComplete(char * buff, const error_code & err, size_t bytes)
{
    //Вывод ошибки для чтения
    if (err)
    {
        cerr << "Read failed: \n" << err.message()<< endl;
        return false;
    }

    bool found = std::find(buff, buff + bytes, '\n') < buff + bytes;//Проверка на наличие конечного символа
    return found ? 0 : 1;
}



//Функция записи результата в файл
void SaveResult(bool win)
{
    ofstream file;
    time_t now = time(0); //Получение локального времени

    file.open("results.txt", ios::app); //Открытие файла для записи результата в конец файла
    if(win)
        file << "Победил " << winnerName << " - " << ctime(&now) << endl; //Запись победившего игрока
    else
        file << "Ничья между " << winnerName << " - " << ctime(&now) << endl; //Запись ничьи

    file.close();
}



//Функция обработки данных хостом
void HandleConnections(Field *game)
{
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(),port));
    char buff[1024];

    bool win = false; //Переменная победы
    bool overflow = false; //Переменная ничьи

    do
    {
start:
        ip::tcp::socket sock(service);

        //Обновление таблицы
        game->RefrestField();
        cout << "Ожидание противника ..." << endl;

        acceptor.accept(sock, ec);
        if (ec)
        {
            cerr << "Accept failed: \n" << ec.message()<< endl;
        }

        //Чтение сообщения от клиента
        int bytes = read(sock, buffer(buff),
                         boost::bind(ReadComplete,buff,_1,_2));
        string msg(buff, bytes - 1); //Убираем конечный символ

        TurnGame(game, msg, game->GetEnemyChar(), win, overflow); //Установка полученного символа

        game->RefrestField();//Обновление таблицы

        if(win ^ !overflow)
        {
            string coordinate;
            do
            {
                cout << "Введите позицию для символа (пример: горизонталь;вертикаль ): " << endl;
                cin >> coordinate;
            }
            while(!TurnGame(game, coordinate, game->GetPlayerChar(), win, overflow));

            if(firstMsg)
            {
                sock.write_some(buffer(coordinate+";"+game->GetName()+"\n"), ec);
                firstMsg = false;
            }
            else
            {
                sock.write_some(buffer(coordinate+"\n"), ec);
            }
            if (ec)
            {
                cerr << "Write failed: \n" << ec.message()<< endl;
            }

            if(win)
            {
                cout << "Ты победил" << endl;
                winnerName = game->GetName();

            }
            else
            {
                if(overflow)
                {
                    cout << "Ничья" << endl;
                    winnerName = game->GetName()+"\40" + game->GetEnemyName();
                }
            }
        }
        else
        {
            if(win)
            {
                cout << "Победил противник" << endl;
                winnerName = game->GetEnemyName();
            }
            else
            {
                if(overflow)
                {
                    cout << "Ничья" << endl;
                    winnerName = game->GetName()+"\40" + game->GetEnemyName();
                }
            }
        }
        sock.close();
    }
    while(win ^ !overflow);

    string newGame;
EnterWrite:
    cout << "Хотите сохранить результат? \x1b[4;1mД\x1b[0mа/\x1b[4;1mН\x1b[0mет" << endl;
    cin >> newGame;

    if(newGame == "д" || newGame == "Д")
    {
        SaveResult(win);
    }
    else
    {
        if((newGame != "Н") ^ (newGame == "н"))
        {
            cout << "Такой команды нет. Повторите ввод." << endl;
            goto EnterWrite;
        }
    }

EnterExit:
    cout << "\x1b[4;1mВ\x1b[0mыйти из игры или \x1b[4;1mн\x1b[0mачать заново?" << endl;
    cin >> newGame;

    if(newGame == "н" || newGame == "Н")
    {
        game->Clear();
        win = false;
        overflow = false;
        goto start;
    }
    else
    {
        if((newGame != "В") ^ (newGame == "в"))
        {
            cout << "Такой команды нет. Повторите ввод." << endl;
            goto EnterExit;
        }
    }
}





string SyncEcho(ip::tcp::endpoint ep, std::string msg, Field *game, bool& win, bool& overflow)
{
    msg += "\n";
    ip::tcp::socket sock(service);

    sock.connect(ep, ec);
    if (ec)
    {
        cerr << "Connection failed: \n" << ec.message()<< endl;
    }

    sock.write_some(buffer(msg), ec);
    if (ec)
    {
        cerr << "Write failed: \n" << ec.message()<< endl;
    }

    string copy = "";

    if(win ^ !overflow)
    {
        cout << "Ожидание противника ..." << endl;
        char buff[1024];
        int bytes = read(sock, buffer(buff),
                         boost::bind(ReadComplete,buff,_1,_2));

        copy = string(buff, bytes - 1);

    }
    else
    {
        if(win)
        {
            cout << "Ты победил" << endl;
            winnerName = game->GetName();
        }
        else
        {
            if(overflow)
            {
                cout << "Ничья" << endl;
                winnerName = game->GetName() + "\40" + game->GetEnemyName();
            }
        }
    }

    sock.close();

    return copy;
}





int main(int argc, char* argv[])
{
    Field *game = new Field ();
    string tr = game->GetStrField();
    cout << tr << endl;
    game->SetSimbol(2, 2, 'X');
    tr = game->GetStrField();
    game->SetField(tr);
    cout << game->GetStrField() << endl;

    string arg(argv[1]);

    //Разделение строки для преобразования
    static const regex rdelim{":"};
    vector<string> str{
        sregex_token_iterator(arg.begin(), arg.end(), rdelim, -1),
                sregex_token_iterator()
    };

    if(str.size() == 2)
    {
    address = new string(str.begin()->c_str());
    port = atoi((str.begin()+1)->c_str());
    }
    else
    {
        cout << "Ошибка аргументов" << endl;
    }

    cout << "Введите имя: ";
    string name;
    cin >> name;

    game->SetName(name);

    string choice;
    bool win = false;
    bool overflow = false;

EnterMode:
    cout << "Выберите режим игры (\x1b[4;1mС\x1b[0mоздать игру, \x1b[4;1mП\x1b[0mоиск игры): ";
    cin >> choice;

    if (choice == "С" || choice == "с")
    {
        HandleConnections(game);
    }
    else
    {
        if (choice == "П" || choice == "п")
        {

            ip::tcp::endpoint ep(ip::address::from_string(*address), port);

            do
            {
start:
                game->RefrestField();

                string coordinate;

                do
                {
                    cout << "Введите позицию для символа (пример: горизонталь;вертикаль ): " << endl;
                    cin >> coordinate;
                }
                while(!TurnGame(game, coordinate, game->GetPlayerChar(), win, overflow));

                string result;
                if(firstMsg)
                    result = SyncEcho(ep, coordinate+";"+string(1,game->GetEnemyChar())+";"+name, game, win, overflow);
                else
                    result = SyncEcho(ep, coordinate, game, win, overflow);

                if(win ^ !overflow)
                {
                    TurnGame(game, result, game->GetEnemyChar(), win, overflow);

                    if(win)
                    {
                        cout << "Противник победил" << endl;
                        winnerName = game->GetName();
                    }
                    else
                    {
                        if(overflow)
                        {
                            cout << "Ничья" << endl;
                            winnerName = game->GetName() + "\40" + game->GetEnemyName();
                        }
                    }
                }

            }
            while(win ^ !overflow);

            string newGame;
EnterWrite:
            cout << "Хотите сохранить результат? \x1b[4;1mД\x1b[0mа/\x1b[4;1mН\x1b[0mет" << endl;
            cin >> newGame;

            if(newGame == "д" || newGame == "Д")
            {
                SaveResult(win);
            }
            else
            {
                if((newGame != "Н") ^ (newGame == "н"))
                {
                    cout << "Такой команды нет. Повторите ввод." << endl;
                    goto EnterWrite;
                }
            }

EnterExit:
            cout << "\x1b[4;1mВ\x1b[0mыйти из игры или \x1b[4;1mн\x1b[0mачать заново?" << endl;
            cin >> newGame;

            if(newGame == "н" || newGame == "Н")
            {
                game->Clear();
                win = false;
                overflow = false;
                goto start;
            }
            else
            {
                if((newGame != "В") ^ (newGame == "в"))
                {
                    cout << "Такой команды нет. Повторите ввод." << endl;
                    goto EnterExit;
                }
            }
        }
        else
        {
            cout << "Такой команды нет. Повторите ввод." << endl;
            goto EnterMode;
        }
    }
    return 0;
}
