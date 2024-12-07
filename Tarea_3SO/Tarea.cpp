#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <map>
#include <thread>
#include <chrono>
#include <algorithm>
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
map<int, int> procesos; // Mapa para llevar seguimiento de procesos y sus páginas

// Función para calcular memoria virtual con validación
float VirtualMemorySize(float PSize) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> distribucion(1.4, 4.5);
    float randomNumber = distribucion(gen);
    VMSize = PSize * randomNumber;

    // Asegurar que VMSize esté dentro de los límites 1.5x a 4.5x de la memoria física
    if (VMSize < 1.5 * PSize) VMSize = 1.5 * PSize;
    if (VMSize > 4.5 * PSize) VMSize = 4.5 * PSize;

    return VMSize;
}

// Función para calcular el tamaño de la memoria swap
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
    int pageToProcess = ramPages.front(); // Declaración y asignación
    ramPages.pop(); // Eliminarla de la cola

    // Moverla al swap
    pageTable[pageToProcess] = -2; // -2 indica que está en swap
    memoriaOcupadaRAM -= pageSize;
    memoriaOcupadaSwap += pageSize;

    cout << "Página " << pageToProcess << " movida de RAM a Swap para liberar espacio." << endl;
}


// Asignar páginas a un proceso
void AsignarPaginas(int id, int tamaño, map<int, int>& pageTable) {
    lock_guard<mutex> lock(mutex1);

    int paginasRequeridas = static_cast<int>(ceil(static_cast<float>(tamaño) / pageSize));

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
int totalPaginasDisponibles = static_cast<int>(SizeHDD / pageSize) + static_cast<int>(SMSize / pageSize);
if (paginasRequeridas > totalPaginasDisponibles) {
    cout << "El proceso requiere más páginas (" << paginasRequeridas << ") de las disponibles (" << totalPaginasDisponibles << ")." << endl;
    return;
}

    // Si no se pudieron asignar todas las páginas
    if (paginasAsignadas < paginasRequeridas) {
    cout << "Memoria insuficiente para asignar proceso " << id << " de tamaño " << tamaño << " MB." << endl;
    cout << "RAM ocupada: " << memoriaOcupadaRAM << " MB, Swap ocupada: " << memoriaOcupadaSwap << " MB." << endl;
    cout << "Termina el programa." << endl;
    exit(0);
}


    // Agregar el proceso al mapa
    procesos[id] = tamaño;

    // Mostrar el estado de la memoria
    cout << "RAM ocupada: " << memoriaOcupadaRAM << " MB | Swap ocupada: " << memoriaOcupadaSwap << " MB" << endl;
    MostrarEstadoMemoria(pageTable);
}

// Acceder a una dirección virtual
void AccederADireccionVirtual(map<int, int>& pageTable) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, numeroPaginas - 1);

    int direccion = distrib(gen);
    cout << "\nAcceso a dirección virtual: P" << direccion << endl;

    // Comprobar si la página está en RAM
    auto it = find_if(pageTable.begin(), pageTable.end(), [direccion](const pair<int, int>& p) { return p.first == direccion && p.second != -1 && p.second != -2; });

    if (it == pageTable.end()) {
        cout << "Page fault: La página P" << direccion << " no se encuentra en RAM." << endl;
        MoverPaginaASwap(pageTable); // Simulación de swap
    } else {
        cout << "Página P" << direccion << " encontrada en RAM." << endl;
    }
}

// Finalizar un proceso aleatorio
void FinalizarProceso(map<int, int>& pageTable) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, procesos.size() - 1);

    if (procesos.empty()) return;

    auto it = procesos.begin();
    advance(it, distrib(gen));
    int procesoFinalizado = it->first;

    // Liberar páginas del proceso
    for (auto& entry : pageTable) {
        if (entry.second == procesoFinalizado) {
            entry.second = -1; // Marcar página como vacía
            memoriaOcupadaRAM -= pageSize;
            cout << "Proceso " << procesoFinalizado << " finalizado. Página " << entry.first << " liberada." << endl;
        }
    }
    procesos.erase(procesoFinalizado);
}


int main() {
    map<int, int> pageTable;
    const int MIN_TAMAÑO = 1;  // Tamaño mínimo de un proceso
    const int MAX_TAMAÑO = 100; // Tamaño máximo de un proceso

    cout << "Ingrese el tamaño de la memoria física (en MB): ";
    cin >> SizeHDD;
    cout << "Ingrese el tamaño de cada página (en MB): ";
    cin >> pageSize;

    numeroPaginas = static_cast<int>(ceil(SizeHDD / pageSize));
    PageTableManagment(pageTable, numeroPaginas);

    // Simular la asignación de procesos y la gestión de la memoria
    while (true) {
    int id = rand() % 100; // ID de proceso aleatorio
    int tamaño = rand() % (static_cast<int>(SizeHDD / 2)) + 1; // Tamaño del proceso entre 1 y la mitad de la memoria física
    AsignarPaginas(id, tamaño, pageTable);

    this_thread::sleep_for(chrono::seconds(2));

    // Acceso a una dirección virtual aleatoria
    AccederADireccionVirtual(pageTable);

    // Finalizar proceso aleatorio
    FinalizarProceso(pageTable);
}


    return 0;
}