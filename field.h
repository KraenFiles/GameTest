#ifndef FIELD_H
#define FIELD_H
#include <cstdint>


class Field
{
public:
	Field();
	Field(char arr[4][4]);

	//Установка символа в позицию
	inline void SetSimbol(uint8_t x, uint8_t y) { field[x][y] = playerChar; }

	//Расчет и проверка выигрышных комбинаций
    bool CheckWin(int x, int y);

	//Очистка таблицы
	void Clear(char clearArray[4][4]);

	//Обновление вида таблицы
	void RefrestField();

	//Рандомизация символов для игры
	void RandomChar();

	//Проверка ничьи
	bool CheckOverflow();

	inline void SwapGameChar(){ char save = enemyChar; enemyChar = playerChar; playerChar = save; }

	inline char GetPlayerChar(){ return playerChar; }
	inline char GetEnemyChar() { return enemyChar; }

	inline char GetElement(uint8_t x, uint8_t y) { return field[x][y]; }
	inline uint8_t GetSize() { return sizeField; }

private:
	const static uint8_t sizeField = 4;
	char** field;

    int winCombination[8][3][2] =
	{
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
};

#endif // FIELD_H
