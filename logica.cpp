#include "logica.h"
#include <iostream>
#include <cstring>
#include <cctype>

using namespace std;

void inicializarSistema(sysTaller &s, const char* nombre, const char* rif, const char* dir, const char* tel) {

    strcpy(s.taller.nombre, nombre);
    strcpy(s.taller.rif, rif);
    strcpy(s.taller.direccion, dir);
    strcpy(s.taller.telefono, tel);

    s.capacidadVehiculos = 4;
    s.numVehiculos = 4;
    s.vehiculos = new Vehiculo[s.capacidadVehiculos];

    s.capacidadMecanicos = 4;
    s.numMecanicos = 4;
    s.mecanicos = new Mecanico[s.capacidadMecanicos];

    s.capacidadRepuestos = 4;
    s.numRepuestos = 4;
    s.repuestos = new Repuesto[s.capacidadRepuestos];

    s.capacidadOrdenes = 4;
    s.numOrdenes = 4;
    s.ordenes = new ordenServicio[s.capacidadOrdenes];

    s.siguienteIdVehiculo = 1;
    s.siguienteIdMecanico = 1;
    s.siguienteIdRepuesto = 1;
    s.siguienteIdOrden = 1;
}

void liberarSistema(sysTaller &s) {

    for (int i=0; i < s.numOrdenes; i++) {
        delete[] s.ordenes[i].detalles;
    }
    delete[] s.ordenes;
    delete[] s.vehiculos;
    delete[] s.mecanicos;
    delete[] s.repuestos;

    s.vehiculos = nullptr;
    s.mecanicos = nullptr;
    s.repuestos = nullptr;
    s.ordenes = nullptr;
}

Vehiculo* buscarVehiculoPorID(sysTaller &s, int id) {
    for (int i=0; i < s.numVehiculos; i++) {
        if (s.vehiculos[i].id == id) {
            return &s.vehiculos[i];
            }
        }
    return nullptr;
    }

Vehiculo* buscarVehiculoPorPlaca(sysTaller &s, const char* placa) {
    for (int i = 0; i < s.numVehiculos; i++) {
        if (strcmp(s.vehiculos[i].placa, placa) == 0) {
            return &s.vehiculos[i];
        }
    }
    return nullptr;
}

Vehiculo* agregarVehiculo(sysTaller &s, Vehiculo datos) {

    if (buscarVehiculoPorPlaca(s, datos.placa) != nullptr) {
        return nullptr;
    }

    if (datos.anio < 1950 || datos.anio > 2027 || datos.km < 0) {
        return nullptr;
    }

    if (s.numVehiculos >= s.capacidadVehiculos) {
        int nuevaCapacidad = s.capacidadVehiculos * 2;
        Vehiculo* nuevoArreglo = new Vehiculo[nuevaCapacidad];

    for (int i = 0; i < s.numVehiculos; i++) {
            nuevoArreglo[i] = s.vehiculos[i];
    }

    delete[] s.vehiculos;
    s.vehiculos = nuevoArreglo;
    s.capacidadVehiculos = nuevaCapacidad;
    }

datos.id = s.siguienteIdVehiculo++;
s.vehiculos[s.numVehiculos] = datos;

return &s.vehiculos[s.numVehiculos++];
}

bool especialidadValida(const char* esp) {
    return (strcmp(esp, "MOTOR") == 0 || 
            strcmp(esp, "FRENOS") == 0 || 
            strcmp(esp, "ELECTRICIDAD") == 0 || 
            strcmp(esp, "SUSPENSION") == 0 || 
            strcmp(esp, "GENERAL") == 0);
}

Mecanico* buscarMecanicoPorID(sysTaller &s, int id) {
    for (int i = 0; i < s.numMecanicos; i++) {
        if (s.mecanicos[i].id == id) {
            return &s.mecanicos[i];
        }
    }
    return nullptr;
}

Mecanico* registrarMecanico(sysTaller &s, Mecanico datos) {
    if (buscarMecanicoPorCedula(s, datos.cedula) != nullptr) {
        return nullptr; // Ya existe
    }

    if (!especialidadValida(datos.especialidad)) {
        return nullptr;
    }

    if (strlen(datos.nombre) == 0 || strlen(datos.fechaIngreso) == 0) {
        return nullptr;
    }

    if (s.numMecanicos >= s.capacidadMecanicos) {
        int nuevaCapacidad = s.capacidadMecanicos * 2;
        Mecanico* nuevoArreglo = new Mecanico[nuevaCapacidad];

        for (int i = 0; i < s.numMecanicos; i++) {
            nuevoArreglo[i] = s.mecanicos[i];
        }

        delete[] s.mecanicos;
        s.mecanicos = nuevoArreglo;
        s.capacidadMecanicos = nuevaCapacidad;
    }

    datos.id = s.siguienteIdMecanico++;
    s.mecanicos[s.numMecanicos] = datos;

    return &s.mecanicos[s.numMecanicos++];
}

Mecanico** listarMecanicos(sysTaller &s, int &cantidad) {
    cantidad = s.numMecanicos;
    if (cantidad == 0) return nullptr;

    Mecanico** resultado = new Mecanico*[cantidad];
    for (int i = 0; i < cantidad; i++) {
        resultado[i] = &s.mecanicos[i];
    }
    return resultado;
}

