#ifndef FIELD_H
#define FIELD_H
#include <cstdint>
#include <string>

class Field
{
public:
    Field();//Базовый конструктор
    Field(char arr[4][4]);//Конструктор установки по шаблону
    Field(Field& newField);//Конструктор копирования




	//Расчет и проверка выигрышных комбинаций
    bool CheckWin(int x, int y, char simbol);

	//Очистка таблицы
    void Clear();

	//Обновление вида таблицы
	void RefrestField();

    //Случайный выбор символов для игры
	void RandomChar();

	//Проверка ничьи
	bool CheckOverflow();

    //Функция смены символов между игроками
    inline void SwapSymbols(){ char copy = playerChar; playerChar = enemyChar; enemyChar = copy; }




    inline void SetSimbol(uint8_t x, uint8_t y, char ch) { field[x][y] = ch; }//Установка символа в позицию
    inline char GetElement(uint8_t x, uint8_t y) { return field[x][y]; }//Получение значения ячейки

    inline char GetPlayerChar(){ return playerChar; }//Получение символа данного игрока
    inline char GetEnemyChar() { return enemyChar; }//Получение символа противника

    inline uint8_t GetSize() { return sizeField; }//Получение размера поля

    inline void SetName(std::string newName) { name = newName;  }//Установка имени данного игрока
    inline std::string GetName() { return name;  }//Получение имени данного игрока

    inline void SetEnemyName(std::string newName) { enemyName = newName;  }//Установка имени противника
    inline std::string GetEnemyName() { return enemyName;  }//Получение имени противника

    void SetField(char clearArray[4][4]); //Установка таблицы с шаблона двумерного массива символов
    void SetField(std::string strField);//Установка таблицы с шаблона строки

    //Получение поля в виде строки
    std::string GetStrField();


private:
    const static uint8_t sizeField = 4;//Размер таблицы
    char** field;//Поле игры

    //Выигрышные комбинации
    int winCombination[8][3][2] = {
		{{1, 1}, {1, 2}, {1,3}},
		{{2, 1}, {2, 2}, {2,3}},
		{{3, 1}, {3, 2}, {3,3}},
		{{1, 1}, {2, 1}, {3,1}},
		{{1, 2}, {2, 2}, {3,2}},
		{{1, 3}, {2, 3}, {3,3}},
		{{1, 1}, {2, 2}, {3,3}},
		{{3, 1}, {2, 2}, {1,3}}
	};

    char playerChar;//Символ данного игрока
    char enemyChar;//Символ противника

    std::string name;//Имя данного игрока
    std::string enemyName;//Имя противника
};

#endif // FIELD_H
