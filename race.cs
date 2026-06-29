// race.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Diagnostics;

class Race
{
    static string Colorize(string text, string color)
    {
        string col = color switch
        {
            "green" => "\x1b[92m",
            "red" => "\x1b[91m",
            "yellow" => "\x1b[93m",
            "blue" => "\x1b[94m",
            "gray" => "\x1b[90m",
            "bold" => "\x1b[1m",
            _ => "\x1b[0m"
        };
        return col + text + "\x1b[0m";
    }

    const int WIDTH = 7;
    const int HEIGHT = 20;
    const int PLAYER_POS = HEIGHT - 1;
    const int INITIAL_DELAY = 300;
    const int MIN_DELAY = 50;
    const int SPEED_INCREMENT = 2;

    static string RecordFile => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), ".race_record");

    static int LoadRecord()
    {
        try { return int.Parse(File.ReadAllText(RecordFile).Trim()); }
        catch { return 0; }
    }

    static void SaveRecord(int rec)
    {
        File.WriteAllText(RecordFile, rec.ToString());
    }

    static void ClearScreen()
    {
        Console.Clear();
    }

    static void DrawRoad(int playerX, List<(int col, int row)> obstacles, int score)
    {
        ClearScreen();
        Console.WriteLine(Colorize("┌" + new string('─', WIDTH * 2 + 1) + "┐", "gray"));
        for (int row = 0; row < HEIGHT; row++)
        {
            string line = "│ ";
            for (int col = 0; col < WIDTH; col++)
            {
                bool isPlayer = (row == PLAYER_POS && col == playerX);
                bool isObstacle = obstacles.Exists(o => o.col == col && o.row == row);
                if (isPlayer)
                    line += Colorize("█", "green") + " ";
                else if (isObstacle)
                    line += Colorize("▓", "red") + " ";
                else
                    line += "  ";
            }
            line += "│";
            Console.WriteLine(line);
        }
        Console.WriteLine(Colorize("└" + new string('─', WIDTH * 2 + 1) + "┘", "gray"));
        Console.WriteLine(Colorize($"Счёт: {score}", "yellow"));
        Console.WriteLine(Colorize("Управление: ← → (A/D)  |  Q - выход", "blue"));
        Console.WriteLine(Colorize($"Рекорд: {LoadRecord()}", "bold"));
    }

    static int Main()
    {
        Random rand = new Random();
        int playerX = WIDTH / 2;
        var obstacles = new List<(int col, int row)>();
        int score = 0;
        int delay = INITIAL_DELAY;
        int record = LoadRecord();

        Console.CancelKeyPress += (sender, e) =>
        {
            Console.WriteLine(Colorize("\nИгра прервана.", "yellow"));
            Environment.Exit(0);
        };

        while (true)
        {
            // Ввод
            if (Console.KeyAvailable)
            {
                var key = Console.ReadKey(true).Key;
                if (key == ConsoleKey.Q)
                {
                    Console.WriteLine(Colorize("Выход из игры.", "yellow"));
                    return 0;
                }
                if (key == ConsoleKey.LeftArrow || key == ConsoleKey.A)
                {
                    if (playerX > 0) playerX--;
                }
                if (key == ConsoleKey.RightArrow || key == ConsoleKey.D)
                {
                    if (playerX < WIDTH - 1) playerX++;
                }
            }

            // Генерация препятствий
            if (rand.NextDouble() < 0.2 + score * 0.001)
            {
                int col = rand.Next(WIDTH);
                obstacles.Add((col, 0));
            }

            // Движение препятствий
            var newObs = new List<(int col, int row)>();
            foreach (var obs in obstacles)
            {
                int newRow = obs.row + 1;
                if (newRow < HEIGHT)
                    newObs.Add((obs.col, newRow));
            }
            obstacles = newObs;

            // Проверка столкновения
            foreach (var obs in obstacles)
            {
                if (obs.row == PLAYER_POS && obs.col == playerX)
                {
                    ClearScreen();
                    Console.WriteLine(Colorize("💥 Авария!", "red"));
                    Console.WriteLine(Colorize($"Ваш счёт: {score}", "yellow"));
                    if (score > record)
                    {
                        record = score;
                        SaveRecord(record);
                        Console.WriteLine(Colorize($"🏆 Новый рекорд: {record}!", "green"));
                    }
                    else
                    {
                        Console.WriteLine(Colorize($"Рекорд: {record}", "blue"));
                    }
                    Console.WriteLine(Colorize("Нажмите любую клавишу для выхода...", "yellow"));
                    Console.ReadKey(true);
                    return 0;
                }
            }

            // Увеличение счёта
            score++;

            // Ускорение
            if (delay > MIN_DELAY)
                delay -= SPEED_INCREMENT;

            // Отрисовка
            DrawRoad(playerX, obstacles, score);

            // Задержка
            Thread.Sleep(delay);
        }
    }
}
