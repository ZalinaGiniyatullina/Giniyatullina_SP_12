// Giniyatullina_SP_12.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <windows.h>
#include <string>

const int MAX_PROCESSES = 10;

struct ProcessInfo
{
    std::string name;
    DWORD pid;
    HANDLE hProcess;
    bool used;
};

ProcessInfo processes[MAX_PROCESSES];
int processCount = 0;


bool IsProcessRunning(HANDLE hProcess)
{
    if (hProcess == NULL)
    {
        return false;
    }

    DWORD result = WaitForSingleObject(hProcess, 0);
    return (result == WAIT_TIMEOUT);
}


std::string GetStateText(HANDLE hProcess)
{
    if (IsProcessRunning(hProcess))
    {
        return "работает";
    }
    else
    {
        return "завершён";
    }
}


void ReleaseProcess(int index)
{
    if (index < 0 || index >= MAX_PROCESSES)
    {
        return;
    }

    if (processes[index].hProcess != NULL)
    {
        CloseHandle(processes[index].hProcess);
        processes[index].hProcess = NULL;
    }
    processes[index].used = false;
}


bool LaunchProcess(const wchar_t* appPath, const std::wstring& commandLine, const std::string& displayName)
{
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; ++i)
    {
        if (!processes[i].used)
        {
            slot = i;
            break;
        }
    }
    if (slot == -1)
    {
        std::cout << "Ошибка: достигнут лимит процессов (" << MAX_PROCESSES << ")." << "\n";
        return false;
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    wchar_t cmdBuffer[1024];
    wcsncpy_s(cmdBuffer, commandLine.c_str(), _TRUNCATE);


    BOOL ok = CreateProcessW(
        appPath,
        cmdBuffer,
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!ok)
    {
        std::cout << "Не удалось создать процесс. Код ошибки: " << GetLastError() << "\n";
        return false;
    }

    processes[slot].name = displayName;
    processes[slot].pid = pi.dwProcessId;
    processes[slot].hProcess = pi.hProcess;
    processes[slot].used = true;
    processCount++;


    CloseHandle(pi.hThread);

    std::cout << "Процесс (" << displayName << ") запущен.PID = " << pi.dwProcessId << "\n";
    return true;
}


void StartTaskManager()
{
    const wchar_t* path = L"C:\\Windows\\System32\\taskmgr.exe";
    std::wstring cmd = L"\"C:\\Windows\\System32\\taskmgr.exe\"";
    LaunchProcess(path, cmd, "taskmgr.exe");
}


void StartPing()
{
    std::cout << "\nВыберите режим ping:" << "\n";
    std::cout << "1) ping <IP> (4 запроса)" << "\n";
    std::cout << "2) ping -n <N> <IP> (N запросов)" << "\n";
    std::cout << "3) ping -t <IP> (непрерывно)" << "\n";
    std::cout << "Ваш выбор: ";

    int mode;
    std::cin >> mode;

    std::string ip;
    std::cout << "Введите IP-адрес или имя узла: ";
    std::cin >> ip;

    std::wstring cmd = L"\"C:\\Windows\\System32\\ping.exe\"";

    std::wstring wip(ip.begin(), ip.end());

    if (mode == 2)
    {
        int n;
        std::cout << "Введите количество запросов: ";
        std::cin >> n;

        //-n <N> <IP>
        cmd += L" -n ";
        cmd += std::to_wstring(n);
        cmd += L" ";
        cmd += wip;
    }
    else if (mode == 3)
    {
        //-t <IP>
        cmd += L" -t ";
        cmd += wip;
    }
    else
    {
        //<IP>
        cmd += L" ";
        cmd += wip;
    }

    LaunchProcess(L"C:\\Windows\\System32\\ping.exe", cmd, "ping.exe");
}


void StartVariant1()
{
    int a, b;
    std::cout << "Введите начало диапазона: ";
    std::cin >> a;
    std::cout << "Введите конец диапазона:  ";
    std::cin >> b;


    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring dir(exePath);
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        dir = dir.substr(0, pos + 1);
    std::wstring variantPath = dir + L"Variant1.exe";


    std::wstring cmd = L"\"" + variantPath + L"\" " + std::to_wstring(a) + L" " + std::to_wstring(b);

    LaunchProcess(variantPath.c_str(), cmd, "Variant1.exe");
}

