# Генератор множества Мандельброта с использованием MPI и OpenCV

## 1. Постановка задачи

Разработать параллельную программу на C++ с использованием MPI, которая:
- Генерирует изображение фрактала Мандельброта
- Сравнивает время выполнения последовательной и параллельной версий
- Визуализирует результат с помощью библиотеки OpenCV

## 2. Описание важных функций

### `calculateFractalPoint(const std::complex<double>& point)`
Вычисляет количество итераций для точки на комплексной плоскости до выхода за границу сходимости (|z| > 2) или достижения максимального числа итераций.

### `determinePixelColor(int counter)`
Преобразует количество итераций в цвет пикселя по специальной цветовой схеме.

### `generateFractalImage(cv::Mat& outputImage)`
Последовательная версия генерации всего изображения фрактала.

### Параллельный блок в `main()`
- Распределяет строки изображения между процессами
- Каждый процесс вычисляет свою часть изображения
- Собирает результаты на главном процессе с помощью `MPI_Gatherv`

## 3. Ключевые функции MPI

**Функции MPI:**
- `MPI_Init` / `MPI_Finalize` - инициализация и завершение MPI
- `MPI_Comm_rank` - получение номера процесса
- `MPI_Comm_size` - получение общего числа процессов
- `MPI_Barrier` - синхронизация процессов
- `MPI_Gatherv` - сбор данных с переменным размером от всех процессов

## 4. Компиляция и запуск

### Компиляция:
```bash
g++ -fopenmp -O3 -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" -IC:/OpenCV-MinGW-Build/include main.cpp -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -LC:/OpenCV-MinGW-Build/x64/mingw/lib -lmsmpi -lopencv_core452 -lopencv_highgui452 -lopencv_imgcodecs452 -lopencv_imgproc452 -o main.exe
```

### Запуск:
```bash
mpiexec -n 4 main.exe
```
Где `4` - количество процессов MPI.

### Вывод
```
mpiexec -n 4 main.exe
Reference computation took 0.127253 seconds
Parallel computation completed in 0.0732577 seconds
```