#include "service.h"

#include <iostream>

#include <boost/bind.hpp>
#include <boost/bind/bind.hpp>

#include <boost/thread/thread.hpp>

io_service io;

using namespace boost::placeholders;

Service::Service(int _port)
    :enemyAddress(""), port(_port),acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), _port)), endpoint(ip::address::from_string("127.0.0.1"), port)
{

}

Service::Service(string _address, int _port)
    :enemyAddress(_address), port(_port), acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), _port)), endpoint(ip::address::from_string(enemyAddress), port)
{

}

size_t Service::ReadComplete(char *buff, const error_code &err, size_t bytes)
{
    if (err){
        cerr << "Read failed: \n" << err.message()<< endl;
		return false;
	}
	bool found = find(buff, buff + bytes, '\n') < buff + bytes;
	return found ? 0 : 1;
}

bool Service::SendConnection()
{
    ip::tcp::socket socket(io);

    socket.connect(endpoint, ec);
	if(ec){
        cerr << "Connection failed: \n" << ec.message()<< endl;
		return false;
	}

    socket.write_some(buffer("Connection"), ec);
    if(ec){
        cerr << "Send connection message failed: \n" << ec.message() << endl;
        socket.close();
        return false;
    }

    socket.close();
	return true;
}

bool Service::SendSymbol(char msg)
{
    ip::tcp::socket socket(io);

	socket.connect(endpoint, ec);
    if(ec){
        cerr << "Connection failed: \n" << ec.message()<< endl;
		return false;
    }

    string sendMsg(1, msg);
    sendMsg += "\n";

   cout << sendMsg;

    socket.write_some(buffer(sendMsg), ec);
	if(ec){
        cerr << "Send connection message failed: \n" << ec.message() << endl;
		socket.close();
		return false;
	}
	socket.close();
	return true;
}

bool Service::SendMessage(string msg)
{
    msg += "\n";

	ip::tcp::socket socket(io);
	socket.connect(endpoint, ec);
	if(ec){
        cerr << "Connection failed: \n" << ec.message()<< endl;
		return false;
	}

	socket.write_some(buffer(msg), ec);
	if(ec){
        cerr << "Send connection message failed: \n" << ec.message() << endl;
		socket.close();
		return false;
	}
	socket.close();
	return true;
}

bool Service::HandleConnection()
{
    ip::tcp::socket socket(io);
    acceptor.accept(socket, ec);
	if(ec){
        cerr << "Accept socket failed: \n" << ec.message() << endl;
		return false;
	}

    char buff[1024];
    char bytes = read(socket, buffer(buff), boost::bind(ReadComplete, buff, _1, _2));
    if(bytes == 0){
        cerr << "Read failed" << endl;
        socket.close();
        return 'E';
    }
    string copy(buff, bytes-1);
    cout << copy << endl;

    socket.close();
	return true;
}

char Service::HandleSymbol()
{
	ip::tcp::socket socket(io);
    acceptor.accept(socket, ec);
    if(ec){
        cerr << "Accept socket failed: \n" << ec.message() << endl;
        return 'E';
    }
    char buff[1024];
    char bytes = read(socket, buffer(buff), boost::bind(ReadComplete, buff, _1, _2));
    if(bytes == 0){
        cerr << "Read failed" << endl;
        socket.close();
        return 'E';
    }
    string copy(buff, bytes-1);
    socket.close();

    return copy[0];
}

string Service::HandleMessage()
{
	ip::tcp::socket socket(io);
    acceptor.accept(socket, ec);
	if(ec){
        cerr << "Accept socket failed: \n" << ec.message() << endl;
		return "error";
	}

	char buff[1024];
    char bytes = read(socket, buffer(buff), boost::bind(ReadComplete, buff, _1, _2));
	if(bytes == 0){
        cerr << "Read failed" << endl;
		socket.close();
		return "error";
	}
	string copy(buff, bytes-1);
	socket.close();

	return copy;
}


