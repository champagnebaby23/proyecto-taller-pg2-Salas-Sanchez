#ifndef LOGICA_H
#define LOGICA_H

#include "struct_taller.h"
#include <string>
#include <vector>

using namespace std;

// ====================================
// CONFIGURACIÓN DEL SISTEMA Y TALLER
// ====================================
bool inicializarSistemaArchivos(string &motivoFallo);
bool cargarTaller(Taller &t, bool &primeraEjecucion, string &motivoFallo);
bool guardarTaller(const Taller &t, string &motivoFallo);
bool fechaValidaISO(const char* fecha);

// ==================================
// MÓDULO DE VEHÍCULOS
// ==================================
bool agregarVehiculo(Vehiculo &datos, string &motivoFallo);
bool buscarVehiculoPorID(int id, Vehiculo &resultado, bool &fueEliminado, long &posicionBytes);
bool buscarVehiculoPorPlaca(const char* placa, Vehiculo &resultado, string &motivoFallo);
bool buscarVehiculosPorPropietario(const char* subcadena, Vehiculo resultados[],
                                   int &cantidad, bool &hayMas, string &motivoFallo);
bool listarVehiculos(Vehiculo resultados[], int &cantidad, bool &hayMas, string &motivoFallo);
bool actualizarVehiculo(int id, const Vehiculo &datosNuevos, string &motivoFallo);
bool eliminarVehiculo(int id, string &motivoFallo);

// =====================
// MÓDULO DE MECÁNICOS
// =====================
bool registrarMecanico(Mecanico &datos, string &motivoFallo);
bool buscarMecanicoPorID(int id, Mecanico &resultado, bool &fueEliminado, long *posicionBytes = nullptr);
bool obtenerMecanicoHistorico(int id, Mecanico &resultado);
bool buscarMecanicoPorCedula(const char* cedula, Mecanico &resultado, string &motivoFallo);
bool listarMecanicos(Mecanico resultados[], int &cantidad, bool &hayMas, string &motivoFallo);
bool especialidadValida(const char* esp);
bool contarOrdenesAbiertasMecanico(int idMecanico, int &total, string &motivoFallo);
bool eliminarMecanico(int id, string &motivoFallo);

// =====================
// MÓDULO DE REPUESTOS 
// =====================
bool registrarRepuesto(Repuesto &datos, string &motivoFallo);
bool buscarRepuestoPorID(int id, Repuesto &resultado, bool &fueEliminado, long *posicionBytes = nullptr);
bool obtenerRepuestoHistorico(int id, Repuesto &resultado);
bool buscarRepuestoPorCodigo(const char* codigo, Repuesto &resultado, string &motivoFallo);
bool buscarRepuestoPorNombreParcial(const char* subcadena, Repuesto resultados[],
                                    int &cantidad, bool &hayMas, string &motivoFallo);
bool registrarEntradaMercancia(int idRepuesto, int cantidadAdicional, string &motivoFallo);
bool listarRepuestos(Repuesto resultados[], int &cantidad, bool &hayMas, string &motivoFallo);
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
bool cerrarOrdenServicio(int idOrden, float manoObra, string &motivoFallo);
bool anularOrdenServicio(int idOrden, string &motivoFallo);
bool validarOrdenServicio(const OrdenServicio &o);
bool validarDetalleOrden(const OrdenServicio &o);
bool calcularTotalOrden(const OrdenServicio &o, float &total);
bool buscarOrdenPorID(int id, OrdenServicio &resultado, long *posicionBytes = nullptr);
bool listarOrdenesPorEstado(const char* estado, OrdenServicio resultados[], int &cantidad,
                            bool &hayMas, string &motivoFallo);

// ====================================
// MÓDULO DE REPORTES Y MANTENIMIENTO
// ====================================
bool generarReporteHistorialVehiculo(const char* placa, Vehiculo &vEncontrado,
                                    int desdeOrden, OrdenServicio ordenesVehiculo[],
                                    int &cantOrdenes, bool &hayMas, float &totalFacturado,
                                    string &motivoFallo);
bool generarReporteTopRepuestos(TopRepuesto top10[], int &cantEncontrados,
                                string &motivoFallo);
bool generarReporteFacturacionMecanicos(FilaFacturacionMecanico filas[], int &cantFilas,
                                       bool &hayMas, string &motivoFallo);
bool crearBackup(string &rutaGenerada, string &motivoFallo);
void verificarIntegridadSistema(vector<string> &errores, int contadores[]);

#endif // LOGICA_H
