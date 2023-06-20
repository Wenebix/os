#include <iostream>
#include <vector>
#include <windows.h>
#include <conio.h>
using namespace std;

int main() {
    char lpszComLine[1000];// Создаем массив управляющих символов командной строки
    HANDLE hEnableRead;// Создаем дескриптор для события чтения и записываем его в память
    char lpszEnableRead[] = "ReadEvent";
    STARTUPINFO si;// Создаем структуры для информации о запуске процесса и о самом процессе
    PROCESS_INFORMATION pi;
    // Создаем два канала (для чтения и записи) для обмена информацией между двумя процессами, используя безопасный доступ
    HANDLE hWritePipe1, hReadPipe1;
    HANDLE hWritePipe2, hReadPipe2;
    SECURITY_ATTRIBUTES sa1;
    SECURITY_ATTRIBUTES sa2;
    // Создаем событие чтения, которое сигнализирует о готовности процесса к чтению информации
    hEnableRead = CreateEvent(NULL, FALSE, FALSE, lpszEnableRead);
    // Задаем атрибуты безопасности дескрипторов и связываем их с каналами
    sa1.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa1.lpSecurityDescriptor = NULL;
    sa1.bInheritHandle = TRUE;
    sa2.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa2.lpSecurityDescriptor = NULL;
    sa2.bInheritHandle = TRUE;
    // Создаем два канала для чтения и записи информации
// Если создание каналов не удалось, выводим сообщение об ошибке и завершаем программу
    if (!CreatePipe(&hReadPipe1,&hWritePipe1,&sa1,0)) {
        _cputs("Create pipe failed.\n");
        _cputs("Press any key to finish.\n");
        _getch();
        return GetLastError();
    }
    if (!CreatePipe(&hReadPipe2,&hWritePipe2,&sa2,0)) {
        _cputs("Create pipe failed.\n");
        _cputs("Press any key to finish.\n");
        _getch();
        return GetLastError();
    }
    // Обнуляем структуру STARTUPINFO и заполняем массив lpszComLine строкой с командой запуска второго процесса и аргументами
    ZeroMemory(&si, sizeof(STARTUPINFO));
   // D:\studing\course2\osi\lab5_new\search\cmake-build-debug
    wsprintf(lpszComLine, "D:\\studing\\course2\\osi\\lab5_new\\search\\cmake-build-debug\\search.exe %d %d %d %d",
             (intptr_t) hWritePipe1, (intptr_t) hReadPipe1, (intptr_t) hWritePipe2, (intptr_t) hReadPipe2);
    // Запрашиваем у пользователя размеры массивов и заполняем их элементами
    cout  << "Enter size for the first array: ";
    int size1;
    cin >> size1;

    int size2;
    cout  << "Enter size for the second array: ";
    cin >> size2;

    vector<int> array1(size1);
    vector<__int16> array2(size1);
    cout  << "Enter elements for the first array: \n";
    for (int i = 0; i < size1; i++){
        cin >> array1[i];
    }

    cout  << "Enter elements for the second array: \n";
    for (int i = 0; i < size2; i++){
        cin >> array2[i];
    }

    // Создаем второй процесс
    if (!CreateProcess(
            NULL, // имя процесса
            lpszComLine, // командная строка
            NULL, // атрибуты защиты процесса по умолчанию
            NULL, // атрибуты защиты первичного потока по умолчанию
            TRUE, // наследуемые дескрипторы текущего процесса
// наследуются новым процессом
            CREATE_NEW_CONSOLE, // новая консоль
            NULL, // используем среду окружения процесса предка
            NULL, // текущий диск и каталог, как и в процессе предке
            &si, // вид главного окна - по умолчанию
            &pi // здесь будут дескрипторы и идентификаторы
// нового процесса и его первичного потока
    )) {
        _cputs("Create process failed.\n");
        _cputs("Press any key to finish.\n");
        _getch();
        return GetLastError();
    }
// закрываем дескрипторы нового процесса
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    //_cputs("Press any key to start communication.\n");
    //_getch();
    DWORD dwBytesWritten;
    //добавляем второму процессу размер первого массива
    WriteFile(hWritePipe1, &size1 ,sizeof(size1), &dwBytesWritten ,NULL);
    //заисываем первый массив
    for (int j = 0; j < array1.size(); j++) {
        if (!WriteFile(
                hWritePipe1,
                &array1[j],
                sizeof(array1[j]),
                &dwBytesWritten,
                NULL)) {
            _cputs("Write to file failed.\n");
            _cputs("Press any key to finish.\n");
            _getch();
            return GetLastError();
        }
    }
    //тоже самое со вторым массивом
    WriteFile(hWritePipe1, &size2,sizeof(size2), &dwBytesWritten ,NULL);
    //заисываем первый массив
    for (int j = 0; j < array2.size(); j++) {
        if (!WriteFile(
                hWritePipe1,
                &array2[j],
                sizeof(array2[j]),
                &dwBytesWritten,
                NULL)) {
            _cputs("Write to file failed.\n");
            _cputs("Press any key to finish.\n");
            _getch();
            return GetLastError();
        }
    }
    WaitForSingleObject(hEnableRead, INFINITE); // ждем своей очереди на чтение из аннонимного канала.
    DWORD dwBytesRead;
    int result_size;
    ReadFile(hReadPipe2,&result_size,sizeof(result_size),&dwBytesRead,NULL);
    vector<int> result;
    for (int j = 0; j < result_size; j++) {
        int nData;
        if (!ReadFile(
                hReadPipe2,
                &nData,
                sizeof(nData),
                &dwBytesRead,
                NULL)) {
            _cputs("Read from the pipe failed.\n");
            _cputs("Press any key to finish.\n");
            _getch();
            return GetLastError();
        }
        result.push_back(nData);
    }

    cout << "Result: ";
    for (int i = 0; i < result.size(); i++){
        cout << result[i] << " ";
    }
    cout << endl;
    //закрываем дескрипторы
    CloseHandle(hReadPipe1);
    CloseHandle(hWritePipe1);
    CloseHandle(hReadPipe2);
    CloseHandle(hWritePipe2);
    CloseHandle(hEnableRead);
    _cputs("The process finished writing to the pipe.\n");
    _cputs("Press any key to exit.\n");
    _getch();
    return 0;
}
