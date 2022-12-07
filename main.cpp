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

bool firstMsg = true;//Переменная первого сообщения

string winnerName; //Имя выигрывшего игрока



//Функция осуществляющая за внесение изменений в таблицу, ее перерисовку и проверку победы
bool TurnGame(Field *game, string coordinate, char simbol, bool& win, bool& overflow)
{
    static const regex rdelim{";"};
    vector<string> str{                                                             //
        sregex_token_iterator(coordinate.begin(), coordinate.end(), rdelim, -1),    //Разделение строки для преобразования в отдельные координаты
                sregex_token_iterator()                                             //
    };                                                                              //

    if(str.size() == 2)  //Условие на выполнение операции чисто с координатами
    {
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
            cout << "Позиции введены не правильно. Повторите ввод. 1" << endl;
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
                game->SwapSymbols(); //Смена символов игроков между собой
                simbol = game->GetEnemyChar(); //Изменение символа для данной функции
            }

            game->SetEnemyName((str.begin()+3)->c_str());//Установка имени противника
            game->RefrestField(); //Обновление таблицы

            goto SetCoordinate; //Переход к установке символа
        }
        else
        {
            if(str.size() == 3 && firstMsg)//Условие для синхронизации символов игры и имен игроков
            {
                game->SetEnemyName((str.begin()+2)->c_str());//Установка имени противника
                game->RefrestField(); //Обновление таблицы
                firstMsg = false;//Закрытие приема первого сообщения

                goto SetCoordinate; //Переход к установке символа
            }
            else
            {
                cout << "Позиции введены не правильно. Повторите ввод. 2" << endl;
                return false;
            }
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
void HandleConnections(Field *game, ip::tcp::acceptor* acceptor, bool& win, bool& overflow)
{

    ip::tcp::socket sock(service);


    game->RefrestField(); //Обновление таблицы
    cout << "Ожидание противника ..." << endl;

    acceptor->accept(sock, ec);//Подключение сокета к ассептору
    if (ec)
    {
        cerr << "Accept failed: \n" << ec.message()<< endl;
    }


    string coordinate;
    do
    {
        cout << "Введите позицию для символа (пример: горизонталь;вертикаль): " << endl;
        cin >> coordinate; //Ввод координат для установки символа
    }
    while(!TurnGame(game, coordinate, game->GetPlayerChar(), win, overflow)); //Проверка введенных координат и установка символа

    //Проверка на отправку первого сообщения
    if(firstMsg)
    {
        sock.write_some(buffer(coordinate+";"+string(1,game->GetEnemyChar())+";"+game->GetName()+"\n"), ec);//Отправка координат, символа противника и имени
    }
    else
    {
        sock.write_some(buffer(coordinate+"\n"), ec); //Отправка координат
    }
    if (ec) //Проверка на коректность отправки сообщения
    {
        cerr << "Write failed: \n" << ec.message()<< endl;
    }

    if(win ^ !overflow) //Условие победы игрока или ничьи
    {
        cout << "Ожидание противника ..." << endl;
        char buff[1024];
        //Чтение сообщения от клиента
        int bytes = read(sock, buffer(buff),
                         boost::bind(ReadComplete,buff,_1,_2));
        string msg(buff, bytes - 1); //Убираем конечный символ

        TurnGame(game, msg, game->GetEnemyChar(), win, overflow); //Установка полученного символа

        game->RefrestField();//Обновление таблицы

        if(win)
        {
            cout << "Победил противник" << endl;
            winnerName = game->GetEnemyName(); //Запись противника в победители
        }
        else
        {
            if(overflow)
            {
                cout << "Ничья" << endl;
                winnerName = game->GetName() + "\40" + game->GetEnemyName(); //Запись обоих игроков как ничья
            }
        }
    }
    else
    {
        if(win) //Проверка победы игрока
        {
            cout << "Ты победил" << endl;
            winnerName = game->GetName();//Запись игрока в победители

        }
        else
        {
            if(overflow) //Проверка ничьи
            {
                cout << "Ничья" << endl;
                winnerName = game->GetName()+"\40" + game->GetEnemyName();//Запись обоих игроков как ничья
            }
        }
    }

    sock.close();//Закрытие сокета
}




//Функция отправли сообщения со стороны клиента
void SyncEcho(ip::tcp::endpoint* ep, Field *game, bool& win, bool& overflow)
{
    ip::tcp::socket sock(service);//Создание сокета

    sock.connect(*ep, ec);//Подключение сокета к ip адресу
    if (ec)
    {
        cerr << "Connection failed: \n" << ec.message()<< endl;
    }

    cout << "Ожидание противника ..." << endl;

    //Чтение ответа с сервера
    char buff[1024];
    int bytes = read(sock, buffer(buff),
                     boost::bind(ReadComplete,buff,_1,_2));

    game->RefrestField();//Обновление таблицы
    string copy(buff, bytes - 1);

    TurnGame(game, copy, game->GetEnemyChar(), win, overflow); //Установка полученного символа и проверка победы
    game->RefrestField();//Обновление таблицы

    if(win ^ !overflow)//Проверка победы или ничьи
    {
        string coordinate;

        do
        {
            cout << "Введите позицию для символа (пример: горизонталь;вертикаль ): " << endl;
            cin >> coordinate;//Ввод координат для установки символа
        }
        while(!TurnGame(game, coordinate, game->GetPlayerChar(), win, overflow));//Проверка введенных координат и установка символа


        if(firstMsg)//Проверка на отправку первого сообщения
        {
            sock.write_some(buffer(coordinate+";"+game->GetName()+"\n"), ec);//Отправка координат и имени
            firstMsg = false;//Закрытие приема первого сообщения
        }
        else
        {
            sock.write_some(buffer(coordinate+"\n"), ec);//Отправка координат
        }

        if (ec)
        {
            cerr << "Write failed: \n" << ec.message()<< endl;
        }

        if(win)//Проверка победы игрока
        {
            cout << "Ты победил" << endl;
            winnerName = game->GetName();//Запись игрока в победители
        }
        else
        {
            if(overflow)//Проверка ничьи
            {
                cout << "Ничья" << endl;
                winnerName = game->GetName() + "\40" + game->GetEnemyName();//Запись обоих игроков как ничья
            }
        }
    }
    else
    {
        if(win)//Проверка победы противника
        {
            cout << "Победил противник" << endl;
            winnerName = game->GetEnemyName();//Запись противника в победители
        }
        else
        {
            if(overflow)//Проверка ничьи
            {
                cout << "Ничья" << endl;
                winnerName = game->GetName() + "\40" + game->GetEnemyName();//Запись обоих игроков как ничья
            }
        }
    }

    sock.close();//Закрытие сокета
}





int main(int argc, char* argv[])
{
    Field *game = new Field ();//Создание поля для игры

    if(argc == 2)//Проверка количества аргументов у программы
    {
        string arg(argv[1]);//Получение первого аргумента

        //Разделение строки для преобразования
        static const regex rdelim{":"};
        vector<string> str{
            sregex_token_iterator(arg.begin(), arg.end(), rdelim, -1),
                    sregex_token_iterator()
        };

        if(str.size() == 2)//Проверка правильности введения ip адреса с сокетом
        {
            address = new string(str.begin()->c_str());//Назначение ip адреса
            port = atoi((str.begin()+1)->c_str());     //Назначение порта
        }
        else
        {
            cout << "Ошибка аргументов" << endl;
            return 1;
        }
    }
    else
    {
        cout << "Ошибка аргументов" << endl;
        return 1;
    }

    ip::tcp::endpoint* ep;
    ip::tcp::acceptor* acceptor;


    cout << "Введите имя: ";
    string name;
    cin >> name;//Ввод имени

    game->SetName(name);//Установка имени

    string choice;
    bool win = false;//Инициализация пременной победы
    bool overflow = false;//Инициализация пременной ничьи

EnterMode:
    cout << "Выберите режим игры (\x1b[4;1mС\x1b[0mоздать игру, \x1b[4;1mП\x1b[0mоиск игры): ";
    cin >> choice; //Ввод выбора подключения или создания хоста

    if (choice == "С" || choice == "с")
    {
        acceptor = new ip::tcp::acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), port));//Инициализация ассептора для хоста
    }
    else
    {
        if (choice == "П" || choice == "п")
        {
            ep = new ip::tcp::endpoint(ip::address::from_string(*address), port);//Инициализация точки подключения для клиента
        }
        else
        {
            cout << "Такой команды нет. Повторите ввод." << endl;
            goto EnterMode;
        }
    }


