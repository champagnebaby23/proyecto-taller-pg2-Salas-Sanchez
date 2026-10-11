#ifndef STRUCT_TALLER_H
#define STRUCT_TALLER_H

// Estructura del taller
struct Taller {
    char nombre[100];
    char rif[20];
    char direccion[200];
    char telefono[20];
};

// Estructura del vehiculo
struct Vehiculo {
    int id;
    char placa[10];
    char marca[50];
    char modelo[50];
    int anio;
    int km;
    char nombrePropietario[100];
    char cedulaPropietario[20];
    char telefonoPropietario[20];
    char fechaRegistro[11];
};

// Estructura del mecanico
struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20];
    char telefono[20];
    char especialidad[20];
    char fechaIngreso[11];
};

// Estructura del Repuesto
struct Repuesto {
    int id;
    char codigo[20];
    char nombre[100];
    char marca[50];
    float precio;
    int stock;
    int stockmin;
    char fechaRegistro[11];
};

// Estructura detalleOrden
struct detalleOrden {
    int idRepuesto;
    int cantidad;
    float precioUnitario;
};

// Estructura ordenServicio
struct ordenServicio {
    int id;
    int idVehiculo;
    int idMecanico;
    char fechaApertura[11];
    char fechaCierre[11];
    char estado[12];
    char descripcionFalla[200];
    int kmIngreso;
    float manoObra;

// Arreglo dinamico propio de cada orden de servicio
detalleOrden* detalles;
int numDetalles;
int capacidadDetalles;
};

// Estructura principal del sistema
struct sysTaller {
    Taller taller;
    Vehiculo*
    vehiculos;
    int numVehiculos;
    int capacidadVehiculos;
    Mecanico*
    mecanicos;
    int numMecanicos;
    int capacidadMecanicos;
    Repuesto*
    repuestos;
    int numRepuestos;
    int capacidadRepuestos;
    ordenServicio*
    ordenes;
    int numOrdenes;
    int capacidadOrdenes;

    int siguienteIdVehiculo;
    int siguienteIdMecanico;
    int siguienteIdRepuesto;
    int siguienteIdOrden;
};

#endif // STRUCT_TALLER_H
