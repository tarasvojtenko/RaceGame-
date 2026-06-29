// race.cpp
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

using namespace std;

const string RESET = "\033[0m";
const string GREEN = "\033[92m";
const string RED = "\033[91m";
const string YELLOW = "\033[93m";
const string BLUE = "\033[94m";
const string GRAY = "\033[90m";
const string BOLD = "\033[1m";

string colorize(const string& text, const string& color) {
    return color + text + RESET;
}

const int WIDTH = 7;
const int HEIGHT = 20;
const int PLAYER_POS = HEIGHT - 1;
const int INITIAL_DELAY = 300;
const int MIN_DELAY = 50;
const int SPEED_INCREMENT = 2;

string getHomeDir() {
    const char* home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE");
    return string(home);
}

int loadRecord() {
    ifstream f(getHomeDir() + "/.race_record");
    int rec = 0;
    if (f) f >> rec;
    return rec;
}

void saveRecord(int rec) {
    ofstream f(getHomeDir() + "/.race_record");
    f << rec;
}

void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

void drawRoad(int playerX, const vector<pair<int,int>>& obstacles, int score) {
    clearScreen();
    // Верх
    cout << colorize("┌" + string(WIDTH * 2 + 1, '─') + "┐", GRAY) << endl;
    for (int row = 0; row < HEIGHT; ++row) {
        string line = "│ ";
        for (int col = 0; col < WIDTH; ++col) {
            bool isPlayer = (row == PLAYER_POS && col == playerX);
            bool isObstacle = false;
            for (auto& obs : obstacles) {
                if (obs.first == col && obs.second == row) {
                    isObstacle = true;
                    break;
                }
            }
            if (isPlayer) {
                line += colorize("█", GREEN) + " ";
            } else if (isObstacle) {
                line += colorize("▓", RED) + " ";
            } else {
                line += "  ";
            }
        }
        line += "│";
        cout << line << endl;
    }
    cout << colorize("└" + string(WIDTH * 2 + 1, '─') + "┘", GRAY) << endl;
    cout << colorize("Счёт: " + to_string(score), YELLOW) << endl;
    cout << colorize("Управление: ← → (A/D)  |  Q - выход", BLUE) << endl;
    cout << colorize("Рекорд: " + to_string(loadRecord()), BOLD) << endl;
}

char getch() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

bool kbhit() {
    struct termios oldt, newt;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}

int main() {
    srand(time(nullptr));
    int playerX = WIDTH / 2;
    vector<pair<int,int>> obstacles;
    int score = 0;
    int delay = INITIAL_DELAY;
    int record = loadRecord();

    while (true) {
        // Неблокирующий ввод
        if (kbhit()) {
            char key = getch();
            if (key == 'q' || key == 'Q') {
                cout << colorize("Выход из игры.", YELLOW) << endl;
                return 0;
            }
            if (key == 'a' || key == 'A' || key == 27) {
                // Стрелка — нужно обработать ESC последовательность
                if (key == 27) {
                    char buf[2];
                    if (getchar() == '[') {
                        char c = getchar();
                        if (c == 'D' && playerX > 0) playerX--;
                        else if (c == 'C' && playerX < WIDTH-1) playerX++;
                    }
                } else {
                    if (key == 'a' || key == 'A') {
                        if (playerX > 0) playerX--;
                    }
                }
            }
            if (key == 'd' || key == 'D') {
                if (playerX < WIDTH-1) playerX++;
            }
        }

        // Генерация препятствий
        if ((double)rand() / RAND_MAX < 0.2 + score * 0.001) {
            int col = rand() % WIDTH;
            obstacles.push_back({col, 0});
        }

        // Движение препятствий
        vector<pair<int,int>> newObs;
        for (auto& obs : obstacles) {
            obs.second++;
            if (obs.second < HEIGHT) {
                newObs.push_back(obs);
            }
        }
        obstacles = newObs;

        // Проверка столкновения
        for (auto& obs : obstacles) {
            if (obs.second == PLAYER_POS && obs.first == playerX) {
                clearScreen();
                cout << colorize("💥 Авария!", RED) << endl;
                cout << colorize("Ваш счёт: " + to_string(score), YELLOW) << endl;
                if (score > record) {
                    record = score;
                    saveRecord(record);
                    cout << colorize("🏆 Новый рекорд: " + to_string(record) + "!", GREEN) << endl;
                } else {
                    cout << colorize("Рекорд: " + to_string(record), BLUE) << endl;
                }
                cout << colorize("Нажмите любую клавишу для выхода...", YELLOW) << endl;
                getch();
                return 0;
            }
        }

        // Увеличение счёта
        score++;

        // Ускорение
        if (delay > MIN_DELAY) {
            delay -= SPEED_INCREMENT;
        }

        // Отрисовка
        drawRoad(playerX, obstacles, score);

        // Задержка
        this_thread::sleep_for(chrono::milliseconds(delay));
    }
    return 0;
}
