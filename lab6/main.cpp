#include <iostream>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>

using namespace std;

pthread_mutex_t lock; // объявление мьютекса
pthread_cond_t work_event; // объявление условной переменной
int* mas; // объявление указателя на массив
int count = 0; // объявление счетчика
int a = 0, k1 = 0, k2 = 0; // объявление переменных
int array_size = 0; // объявление размера массива
int sum = 0; // объявление переменной для хранения суммы элементов массива

// Функция потока, которая обрабатывает элементы массива
void* WorkFunction(void*) {
    int sleep_time;
    cout << "Enter sleep time:"; // вывод на экран сообщения о вводе времени задержки
    cin >> sleep_time; // считывание времени задержки
    int* temp_array = new int[array_size]; // выделение памяти для временного массива
    fill(temp_array, temp_array + array_size, 0); // заполнение временного массива нулями
    int k = 0; // объявление переменной для хранения количества обработанных элементов
    int handled_count = 0; // объявление переменной для хранения количества обработанных элементов
    for (int i = 0; i < array_size; i++){
        if (mas[i] > a){ // если текущий элемент массива больше a, то добавляем его во временный массив
            temp_array[k] = mas[i];
            k++;
            usleep(sleep_time * 1000); // задержка на указанное время
        }
    }
    for (int i = 0; i < array_size; i++){
        if (handled_count == k2){ // если количество обработанных элементов равно k2, то отправляем сигнал на выполнение условной переменной
            pthread_cond_signal(&work_event);
            pthread_mutex_lock(&lock);
            pthread_cond_wait(&work_event, &lock); // блокируем поток, ожидая выполнения условной переменной
            pthread_mutex_unlock(&lock); // разблокируем мьютекс после выполнения условной переменной
        }
        mas[i] = temp_array[i]; // перезаписываем элементы из временного массива в основной массив
        handled_count++; // увеличиваем счетчик обработанных элементов
    }
}

// Функция потока, которая вычисляет сумму элементов массива
void* SumElementFunction(void*) {
    pthread_mutex_lock(&lock); // блокируем мьютекс
    pthread_cond_wait(&work_event, &lock); // ожидаем выполнения условной переменной
    for (int i = k1; i < k2; i++){
        sum += mas[i]; // суммируем элементы массива в заданном диапазоне
    }
    pthread_mutex_unlock(&lock); // разблокируем мьютекс
    return 0;
}

int main() {
    pthread_mutex_init(&lock, NULL); // инициализация мьютекса
    pthread_cond_init(&work_event, NULL); // инициализация условной переменной

    cout << "Enter size of array: ";
    cin >> array_size;
    mas = new int[array_size]; // выделение памяти для массива
    for (int i = 0; i < array_size; i++) {
        cin >> mas[i];
    }
    cout << "Array's size: " << array_size << endl;
    for (int i = 0; i < array_size; i++) {
        cout << mas[i] << " ";
    }
    cout << endl;

    cout << "Enter a: ";
    cin >> a;
    cout << "Enter k1: ";
    cin >> k1;
    cout << "Enter k2: ";
    cin >> k2;

    pthread_t worker; // объявление потока для обработки элементов массива
    pthread_create(&worker, NULL, WorkFunction, NULL); // создание потока для обработки элементов массива

    pthread_t sum_element; // объявление потока для вычисления суммы элементов массива
    pthread_create(&sum_element, NULL, SumElementFunction, NULL); // создание потока для вычисления суммы элементов массива

    pthread_cond_wait(&work_event, &lock);

    cout << "Resulting array from main thread to k2: ";

    for (int i = 0; i < k2; i++) {
        cout << mas[i] << " ";
    }

    cout << endl;

    cout << "Resulting sum: " << sum << endl;
}