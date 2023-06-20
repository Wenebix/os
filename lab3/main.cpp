#include <iostream>
#include <windows.h>
#include <algorithm>
#include <iterator>
using namespace std;

HANDLE work_event; // Объявление дескрипторов событий
int* mas; // Динамический массив
CRITICAL_SECTION cs; // Критическая секция для синхронизации потоков
int a = 0, k1 = 0, k2 = 0; // Переменные для работы с массивом
int array_size = 0; // Размер массива
int sum = 0; // Сумма элементов массива, больших чем a

// Функция, выполняемая рабочим потоком
DWORD WINAPI WorkFunction(LPVOID) {
    int sleep_time;
    cout << "Enter sleep time:"; // Просьба ввести время задержки
    cin >> sleep_time; // Считывание времени задержки
    int* temp_array = new int[array_size];
    fill(temp_array, temp_array + array_size, 0); // Инициализация массива нулями
    int k = 0;
    int handled_count = 0;
    for (int i = 0; i < array_size; i++){
        if (mas[i] > a){
            temp_array[k] = mas[i];
            k++;
            Sleep(sleep_time); // Задержка на указанное время
        }
    }
    for (int i = 0; i < array_size; i++){
        if (i == k2){
            SetEvent(work_event); // Установка события для сигнализации потоку подсчета суммы
        }
        mas[i] = temp_array[i]; // Копирование массива temp_array обратно в mas
    }
}

// Функция, выполняемая потоком подсчета суммы
DWORD WINAPI SumElementFunction(LPVOID) {
    EnterCriticalSection(&cs); // Вход в критическую секцию
    DWORD dwWaitResult = WaitForSingleObject(work_event, INFINITE); // Ожидание установки события рабочим потоком
    //ResetEvent(work_event);
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        // Подсчет суммы элементов в диапазоне [k1,k2)
        for (int i = k1; i < k2; i++){
            sum += mas[i];
        }
    }
    LeaveCriticalSection(&cs); // Выход из критической секции

    return 0;
}

int main() {

    InitializeCriticalSection(&cs); // Инициализация критической секции
    cout << "Enter size of array: ";
    cin >> array_size;
    mas = new int [array_size];
    for (int i = 0; i < array_size; i++){
        cin >> mas[i];
    }
    cout << "Array's size: " << array_size << endl;
    for (int i = 0; i < array_size; i++){
        cout << mas[i] << " ";
    }
    cout << endl;
    DWORD workerid;
    work_event = CreateEvent(NULL, TRUE, FALSE, NULL); // Создание события с ручным сбросом
    cout << "Enter a: ";
    cin >> a;
    cout << "Enter k1: ";
    cin >> k1;
    cout << "Enter k2: ";
    cin>> k2;
    HANDLE worker = CreateThread(NULL, 0, &WorkFunction, NULL, NULL, &workerid); // Создание рабочего потока
    DWORD sum_element_id;
    HANDLE sum_element = CreateThread(NULL, 0, &SumElementFunction, NULL, NULL, &sum_element_id); // Создание потока подсчета суммы
    if (worker == NULL || sum_element == NULL){
        return GetLastError(); // Завершение программы, если не удалось создать потоки
    }
    //  ResumeThread(worker);
    //  ResumeThread(sum_element);
    WaitForSingleObject(work_event, INFINITE); // Ожидание установки события рабочим потоком
    cout << "Resulting array from main thread to k2: " ;

    for (int i = 0; i < k2; i++){
        cout << mas[i] << " ";
    }
    cout << endl;

    EnterCriticalSection(&cs); // Вход в критическую секцию
    LeaveCriticalSection(&cs);
    cout << "Resulting sum: " << sum <<  endl;

    WaitForSingleObject(worker, INFINITE); // Ожидание завершения работы рабочего потока
    for (int i = k2; i < array_size; i++){
        cout << mas[i] << " ";
    }
    DeleteCriticalSection(&cs); // Удаление критической секции
    CloseHandle(worker); // Закрытие дескриптора рабочего потока
    CloseHandle(sum_element); // Закрытие дескриптора потока подсчета суммы
    CloseHandle(work_event); // Закрытие дескриптора события с ручным сбросом
    return 0;
}