#ifndef LOGICA_H
#define LOGICA_H

#include "struct_taller.h"

#include <string>

bool cargarTaller(Taller &t);
void guardarTaller(Taller t);

void inicializarArchivoBase(const char* ruta);
long calcularPosicion(int id, size_t sizeEstructura);

bool agregarVehiculo(Vehiculo &datos);
bool buscarVehiculoPorID(int id, Vehiculo &resultado);
bool buscarVehiculoPorPlaca(const char* placa, Vehiculo &resultado);
bool buscarVehiculosPorPropietario(const char* subcadena, Vehiculo resultados[], int &cantidad);
bool listarVehiculos(Vehiculo resultados[], int &cantidad);
bool actualizarVehiculo(int id, Vehiculo datos);
bool eliminarVehiculo(int id);

bool registrarMecanico(Mecanico &datos);
bool buscarMecanicoPorID(int id, Mecanico &resultado);
bool buscarMecanicoPorCedula(const char* cedula, Mecanico &resultado);
bool listarMecanicos(Mecanico resultados[], int &cantidad);
bool especialidadValida(const char* esp);
bool eliminarMecanico(int id);

bool registrarRepuesto(Repuesto &datos);
bool buscarRepuestoPorID(int id, Repuesto &resultado);
bool buscarRepuestoPorCodigo(const char* codigo, Repuesto &resultado);
bool registrarEntradaMercancia(int idRepuesto, int cantidadAdicional);
bool listarRepuestos(Repuesto resultados[], int &cantidad);
bool cambiarPrecioRepuesto(int idRepuesto, float nuevoPrecio);
bool eliminarRepuesto(int id);

bool abrirOrdenServicio(int idVehiculo, int idMecanico, const char* fechaApertura, const char* descripcionFalla, int kmIngreso, ordenServicio &resultado);
bool agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad);
bool quitarRepuestoDeOrden(int idOrden, int idRepuesto);
bool cerrarOrdenServicio(int idOrden, const char* fechaCierre, float manoObra);
bool anularOrdenServicio(int idOrden);
float calcularTotalOrden(const ordenServicio &o);
bool buscarOrdenPorID(int id, ordenServicio &resultado);
bool listarOrdenesPorEstado(const char* estado, ordenServicio resultados[], int &cantidad);

#endif
