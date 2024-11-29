#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <map>
using namespace std;

float SizeHDD;  // Tamaño de memoria física
float VMSize;   // Tamaño de memoria virtual
float pageSize; // Tamaño de cada página
int numeroPaginas;
float memoryAssigned;
bool inicializacion = true;

// Función para calcular memoria virtual
float VirtualMemorySize(float PSize) {
    random_device rd;           // Generador de números aleatorios basado en hardware
    mt19937 gen(rd());          // Motor de Mersenne Twister
    uniform_real_distribution<float> distribucion(1.4, 4.5); // Rango para números reales
    float randomNumber = distribucion(gen); // Generar número aleatorio
    VMSize = PSize * randomNumber;
    return VMSize;
}

// Función para gestionar la tabla de páginas (inicialización a -1)
void PageTableManagment(map<int, int>& pageTable, int numeroPaginas){
    if(inicializacion == true){
        // Inicializar la tabla de páginas con valor -1 (vacía) para todas las páginas
        for (int i = 0; i < numeroPaginas; ++i) {
            pageTable[i] = -1; // -1 indica que la página está vacía
        }
        inicializacion = false;
    }
}

// Función para mostrar el contenido de la tabla de páginas
void ShowPageTable(const map<int, int>& pageTable) {
    cout << "\nContenido de la tabla de páginas:" << endl;
    for (const auto& entry : pageTable) {
        cout << "Página " << entry.first << ": ";
        if (entry.second == -1) {
            cout << "Vacía" << endl;
        } else {
            cout << "Asignada a dirección física " << entry.second << " MB" << endl;
        }
    }
}

int main() {
    map<int, int> pageTable;

    cout << "Ingrese el tamaño de la memoria física (Considere que el tamaño está en MB): ";
    cin >> SizeHDD;
    VMSize = VirtualMemorySize(SizeHDD);
    // Mostrar memoria virtual calculada con 1 decimal
    cout << fixed << setprecision(1);
    cout << "El tamaño de memoria virtual es de " << VMSize << " MB" << endl;
    cout << "Ingrese el tamaño de cada página (en MB): ";
    cin >> pageSize;
    
    // Calcular el número de páginas necesarias
    numeroPaginas = static_cast<int>(ceil(VMSize / pageSize));
    memoryAssigned = numeroPaginas * pageSize; // Memoria virtual realmente asignada
    cout << "Con el tamaño de páginas ingresado, el número de páginas totales es de: " << numeroPaginas << endl;
    cout << "Memoria virtual realmente asignada para el número de páginas del tamaño requerido: " << memoryAssigned << " MB" << endl;    

    // Llamar a la función para inicializar la tabla de páginas
    PageTableManagment(pageTable, numeroPaginas);

    // Mostrar el contenido de la tabla de páginas
    ShowPageTable(pageTable);

    return 0;
}


