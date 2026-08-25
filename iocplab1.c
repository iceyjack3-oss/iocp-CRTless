#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>

HANDLE co = NULL;
LPFN_ACCEPTEX paex = NULL;
SOCKET lst = INVALID_SOCKET;
SRWLOCK wlock;

typedef struct
{
    OVERLAPPED la;
    SOCKET clnts;
    char bf[1024];
} pod;

void logerr(const char *msg)
{

    DWORD len = 0;
    while (msg[len] != '\0')
    {
        len++;
    }

    HANDLE hf = CreateFileA("C:\\Public\\server_log.txt", FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hf != INVALID_HANDLE_VALUE)
    {
        DWORD bytwrt;
        WriteFile(hf, msg, len, &bytwrt, NULL);
        CloseHandle(hf);
    }
}

DWORD WINAPI dmp(LPVOID lpParam)
{

    DWORD bytrcv;
    ULONG_PTR skp;
    LPOVERLAPPED lap;

    HANDLE hC = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD bytwr;

    while (1)
    {
        
        GetQueuedCompletionStatus(co, &bytrcv, &skp, &lap, INFINITE);

            pod *iod = (pod *)lap;

            if (skp == lst)
            {

                CreateIoCompletionPort((HANDLE)iod->clnts, co, (ULONG_PTR)iod->clnts, 0);

                WSABUF wbf;
                wbf.buf = iod->bf;
                wbf.len = 1024 - 32;
                DWORD fl = 0;
                DWORD tbyt = 0;

                WSARecv(iod->clnts, &wbf, 1, &tbyt, &fl, &(iod->la), NULL);

                SOCKET nsx = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
                HANDLE h = GetProcessHeap();
                pod *niod = (pod *)HeapAlloc(h, HEAP_ZERO_MEMORY, sizeof(pod));
                niod->clnts = nsx;

                DWORD dyb = 0;
                DWORD adln = sizeof(SOCKADDR_IN) + 16;
                paex(skp, niod->clnts, niod->bf, 0, adln, adln, &dyb, &(niod->la));
            }

            else if (bytrcv > 0)
            {

                iod->bf[bytrcv] = '\0';

                AcquireSRWLockExclusive(&wlock);

                WriteConsoleA(hC, iod->bf, bytrcv, &bytwr, NULL);
                ReleaseSRWLockExclusive(&wlock);

                SecureZeroMemory(&(iod->la), sizeof(iod->la));

                WSABUF wbf;
                wbf.buf = iod->bf;
                wbf.len = 1024 - 32;

                DWORD fl = 0;
                DWORD tbyt = 0;

                WSARecv(iod->clnts, &wbf, 1, &tbyt, &fl, &(iod->la), NULL);
            }
        else if (bytrcv == 0 && skp != lst)
        {
            logerr("client disconnected\n");

            pod *iod = (pod *)lap;

            closesocket(iod->clnts);
            HeapFree(GetProcessHeap(), 0, iod);
        }
    }
}

void entry(void)
{

    co = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    WSADATA wsaData;
    WSAStartup(0x0202, &wsaData);

    lst = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (lst == INVALID_SOCKET)
    {
        logerr("fail socket creation\n");
        ExitProcess(1);
    }

    struct sockaddr_in sdr;
    sdr.sin_family = AF_INET;
    sdr.sin_port = htons(8080);
    sdr.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(lst, (struct sockaddr *)&sdr, sizeof(sdr)) == SOCKET_ERROR)
    {
        logerr("fail bind\n");
        ExitProcess(1);
    }
    if (listen(lst, SOMAXCONN) == SOCKET_ERROR)
    {
        logerr("fail listen\n");
        ExitProcess(1);
    }

    InitializeSRWLock(&wlock);

    for (int i = 0; i < 8; i++)
    {

        HANDLE hthrd = CreateThread(NULL, 0, dmp, (LPVOID)co, 0, NULL);
        if (hthrd)
        {
            CloseHandle(hthrd);
        }
    }
    GUID gex = WSAID_ACCEPTEX;
    DWORD dbyw = 0;

    WSAIoctl(lst, SIO_GET_EXTENSION_FUNCTION_POINTER, &gex, sizeof(gex), &paex, sizeof(paex), &dbyw, NULL, NULL);

    CreateIoCompletionPort((HANDLE)lst, co, (ULONG_PTR)lst, 0);

    HANDLE ho = GetProcessHeap();
    for (int j = 0; j < 8000; j++)
    {
        pod *iod = (pod *)HeapAlloc(ho, HEAP_ZERO_MEMORY, sizeof(pod));

        iod->clnts = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

        DWORD bytrtrn;
        DWORD adln = sizeof(SOCKADDR_IN) + 16;
        paex(lst, iod->clnts, iod->bf, 0, adln, adln, &bytrtrn, &(iod->la));
    }

    Sleep(INFINITE);

    ExitProcess(0);
}
