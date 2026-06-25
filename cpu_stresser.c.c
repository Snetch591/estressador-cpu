#include <windows.h>
#include <stdio.h>
#include <math.h>

/*VARIÁVEIS GLOBAIS
   Essas variáveis são acessadas por múltiplas partes do programa
   (janela principal, threads de estresse e timer).
*/
volatile int stop_flag = 0;           // Flag compartilhada para parar todas as threads
volatile long long operations = 0;    // Contador de operações (aproximação de carga)
HANDLE* threads = NULL;               // Array de handles das threads de estresse
int num_threads = 0;                  // Quantidade de threads em execução
int intensity = 5;                    // Intensidade do estresse (1-10)
int duration = 0;                     // Duração máxima em segundos (0 = infinito)
DWORD start_time = 0;                 // Momento em que o estresse começou
DWORD end_time = 0;                   // Momento em que o estresse deve terminar

/* Handles dos controles da interface gráfica */
HWND hEditThreads, hEditIntensity, hEditDuration;
HWND hBtnStart, hBtnStop, hStatusLabel, hLog;

/*ESTRUTURA DE DADOS */
typedef struct {
    int intensity;     // Intensidade passada para cada thread
    int thread_id;
} ThreadData;


/*FUNÇÃO PRINCIPAL DE ESTRESSE
   Esta é a função executada por cada thread.
   Ela realiza cálculos matemáticos intensos para consumir CPU.
*/
DWORD WINAPI stress_worker(LPVOID arg) {
    ThreadData* data = (ThreadData*)arg;
    int intensity = data->intensity;
    volatile double x = 0.0;        // volatile impede que o compilador otimize demais o loop

    while (!stop_flag) {
        // Loop interno: quanto maior a intensity, mais pesado o trabalho
        for (long i = 0; i < 100000 * intensity; i++) {
            // Operações matemáticas variadas para maximizar uso da CPU
            x += sin(i) * cos(i) + sqrt(i + 1.0) * tan(i * 0.001);
            x = x * 0.999 + 1.0;   // Evita que o compilador simplifique o cálculo
        }
        operations += 100000LL * intensity;  // Atualiza contador global
    }
    return 0;
}


/*ATUALIZAÇÃO DO STATUS
   Atualiza o label de status a cada 300ms com informações em tempo real.
*/
void UpdateStatus(HWND hwnd) {
    char buffer[300];
    DWORD elapsed = (GetTickCount() - start_time) / 1000;
    int remaining = 0;

    if (duration > 0) {
        remaining = (int)((end_time - GetTickCount()) / 1000);
        if (remaining < 0) remaining = 0;

        sprintf(buffer, "Threads: %d | Intensidade: %d | Ops: %lld | Tempo: %lus | Restante: %ds",
                num_threads, intensity, operations, elapsed, remaining);
    } else {
        sprintf(buffer, "Threads: %d | Intensidade: %d | Ops: %lld | Tempo: %lus (Infinito)",
                num_threads, intensity, operations, elapsed);
    }

    SetWindowText(hStatusLabel, buffer);
}


/*FUNÇÃO PARA PARAR O ESTRESSE
   Função reutilizável para encerrar todas as threads de forma segura.
*/
void StopStress(HWND hwnd) {
    if (threads == NULL) return;

    stop_flag = 1;                    // Sinaliza para todas as threads pararem

    // Aguarda o término de cada thread
    for (int i = 0; i < num_threads; i++) {
        if (threads[i]) {
            WaitForSingleObject(threads[i], 1500);  // Timeout de 1.5s
            CloseHandle(threads[i]);
        }
    }

    free(threads);
    threads = NULL;

    // Restaura botões da interface
    EnableWindow(hBtnStart, TRUE);
    EnableWindow(hBtnStop, FALSE);
    SetWindowText(hLog, "Estresse FINALIZADO.\r\n");
}