start:


    do
    {
        if (choice == "С" || choice == "с")
        {
            HandleConnections(game, acceptor, win, overflow);//Операции со стороны хоста
        }
        else
        {
            if (choice == "П" || choice == "п")
            {
                SyncEcho(ep, game, win, overflow);//Операции со стороны клиента
            }
        }

    }
    while(win ^ !overflow);//Условие цикла победы или ничьи

    string newGame;


EnterWrite:
    cout << "Хотите сохранить результат? \x1b[4;1mД\x1b[0mа/\x1b[4;1mН\x1b[0mет" << endl;
    cin >> newGame;

    if(newGame == "д" || newGame == "Д")
    {
        SaveResult(win);//Сохрание результата
    }
    else
    {
        if((newGame != "Н") ^ (newGame == "н"))
        {
            cout << "Такой команды нет. Повторите ввод." << endl;
            goto EnterWrite;//Переход к повторному вводу
        }
    }


EnterExit:
    cout << "\x1b[4;1mВ\x1b[0mыйти из игры или \x1b[4;1mн\x1b[0mачать заново?" << endl;
    cin >> newGame;

    if(newGame == "н" || newGame == "Н")
    {
        game->Clear();//Очистка поля для новой игры
        win = false;//Очистка значения победы
        overflow = false;//Очистка значения ничьи
        goto start;//Переход к циклу игры
    }
    else
    {
        if((newGame != "В") ^ (newGame == "в"))
        {
            cout << "Такой команды нет. Повторите ввод." << endl;
            goto EnterExit;//Переход к повторному вводу
        }
    }

    return 0;
}
