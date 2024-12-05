#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <map>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <queue> // Para implementar FIFO

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

queue<int> ramPages; // Cola para manejar las páginas en RAM (FIFO)

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

// Mover una página de RAM a Swap para liberar espacio
void MoverPaginaASwap(map<int, int>& pageTable) {
    if (ramPages.empty()) return;

    // Obtener la página más antigua de la RAM (FIFO)
    int pagina = ramPages.front();
    ramPages.pop(); // Eliminarla de la cola

    // Moverla al swap
    pageTable[pagina] = -2; // -2 indica que está en swap
    memoriaOcupadaRAM -= pageSize;
    memoriaOcupadaSwap += pageSize;

    cout << "Página " << pagina << " movida de RAM a Swap para liberar espacio." << endl;
}

void AsignarPaginas(int id, int tamaño, map<int, int>& pageTable) {
    lock_guard<mutex> lock(mutex1);

    int paginasRequeridas = static_cast<int>(ceil(tamaño / pageSize));
    cout << "Proceso " << id << " de tamaño " << tamaño << " MB requiere " << paginasRequeridas << " páginas." << endl;

    int paginasAsignadas = 0;

    // Intentar asignar páginas a la RAM
    for (auto& entry : pageTable) {
        if (entry.second == -1 && paginasAsignadas < paginasRequeridas) {
            entry.second = id; // Marcar frame ocupado por este proceso
            memoriaOcupadaRAM += pageSize;
            ramPages.push(entry.first); // Agregar la página a la cola de RAM
            paginasAsignadas++;
        }
    }

    // Si la RAM está llena, mover páginas al swap
    while (memoriaOcupadaRAM > SizeHDD) {
        MoverPaginaASwap(pageTable);
    }

    // Intentar asignar las páginas restantes después de mover al swap
    for (auto& entry : pageTable) {
        if (entry.second == -1 && paginasAsignadas < paginasRequeridas) {
            entry.second = id;
            memoriaOcupadaRAM += pageSize;
            ramPages.push(entry.first);
            paginasAsignadas++;
        }
    }

    // Si no se pudieron asignar todas las páginas
    if (paginasAsignadas < paginasRequeridas) {
        cout << "Memoria insuficiente. Terminando programa..." << endl;
        exit(0);
    }

    // Mostrar el estado de la memoria
    cout << "RAM ocupada: " << memoriaOcupadaRAM << " MB | Swap ocupada: " << memoriaOcupadaSwap << " MB" << endl;
    MostrarEstadoMemoria(pageTable);
}

int main() {
    map<int, int> pageTable;
    const int MIN_TAMAÑO = 1;  // Tamaño mínimo de un proceso
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

