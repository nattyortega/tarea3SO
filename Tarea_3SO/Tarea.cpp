#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

using namespace std;

float SizeHDD;      // Tamaño de memoria física (RAM)
float VMSize;       // Tamaño de memoria virtual
float pageSize;     // Tamaño de cada página
int numeroPaginas;  // Número total de páginas
float SMSize;       // Tamaño de memoria swap
float memoriaOcupadaRAM = 0;  // Memoria RAM ocupada
float memoriaOcupadaSwap = 0; // Memoria Swap ocupada
bool inicializacion = true;
mutex mutex1;

// Función para calcular memoria virtual
float VirtualMemorySize(float PSize) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distribucion(1.4, 4.5);
    float randomNumber = distribucion(gen);
    VMSize = PSize * randomNumber;
    return VMSize;
}

void SwapMemory(float VMSize) {
    if (VMSize <= 2000) {
        SMSize = 2 * VMSize;
    } else if (VMSize <= 8000) {
        SMSize = VMSize;
    } else {
        SMSize = 8000;
    }
}

// Inicializar tabla de páginas
void PageTableManagment(map<int, int>& pageTable, int numeroPaginas) {
    if (inicializacion) {
        for (int i = 0; i < numeroPaginas; ++i) {
            pageTable[i] = -1; // -1 indica que la página está vacía
        }
        inicializacion = false;
    }
}

// Mostrar estado gráfico de la memoria física y swap
void MostrarEstadoMemoria(const map<int, int>& pageTable) {
    cout << "\nEstado de la memoria física:" << endl;
    cout << "-----------------------------" << endl;

    int frame = 0;
    for (const auto& entry : pageTable) {
        if (entry.second != -1 && entry.second != -2) {
            cout << "| Frame " << frame << ": P" << entry.first << " ";
        } else {
            cout << "| Frame " << frame << ": Vacío ";
        }
        frame++;
        if (frame % 4 == 0) cout << "|" << endl;
    }

    cout << "\nEstado de la memoria Swap:" << endl;
    cout << "---------------------------" << endl;
    for (const auto& entry : pageTable) {
        if (entry.second == -2) {
            cout << "P" << entry.first << " en Swap" << endl;
        }
    }
    cout << "---------------------------" << endl;
}

// Función para asignar páginas a un proceso
void AsignarPaginas(int id, int tamaño, map<int, int>& pageTable) {
    lock_guard<mutex> lock(mutex1);

    int paginasRequeridas = static_cast<int>(ceil(tamaño / pageSize));
    cout << "Proceso " << id << " de tamaño " << tamaño << " MB requiere " << paginasRequeridas << " páginas." << endl;

    for (int i = 0; i < paginasRequeridas; ++i) {
        if (memoriaOcupadaRAM + pageSize <= SizeHDD) {
            memoriaOcupadaRAM += pageSize;
            pageTable[id * 100 + i] = memoriaOcupadaRAM; // Dirección ficticia
        } else if (memoriaOcupadaSwap + pageSize <= SMSize) {
            memoriaOcupadaSwap += pageSize;
            pageTable[id * 100 + i] = -2; // -2 indica Swap
        } else {
            cout << "Memoria insuficiente. Terminando programa..." << endl;
            exit(0);
        }
    }

    cout << "RAM ocupada: " << memoriaOcupadaRAM << " MB | Swap ocupada: " << memoriaOcupadaSwap << " MB" << endl;
    MostrarEstadoMemoria(pageTable);
}

int main() {
    map<int, int> pageTable;
    const int MIN_TAMAÑO = 1; // Tamaño mínimo de un proceso
    const int MAX_TAMAÑO = 100; // Tamaño máximo de un proceso

    cout << "Ingrese el tamaño de la memoria física (en MB): ";
    cin >> SizeHDD;
    VMSize = VirtualMemorySize(SizeHDD);
    SwapMemory(VMSize);

    cout << fixed << setprecision(1);
    cout << "Tamaño de memoria virtual: " << VMSize << " MB" << endl;
    cout << "Tamaño de la memoria swap: " << SMSize << " MB" << endl;

    cout << "Ingrese el tamaño de cada página (en MB): ";
    cin >> pageSize;

    numeroPaginas = static_cast<int>(ceil(VMSize / pageSize));
    PageTableManagment(pageTable, numeroPaginas);

    int contador = 0;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(MIN_TAMAÑO, MAX_TAMAÑO);

    while (true) {
        int tamaño = distrib(gen);

        thread(AsignarPaginas, contador, tamaño, ref(pageTable)).detach();
        contador++;

        this_thread::sleep_for(chrono::seconds(2));
    }

    return 0;
}
