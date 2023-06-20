#include <iostream>
#include <windows.h>
#include <vector>
#include <conio.h>
#include <algorithm>
using namespace  std;

void removeDuplicates(std::vector<int>& array) {
    sort(array.begin(), array.end());
    array.erase(unique(array.begin(), array.end()), array.end());
}

// Функция для удаления элементов из массивов, которые не присутствуют в другом массиве
void removeMismatchedElements(std::vector<int>& array1, std::vector<__int16>& array2) {
    // Удаление дублирующих элементов

    // Удаляем элементы из array1, которых нет в array2
    array1.erase(std::remove_if(array1.begin(), array1.end(), [&](int num) {
        return std::find(array2.begin(), array2.end(), num) == array2.end();
    }), array1.end());

    // Удаляем элементы из array2, которых нет в array1
    array2.erase(std::remove_if(array2.begin(), array2.end(), [&](__int16 num) {
        return std::find(array1.begin(), array1.end(), num) == array1.end();
    }), array2.end());
}
// Функция для обработки информации, переданной через каналы
int main(int argc, char *argv[]) {
    // Создаем дескрипторы каналов и события чтения
    HANDLE hWritePipe1, hReadPipe1;
    HANDLE hWritePipe2, hReadPipe2;
    HANDLE hEnableRead;
    char lpszEnableRead[] = "ReadEvent";
    // Открываем существующее событие чтения
    hEnableRead = OpenEvent(EVENT_ALL_ACCESS, FALSE, lpszEnableRead);
    // Получаем дескрипторы каналов из аргументов командной строки
    hWritePipe1 = (HANDLE) atoi(argv[1]);
    hReadPipe1 = (HANDLE) atoi(argv[2]);
    hWritePipe2 = (HANDLE) atoi(argv[3]);
    hReadPipe2 = (HANDLE) atoi(argv[4]);
    // Считываем размер и элементы первого массива из канала для чтения
    DWORD dwBytesRead;
    int size1 = 0;
    ReadFile(hReadPipe1,&size1,sizeof(size1),&dwBytesRead,NULL);

    vector<int> vec1;
    for (int j = 0; j < size1; j++) {
        int nData;
        if (!ReadFile(
                hReadPipe1,
                &nData,
                sizeof(nData),
                &dwBytesRead,
                NULL)) {
            _cputs("Read from the pipe failed.\n");
            _cputs("Press any key to finish.\n");
            _getch();
            return GetLastError();
        }
        vec1.push_back(nData);
    }
    // Считываем размер и элементы второго массива из канала для чтения
    int size2 = 0;
    ReadFile(hReadPipe1,&size2,sizeof(size2),&dwBytesRead,NULL);
    vector<__int16> vec2;
    for (int j = 0; j < size2; j++) {
        __int16 nData;
        if (!ReadFile(
                hReadPipe1,
                &nData,
                sizeof(nData),
                &dwBytesRead,
                NULL)) {
            _cputs("Read from the pipe failed.\n");
            _cputs("Press any key to finish.\n");
            _getch();
            return GetLastError();
        }
        vec2.push_back(nData);
    }
    cout << "Size of the first array:" << size1 << endl;
    cout << "First array: ";
    for (int i = 0; i < size1; i++){
        cout << vec1[i] << " ";
    }
    cout << endl;
    cout << "Size of the second array:" << size2 << endl;
    cout << "Second array: ";

    for (int i = 0; i < size2; i++){
        cout << vec2[i] << " ";
    }
    cout << endl;
    // Обрабатываем массивы и получаем результирующий массив
    removeMismatchedElements(vec1, vec2);


    vector<int> result;
    for (int i = 0; i < result.size(); i++){
        cout << result[i] << " ";
    }
    result.insert(result.end(), vec1.begin(), vec1.end());
    result.insert(result.end(), vec2.begin(), vec2.end());
    removeDuplicates(result);
    cout << "Result array: ";
    for (int j = 0; j < result.size(); j++) {
        cout << result[j] << " ";
    }
    cout << endl;
    // Записываем результаты в канал для записи
    DWORD dwBytesWritten;
    int result_size = result.size();
    WriteFile(hWritePipe2, &result_size ,sizeof(result_size), &dwBytesWritten ,NULL);
    for (int j = 0; j < result.size(); j++) {
        if (!WriteFile(
                hWritePipe2,
                &result[j],
                sizeof(result[j]),
                &dwBytesWritten,
                NULL)) {
            _cputs("Write to file failed.\n");
            _cputs("Press any key to finish.\n");
            _getch();
            return GetLastError();
        }
    }
    // Сигнализируем о завершении записи в канал для чтения
    SetEvent(hEnableRead);
    _cputs("The process finished writing to the pipe.\n");
    _cputs("Press any key to finish.\n");
    _getch();
    // Закрываем дескрипторы каналов и события чтения
    CloseHandle(hWritePipe1);
    CloseHandle(hReadPipe1);
    CloseHandle(hWritePipe2);
    CloseHandle(hReadPipe2);
    CloseHandle(hEnableRead);

    return 0;
}
