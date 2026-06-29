// race.js
#!/usr/bin/env node
'use strict';

const readline = require('readline');
const fs = require('fs');
const path = require('path');
const os = require('os');

const COLORS = {
    reset: '\x1b[0m',
    green: '\x1b[92m',
    red: '\x1b[91m',
    yellow: '\x1b[93m',
    blue: '\x1b[94m',
    gray: '\x1b[90m',
    bold: '\x1b[1m'
};

function colorize(text, color) {
    return COLORS[color] + text + COLORS.reset;
}

const WIDTH = 7;
const HEIGHT = 20;
const PLAYER_POS = HEIGHT - 1;
const INITIAL_DELAY = 300;
const MIN_DELAY = 50;
const SPEED_INCREMENT = 2;

const RECORD_FILE = path.join(os.homedir(), '.race_record');

function loadRecord() {
    try {
        return parseInt(fs.readFileSync(RECORD_FILE, 'utf8').trim(), 10);
    } catch { return 0; }
}

function saveRecord(record) {
    fs.writeFileSync(RECORD_FILE, String(record));
}

function clearScreen() {
    console.clear();
}

function drawRoad(playerX, obstacles, score) {
    clearScreen();
    console.log(colorize('┌' + '─'.repeat(WIDTH * 2 + 1) + '┐', 'gray'));
    for (let row = 0; row < HEIGHT; row++) {
        let line = '│ ';
        for (let col = 0; col < WIDTH; col++) {
            if (row === PLAYER_POS && col === playerX) {
                line += colorize('█', 'green') + ' ';
            } else if (obstacles.some(obs => obs[0] === col && obs[1] === row)) {
                line += colorize('▓', 'red') + ' ';
            } else {
                line += '  ';
            }
        }
        line += '│';
        console.log(line);
    }
    console.log(colorize('└' + '─'.repeat(WIDTH * 2 + 1) + '┘', 'gray'));
    console.log(colorize(`Счёт: ${score}`, 'yellow'));
    console.log(colorize('Управление: ← → (A/D)  |  Q - выход', 'blue'));
    console.log(colorize(`Рекорд: ${loadRecord()}`, 'bold'));
}

function getKey(callback) {
    readline.emitKeypressEvents(process.stdin);
    process.stdin.setRawMode(true);
    process.stdin.once('keypress', (str, key) => {
        callback(str, key);
    });
}

function main() {
    let playerX = Math.floor(WIDTH / 2);
    let obstacles = [];
    let score = 0;
    let delay = INITIAL_DELAY;
    const record = loadRecord();

    function gameLoop() {
        // Генерация препятствий
        if (Math.random() < 0.2 + score * 0.001) {
            const col = Math.floor(Math.random() * WIDTH);
            obstacles.push([col, 0]);
        }

        // Движение препятствий
        const newObs = [];
        for (const obs of obstacles) {
            obs[1]++;
            if (obs[1] < HEIGHT) {
                newObs.push(obs);
            }
        }
        obstacles = newObs;

        // Проверка столкновения
        for (const obs of obstacles) {
            if (obs[1] === PLAYER_POS && obs[0] === playerX) {
                clearScreen();
                console.log(colorize('💥 Авария!', 'red'));
                console.log(colorize(`Ваш счёт: ${score}`, 'yellow'));
                if (score > record) {
                    const newRecord = score;
                    saveRecord(newRecord);
                    console.log(colorize(`🏆 Новый рекорд: ${newRecord}!`, 'green'));
                } else {
                    console.log(colorize(`Рекорд: ${record}`, 'blue'));
                }
                console.log(colorize('Нажмите любую клавишу для выхода...', 'yellow'));
                process.stdin.setRawMode(false);
                process.stdin.once('data', () => process.exit(0));
                return;
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

        // Ожидание ввода
        getKey((str, key) => {
            if (key && key.name === 'q') {
                console.log(colorize('Выход из игры.', 'yellow'));
                process.exit(0);
            }
            if (key && (key.name === 'left' || str === 'a' || str === 'A')) {
                if (playerX > 0) playerX--;
            }
            if (key && (key.name === 'right' || str === 'd' || str === 'D')) {
                if (playerX < WIDTH - 1) playerX++;
            }
            // Запускаем следующий кадр
            setTimeout(gameLoop, delay);
        });
    }

    // Запуск
    gameLoop();
}

main();
