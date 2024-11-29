#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
using namespace std;

float SizeHDD;  // Tamaño de memoria física
float VMSize;   // Tamaño de memoria virtual
float pageSize; // Tamaño de cada página
int numeroPaginas;
float memoryAssigned;

// Función para calcular memoria virtual
float VirtualMemorySize(float PSize) {
    random_device rd;           // Generador de números aleatorios basado en hardware
    mt19937 gen(rd());          // Motor de Mersenne Twister
    uniform_real_distribution<float> distribucion(1.4, 4.5); // Rango para números reales
    float randomNumber = distribucion(gen); // Generar número aleatorio
    VMSize = PSize * randomNumber;
    return VMSize;
}

int main() {
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

    return 0;
}
