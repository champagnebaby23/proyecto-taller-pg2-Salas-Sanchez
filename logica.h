#ifndef LOGICA_H
#define LOGICA_H

#include "struct_taller.h"
#include <string>

using namespace std;

// ====================================
// CONFIGURACIÓN DEL SISTEMA Y TALLER
// ====================================
void inicializarSistemaArchivos();
bool cargarTaller(Taller &t);
void guardarTaller(const Taller &t);

// ==================================
// MÓDULO DE VEHÍCULOS
// ==================================
bool agregarVehiculo(Vehiculo &datos, string &motivoFallo);
bool buscarVehiculoPorID(int id, Vehiculo &resultado, bool &fueEliminado, long &posicionBytes);
bool buscarVehiculoPorPlaca(const char* placa, Vehiculo &resultado);
bool buscarVehiculosPorPropietario(const char* subcadena, Vehiculo resultados[], int &cantidad, bool &hayMas);
bool listarVehiculos(Vehiculo resultados[], int &cantidad, bool &hayMas);
bool actualizarVehiculo(int id, const Vehiculo &datosNuevos, string &motivoFallo);
bool eliminarVehiculo(int id, string &motivoFallo);

// =====================
// MÓDULO DE MECÁNICOS
// =====================
bool registrarMecanico(Mecanico &datos, string &motivoFallo);
bool buscarMecanicoPorID(int id, Mecanico &resultado, bool &fueEliminado, long *posicionBytes = nullptr);
bool obtenerMecanicoHistorico(int id, Mecanico &resultado);
bool buscarMecanicoPorCedula(const char* cedula, Mecanico &resultado);
bool listarMecanicos(Mecanico resultados[], int &cantidad, bool &hayMas);
bool especialidadValida(const char* esp);
int contarOrdenesAbiertasMecanico(int idMecanico);
bool eliminarMecanico(int id, string &motivoFallo);

// =====================
// MÓDULO DE REPUESTOS 
// =====================
bool registrarRepuesto(Repuesto &datos, string &motivoFallo);
bool buscarRepuestoPorID(int id, Repuesto &resultado, bool &fueEliminado, long *posicionBytes = nullptr);
bool buscarRepuestoPorCodigo(const char* codigo, Repuesto &resultado);
bool buscarRepuestoPorNombreParcial(const char* subcadena, Repuesto resultados[], int &cantidad, bool &hayMas);
bool registrarEntradaMercancia(int idRepuesto, int cantidadAdicional, string &motivoFallo);
bool listarRepuestos(Repuesto resultados[], int &cantidad, bool &hayMas);
bool cambiarPrecioRepuesto(int idRepuesto, float nuevoPrecio, string &motivoFallo);
bool eliminarRepuesto(int id, string &motivoFallo);

// ===============================
// MÓDULO DE ÓRDENES DE SERVICIO
// ===============================
bool abrirOrdenServicio(int idVehiculo, int idMecanico, const char* fechaApertura,
                       const char* descripcionFalla, int kilometrajeIngreso,
                       OrdenServicio &resultado, string &motivoFallo);
bool agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad, string &motivoFallo);
bool quitarRepuestoDeOrden(int idOrden, int idRepuesto, string &motivoFallo);
bool cerrarOrdenServicio(int idOrden, const char* fechaCierre, float manoObra, string &motivoFallo);
bool anularOrdenServicio(int idOrden, string &motivoFallo);
float calcularTotalOrden(const OrdenServicio &o);
bool buscarOrdenPorID(int id, OrdenServicio &resultado, long *posicionBytes = nullptr);
bool listarOrdenesPorEstado(const char* estado, OrdenServicio resultados[], int &cantidad, bool &hayMas);

// ====================================
// MÓDULO DE REPORTES Y MANTENIMIENTO
// ====================================
bool generarReporteHistorialVehiculo(const char* placa, Vehiculo &vEncontrado,
                                    OrdenServicio ordenesVehiculo[], int &cantOrdenes, float &totalFacturado);
bool generarReporteTopRepuestos(TopRepuesto top10[], int &cantEncontrados);
bool generarReporteFacturacionMecanicos(FilaFacturacionMecanico filas[], int &cantFilas, bool &hayMas);
bool crearBackup(string &rutaGenerada);
void verificarIntegridadSistema(string errores[], int &cantErrores, int contadores[]);

#endif // LOGICA_H
