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
//#include "service.h"

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <regex>

using namespace std;

//Таблица с позициями
char field[4][4] =
{{' ', '1', '2', '3' },
 {'1', ' ', ' ', ' '},
 {'2', ' ', ' ', ' '},
 {'3', ' ', ' ', ' '}};


int main(){

    Field *game = new Field (field);

    string newGame;
    do{
        bool win = false;
        bool overflow = false;

        do{
            //Обновление таблицы
            game->RefrestField();

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
                int x = atoi((str.end()-1)->c_str());

                //Проверка введеных позиций
                if((x < game->GetSize() && x > 0) && (y < game->GetSize() && y > 0) &&
                        ((game->GetElement(x,y) != game->GetPlayerChar()) && (game->GetElement(x,y) != game->GetEnemyChar()))){

                    //Установка символа в позицию
                    game->SetSimbol(x, y);
                    //Обновление таблицы
                    game->RefrestField();

                    //Проверка победы
                    win = game->CheckWin(x, y);
                    overflow = game->CheckOverflow();
                    if(!win){
                        game->SwapGameChar();
                    }
                }
                else
                    cout << "Позиции введены не правильно. Повторите ввод." << endl;
            }
            else
                cout << "Позиции введены не правильно. Повторите ввод." << endl;
        }
        while(win ^ !overflow);

        if(win){
            cout << "Победил " << game->GetPlayerChar() << endl;
        }else{
            cout << "Ничья" << endl;
        }
        game->Clear(field);

        cout << "Начать заново или выход? " << endl;
        cin >> newGame;
    }
    while(newGame == "з");*/
    return 0;
}
