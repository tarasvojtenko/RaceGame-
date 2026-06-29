// race.go
package main

import (
	"bufio"
	"fmt"
	"math/rand"
	"os"
	"os/exec"
	"runtime"
	"strconv"
	"strings"
	"time"
	"golang.org/x/term" // go get golang.org/x/term
)

const (
	WIDTH           = 7
	HEIGHT          = 20
	PLAYER_POS      = HEIGHT - 1
	INITIAL_DELAY   = 300 * time.Millisecond
	MIN_DELAY       = 50 * time.Millisecond
	SPEED_INCREMENT = 2 * time.Millisecond
)

var reset = "\033[0m"
var green = "\033[92m"
var red = "\033[91m"
var yellow = "\033[93m"
var blue = "\033[94m"
var gray = "\033[90m"
var bold = "\033[1m"

func colorize(text, color string) string {
	return color + text + reset
}

func clearScreen() {
	cmd := exec.Command("clear")
	if runtime.GOOS == "windows" {
		cmd = exec.Command("cmd", "/c", "cls")
	}
	cmd.Stdout = os.Stdout
	cmd.Run()
}

func loadRecord() int {
	data, err := os.ReadFile(os.Getenv("HOME") + "/.race_record")
	if err != nil {
		return 0
	}
	record, _ := strconv.Atoi(strings.TrimSpace(string(data)))
	return record
}

func saveRecord(record int) {
	os.WriteFile(os.Getenv("HOME")+"/.race_record", []byte(strconv.Itoa(record)), 0644)
}

func drawRoad(playerX int, obstacles [][2]int, score int) {
	clearScreen()
	// Верхняя граница
	fmt.Println(colorize("┌"+strings.Repeat("─", WIDTH*2+1)+"┐", gray))
	// Поле
	for row := 0; row < HEIGHT; row++ {
		line := "│ "
		for col := 0; col < WIDTH; col++ {
			if row == PLAYER_POS && col == playerX {
				line += colorize("█", green) + " "
			} else if obstacleAt(col, row, obstacles) {
				line += colorize("▓", red) + " "
			} else {
				line += "  "
			}
		}
		line += "│"
		fmt.Println(line)
	}
	fmt.Println(colorize("└"+strings.Repeat("─", WIDTH*2+1)+"┘", gray))
	fmt.Println(colorize(fmt.Sprintf("Счёт: %d", score), yellow))
	fmt.Println(colorize("Управление: ← → (A/D)  |  Q - выход", blue))
	fmt.Println(colorize("Рекорд: "+strconv.Itoa(loadRecord()), bold))
}

func obstacleAt(col, row int, obstacles [][2]int) bool {
	for _, obs := range obstacles {
		if obs[0] == col && obs[1] == row {
			return true
		}
	}
	return false
}

func getKey() string {
	oldState, err := term.MakeRaw(int(os.Stdin.Fd()))
	if err != nil {
		panic(err)
	}
	defer term.Restore(int(os.Stdin.Fd()), oldState)
	var buf [1]byte
	os.Stdin.Read(buf[:])
	return string(buf[:])
}

func main() {
	rand.Seed(time.Now().UnixNano())
	playerX := WIDTH / 2
	obstacles := [][2]int{}
	score := 0
	delay := INITIAL_DELAY
	record := loadRecord()

	for {
		// Ввод
		key := getKey()
		if key == "q" || key == "Q" {
			fmt.Println(colorize("Выход из игры.", yellow))
			return
		}
		if key == "\x1b" {
			// Стрелка — читаем ещё два байта
			buf := make([]byte, 2)
			os.Stdin.Read(buf)
			if buf[0] == '[' && buf[1] == 'D' { // влево
				if playerX > 0 {
					playerX--
				}
			} else if buf[0] == '[' && buf[1] == 'C' { // вправо
				if playerX < WIDTH-1 {
					playerX++
				}
			}
		} else if key == "a" || key == "A" {
			if playerX > 0 {
				playerX--
			}
		} else if key == "d" || key == "D" {
			if playerX < WIDTH-1 {
				playerX++
			}
		}

		// Генерация препятствий
		if rand.Float64() < 0.2+float64(score)*0.001 {
			col := rand.Intn(WIDTH)
			obstacles = append(obstacles, [2]int{col, 0})
		}

		// Движение препятствий
		newObs := [][2]int{}
		for _, obs := range obstacles {
			obs[1]++
			if obs[1] < HEIGHT {
				newObs = append(newObs, obs)
			}
		}
		obstacles = newObs

		// Проверка столкновения
		for _, obs := range obstacles {
			if obs[1] == PLAYER_POS && obs[0] == playerX {
				clearScreen()
				fmt.Println(colorize("💥 Авария!", red))
				fmt.Println(colorize(fmt.Sprintf("Ваш счёт: %d", score), yellow))
				if score > record {
					record = score
					saveRecord(record)
					fmt.Println(colorize(fmt.Sprintf("🏆 Новый рекорд: %d!", record), green))
				} else {
					fmt.Println(colorize(fmt.Sprintf("Рекорд: %d", record), blue))
				}
				fmt.Println(colorize("Нажмите любую клавишу для выхода...", yellow))
				fmt.Scanln()
				return
			}
		}

		// Увеличение счёта
		score++

		// Ускорение
		if delay > MIN_DELAY {
			delay -= SPEED_INCREMENT
		}

		// Отрисовка
		drawRoad(playerX, obstacles, score)

		// Задержка
		time.Sleep(delay)
	}
}
