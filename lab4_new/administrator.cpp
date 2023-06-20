#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#pragma comment(lib, "kernel32.lib")
#define POSSIBLE_MESSAGES 2// Определяем константу для количества возможных сообщений
using namespace std;
// Объявляем функцию для создания процесса Reader
bool createReaderProcess(int i, int nReaderMsg,
                         vector<STARTUPINFOW*>& siReader, vector<PROCESS_INFORMATION*>& piReader);

int main()
{   // Создаем дескрипторы мьютекса и семафора
    HANDLE hMutex = NULL;
    hMutex = CreateMutexW(NULL, FALSE, L"Mutex");
    if (hMutex == NULL)
    {
        cout << "Create mutex failed." << endl;
        cout << "Press any key to exit." << endl;
        cin.get();
        return GetLastError();
    }

    HANDLE hSemaphore = NULL;
    hSemaphore = CreateSemaphoreW(NULL, 1,1,  L"Semaphore");
    if (hSemaphore == NULL)
    {
        cout << "Create semaphore failed." << endl;
        cout << "Press any key to exit." << endl;
        cin.get();
        return GetLastError();
    }
    // Создаем переменные для хранения количества процессов Writer, Reader и сообщений
    int nWriterPr;
    int nReaderPr;
    int nSumPr;
    int nWriterMsg;
    int nReaderMsg;
    int nSumMsgTmp = 0;
    cout << "Enter the number of Writer processes: ";
    cin >> nWriterPr;
    cout << "Enter the number of Reader processes: ";
    cin >> nReaderPr;

    nSumPr = nReaderPr + nWriterPr;
// Создаем массив дескрипторов событий
    HANDLE* hEvents = new HANDLE[POSSIBLE_MESSAGES + nSumPr];
    // Создаем события A и B
    hEvents[0] = CreateEventW(NULL, FALSE, FALSE, L"Event A");
    hEvents[1] = CreateEventW(NULL, FALSE, FALSE, L"Event B");
    HANDLE hEventC = CreateEventW(NULL, FALSE, FALSE, L"Event C");
    HANDLE hEventD = CreateEventW(NULL, FALSE, FALSE, L"Event D");

// Создаем векторы для хранения информации о процессах Writer и Reader
    vector<STARTUPINFOW*> siWriter(nWriterPr);
    vector<STARTUPINFOW*> siReader(nReaderPr);
    vector<PROCESS_INFORMATION*> piWriter(nWriterPr);
    vector<PROCESS_INFORMATION*> piReader(nReaderPr);
    // Создаем переменные для хранения информации о командной строке
    wstring temp;
    wchar_t* lpszCommandLine;
    // Создаем процессы Writer
    for (int i = 0; i < nWriterPr; i++)
    {   // Создаем событие для каждого процесса Writer
        hEvents[i + POSSIBLE_MESSAGES] = CreateEventW(NULL, FALSE, FALSE, L"WriterEndedEvent " + (i + 1));
        if (hEvents[i + POSSIBLE_MESSAGES] == NULL)
        {
            return GetLastError();
        }
        // Создаем структуру STARTUPINFO для процесса Writer
        siWriter[i] = new STARTUPINFOW;
        ZeroMemory(siWriter[i], sizeof(STARTUPINFOW));
        siWriter[i]->cb = sizeof(STARTUPINFOW);
        // Создаем структуру PROCESS_INFORMATION для процесса Writer
        piWriter[i] = new PROCESS_INFORMATION;
        cout << "Enter the number of messages from Writer " << i + 1 << ": ";
        cin >> nWriterMsg;
        // Увеличиваем счетчик общего количества сообщений
        nSumMsgTmp += nWriterMsg;
        // Формируем командную строку для запуска процесса Writer
        temp = to_wstring(nWriterMsg) + L" " + to_wstring(i + 1);
        lpszCommandLine = new wchar_t[temp.length()];
        wcscpy_s(lpszCommandLine, temp.length() + 1, temp.c_str());
        // Создаем процесс Writer
        if (!CreateProcessW(L"writer.exe", lpszCommandLine, NULL, NULL, FALSE,
                            CREATE_NEW_CONSOLE, nullptr, NULL, siWriter[i], piWriter[i]))
        {
            cout << "The new process is not created." << endl;
            cout << "Press any key to exit." << endl;
            cin.get();
            return GetLastError();
        }
    }

    int nSumMsg = nSumMsgTmp;// Сохраняем общее количество сообщений
    // Создаем процессы Reader
    for (int i = 0; i < nReaderPr; i++)
    {   // Создаем событие для каждого процесса Reader
        hEvents[i + POSSIBLE_MESSAGES + nWriterPr] = CreateEventW(NULL, FALSE, FALSE, L"ReaderEndedEvent " + (i + 1));

        if (hEvents[i + POSSIBLE_MESSAGES + nWriterPr] == NULL)
        {
            return GetLastError();
        }
        // Если остались непрочитанные сообщения
        if (nSumMsgTmp > 0)
        { do// Считываем количество сообщений для Reader с консоли
            { cout << "Enter the number of received messages for Reader " << i + 1 << " (max: " << nSumMsgTmp << "): ";
                cin >> nReaderMsg;

                if (nReaderMsg <= nSumMsgTmp)
                {
                    break;
                }

                cout << "Incorrect number. Try again\n";
            } while (true);
            nSumMsgTmp -= nReaderMsg;// Уменьшаем счетчик непрочитанных сообщений
            // Создаем процесс Reader
            if (!createReaderProcess(i, nReaderMsg, siReader, piReader))
            {
                return GetLastError();
            }

        }// Если все сообщения уже прочитаны
        else
        { // Создаем процесс Reader без сообщений
            if (!createReaderProcess(i, 0, siReader, piReader))
            {
                return GetLastError();
            }
        }

    }
    // Если остались непрочитанные сообщения, выводим их количество на консоль
    if (nSumMsgTmp > 0)
    {
        cout << "The number of received messages for Reader " << nReaderPr << ": " << nSumMsgTmp << "\n";
    }
    // Создаем событие для последнего процесса Reader
    hEvents[POSSIBLE_MESSAGES + nSumPr - 1] = CreateEventW(NULL, FALSE, FALSE, L"ReaderEndedEvent " + nReaderPr);
    if (hEvents[POSSIBLE_MESSAGES + nSumPr - 1] == NULL)
    {
        return GetLastError();
    }
    // Создаем процесс Reader для текущего процесса
    if (!createReaderProcess(nReaderPr - 1, nSumMsgTmp, siReader, piReader))
    {
        return GetLastError();
    }
    // Ожидаем завершения всех процессов и событий
    for (int i = 0; i < nSumMsg + nSumPr; i++)
    {
        int eventIndex = WaitForMultipleObjects(POSSIBLE_MESSAGES + nSumPr, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
        if (eventIndex < POSSIBLE_MESSAGES)
        {   // Если событие A, выводим сообщение и устанавливаем событие C
            if (eventIndex == 0)
            {
                cout << "Message A";
                SetEvent(hEventC);
            }
            else// Если событие B, выводим сообщение и устанавливаем событие D
            {
                cout << "Message B";
                SetEvent(hEventD);
            }
        }
        else
        {   // Если завершился процесс Writer, выводим сообщение об этом
            if (eventIndex < POSSIBLE_MESSAGES + nWriterPr)
            {
                cout << "Writer " << eventIndex - POSSIBLE_MESSAGES + 1 << " ended";
            }
            else// Если завершился процесс Reader, выводим сообщение об этом
            {
                cout << "Reader " << eventIndex - POSSIBLE_MESSAGES - nWriterPr + 1 << " ended";
            }
        }

        cout << "\n";
    }
    // Ожидаем завершения всех процессов Writer и освобождаем ресурсы
    for (int i = 0; i < nWriterPr; i++)
    {
        WaitForSingleObject(piWriter[i]->hProcess, INFINITE);
        CloseHandle(piWriter[i]->hThread);
        CloseHandle(piWriter[i]->hProcess);
        CloseHandle(hEvents[i + POSSIBLE_MESSAGES]);
        delete siWriter[i];
        delete piWriter[i];
    }
    // Ожидаем завершения всех процессов Reader и освобождаем ресурсы
    for (int i = 0; i < nReaderPr; i++)
    {
        WaitForSingleObject(piReader[i]->hProcess, INFINITE);
        CloseHandle(piReader[i]->hThread);
        CloseHandle(piReader[i]->hProcess);
        CloseHandle(hEvents[i + POSSIBLE_MESSAGES + nWriterPr]);
        delete siReader[i];
        delete piReader[i];
    }
    // Освобождаем память и ресурсы
    delete[] hEvents;
    CloseHandle(hMutex);
    CloseHandle(hSemaphore);
    return 0;
}
// Создает процесс Reader и передает ему необходимые данные через командную строку
bool createReaderProcess(int i, int nReaderMsg,
                         vector<STARTUPINFOW*>& siReader, vector<PROCESS_INFORMATION*>& piReader)
{
    wstring temp;
    wchar_t* lpszCommandLine;
    // Создаем структуру STARTUPINFO для нового процесса Reader
    siReader[i] = new STARTUPINFOW;
    ZeroMemory(siReader[i], sizeof(STARTUPINFOW));
    siReader[i]->cb = sizeof(STARTUPINFOW);
    // Создаем структуру PROCESS_INFORMATION для нового процесса Reader
    piReader[i] = new PROCESS_INFORMATION;
    // Создаем командную строку для запуска нового процесса Reader
    temp = to_wstring(nReaderMsg) + L" " + to_wstring(i + 1);
    lpszCommandLine = new wchar_t[temp.length()];
    wcscpy_s(lpszCommandLine, temp.length() + 1, temp.c_str());
    // Создаем процесс Reader и передаем ему командную строку
    if (!CreateProcessW(L"reader.exe", lpszCommandLine, NULL, NULL, FALSE,
                        CREATE_NEW_CONSOLE, nullptr, NULL, siReader[i], piReader[i]))
    {
        cout << "The new process is not created." << endl;
        cout << "Press any key to exit." << endl;
        cin.get();
        return false;
    }

    return true;
}