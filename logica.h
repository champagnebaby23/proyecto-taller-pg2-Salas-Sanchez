#ifndef LOGICA_H
#define LOGICA_H

#include "struct_taller.h"

void inicializarSistema(sysTaller &s, const char* nombre, const char *rif, const char* dir, const char* tel);
void liberarSistema(sysTaller &s);

Vehiculo* agregarVehiculo(sysTaller &s, Vehiculo datos);
Vehiculo* buscarVehiculoPorID(sysTaller &s, int id);
Vehiculo* buscarVehiculoPorPlaca(sysTaller &s, const char* placa);
Vehiculo** buscarVehiculosPorPropietario(sysTaller &s, const char* subcadena, int &cantidad);
Vehiculo** listarVehiculos(sysTaller &s, int &cantidad);
bool actualizarVehiculo(sysTaller &s, int id, Vehiculo datos);
bool eliminarVehiculo(sysTaller &s, int id);

Mecanico* registrarMecanico(sysTaller &s, Mecanico datos);
Mecanico* buscarMecanicoPorID(sysTaller &s, int id);
Mecanico* buscarMecanicoPorCedula(sysTaller &s, const char* cedula);
Mecanico** listarMecanicos(sysTaller &s, int &cantidad);
bool especialidadValida(const char* esp);

Repuesto* registrarRepuesto(sysTaller &s, Repuesto datos);
Repuesto* buscarRepuestoPorID(sysTaller &s, int id);
Repuesto* buscarRepuestoPorCodigo(sysTaller &s, const char* codigo);
bool registrarEntradaMercancia(sysTaller &s, int idRepuesto, int cantidadAdicional);
Repuesto** listarRepuestos(sysTaller &s, int &cantidad);
bool cambiarPrecioRepuesto(sysTaller &s, int idRepuesto, float nuevoPrecio);
bool eliminarRepuesto(sysTaller &s, int id);

ordenServicio* abrirOrdenServicio(sysTaller &s, int idVehiculo, int idMecanico, const char* fechaApertura, const char* descripcionFalla, int kmIngreso);
bool agregarRepuestoAOrden(sysTaller &s, int idOrden, int idRepuesto, int cantidad);
bool quitarRepuestoDeOrden(sysTaller &s, int idOrden, int idRepuesto);
bool cerrarOrdenServicio(sysTaller &s, int idOrden, const char* fechaCierre, float manoObra);
bool anularOrdenServicio(sysTaller &s, int idOrden);
float calcularTotalOrden(const ordenServicio &o);
ordenServicio* buscarOrdenPorID(sysTaller &s, int id);
ordenServicio** listarOrdenesPorEstado(sysTaller &s, const char* estado, int &cantidad);

#endif