/*Отправка сообщения на сервер
bool syncEcho(string address, MessageType type, string msg){
	msg += "\n";
	ip::tcp::endpoint epoint(ip::address::from_string(address), 8001);
    ip::tcp::socketet socket(service);

	//Создание точки соединения
    socket.connect(epoint);

	if(type == Message){
		//Отправка сообщения для принятия координат
        socket.write_some(buffer("Message\n"));
		//Отправка координат
        socket.write_some(buffer(msg));

		//Считывание ответа от сервера
		char buff[1024];
        char bytes = read(socket, buffer(buff), boost::bind(readComplete, buff, _1, _2));

		string coordinate(buff, bytes-1);
		if(coordinate != "win"){
			//Разделение строки для преобразования
			static const regex rdelim{";"};
			vector<string> str{
				sregex_token_iterator(coordinate.begin(), coordinate.end(), rdelim, -1),
						sregex_token_iterator()
			};

			//Преобразование в числа значений
			int y = atoi(str.begin()->c_str());
			int x = atoi((str.begin()+1)->c_str());

			char enemyChar = (str.end()->c_str())[0];

			//Установка символа в позицию
			setSimbol(x, y, enemyChar);
			//Обновление таблицы
			refrestField(enemyChar);
			//Проверка победы
			return checkWin(x, y, enemyChar);
		}
        socket.close();
		return false;
	}
	else{

		if(type == Symbol){
			//Отправка сообщения для передачи символа игры
            socket.write_some(buffer("Symbol\n"));

			//Передача символа игры
            socket.write_some(buffer(msg));
            socket.close();
			return true;
		}
		else{
			if(type == Connection){
				//Отправка сообщения для соединения'
                socket.write_some(buffer("Connection\n"));

				//Считывание ответа от сервера
				char buff[1024];
                char bytes = read(socket, buffer(buff), boost::bind(readComplete, buff, _1, _2));
				string copy(buff, bytes-1);

				cout << copy << endl;
                socket.close();
				return true;
			}
		}
	}
	return false;
}

int main(){
	//Выбор режима приложения хост или клиент
	cout << "Выберите режим игры (\x1b[4;1mС\x1b[0mоздать игру, \x1b[4;1mП\x1b[0mоиск игры): ";
	string choice;
	cin >> choice;

	char PlayChar;

	string address;
	if (choice == "С" || choice == "с"){
		ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));

		char buff[1024];

		while (true) {
			cout << "Ожидание противника..." << endl;
            ip::tcp::socketet socket(service);
            acceptor.accept(socket);

			//Считывание типа cообщения с сокета
            int bytes = read(socket, buffer(buff),
							 boost::bind(readComplete, buff, _1, _2));

			string copy(buff, bytes-1);

			//Обработка сообщения с введенными координатами
			if(copy == "Message"){
                bytes = read(socket, buffer(buff),
							 boost::bind(readComplete, buff, _1, _2));

				string coordinate(buff, bytes-1);

				//Разделение строки для преобразования
				static const regex rdelim{";"};
				vector<string> str{
					sregex_token_iterator(coordinate.begin(), coordinate.end(), rdelim, -1),
							sregex_token_iterator()
				};

				//Преобразование в числа значений
				int y = atoi(str.begin()->c_str());
				int x = atoi((str.begin()+1)->c_str());

				char enemyChar = (str.end()->c_str())[0];

				//Установка символа в позицию
				setSimbol(x, y, enemyChar);
				//Обновление таблицы
				refrestField(enemyChar);
				//Проверка победы
				bool win = checkWin(x, y, enemyChar);

				if(!win){
					string coordinate;
					cout << "Введите позицию для символа (пример: горизонталь;вертикаль ): " << endl;
					cin >> coordinate;

					//Разделение строки для преобразования
					static const regex rdelim{";"};
					vector<string> str{
						sregex_token_iterator(coordinate.begin(), coordinate.end(), rdelim, -1),
								sregex_token_iterator()
					};

					if(str.size() < 3){
						//Преобразование в числа значений
						int y = atoi(str.begin()->c_str());
						int x = atoi(str.end()->c_str());

						//Проверка введеных позиций
						if((x < sizeField && x > 0) && (y < sizeField && y > 0) && field[x][y] == ' '){
							//Установка символа в позицию
							setSimbol(x, y, PlayChar);
							//Обновление таблицы
							refrestField(PlayChar);
							//Отправка поставленной позиции сопернику
                            socket.write_some(buffer(coordinate+";"+PlayChar));
						}
					}
				}
				else{
                    socket.write_some(buffer("win\n"));
				}
			}
			else{
				//Обработка сигнала соединения
				if(copy == "Connection"){
					cout << "Connection success" << endl;
                    socket.write_some(buffer("Connection success\n"));
				}
				else{
					//Обработка сигнала передачи символа
					if(copy == "Symbol"){
                        bytes = read(socket, buffer(buff),
									 boost::bind(readComplete, buff, _1, _2));

						PlayChar = buff[1];
						refrestField(PlayChar);
					}
					else{
						if(copy == "Win"){
                            bytes = read(socket, buffer(buff),
										 boost::bind(readComplete, buff, _1, _2));

							string strEnd(buff, bytes-1);
							cout << "Ты " << strEnd << endl;
						}
					}
				}
			}
            socket.close();
		}
	}
	else{
		if (choice == "П" || choice == "п") {
			cout << "Введите ip адрес: ";
			address = "192.168.0.105";
			//Отправка сообщения для соединения
			syncEcho(address, Connection, "Connection");
			//Установка игрового символа
			PlayChar = randomChar(address);
			refrestField(PlayChar);
			bool win = false;
			bool enemyWin = false;

			do{
				string coordinate;
				cout << "Введите позицию для символа (пример: горизонталь;вертикаль ): " << endl;
				cin >> coordinate;

				//Разделение строки для преобразования
				static const regex rdelim{";"};
				vector<string> str{
					sregex_token_iterator(coordinate.begin(), coordinate.end(), rdelim, -1),
							sregex_token_iterator()
				};

				if(str.size() < 3){
					//Преобразование в числа значений
					int y = atoi(str.begin()->c_str());
					int x = atoi(str.end()->c_str());

					//Проверка введеных позиций
					if((x < sizeField && x > 0) && (y < sizeField && y > 0) && field[x][y] == ' '){
						//Установка символа в позицию
						setSimbol(x, y, PlayChar);
						//Обновление таблицы
						refrestField(PlayChar);
						//Проверка победы
						win = checkWin(x, y, PlayChar);
						//Отправка поставленной позиции сопернику
						enemyWin = syncEcho(address, Message, coordinate+";"+PlayChar);
					}
					else
						cout << "Позиции введены не правильно. Повторите ввод." << endl;
				}
				else
					cout << "Позиции введены не правильно. Повторите ввод." << endl;
			}
			while(!win | !enemyWin);

			if(win){
				cout << "Ты выиграл" << endl;

				syncEcho(address, Win, "проиграл\n");
			}
			else{
				cout << "Ты проиграл" << endl;

				syncEcho(address, Win, "выиграл\n");
			}
		}
	}
	return 0;
}*/
