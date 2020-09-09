// Морской бой

#include <SFML/Graphics.hpp>
using namespace sf;
#include <iostream>
#include <Windows.h>
#include <sstream>
#include <random>
using namespace std;

const int LANGUAGE = 1251, WIN_W = 1280, WIN_H = 720, width = 40; // Сторона квадрата в пикселях
const int myFleet_start[2] = { 290, 150 }, rivalFleet_start[2] = { 780, 150 };
const int unit[2] = { 0, 100 };
const int deuceVert[2] = { 70, 65 }, deuceHor[2] = { 0, 200 };
const int tripleVert[2] = { 130, 163 }, tripleHor[2] = { 0, 300 };
const int fourVert[2] = { 195, 290 }, fourHor[2] = { 0, 400 };
const int main_text_pos[2] = { 580, 20 }, wrong_pos[2] = { 20, 560 };
const int transparent_red[4] = { 194, 107, 107, 255 }, grey[4] = { 194, 192, 192, 255 };
const int text_size = 20;

Font font; // Шрифт
RectangleShape selectedShip; // Выделение выбранного квадрата
vector<RectangleShape> myShips(10); // Мои корабли
vector<RectangleShape> mySquares(150); // Клетки поля пользователя
vector<RectangleShape> rivalSquares(150); // Клетки поля противника
vector<int> my_hit_index; // Индексы подбитых ячеек пользователем
vector<int> rival_hit_index; // Индексы подбитых ячеек компьютером

string winner;
int next_part[2]; // Для заполнения вокруг разбитого корабля
int my_printed_count = 0; // Индекс для прорисованных кораблей пользователя
int my_hit_squares = 0, rival_hit_squares = 0; // Количество закрашенных клеток
bool game_is_over = false;
bool is_user_move = true; // true - Ход пользователя, false - ход компьютера
bool is_ships_placed = false; // Корабли расставлены?
bool user_is_set = false; // Пользователь расставил свои корабли?
bool ship_is_selected = false; // Корабль выбран?
int my_dead_ships = 0, rival_dead_ships = 0;
int a, b; // Индексы ячейки
int num_of_ships = 10; // Количество оставшихся кораблей для расстановки
int ships_count[4] = { 4, 3, 2, 1 }; // Количество оставшихся для расстановки кораблей
int selected_type = 0; // Тип выбранного корабля
int myFleet[10][10], rivalFleet[10][10]; // Массивы для хранения информации о расположенных кораблях
/* 0 - Клетка не содержит корабля и не обстрелена
   1 - Клетка содержит корабль и не обстрелена
   2 - Клетка не содержит корабль и обстрелена
   3 - Клетка содержит корабль и обстрелена
*/

// Для случайных чисел
mt19937 engine; 
random_device device;

void computer_arrangement(); // Расстановка кораблей компьютера
bool insertion_is_possible(int fleet[10][10]); // Возможна ли вставка в эту ячейку
void insertion(int fleet[10][10]); // Вставка корабля 
void ship_selection(Vector2i); // Выбор корабля
void user_move(Vector2i); // Ход пользователя
void computer_move(); // Ход компьютера
bool ship_is_dead(int fleet[10][10]); // Корабль потоплен? 
void filling_around_dead(int fleet[10][10], bool); // Закрашивание клеток вокруг убитого корабля
void my_print_square(int, int); // Окрашивание клетки пользователя
void rival_print_square(int, int); // Окрашивание клетки противника