/*PROCEDIMENTO DA JANELA (Window Procedure)
   Esta função é o "coração" do programa Win32.
   Ela recebe e processa todas as mensagens do Windows (cliques, timer, fechamento, etc).
*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

        case WM_COMMAND:                    // Evento de clique em botões
            if (LOWORD(wParam) == 1) {      // Botão INICIAR ESTRESSE
                if (threads != NULL) return 0;  // Já está rodando

                //LEITURA DAS ENTRADAS
                char buf[32];
                GetWindowText(hEditThreads, buf, 32);
                num_threads = atoi(buf);

                GetWindowText(hEditIntensity, buf, 32);
                intensity = atoi(buf);

                GetWindowText(hEditDuration, buf, 32);
                duration = atoi(buf);

                //VALIDAÇÕES
                SYSTEM_INFO sysinfo;
                GetSystemInfo(&sysinfo);

                if (num_threads < 1) num_threads = 1;
                if (num_threads > sysinfo.dwNumberOfProcessors)
                    num_threads = sysinfo.dwNumberOfProcessors;

                if (intensity < 1) intensity = 1;
                if (intensity > 10) intensity = 10;
                if (duration < 0) duration = 0;

                // Atualiza os campos com valores validados
                sprintf(buf, "%d", num_threads);   SetWindowText(hEditThreads, buf);
                sprintf(buf, "%d", intensity);     SetWindowText(hEditIntensity, buf);
                sprintf(buf, "%d", duration);      SetWindowText(hEditDuration, buf);

                //INICIALIZAÇÃO DO ESTRESSE
                stop_flag = 0;
                operations = 0;
                start_time = GetTickCount();

                if (duration > 0)
                    end_time = start_time + (duration * 1000);
                else
                    end_time = 0;

                // Aloca memória para as threads
                threads = (HANDLE*)malloc(num_threads * sizeof(HANDLE));
                ThreadData* data = (ThreadData*)malloc(num_threads * sizeof(ThreadData));

                // Cria as threads de estresse
                for (int i = 0; i < num_threads; i++) {
                    data[i].intensity = intensity;
                    data[i].thread_id = i;
                    threads[i] = CreateThread(NULL, 0, stress_worker, &data[i], 0, NULL);
                }

                // Atualiza interface
                EnableWindow(hBtnStart, FALSE);
                EnableWindow(hBtnStop, TRUE);
                SetWindowText(hLog, "Estresse INICIADO...\r\n");

            }
            else if (LOWORD(wParam) == 2) {   // Botão PARAR ESTRESSE
                StopStress(hwnd);
            }
            break;

        case WM_TIMER:                        // Timer para atualização periódica
            if (wParam == 1) {
                UpdateStatus(hwnd);

                // Verifica se o tempo de duração acabou
                if (duration > 0 && GetTickCount() >= end_time) {
                    SetWindowText(hLog, "Tempo máximo atingido. Estresse finalizado automaticamente.\r\n");
                    StopStress(hwnd);
                }
            }
            break;

        case WM_DESTROY:                      // Usuário fechou a janela
            stop_flag = 1;
            if (threads) {
                StopStress(hwnd);
            }
            PostQuitMessage(0);
            return 0;
    }

    // Passa mensagens não tratadas para o Windows processar
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


/*PONTO DE ENTRADA DO PROGRAMA (WinMain)
   Função principal para aplicações Windows com interface gráfica.
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    const char CLASS_NAME[] = "CPUStresserClass";

    //REGISTRO DA CLASSE DA JANELA
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    //CRIAÇÃO DA JANELA PRINCIPAL
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Estressador de CPU - Win32",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 500,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    //CRIAÇÃO DOS CONTROLES (Interface)
    CreateWindow("STATIC", "Numero de Threads:", WS_VISIBLE | WS_CHILD, 20, 20, 180, 25, hwnd, NULL, hInstance, NULL);
    hEditThreads = CreateWindow("EDIT", "8", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 210, 20, 100, 25, hwnd, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Intensidade (1-10):", WS_VISIBLE | WS_CHILD, 20, 60, 180, 25, hwnd, NULL, hInstance, NULL);
    hEditIntensity = CreateWindow("EDIT", "5", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 210, 60, 100, 25, hwnd, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Duracao (segundos, 0=infinito):", WS_VISIBLE | WS_CHILD, 20, 100, 230, 25, hwnd, NULL, hInstance, NULL);
    hEditDuration = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 260, 100, 100, 25, hwnd, NULL, hInstance, NULL);

    hBtnStart = CreateWindow("BUTTON", "INICIAR ESTRESSE", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 20, 150, 190, 45, hwnd, (HMENU)1, hInstance, NULL);
    hBtnStop  = CreateWindow("BUTTON", "PARAR ESTRESSE",  WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 230, 150, 190, 45, hwnd, (HMENU)2, hInstance, NULL);

    hStatusLabel = CreateWindow("STATIC", "Status: Aguardando...", WS_VISIBLE | WS_CHILD | SS_SUNKEN, 20, 210, 600, 35, hwnd, NULL, hInstance, NULL);

    CreateWindow("STATIC", "Log:", WS_VISIBLE | WS_CHILD, 20, 260, 100, 20, hwnd, NULL, hInstance, NULL);
    hLog = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                        20, 280, 600, 160, hwnd, NULL, hInstance, NULL);

    EnableWindow(hBtnStop, FALSE);

    // Timer para atualizar status e verificar duração
    SetTimer(hwnd, 1, 300, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    //LOOP DE MENSAGENS
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
