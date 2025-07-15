# Brownian Motion Simulation

Программа для демонстрации оптимизации производительности с помощью Perforator на C++ Zero Cost Conf.

## Описание

Программа симулирует броуновское движение частиц и визуализирует их в реальном времени. Включает счетчик FPS и специально неоптимизированное перемножение матриц для демонстрации профилирования.

## Зависимости

- SFML 2.5+ (Simple and Fast Multimedia Library)
- CMake 3.16+
- C++17 совместимый компилятор

## Установка зависимостей

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libsfml-dev cmake build-essential
```

### macOS
```bash
brew install sfml cmake
```

## Сборка

```bash
mkdir build
cd build
cmake ..
make
```

## Запуск

```bash
./brownian_simulation
```

## Использование с Perforator

1. Соберите программу в Debug режиме для лучшего профилирования:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

2. Запустите с Perforator для анализа производительности
3. Найдите узкие места в матричных операциях
4. Оптимизируйте код для улучшения кэш-локальности

## Структура

- `src/main.cpp` - основной цикл программы
- `src/simulation.cpp` - логика броуновского движения
- `src/matrix_operations.cpp` - матричные операции (здесь находятся "медленные" операции)
- `src/fps_counter.cpp` - счетчик FPS 