#ifndef LOGICA_H
#define LOGICA_H

#include "struct_taller.h"

void inicializarSistema(sysTaller &s, const char* nombre, const char *rif, const char* dir, const char* tel);
void liberarSistema(sysTaller &s);

Vehiculo* agregarVehiculo(sysTaller &s, Vehiculo datos);
Vehiculo* buscarVehiculoPorID(sysTaller &s, int id);
Vehiculo* buscarVehiculoPorPlaca(sysTaller &s, const char placa);
bool actualizarVehiculo(sysTaller &s, int id, Vehiculo datos);
bool eliminarVehiculo(sysTaller &s, int id);

#endif

