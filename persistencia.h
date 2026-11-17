#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include "struct_taller.h"

// ============================================================
// OPERACIONES SOBRE EL HEADER Y CONTROL DE ARCHIVOS
// ============================================================

// Si el archivo no existe, lo crea con su header inicial
// Si ya existe, no lo modifica.
bool inicializarArchivo(const char* ruta);

// Obtiene el header de un archivo sin leer ningun registro.
bool leerHeader(const char* ruta, ArchivoHeader &header);

// Sobrescribe unicamente el header (posición 0), sin tocar los registros.
bool actualizarHeader(const char* ruta, const ArchivoHeader &header);

// Calcula la posición en bytes del registro con el indice 
// posición = sizeof(ArchivoHeader) + (id - 1) * sizeof(Estructura)
long calcularOffset(int id, size_t tamanoRegistro);

// ======================================
// PERSISTENCIA DE TALLER 
// taller.bin no tiene header
// ======================================
bool cargarTallerArchivo(Taller &t);
bool guardarTallerArchivo(const Taller &t);

// ========================
// PERSISTENCIA: VEHICULOS
// ========================
bool guardarVehiculoCrudo(Vehiculo &v);
bool leerVehiculoPorID(int id, Vehiculo &v, long *offsetSalida = nullptr);
bool sobrescribirVehiculo(const Vehiculo &v);
bool marcarVehiculoEliminado(int id);
int contarVehiculosActivos();

// ========================
// PERSISTENCIA: MECANICO
// ========================
bool guardarMecanicoCrudo(Mecanico &m);
bool leerMecanicoPorID(int id, Mecanico &m, long *offsetSalida = nullptr);
bool sobrescribirMecanico(const Mecanico &m);
bool marcarMecanicoEliminado(int id);
int contarMecanicosActivos();

// ==========================
// PERSISTENCIA: REPUESTOS
// ==========================
bool guardarRepuestoCrudo(Repuesto &r);
bool leerRepuestoPorID(int id, Repuesto &r, long *offsetSalida = nullptr);
bool sobrescribirRepuesto(const Repuesto &r);
bool marcarRepuestoEliminado(int id);
int contarRepuestosActivos();

// ===================================
// PERSISTENCIA: ORDENES DE SERVICIO
// ===================================
bool guardarOrdenCruda(OrdenServicio &o);
bool leerOrdenPorID(int id, OrdenServicio &o, long *offsetSalida = nullptr);
bool sobrescribirOrden(const OrdenServicio &o);
bool marcarOrdenEliminada(int id);
int contarOrdenesActivas();

#endif // PERSISTENCIA_H