int main()
{
    SetConsoleCP(LANGUAGE);
    SetConsoleOutputCP(LANGUAGE);

    RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "SeaBattle");
    Texture Background;
    Background.loadFromFile("BG.png");
    Sprite sprite;
    Vector2u size = Background.getSize();
    sprite.setTexture(Background);

    font.loadFromFile("CyrilicOld.ttf");

    Text Arrangement("РАССТАВЬТЕ СВОИ КОРАБЛИ", font, text_size);
    Arrangement.setFillColor(Color::Black);
    Arrangement.setOutlineColor(Color::Black);
    Arrangement.setPosition(main_text_pos[0], main_text_pos[1]);
    
    Text Gameplay("", font, text_size);
    Gameplay.setFillColor(Color::Black);
    Gameplay.setOutlineColor(Color::Black);
    Gameplay.setPosition(main_text_pos[0], main_text_pos[1]);

    Text Wrong_Arrangement("НЕДОПУСТИМАЯ РАССТАНОВКА КОРАБЛЕЙ:", font, text_size);
    Wrong_Arrangement.setFillColor(Color::Black);
    Wrong_Arrangement.setOutlineColor(Color::Black);
    Wrong_Arrangement.setPosition(wrong_pos[0], wrong_pos[1]);

    Text Game_Over("", font, text_size);
    Game_Over.setFillColor(Color::Black);
    Game_Over.setOutlineColor(Color::Black);
    Game_Over.setPosition(main_text_pos[0], main_text_pos[1]);

    while (window.isOpen()) {
        Vector2i position = Mouse::getPosition(window); // Координаты курсора
        Gameplay.setString((is_user_move) ? "ВАШ ХОД" : "ХОД КОМПЬЮТЕРА");
        Game_Over.setString(winner.c_str());
        Event event;
        // Обработчик событий
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            switch (game_is_over) {
            case true: {
                break;
            }
            case false: {
                switch (is_ships_placed) {
                case true: {
                    switch (is_user_move) {
                    case true: { // Ход пользователя
                        if (event.type == Event::MouseButtonPressed)
                            if (event.key.code == Mouse::Left) {
                                if ((position.x >= rivalFleet_start[0]) && (position.y >= rivalFleet_start[1]) && (position.x <= rivalFleet_start[0] + width * 10) && (position.y <= rivalFleet_start[1] + width * 10))
                                    user_move(position);
                            }
                        break;
                    }
                    case false: { // Ход компьютера
                        computer_move();
                        break;
                    }
                    }
                    break;
                }
                case false: {
                    if (!user_is_set) {
                        if (event.type == Event::MouseButtonPressed) {
                            if (event.key.code == Mouse::Left) {
                                if (!ship_is_selected) { // Выбор корабля
                                    ship_selection(position);
                                }
                                else { // Вставка корабля в поле
                                    if ((position.x >= myFleet_start[0]) && (position.y >= myFleet_start[1]) && (position.x <= myFleet_start[0] + width * 10) && (position.y <= myFleet_start[1] + width * 10)) {
                                        b = (position.x - myFleet_start[0]) / width + 1; // Столбец выбранной ячейки
                                        a = (position.y - myFleet_start[1]) / width + 1; // Строка выбранной ячейки
                                        if ((myFleet[a - 1][b - 1] == 0) && (insertion_is_possible(myFleet))) {
                                            insertion(myFleet);
                                            num_of_ships--;
                                            ship_is_selected = false;
                                            if (selected_type == 1)
                                                ships_count[0]--;
                                            else
                                                ships_count[selected_type / 10 - 1]--;
                                            if (num_of_ships == 0) {
                                                user_is_set = true;
                                                for (int i = 0; i < 10; i++) {
                                                    for (int j = 0; j < 10; j++)
                                                        cout << myFleet[i][j] << "\t";
                                                    cout << endl;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else {
                        num_of_ships = 10;
                        ships_count[0] = 4;
                        ships_count[1] = 3;
                        ships_count[2] = 2;
                        ships_count[3] = 1;
                        computer_arrangement();
                        is_ships_placed = true;
                    }
                    break;
                }
                }
                break;
            }
            }
        }
        window.clear();
        window.draw(sprite);
        window.draw(Wrong_Arrangement);
        if (game_is_over)
            window.draw(Game_Over);
        else {
            if (!user_is_set)
                window.draw(Arrangement);
            else
                window.draw(Gameplay);
        }
        if (ship_is_selected)
            window.draw(selectedShip);
        for (size_t i = 0; i < 10; i++)
            window.draw(myShips[i]);
        for (size_t i = 0; i < rival_hit_squares; i++)
            window.draw(rivalSquares[i]);
        for (size_t i = 0; i < my_hit_squares; i++)
            window.draw(mySquares[i]);
        window.display();
    }
}

void computer_move() {
    engine.seed(device());
    uniform_int_distribution<unsigned> distribution(1, 100);
    int temp = distribution(engine);
    a = (temp == 100) ? 10 : temp / 10 + 1;
    b = (temp % 10 == 0) ? 1 : temp % 10;
    cout << a << "  " << b << endl;
    if (myFleet[a - 1][b - 1] == 0) {
        myFleet[a - 1][b - 1] = 2;
        mySquares[my_hit_squares].setSize(Vector2f(width, width));
        mySquares[my_hit_squares].setPosition(myFleet_start[0] + width * (b - 1), myFleet_start[1] + width * (a - 1));
        mySquares[my_hit_squares++].setFillColor(Color(grey[0], grey[1], grey[2], grey[3]));
        is_user_move = true;
    }
    else if (myFleet[a - 1][b - 1] == 1) {
        myFleet[a - 1][b - 1] = 3;
        if (ship_is_dead(myFleet)) {
            my_dead_ships++;
            rival_hit_index.clear();
            next_part[0] = a;
            next_part[1] = b;
            filling_around_dead(myFleet, true);
            if (my_dead_ships == 10) {
                game_is_over = true;
                winner = "КОМПЬЮТЕР ПОБЕДИЛ!";
            }
        }
        else {
            mySquares[my_hit_squares].setSize(Vector2f(width, width));
            mySquares[my_hit_squares].setPosition(myFleet_start[0] + width * (b - 1), myFleet_start[1] + width * (a - 1));
            mySquares[my_hit_squares++].setFillColor(Color(transparent_red[0], transparent_red[1], transparent_red[2], transparent_red[3]));
        }
    }
}

bool ship_is_dead(int fleet[10][10]) {
    if (a - 1 > 0)
        if (fleet[a - 2][b - 1] == 1) return false;
    if (b - 1 > 0)
        if (fleet[a - 1][b - 2] == 1) return false;
    if (a + 1 <= 10)
        if (fleet[a][b - 1] == 1) return false;
    if (b + 1 <= 10)
        if (fleet[a - 1][b] == 1) return false;
    return true;
}

void user_move(Vector2i pos) {
    b = (pos.x - rivalFleet_start[0]) / width + 1; // Столбец выбранной ячейки
    a = (pos.y - rivalFleet_start[1]) / width + 1; // Строка выбранной ячейки
    if (rivalFleet[a - 1][b - 1] == 0) {
        rivalFleet[a - 1][b - 1] = 2;
        is_user_move = false;
        rivalSquares[rival_hit_squares].setSize(Vector2f(width, width));
        rivalSquares[rival_hit_squares].setPosition(rivalFleet_start[0] + width * (b - 1), rivalFleet_start[1] + width * (a - 1));
        rivalSquares[rival_hit_squares++].setFillColor(Color(grey[0], grey[1], grey[2], grey[3]));
    }
    else if (rivalFleet[a - 1][b - 1] == 1) {
        rivalFleet[a - 1][b - 1] = 3;
        my_hit_index.push_back(rival_hit_squares);
        if (ship_is_dead(rivalFleet)) {
            rival_dead_ships++;
            for(size_t i = 0; i < my_hit_index.size(); i++)
                rivalSquares[my_hit_index[i]].setFillColor(Color::Red);
            rivalSquares[rival_hit_squares].setSize(Vector2f(width, width));
            rivalSquares[rival_hit_squares].setPosition(Vector2f(rivalFleet_start[0] + width * (b - 1), rivalFleet_start[1] + width * (a - 1)));
            rivalSquares[rival_hit_squares++].setFillColor(Color::Red);
            my_hit_index.clear();
            next_part[0] = a;
            next_part[1] = b;
            filling_around_dead(rivalFleet, false);
            if (rival_dead_ships == 10) {
                game_is_over = true;
                winner = "ВЫ ПОБЕДИЛИ!";
            }
        }
        else {
            rivalSquares[rival_hit_squares].setSize(Vector2f(width, width));
            rivalSquares[rival_hit_squares].setPosition(rivalFleet_start[0] + width * (b - 1), rivalFleet_start[1] + width * (a - 1));
            rivalSquares[rival_hit_squares++].setFillColor(Color(transparent_red[0], transparent_red[1], transparent_red[2], transparent_red[3]));
        }
    }
}

void computer_arrangement() {
    bool ship_is_set = false;
    while (num_of_ships != 0) {
        if (ships_count[3] != 0) {
            selected_type = ((rand() % 2 + 1 == 1) ? 41 : 42);
            ships_count[3]--;
            num_of_ships--;
        }
        else if (ships_count[2] != 0) {
            selected_type = ((rand() % 2 + 1 == 1) ? 31 : 32);
            ships_count[2]--;
            num_of_ships--;
        }
        else if (ships_count[1] != 0) {
            selected_type = ((rand() % 2 + 1 == 1) ? 21 : 22);
            ships_count[1]--;
            num_of_ships--;
        }
        else if (ships_count[0] != 0) {
            selected_type = 1;
            ships_count[0]--;
            num_of_ships--;
        }
        while (!ship_is_set) {
            engine.seed(device());
            uniform_int_distribution<unsigned> distribution(1, 100);
            int temp = distribution(engine);
            a = temp / 10 + 1;
            b = temp % 10;
            if ((rivalFleet[a - 1][b - 1] == 0) && (insertion_is_possible(rivalFleet))) {
                insertion(rivalFleet);
                ship_is_set = true;
            }
        }
        ship_is_set = false;
    }
    cout << endl << endl;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++)
            cout << rivalFleet[i][j] << "\t";
        cout << endl;
    }
}

bool insertion_is_possible(int fleet[10][10]) {
    switch (selected_type) {
    case 1: {
        if (a - 1 > 0) {
            if (fleet[a - 2][b - 1] == 1) return false;
            if (b + 1 <= 10)
                if ((fleet[a - 2][b] == 1) || (fleet[a - 1][b] == 1)) return false;
            if (b - 1 > 0)
                if ((fleet[a - 2][b - 2] == 1) || (fleet[a - 1][b - 2] == 1)) return false;
        }
        if (a + 1 <= 10) {
            if (fleet[a][b - 1] == 1) return false;
            if (b + 1 <= 10)
                if (fleet[a][b] == 1) return false;
            if (b - 1 > 0)
                if (fleet[a][b - 2] == 1) return false;
        }
        if (b + 1 <= 10)
            if (fleet[a - 1][b] == 1) return false;
        if (b - 1 > 0)
            if (fleet[a - 1][b - 2] == 1) return false;
        return true;
        break;
    }
    case 22:
    case 32:
    case 42: {
        int edge = selected_type / 10 - 1, temp = 1;
        if (b + edge > 10) return false;
        while (temp != edge + 1) {
            if (fleet[a - 1][b + temp - 1] == 1) return false;
            temp++;
        }
        temp = 1;
        if (a - 1 > 0) {
            if (fleet[a - 2][b - 1] == 1) return false;
            while (temp != edge + 1) {
                if (fleet[a - 2][b + temp - 1] == 1) return false;
                temp++;
            }
            temp = 1;
            if (b - 1 > 0)
                if ((fleet[a - 2][b - 2] == 1) || (fleet[a - 1][b - 2] == 1)) return false;
            if (b + edge + 1 <= 10)
                if ((fleet[a - 2][b + edge] == 1) || (fleet[a - 1][b + edge] == 1)) return false;
        }
        if (a + 1 <= 10) {
            if (fleet[a][b - 1] == 1) return false;
            while (temp != edge + 1) {
                if (fleet[a][b + temp - 1] == 1) return false;
                temp++;
            }
            temp = 1;
            if (b - 1 > 0)
                if (fleet[a][b - 2] == 1) return false;
            if (b + edge + 1 <= 10)
                if (fleet[a][b + edge] == 1) return false;
        }
        if (b + edge + 1 <= 10)
            if (fleet[a - 1][b + edge] == 1) return false;
        if (b - 1 > 0)
            if (fleet[a - 1][b - 2] == 1) return false;
        return true;
        break;
    }
    case 21:
    case 31:
    case 41: {
        int edge = selected_type / 10 - 1, temp = 1;
        if (a - edge <= 0) return false;
        while (temp != edge + 1) {
            if (fleet[a - temp - 1][b - 1] == 1) return false;
            temp++;
        }
        temp = 1;
        if (b - 1 > 0) {
            if (fleet[a][b - 1] == 1) return false;
            while (temp != edge + 1) {
                if (fleet[a - temp - 1][b - 2] == 1) return false;
                temp++;
            }
            temp = 1;
            if (a - edge - 1 > 0)
                if ((fleet[a - edge - 2][b - 2] == 1) || (fleet[a - edge - 2][b - 1] == 1)) return false;
            if (a + 1 <= 10)
                if ((fleet[a][b - 2] == 1) || (fleet[a][b - 1] == 1)) return false;
        }
        if (b + 1 <= 10) {
            if (fleet[a - 1][b] == 1) return false;
            while (temp != edge + 1) {
                if (fleet[a - temp - 1][b] == 1) return false;
                temp++;
            }
            temp = 1;
            if (a - edge - 1 > 0)
                if (fleet[a - edge - 2][b] == 1) return false;
            if (a + 1 <= 10)
                if (fleet[a][b] == 1) return false;
        }
        if (a - edge - 1 > 0)
            if (fleet[a - edge - 2][b - 1] == 1) return false;
        if (a + 1 <= 10)
            if (fleet[a][b - 1] == 1) return false;
        return true;
        break;
    }
    default:
        return true; // Просто чтобы убрать предупреждение)
        break;
    }
}

void insertion(int fleet[10][10]) {
    int edge = ((selected_type == 1) ? 0 : (selected_type / 10 - 1)), temp = 1;
    fleet[a - 1][b - 1] = 1;
    if (selected_type != 1) {
        if (selected_type % 10 == 1) {
            while (temp != edge + 1) {
                fleet[a - temp - 1][b - 1] = 1;
                temp++;
            }
            if (!user_is_set) {
                myShips[my_printed_count].setSize(Vector2f(width, width * (edge + 1)));
                myShips[my_printed_count].setPosition(Vector2f(myFleet_start[0] + width * (b - 1), myFleet_start[1] + width * (a - edge - 1)));
                myShips[my_printed_count++].setFillColor(Color::Black);
            }
        }
        else {
            while (temp != edge + 1) {
                fleet[a - 1][b + temp - 1] = 1;
                temp++;
            }
            if (!user_is_set) {
                myShips[my_printed_count].setSize(Vector2f(width * (edge + 1), width));
                myShips[my_printed_count].setPosition(Vector2f(myFleet_start[0] + width * (b - 1), myFleet_start[1] + width * (a - 1)));
                myShips[my_printed_count++].setFillColor(Color::Black);
            }
        }
    }
    else {
        if (!user_is_set) {
            myShips[my_printed_count].setSize(Vector2f(width, width));
            myShips[my_printed_count].setPosition(Vector2f(myFleet_start[0] + width * (b - 1), myFleet_start[1] + width * (a - 1)));
            myShips[my_printed_count++].setFillColor(Color::Black);
        }
    }
}

void ship_selection(Vector2i position) {
    if ((position.x >= unit[0]) && (position.y >= unit[1]) && (position.x <= unit[0] + width) && (position.y <= unit[1] + width) && (ships_count[0] != 0)) { // Выбран единичный корабль
        ship_is_selected = true;
        selected_type = 1;
        selectedShip.setSize(Vector2f(width, width));
        selectedShip.setPosition(Vector2f(unit[0], unit[1]));
        selectedShip.setFillColor(Color::Blue);
    }
    if ((position.x >= deuceVert[0]) && (position.y >= deuceVert[1]) && (position.x <= deuceVert[0] + width * 2) && (position.y <= deuceVert[1] + width * 2) && (ships_count[1] != 0)) { // Выбрана вертикальная двойка
        ship_is_selected = true;
        selected_type = 21;
        selectedShip.setSize(Vector2f(width, width * 2));
        selectedShip.setPosition(Vector2f(deuceVert[0], deuceVert[1]));
        selectedShip.setFillColor(Color::Blue);
    }
    if ((position.x >= deuceHor[0]) && (position.y >= deuceHor[1]) && (position.x <= deuceHor[0] + width * 2) && (position.y <= deuceHor[1] + width * 2) && (ships_count[1] != 0)) { // Выбрана горизонтальная двойка
        ship_is_selected = true;
        selected_type = 22;
        selectedShip.setSize(Vector2f(width * 2, width));
        selectedShip.setPosition(Vector2f(deuceHor[0], deuceHor[1]));
        selectedShip.setFillColor(Color::Blue);
    }
    if ((position.x >= tripleVert[0]) && (position.y >= tripleVert[1]) && (position.x <= tripleVert[0] + width * 3) && (position.y <= tripleVert[1] + width * 3) && (ships_count[2] != 0)) { // Выбрана вертикальная тройка
        ship_is_selected = true;
        selected_type = 31;
        selectedShip.setSize(Vector2f(width, width * 3));
        selectedShip.setPosition(Vector2f(tripleVert[0], tripleVert[1]));
        selectedShip.setFillColor(Color::Blue);
    }
    if ((position.x >= tripleHor[0]) && (position.y >= tripleHor[1]) && (position.x <= tripleHor[0] + width * 3) && (position.y <= tripleHor[1] + width * 3) && (ships_count[2] != 0)) { // Выбрана горизонтальная тройка
        ship_is_selected = true;
        selected_type = 32;
        selectedShip.setSize(Vector2f(width * 3, width));
        selectedShip.setPosition(Vector2f(tripleHor[0], tripleHor[1]));
        selectedShip.setFillColor(Color::Blue);
    }
    if ((position.x >= fourVert[0]) && (position.y >= fourVert[1]) && (position.x <= fourVert[0] + width * 4) && (position.y <= fourVert[1] + width * 4) && (ships_count[3] != 0)) { // Выбрана вертикальная четверка
        ship_is_selected = true;
        selected_type = 41;
        selectedShip.setSize(Vector2f(width, width * 4));
        selectedShip.setPosition(Vector2f(fourVert[0], fourVert[1]));
        selectedShip.setFillColor(Color::Blue);
    }
    if ((position.x >= fourHor[0]) && (position.y >= fourHor[1]) && (position.x <= fourHor[0] + width * 4) && (position.y <= fourHor[1] + width * 4) && (ships_count[3] != 0)) { // Выбрана горизонтальная четверка
        ship_is_selected = true;
        selected_type = 42;
        selectedShip.setSize(Vector2f(width * 4, width));
        selectedShip.setPosition(Vector2f(fourHor[0], fourHor[1]));
        selectedShip.setFillColor(Color::Blue);
    }
}

void filling_around_dead(int fleet[10][10], bool fleet_is_rival) {
    int prev_part[2] = { next_part[0], next_part[1] };
    bool flag;
    while (true) {
        flag = false;
        if ((next_part[0] - 1 > 0) && (next_part[1] - 1 > 0)) {
            fleet[next_part[0] - 2][next_part[1] - 2] = 2;
            if (fleet_is_rival)
                rival_print_square(next_part[0] - 1, next_part[1] - 1);
            else
                my_print_square(next_part[0] - 1, next_part[1] - 1);
        }
        if ((next_part[0] - 1 > 0) && (next_part[1] + 1 <= 10)) {
            fleet[next_part[0] - 2][next_part[1]] = 2;
            if (fleet_is_rival)
                rival_print_square(next_part[0] - 1, next_part[1] + 1);
            else
                my_print_square(next_part[0] - 1, next_part[1] + 1);
        }
        if ((next_part[0] + 1 <= 10) && (next_part[1] - 1 > 0)) {
            fleet[next_part[0]][next_part[1] - 2] = 2;
            if (fleet_is_rival)
                rival_print_square(next_part[0] + 1, next_part[1] - 1);
            else
                my_print_square(next_part[0] + 1, next_part[1] - 1);
        }
        if ((next_part[0] + 1 <= 10) && (next_part[1] + 1 <= 10)) {
            fleet[next_part[0]][next_part[1]] = 2;
            if (fleet_is_rival)
                rival_print_square(next_part[0] + 1, next_part[1] + 1);
            else
                my_print_square(next_part[0] + 1, next_part[1] + 1);
        }
        if (next_part[0] - 1 > 0) {
            if (fleet[next_part[0] - 2][next_part[1] - 1] == 0) {
                fleet[next_part[0] - 2][next_part[1] - 1] = 2;
                if (fleet_is_rival)
                    rival_print_square(next_part[0] - 1, next_part[1]);
                else
                    my_print_square(next_part[0] - 1, next_part[1]);
            }
            else if ((fleet[next_part[0] - 2][next_part[1] - 1] == 3) && (prev_part[0] != next_part[0] - 1)) {
                prev_part[0] = next_part[0];
                prev_part[1] = next_part[1];
                next_part[0] = next_part[0] - 1;
                next_part[1] = next_part[1];
                flag = true;
            }
        }
        if (next_part[1] - 1 > 0) {
            if (fleet[next_part[0] - 1][next_part[1] - 2] == 0) {
                fleet[next_part[0] - 1][next_part[1] - 2] = 2;
                if (fleet_is_rival)
                    rival_print_square(next_part[0], next_part[1] - 1);
                else
                    my_print_square(next_part[0], next_part[1] - 1);
            }
            else if ((fleet[next_part[0] - 1][next_part[1] - 2] == 3) && (prev_part[1] != next_part[1] - 1)) {
                prev_part[0] = next_part[0];
                prev_part[1] = next_part[1];
                next_part[0] = next_part[0];
                next_part[1] = next_part[1] - 1;
                flag = true;
            }
        }
        if (next_part[0] + 1 <= 10) {
            if (fleet[next_part[0]][next_part[1] - 1] == 0) {
                fleet[next_part[0]][next_part[1] - 1] = 2;
                if (fleet_is_rival)
                    rival_print_square(next_part[0] + 1, next_part[1]);
                else
                    my_print_square(next_part[0] + 1, next_part[1]);
            }
            else if ((fleet[next_part[0]][next_part[1] - 1] == 3) && (prev_part[0] != next_part[0] + 1)) {
                prev_part[0] = next_part[0];
                prev_part[1] = next_part[1];
                next_part[0] = next_part[0] + 1;
                next_part[1] = next_part[1];
                flag = true;
            }
        }
        if (next_part[1] + 1 <= 10) {
            if (fleet[next_part[0] - 1][next_part[1]] == 0) {
                fleet[next_part[0] - 1][next_part[1]] = 2;
                if (fleet_is_rival)
                    rival_print_square(next_part[0], next_part[1] + 1);
                else
                    my_print_square(next_part[0], next_part[1] + 1);
            }
            else if ((fleet[next_part[0] - 1][next_part[1]] == 3) && (prev_part[1] != next_part[1] + 1)) {
                prev_part[0] = next_part[0];
                prev_part[1] = next_part[1];
                next_part[0] = next_part[0];
                next_part[1] = next_part[1] + 1;
                flag = true;
            }
        }
        if (!flag)
            break;
    }
}

void my_print_square(int x, int y) {
    rivalSquares[rival_hit_squares].setSize(Vector2f(width, width));
    rivalSquares[rival_hit_squares].setPosition(rivalFleet_start[0] + width * (y - 1), rivalFleet_start[1] + width * (x - 1));
    rivalSquares[rival_hit_squares++].setFillColor(Color(grey[0], grey[1], grey[2], grey[3]));
}

void rival_print_square(int x, int y) {
    mySquares[my_hit_squares].setSize(Vector2f(width, width));
    mySquares[my_hit_squares].setPosition(myFleet_start[0] + width * (y - 1), myFleet_start[1] + width * (x - 1));
    mySquares[my_hit_squares++].setFillColor(Color(grey[0], grey[1], grey[2], grey[3]));
}
