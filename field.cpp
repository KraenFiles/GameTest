#include "field.h"

#include <iostream>
#include <random>

using namespace std;

//Стандартный конструктор
Field::Field()
{
    char clearArray[sizeField][sizeField] = {{' ', '1', '2', '3' },
                                             {'1', ' ', ' ', ' '},
                                             {'2', ' ', ' ', ' '},
                                             {'3', ' ', ' ', ' '}};

    field = new char*[sizeField];
    for(int i = 0; i < sizeField; i++)
    {
        field[i] = new char[sizeField];
        for(int j = 0; j < sizeField; j++)
        {
            field[i][j] = clearArray[i][j];
        }
    }

    RandomChar();
}

//Конструктор по шаблону
Field::Field(char arr[4][4])
{
    field = new char*[sizeField];
    for(int i = 0; i < sizeField; i++)
    {
        field[i] = new char[sizeField];
        for(int j = 0; j < sizeField; j++)
        {
            field[i][j] = arr[i][j];
        }
    }

    RandomChar();
}

//Конструктор копирования
Field::Field(Field &newField)
{
    this->field = newField.field;
    this->playerChar = newField.playerChar;
    this->enemyChar = newField.enemyChar;
    this->name = newField.name;
    this->enemyName = newField.enemyName;
}


Field::~Field()
{
    delete [] field;
}

//Расчет и проверка выигрышных комбинаций
bool Field::CheckWin(int x, int y, char simbol)
{
    int* numCombination;
    int sizeArray = 0;
    bool win = false;

    //Расчет выиграшных комбинаций
    if(x == 1)
    {
        if(y == 1)
        {
            numCombination = new int[3]{0,3,6};
            sizeArray = 3;
        }
        else
        {
            if(y == 2)
            {
                numCombination = new int[2]{0,4};
                sizeArray = 2;
            }
            else
            {
                numCombination = new int[3]{0,5,7};
                sizeArray = 3;
            }
        }
    }
    else
    {
        if(x == 2)
        {
            if(y == 1)
            {
                numCombination = new int[2]{1,3};
                sizeArray = 2;
            }
            else
            {
                if(y == 2)
                {
                    numCombination = new int[4]{1,4,6,7};
                    sizeArray = 4;
                }
                else
                {
                    numCombination = new int[2]{1,5};
                    sizeArray = 2;
                }
            }
        }
        else
        {
            if(y == 1)
            {
                numCombination = new int[3]{2,3,7};
                sizeArray = 3;
            }
            else
            {
                if(y == 2)
                {
                    numCombination = new int[2]{2,4};
                    sizeArray = 2;
                }
                else
                {
                    numCombination = new int[3]{2,5,6};
                    sizeArray = 3;
                }
            }
        }
    }

    //Проверка комбинаций на победу
    for(int i = 0; i < sizeArray; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            if(field[winCombination[numCombination[i]][j][0]][winCombination[numCombination[i]][j][1]] == simbol)
            {
                win = true;
            }
            else
            {
                win = false;
                break;
            }
        }
        if(win) break;
    }

    return win;
}

//Очистка таблицы
void Field::Clear()
{
    char clearArray[sizeField][sizeField] = {{' ', '1', '2', '3' },
                                             {'1', ' ', ' ', ' '},
                                             {'2', ' ', ' ', ' '},
                                             {'3', ' ', ' ', ' '}};

    for(int i = 0; i < sizeField; i++)
    {
        for(int j = 0; j < sizeField; j++)
        {
            field[i][j] = clearArray[i][j];
        }
    }
}

//Установка таблицы по шаблону
void Field::SetField(char clearArray[4][4])
{
    for(int i = 0; i < sizeField; i++)
    {
        for(int j = 0; j < sizeField; j++)
        {
            field[i][j] = clearArray[i][j];
        }
    }
}

//Установка таблицы из строки
void Field::SetField(std::string strField)
{
    int stringIndex = 0;

    for(int i = 0; i < sizeField; i++)
    {
        for(int j = 0; j < sizeField; j++)
        {
            field[i][j] = strField[stringIndex+j];
        }
        stringIndex += 4;
    }
}

//Получение таблицы в виде строки
std::string Field::GetStrField()
{
    std::string result = "";
    for(int i = 0; i < sizeField; i++)
    {
        result += *(field+i);
    }
    return result;
}

//Обновление вида таблицы
void Field::RefrestField()
{
    system("clear");
    cout << "Ты играешь за " << playerChar << endl;
    for(uint8_t i = 0; i < sizeField; i++ )
    {
        for(uint8_t j = 0; j < sizeField; j++ )
        {
            if (j != sizeField - 1)
                cout << field[i][j] << " | ";
            else
                cout << field[i][j] << endl;
        }
        if(i != sizeField - 1)
            cout << "--------------" << endl;
    }
    cout << endl;
}

//Проверка ничьи
bool Field::CheckOverflow()
{
    for(int i = 1; i < sizeField; i++)
    {
        for(int j = 1; j < sizeField; j++)
        {
            if(field[i][j] == ' ')
            {
                return false;
            }
        }
    }

    return true;
}

//Рандомизация символов для игры
void Field::RandomChar()
{
    srand(time(NULL));
    if(rand() % 2 == 0)
    {
        playerChar = 'O';
        enemyChar = 'X';
    }
    else
    {
        playerChar = 'X';
        enemyChar = 'O';
    }
}
