#!/usr/bin/env ruby
# race.rb
# encoding: UTF-8

require 'io/console'
require 'fileutils'
require 'timeout'

# ANSI colors
COLORS = {
  reset: "\e[0m",
  green: "\e[92m",
  red: "\e[91m",
  yellow: "\e[93m",
  blue: "\e[94m",
  gray: "\e[90m",
  bold: "\e[1m"
}

def colorize(text, color)
  "#{COLORS[color]}#{text}#{COLORS[:reset]}"
end

WIDTH = 7
HEIGHT = 20
PLAYER_POS = HEIGHT - 1
INITIAL_DELAY = 0.3
MIN_DELAY = 0.05
SPEED_INCREMENT = 0.002

RECORD_FILE = File.join(Dir.home, '.race_record')

def load_record
  File.exist?(RECORD_FILE) ? File.read(RECORD_FILE).strip.to_i : 0
end

def save_record(record)
  File.write(RECORD_FILE, record.to_s)
end

def clear_screen
  system('clear') || system('cls')
end

def draw_road(player_x, obstacles, score)
  clear_screen
  puts colorize('┌' + '─' * (WIDTH * 2 + 1) + '┐', :gray)
  (0...HEIGHT).each do |row|
    line = '│ '
    (0...WIDTH).each do |col|
      if row == PLAYER_POS && col == player_x
        line += colorize('█', :green) + ' '
      elsif obstacles.any? { |obs| obs[0] == col && obs[1] == row }
        line += colorize('▓', :red) + ' '
      else
        line += '  '
      end
    end
    line += '│'
    puts line
  end
  puts colorize('└' + '─' * (WIDTH * 2 + 1) + '┘', :gray)
  puts colorize("Счёт: #{score}", :yellow)
  puts colorize('Управление: ← → (A/D)  |  Q - выход', :blue)
  puts colorize("Рекорд: #{load_record}", :bold)
end

def get_key
  # Читаем один символ без echo
  state = `stty -g` rescue nil
  `stty raw -echo -icanon isig` rescue nil
  ch = STDIN.getc
  if ch == "\e"
    # Стрелка
    seq = STDIN.read_nonblock(3) rescue nil
    if seq && seq == "[D"
      ch = :left
    elsif seq && seq == "[C"
      ch = :right
    else
      ch = seq
    end
  end
  ch
ensure
  `stty #{state}` rescue nil
end

def main
  player_x = WIDTH / 2
  obstacles = []
  score = 0
  delay = INITIAL_DELAY
  record = load_record

  trap('INT') do
    puts colorize("\nИгра прервана.", :yellow)
    exit
  end

  loop do
    # Ввод
    key = get_key
    if key == 'q' || key == 'Q'
      puts colorize('Выход из игры.', :yellow)
      exit
    end
    if key == :left || key == 'a' || key == 'A'
      player_x -= 1 if player_x > 0
    end
    if key == :right || key == 'd' || key == 'D'
      player_x += 1 if player_x < WIDTH - 1
    end

    # Генерация препятствий
    if rand < 0.2 + score * 0.001
      col = rand(WIDTH)
      obstacles << [col, 0]
    end

    # Движение препятствий
    new_obs = []
    obstacles.each do |obs|
      obs[1] += 1
      new_obs << obs if obs[1] < HEIGHT
    end
    obstacles = new_obs

    # Проверка столкновения
    if obstacles.any? { |obs| obs[0] == player_x && obs[1] == PLAYER_POS }
      clear_screen
      puts colorize('💥 Авария!', :red)
      puts colorize("Ваш счёт: #{score}", :yellow)
      if score > record
        record = score
        save_record(record)
        puts colorize("🏆 Новый рекорд: #{record}!", :green)
      else
        puts colorize("Рекорд: #{record}", :blue)
      end
      puts colorize('Нажмите любую клавишу для выхода...', :yellow)
      STDIN.getc
      exit
    end

    # Увеличение счёта
    score += 1

    # Ускорение
    delay -= SPEED_INCREMENT if delay > MIN_DELAY

    # Отрисовка
    draw_road(player_x, obstacles, score)

    # Задержка
    sleep(delay)
  end
end

main if __FILE__ == $0
