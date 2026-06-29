# race.py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import random
import time
import json
import threading
import termios
import tty
import fcntl
from pathlib import Path

# ANSI colors
COLORS = {
    'reset': '\033[0m',
    'green': '\033[92m',
    'red': '\033[91m',
    'yellow': '\033[93m',
    'blue': '\033[94m',
    'gray': '\033[90m',
    'bold': '\033[1m'
}

def colorize(text, color):
    return f"{COLORS.get(color, '')}{text}{COLORS['reset']}"

# Конфигурация
WIDTH = 7
HEIGHT = 20
PLAYER_POS = HEIGHT - 1
INITIAL_DELAY = 0.3
MIN_DELAY = 0.05
SPEED_INCREMENT = 0.002

RECORD_FILE = Path.home() / '.race_record'

def load_record():
    if RECORD_FILE.exists():
        try:
            with open(RECORD_FILE, 'r') as f:
                return int(f.read().strip())
        except:
            return 0
    return 0

def save_record(record):
    with open(RECORD_FILE, 'w') as f:
        f.write(str(record))

def clear_screen():
    os.system('clear' if os.name == 'posix' else 'cls')

def get_key():
    # Неблокирующий ввод с терминала
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        fcntl.fcntl(fd, fcntl.F_SETFL, os.O_NONBLOCK)
        ch = sys.stdin.read(1)
        return ch
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)
        fcntl.fcntl(fd, fcntl.F_SETFL, 0)

def draw_road(player_x, obstacles, score):
    clear_screen()
    # Верхняя граница
    print(colorize('┌' + '─' * (WIDTH * 2 + 1) + '┐', 'gray'))
    # Игровое поле
    for row in range(HEIGHT):
        line = '│ '
        for col in range(WIDTH):
            if row == PLAYER_POS and col == player_x:
                line += colorize('█', 'green')
            elif any(obs[0] == col and obs[1] == row for obs in obstacles):
                line += colorize('▓', 'red')
            else:
                line += ' '
            line += ' '
        line += '│'
        print(line)
    # Нижняя граница
    print(colorize('└' + '─' * (WIDTH * 2 + 1) + '┘', 'gray'))
    print(colorize(f'Счёт: {score}', 'yellow'))
    print(colorize('Управление: ← → (A/D)  |  Q - выход', 'blue'))
    print(colorize('Рекорд: ' + str(load_record()), 'bold'))

def main():
    # Инициализация
    player_x = WIDTH // 2
    obstacles = []
    score = 0
    delay = INITIAL_DELAY
    frame = 0

    # Загрузка рекорда
    record = load_record()

    # Цикл игры
    while True:
        # Обработка ввода
        key = get_key()
        if key:
            if key == 'q' or key == 'Q':
                print(colorize('Выход из игры.', 'yellow'))
                sys.exit(0)
            elif key == 'a' or key == 'A' or key == '\x1b[D':  # влево
                if player_x > 0:
                    player_x -= 1
            elif key == 'd' or key == 'D' or key == '\x1b[C':  # вправо
                if player_x < WIDTH - 1:
                    player_x += 1

        # Генерация препятствий
        if random.random() < 0.2 + score * 0.001:
            col = random.randint(0, WIDTH - 1)
            obstacles.append([col, 0])

        # Движение препятствий вниз
        new_obstacles = []
        for obs in obstacles:
            obs[1] += 1
            if obs[1] < HEIGHT:
                new_obstacles.append(obs)
        obstacles = new_obstacles

        # Проверка столкновения
        for obs in obstacles:
            if obs[1] == PLAYER_POS and obs[0] == player_x:
                # Конец игры
                clear_screen()
                print(colorize('💥 Авария!', 'red'))
                print(colorize(f'Ваш счёт: {score}', 'yellow'))
                if score > record:
                    record = score
                    save_record(record)
                    print(colorize(f'🏆 Новый рекорд: {record}!', 'green'))
                else:
                    print(colorize(f'Рекорд: {record}', 'blue'))
                print(colorize('Нажмите любую клавишу для выхода...', 'yellow'))
                # Ожидаем ввода для завершения
                input()
                sys.exit(0)

        # Увеличение счёта (за каждый кадр без столкновения)
        score += 1

        # Увеличение скорости
        if delay > MIN_DELAY:
            delay -= SPEED_INCREMENT

        # Отрисовка
        draw_road(player_x, obstacles, score)

        # Пауза
        time.sleep(delay)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print(colorize('\nИгра прервана.', 'yellow'))
        sys.exit(0)
