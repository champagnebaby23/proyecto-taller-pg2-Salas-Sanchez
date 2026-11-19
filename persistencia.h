#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include "struct_taller.h"
#include <string>

// ============================================================
// OPERACIONES SOBRE EL HEADER Y CONTROL DE ARCHIVOS
// ============================================================

bool asegurarDirectorioDatos(std::string &motivoFallo);
// Crea el header inicial o valida el existente; las entidades tambien validan
// el conteo fisico cuando se proporciona tamanoRegistro.
bool inicializarArchivo(const char* ruta, size_t tamanoRegistro = 0);

// Obtiene el header de un archivo sin leer ningun registro.
bool leerHeader(const char* ruta, ArchivoHeader &header);
bool inspeccionarArchivoEntidad(const char* ruta, size_t tamanoRegistro,
                                int &registrosFisicos, bool &registroIncompleto,
                                std::string &motivoFallo);

// Sobrescribe unicamente el header (posición 0), sin tocar los registros.
bool actualizarHeader(const char* ruta, const ArchivoHeader &header);

// Calcula la posición en bytes del registro con el indice 
// posición = sizeof(ArchivoHeader) + (id - 1) * sizeof(Estructura)
long calcularOffset(int id, size_t tamanoRegistro);

// ======================================
// PERSISTENCIA DE TALLER 
// taller.bin no tiene header
// ======================================
bool cargarTallerArchivo(Taller &t, bool &noExiste);
bool guardarTallerArchivo(const Taller &t);
bool crearBackupArchivos(std::string &rutaGenerada, std::string &motivoFallo);

// ========================
// PERSISTENCIA: VEHICULOS
// ========================
bool guardarVehiculoCrudo(Vehiculo &v, std::string &motivoFallo);
bool leerVehiculoPorID(int id, Vehiculo &v, long *offsetSalida = nullptr);
bool sobrescribirVehiculo(const Vehiculo &v);
bool marcarVehiculoEliminado(int id, std::string &motivoFallo);
int contarVehiculosActivos();

// ========================
// PERSISTENCIA: MECANICO
// ========================
bool guardarMecanicoCrudo(Mecanico &m, std::string &motivoFallo);
bool leerMecanicoPorID(int id, Mecanico &m, long *offsetSalida = nullptr);
bool sobrescribirMecanico(const Mecanico &m);
bool marcarMecanicoEliminado(int id, std::string &motivoFallo);
int contarMecanicosActivos();

// ==========================
// PERSISTENCIA: REPUESTOS
// ==========================
bool guardarRepuestoCrudo(Repuesto &r, std::string &motivoFallo);
bool leerRepuestoPorID(int id, Repuesto &r, long *offsetSalida = nullptr);
bool sobrescribirRepuesto(const Repuesto &r);
bool marcarRepuestoEliminado(int id, std::string &motivoFallo);
int contarRepuestosActivos();

// ===================================
// PERSISTENCIA: ORDENES DE SERVICIO
// ===================================
bool guardarOrdenCruda(OrdenServicio &o, std::string &motivoFallo);
bool leerOrdenPorID(int id, OrdenServicio &o, long *offsetSalida = nullptr);
bool sobrescribirOrden(const OrdenServicio &o);
bool marcarOrdenEliminada(int id, std::string &motivoFallo);
int contarOrdenesActivas();

#endif // PERSISTENCIA_H
