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

    bool checkSize = 0;
    size_t sizeMsg = str.size();

    while(!checkSize)
    {
        switch (sizeMsg)
        {
        case 2:
        {
            int y = atoi(str.begin()->c_str());     //Преобразование в числа значений
            int x = atoi((str.begin()+1)->c_str()); //Преобразование в числа значений

            if(((game->GetElement(x,y) != game->GetPlayerChar()) && (game->GetElement(x,y) != game->GetEnemyChar()))) //Проверка пустоты позиции
            {
                //Установка символа в позицию
                game->SetSimbol(x, y, simbol);

                //Обновление таблицы
                game->RefrestField();

                //Проверка победы
                win = game->CheckWin(x, y, simbol);

                //Проверка на наличие пустых мест
                overflow = game->CheckOverflow();
                checkSize = 1;

                return true;
            }
            else
            {
                cout << "Данная позиция занята, введите другую." << endl;
                return false;
            }
        }
        case 3:
        {
            game->SetEnemyName((str.begin()+2)->c_str());//Установка имени противника
            game->RefrestField(); //Обновление таблицы
            firstMsg = false;//Закрытие приема первого сообщения
            sizeMsg = 2;
            break;
        }

        case 4:
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
            sizeMsg = 2;
            break;
        }

        default:
        {
            cout << "Позиции введены не правильно. Повторите ввод." << endl;
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
        cerr << "Ошибка чтения сообщения: \n" << err.message()<< endl;
        return false;
    }

    bool found = find(buff, buff + bytes, '\n') < buff + bytes;//Проверка на наличие конечного символа
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
        cerr << "Ошибка создания соединения: \n" << ec.message()<< endl;
    }

    game->RefrestField(); //Обновление таблицы

    string coordinate;
    do
    {
        regex enterCoordinateReg("([1-3]+?)\;([1-3]+?)");//Шаблон для проверки ввода
        bool checkCoordinates = 0;
        do
        {
            cout << "Введите позицию для символа (пример: вертикаль;горизонталь): " << endl;
            cin >> coordinate; //Ввод координат для установки символа

            checkCoordinates = !regex_match(coordinate,enterCoordinateReg);//Проверка введенных координат
            if(checkCoordinates){
                cout << "Позиция символа была введена неправильно. Повторите попытку." << endl;
            }
        }
        while(checkCoordinates);
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
        cerr << "Ошибка отправки сообщения: \n" << ec.message()<< endl;
    }

    if(!(win || overflow)) //Условие победы игрока или ничьи
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
        cerr << "Ошибка соединения с противником: \n" << ec.message()<< endl;
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

    if(!(win || overflow))//Проверка победы или ничьи
    {
        string coordinate;

        do
        {
            regex enterCoordinateReg("([1-3]+?)\;([1-3]+?)");//Шаблон для проверки ввода
            bool checkCoordinates = 0;
            do
            {
                cout << "Введите позицию для символа (пример: вертикаль;горизонталь): " << endl;
                cin >> coordinate; //Ввод координат для установки символа

                checkCoordinates = !regex_match(coordinate,enterCoordinateReg);//Проверка введенных координат
                if(checkCoordinates){
                    cout << "Позиция символа была введена неправильно. Повторите попытку." << endl;
                }
            }
            while(checkCoordinates);
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
            cerr << "Ошибка отправки сообщения: \n" << ec.message()<< endl;
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
        regex ipv4("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\:([0-9]+?)$");

        if(regex_match(arg, ipv4))//Проверка правильности введения ip адреса с сокетом
        {
            size_t pos = arg.find(":");
            address = new string(arg.substr(0, pos));//Назначение ip адреса
            port = atoi((arg.substr(pos+1)).c_str());//Назначение порта
        }
        else
        {
            cout << "Ip адрес и порт введены не корректно." << endl;
            return 1;
        }
    }
    else
    {
        cout << "Не был введен ip адрес и порт игрока." << endl;
        return 1;
    }

    ip::tcp::endpoint* ep;
    ip::tcp::acceptor* acceptor;


    cout << "Введите имя: ";
    string name;
    cin >> name;//Ввод имени

    game->SetName(name);//Установка имени

    char choice(0);
    bool win = false;//Инициализация пременной победы
    bool overflow = false;//Инициализация пременной ничьи

    while(!choice)
    {
        cout << "Выберите режим игры (\x1b[4;1mC\x1b[0mreate, \x1b[4;1mS\x1b[0mearch): ";
        cin >> choice; //Ввод выбора подключения или создания хоста

        switch (choice)
        {
        case 'c': case 'C':
        {
            acceptor = new ip::tcp::acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), port));//Инициализация ассептора для хоста
            break;
        }
        case 's': case 'S':
        {
            ep = new ip::tcp::endpoint(ip::address::from_string(*address), port);//Инициализация точки подключения для клиента
            delete address;
            break;
        }
        default:
        {
            cout << "Такой команды нет. Повторите ввод." << endl;
            choice = 0;
        }
        }
    }

    bool startNewGame = true;
    do
    {
        do
        {
            if (choice == 'c' || choice == 'C')
            {
                HandleConnections(game, acceptor, win, overflow);//Операции со стороны хоста
            }
            else
            {
                if (choice == 's' || choice == 'S')
                {
                    SyncEcho(ep, game, win, overflow);//Операции со стороны клиента
                }
            }

        }
        while(!(win || overflow));//Условие цикла победы или ничьи



        char saveResult(0);

        while(!saveResult)
        {
            cout << "Хотите сохранить результат? (Y/N)" << endl;
            cin >> saveResult;

            switch (saveResult)
            {
            case 'y': case 'Y':
            {
                SaveResult(win);//Сохрание результата
                break;
            }
            case 'n': case 'N':
            {
                break;
            }
            default:
            {
                cout << "Такой команды нет. Повторите ввод." << endl;
                saveResult = 0;
            }
            }
        }

        char newGame(0) ;

        while(!newGame)
        {
            cout << "Начать новую игру? (Y/N)" << endl;
            cin >> newGame;

            switch (newGame)
            {
            case 'n': case 'N':
            {
                startNewGame = false;
                break;
            }

            case 'y': case 'Y':
            {
                game->Clear();//Очистка поля для новой игры
                win = false;//Очистка значения победы
                overflow = false;//Очистка значения ничьи
                startNewGame = true;
                break;
            }
            default:
            {
                cout << "Такой команды нет. Повторите ввод." << endl;
                newGame = 0;
            }
            }
        }
    }
    while(startNewGame);

    delete ep;
    delete acceptor;
    delete game;

    return 0;
}
