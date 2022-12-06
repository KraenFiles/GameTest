#ifndef FIELD_H
#define FIELD_H
#include <cstdint>
#include <string>

class Field
{
public:
	Field();
	Field(char arr[4][4]);


	//Расчет и проверка выигрышных комбинаций
    bool CheckWin(int x, int y, char simbol);

	//Очистка таблицы
    void Clear();

	//Обновление вида таблицы
	void RefrestField();

	//Рандомизация символов для игры
	void RandomChar();

	//Проверка ничьи
	bool CheckOverflow();

    inline void SwapSymbols(){ char copy = playerChar; playerChar = enemyChar; enemyChar = copy; }



    //Установка символа в позицию
    inline void SetSimbol(uint8_t x, uint8_t y, char ch) { field[x][y] = ch; }

    void SetField(char clearArray[4][4]);
    void SetField(std::string strField);

    inline void SetName(std::string newName) { name = newName;  }
    inline void SetEnemyName(std::string newName) { enemyName = newName;  }



    inline char GetPlayerChar(){ return playerChar; }
    inline char GetEnemyChar() { return enemyChar; }

	inline char GetElement(uint8_t x, uint8_t y) { return field[x][y]; }
	inline uint8_t GetSize() { return sizeField; }

    inline std::string GetName() { return name;  }
    inline std::string GetEnemyName() { return enemyName;  }

    std::string GetStrField();


private:
	const static uint8_t sizeField = 4;
	char** field;

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

	char playerChar;
	char enemyChar;

    std::string name;
    std::string enemyName;
};

#endif // FIELD_H
