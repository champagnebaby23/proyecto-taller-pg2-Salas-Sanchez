#ifndef STRUCT_TALLER_H
#define STRUCT_TALLER_H

#include <ctime>
#include <string>
using namespace std;

// Estructura administrativa requerida para cada archivo (excepto taller.bin)
struct ArchivoHeader {
    int cantidadRegistros; // Total histórico (incluye eliminados lógicamente)
    int proximoID;         // Siguiente ID a asignar
    int registrosActivos;  // Registros con eliminado == false
    int version;           // Versión del formato del archivo (iniciar en 1)
};

// Estructura del taller (Informacion unica)
struct Taller {
    char nombre[100]; // Nombre del establecimiento
    char rif[20]; // Identificar fiscal
    char direccion[200]; // Ubicacion fisica del taller
    char telefono[20]; // Telefono de contacto
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// Estructura del vehiculo
struct Vehiculo {
    int id; // Llave primaria unica y autoincremental
    char placa[10]; // Se valida en logica para que sea estrictamente unica
    char marca[50]; 
    char modelo[50];
    int anio; // Validado entre 1950 y 2027
    int km; // Se actualiza automaticamente al abrir una orden
    
    // Los datos del propietario del vehiculo van anidados en la misma estructura del Vehiculo
    
    char nombrePropietario[100];
    char cedulaPropietario[20];
    char telefonoPropietario[20];
    bool eliminado; // Para el borrado lógico
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
    
};

// Estructura del mecanico
struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20]; // Validada en logica para ser unica
    char telefono[20];
    char especialidad[20]; // Validada como "MOTOR" "FRENOS", etc en la logica
    char fechaIngreso[11];
    bool eliminado; // true = dado de baja
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// Estructura del Repuesto
struct Repuesto {
    int id;
    char codigo[20]; // Codigo unico alfanumerico
    char nombre[100];
    char marca[50];
    float precio; // Precio actual vigente  > 0
    int stock; // Unidades disponibles
    int stockmin; // Si stock <= stockmin se muestra 'Bajo stock'
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// Estructura detalleOrden
struct detalleOrden {
    int idRepuesto; // Id del repuesto consumido
    int cantidad; // Unidades usadas de este repuesto

    // Se guarda el precio que tenia el repuesto en el momento exacto en el que se agrego, si se cambia el precio en el inventario la orden no se altera
    float precioUnitario;
};

// Estructura ordenServicio
struct ordenServicio {
    int id;
    int idVehiculo; // Relacion con el vehiculo atendido
    int idMecanico; // Relacion con el mecanico asignado
    char fechaApertura[11];
    char fechaCierre[11]; // Vacia mientras el estado sea 'Abierta'
    char estado[12]; // 'Abierta, cerrada, anulada'
    char descripcionFalla[200]; 
    int kmIngreso; // Kilometraje al entrar, se valida que no sea menor al valor historico

    // Se calcula dinamicamente sumando manoObra + sumatoria(cantidad * precioUnitario)
    float manoObra;

    detalleOrden detalles[15]; 
    int numDetalles; 
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// Estructura principal del sistema
// El sysTaller gigante en RAM desaparece. Ya no hay arreglos dinámicos.
struct sysTaller {
    Taller taller; 
};

#endif // STRUCT_TALLER_H
