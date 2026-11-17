#ifndef STRUCT_TALLER_H
#define STRUCT_TALLER_H

#include <ctime>

// ==================================
// CONSTANTES GLOBALES DEL SISTEMA
// ==================================
const int MAX_LINEAS_DETALLE = 15;  // Mínimo 15 lineas
const int MAX_RESULTADOS = 20;      // Cantidad fija de resultados para búsquedas

// ==============================================
// METADATA ADMINISTRATIVA DE ARCHIVOS BINARIOS
// ==============================================
struct ArchivoHeader {
    int cantidadRegistros; // Total histórico (incluye eliminados)
    int proximoID;         // Siguiente ID a asignar
    int registrosActivos;  // Registros con eliminado == false
    int version;           // Versión del formato del archivo (iniciar en 1)
};

// ==================
// ENTIDAD: TALLER
// ==================
struct Taller {
    char nombre[100];
    char rif[20];
    char direccion[200];
    char telefono[20];
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// ==================
// ENTIDAD: VEHICULO
// ==================
struct Vehiculo {
    int id;
    char placa[10];
    char marca[50];
    char modelo[50];
    int anio;
    int kilometraje;
    char nombrePropietario[100];
    char cedulaPropietario[20];
    char telefonoPropietario[20];
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// ===================
// ENTIDAD: MECANICO 
// ===================
struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20];
    char especialidad[20];
    char fechaIngreso[11];
    bool eliminado; // true = dado de baja
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// ================================
// ENTIDAD: REPUESTO
// ================================
struct Repuesto {
    int id;
    char codigo[20];
    char nombre[100];
    char marca[50];
    float precio;
    int stock;
    int stockMinimo;
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// ============================
// LÍNEA DE DETALLE DE ORDEN
// ============================
struct DetalleOrden {
    int idRepuesto;       // ID del repuesto consumido
    int cantidad;         // Unidades usadas
    float precioUnitario; // Precio histórico vigente al momento de agregar
};

// =============================
// ENTIDAD: ORDEN DE SERVICIO
// =============================
struct OrdenServicio {
    int id;
    int idVehiculo;
    int idMecanico;
    char fechaApertura[11];
    char fechaCierre[11];
    char estado[12]; // "ABIERTA", "CERRADA" o "ANULADA"
    char descripcionFalla[200];
    int kilometrajeIngreso;
    float manoObra;

    DetalleOrden detalles[MAX_LINEAS_DETALLE];
    int numDetalles;

    bool eliminado; // Las ordenes no se eliminan: siempre false
    time_t fechaCreacion;
    time_t fechaUltimaModificacion;
};

// Alias de compatibilidad temporal durante la migración
using ordenServicio = OrdenServicio;
using detalleOrden = DetalleOrden;

// ======================================
// ESTRUCTURAS AUXILIARES PARA REPORTES
// ======================================
struct TopRepuesto {
    char codigo[20];
    char nombre[100];
    int unidades;
};

struct FilaFacturacionMecanico {
    char nombre[100];
    bool dadoDeBaja;
    int ordenesCerradas;
    float totalManoObra;
    float totalRepuestos;
};

#endif // STRUCT_TALLER_H
