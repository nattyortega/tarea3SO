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
map<int, int> pageToProcess; // Mapa que asigna una página a un proceso

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
            pageToProcess[i] = -1; // Inicializar las páginas sin proceso
        }
        inicializacion = false;
    }
}

// Mostrar estado gráfico de la memoria física y swap de forma compacta
void MostrarEstadoMemoriaCompacto(const map<int, int>& pageTable) {
    cout << "\nMemoria RAM (Páginas ocupadas): ";
    for (const auto& entry : pageTable) {
        if (entry.second != -1 && entry.second != -2) {
            cout << "P" << entry.first << "(P" << pageToProcess[entry.first] << ") ";
        }
    }
    cout << "\nMemoria Swap: ";
    for (const auto& entry : pageTable) {
        if (entry.second == -2) {
            cout << "P" << entry.first << "(P" << pageToProcess[entry.first] << ") ";
        }
    }
    cout << endl;
}

// Mover una página de RAM a Swap para liberar espacio, de forma compacta
void MoverPaginaASwap(map<int, int>& pageTable, vector<int>& movidas) {
    if (ramPages.empty()) return;

    // Obtener la página más antigua de la RAM (FIFO)
    int pagina = ramPages.front();
    ramPages.pop(); // Eliminarla de la cola

    // Moverla al swap
    pageTable[pagina] = -2; // -2 indica que está en swap
    memoriaOcupadaRAM -= pageSize;
    memoriaOcupadaSwap += pageSize;
    pageToProcess[pagina] = -1; // La página ya no está ocupada por ningún proceso

    movidas.push_back(pagina); // Guardar la página que se movió a swap
}

// Mostrar un resumen de las páginas movidas a swap
void ResumenMovidasASwap(const vector<int>& movidas) {
    if (!movidas.empty()) {
        cout << "Se movieron las páginas ";
        for (size_t i = 0; i < movidas.size(); ++i) {
            cout << "P" << movidas[i];
            if (i < movidas.size() - 1) cout << ", ";
        }
        cout << " a swap." << endl;
    }
}

void AsignarPaginas(int id, int tamaño, map<int, int>& pageTable) {
    lock_guard<mutex> lock(mutex1);

    int paginasRequeridas = static_cast<int>(ceil(tamaño / pageSize));
    cout << "Proceso P" << id << " de tamaño " << tamaño << " MB requiere " << paginasRequeridas << " páginas." << endl;

    int paginasAsignadas = 0;

    // Intentar asignar páginas a la RAM
    for (auto& entry : pageTable) {
        if (entry.second == -1 && paginasAsignadas < paginasRequeridas) {
            entry.second = id; // Marcar frame ocupado por este proceso
            pageToProcess[entry.first] = id; // Asignar el proceso a la página
            memoriaOcupadaRAM += pageSize;
            ramPages.push(entry.first); // Agregar la página a la cola de RAM
            paginasAsignadas++;
        }
    }

    // Si la RAM está llena, mover páginas al swap
    vector<int> movidas; // Almacenar las páginas movidas
    while (memoriaOcupadaRAM > SizeHDD) {
        MoverPaginaASwap(pageTable, movidas);
    }

    // Intentar asignar las páginas restantes después de mover al swap
    for (auto& entry : pageTable) {
        if (entry.second == -1 && paginasAsignadas < paginasRequeridas) {
            entry.second = id;
            pageToProcess[entry.first] = id;
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

    // Mostrar el estado de la memoria de forma compacta
    cout << "RAM ocupada: " << memoriaOcupadaRAM << " MB | Swap ocupada: " << memoriaOcupadaSwap << " MB" << endl;
    MostrarEstadoMemoriaCompacto(pageTable);

    // Mostrar resumen de las páginas movidas a swap
    ResumenMovidasASwap(movidas);
}

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
        vector<int> movidas;
        MoverPaginaASwap(pageTable, movidas); // Simulación de swap
        ResumenMovidasASwap(movidas);
    } else {
        cout << "Página P" << direccion << " encontrada en RAM." << endl;
    }
}

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
            pageToProcess[entry.first] = -1; // Desasignar proceso de la página
            memoriaOcupadaRAM -= pageSize;
            cout << "Proceso P" << procesoFinalizado << " finalizado. Página P" << entry.first << " liberada." << endl;
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

    // Ejecutar las simulaciones en un hilo separado para las acciones cada 5 segundos
    thread t([&]() {
        this_thread::sleep_for(chrono::seconds(30));
        while (true) {
            this_thread::sleep_for(chrono::seconds(5));
            FinalizarProceso(pageTable);  // Finalizar un proceso aleatorio
            AccederADireccionVirtual(pageTable); // Acceder a una dirección virtual aleatoria
        }
    });

    while (true) {
        int tamaño = distrib(gen);

        thread(AsignarPaginas, contador, tamaño, ref(pageTable)).detach();
        procesos[contador] = tamaño;

        contador++;

        this_thread::sleep_for(chrono::seconds(2));
    }

    t.join(); // Espera hasta que el hilo que simula las acciones cada 5 segundos termine

    return 0;
}