//1
void ShowCreateMenu()
{
    std::cout << "\nЧто запустить?" << "\n";
    std::cout << "1) taskmgr.exe (диспетчер задач)" << "\n";
    std::cout << "2) ping.exe" << "\n";
    std::cout << "3) Variant1.exe (программа варианта №1)" << "\n";
    std::cout << "Ваш выбор: ";

    int vibor;
    std::cin >> vibor;

    switch (vibor)
    {
    case 1: StartTaskManager(); break;
    case 2: StartPing(); break;
    case 3: StartVariant1(); break;
    default: std::cout << "Неверный выбор." << "\n"; break;
    }
}


//3
void ShowProcessList()
{
    std::cout << "\n       Список процессов       " << "\n";
    std::cout << "№ / Имя / PID / Состояние" << "\n";

    bool any = false;
    for (int i = 0; i < MAX_PROCESSES; ++i)
    {
        if (!processes[i].used)
            continue;

        any = true;
        std::cout << (i + 1) << " " << processes[i].name << " ";
        std::cout << processes[i].pid << " " << GetStateText(processes[i].hProcess) << "\n";
    }

    if (!any)
        std::cout << "процессы ещё не создавались!" << "\n";
}

//2
void TerminateSelected()
{
    ShowProcessList();

    std::cout << "Введите номер процесса для завершения (0—отмена): ";
    int num;
    std::cin >> num;

    if (num == 0)
        return;

    int index = num - 1;

    if (index < 0 || index >= MAX_PROCESSES || !processes[index].used)
    {
        std::cout << "Неверный номер." << std::endl;
        return;
    }

    if (!IsProcessRunning(processes[index].hProcess))
    {
        std::cout << "Процесс уже завершён." << "\n";
        ReleaseProcess(index);
        processCount--;
        return;
    }


    if (TerminateProcess(processes[index].hProcess, 1))
    {
        std::cout << "Процесс (" << processes[index].name << ") PID (" << processes[index].pid << ") завершён." << "\n";


        WaitForSingleObject(processes[index].hProcess, INFINITE);

        ReleaseProcess(index);
        processCount--;
    }
    else
    {
        std::cout << "Не удалось завершить процесс. Код ошибки: "
            << GetLastError() << "\n";
    }
}

int CountRunning()
{
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; ++i)
    {
        if (processes[i].used && IsProcessRunning(processes[i].hProcess))
        {
            count++;\
        }
    }
    return count;
}

//4
void ShowRunningCount()
{
    std::cout << "Работающих процессов: " << CountRunning() << "\n";
}


void CleanupAll()
{
    for (int i = 0; i < MAX_PROCESSES; ++i)
    {
        if (processes[i].used)
        {
            ReleaseProcess(i);
        }
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");

    for (int i = 0; i < MAX_PROCESSES; ++i)
    {
        processes[i].used = false;
        processes[i].hProcess = NULL;
        processes[i].pid = 0;
    }

    std::cout << "       МЕНЕДЖЕР ПРОЦЕССОВ       " << "\n";

    bool running = true;
    while (running)
    {
        std::cout << "\nМеню:" << "\n";
        std::cout << "1) Создать процесс" << "\n";
        std::cout << "2) Завершить процесс" << "\n";
        std::cout << "3) Показать список процессов" << "\n";
        std::cout << "4) Показать количество работающих процессов" << "\n";
        std::cout << "0) Выход" << "\n";
        std::cout << "Выбор: ";

        int vibor;
        std::cin >> vibor;

        switch (vibor)
        {
        case 1: ShowCreateMenu(); break;
        case 2: TerminateSelected(); break;
        case 3: ShowProcessList(); break;
        case 4: ShowRunningCount(); break;
        case 0: running = false; break;
        default: std::cout << "Неверный пункт меню." << "\n"; break;
        }
    }

    CleanupAll();
    std::cout << "Выход" << "\n";
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
