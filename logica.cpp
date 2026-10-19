#include "logica.h"
#include <iostream>
#include <cstring>

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