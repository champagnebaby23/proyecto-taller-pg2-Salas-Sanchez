#ifndef STRUCT_TALLER_H
#define STRUCT_TALLER_H

// Estructura del taller (Informacion unica)
struct Taller {
    char nombre[100]; // Nombre del establecimiento
    char rif[20]; // Identificar fiscal
    char direccion[200]; // Ubicacion fisica del taller
    char telefono[20]; // Telefono de contacto
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
    char fechaRegistro[11]; // YYYY-MM-DD
};

// Estructura del mecanico
struct Mecanico {
    int id;
    char nombre[100];
    char cedula[20]; // Validada en logica para ser unica
    char telefono[20];
    char especialidad[20]; // Validada como "MOTOR" "FRENOS", etc en la logica
    char fechaIngreso[11];
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
    char fechaRegistro[11];
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

// Arreglo dinamico propio de cada orden de servicio
// Al abrir la orden, se inicializa con new detalleOrden[2] (capacidad 2)
detalleOrden* detalles;
int numDetalles; // Cuantas lineas de repuestos tiene actualmente
int capacidadDetalles; // Cuanta RAM esta asignada para la orden
};

// Estructura principal del sistema
struct sysTaller {
    Taller taller; // Instancia unica con los datos del local

    // Arreglo dinamico de vehiculos
    Vehiculo* vehiculos; // Puntero al inicio del arreglo dinamico en RAM
    int numVehiculos; // Cantidad de vehiculos registrados
    int capacidadVehiculos; // Espacio reservado con new (inicia con 4)
    
    // Arreglo dinamico de mecanicos
    Mecanico* mecanicos;
    int numMecanicos;
    int capacidadMecanicos; // Si num == capacidad se duplica la capacidad
    
    // Arreglo dinamico de repuestos
    Repuesto* repuestos;
    int numRepuestos;
    int capacidadRepuestos;

    // Arreglo dinamico de ordenes
    ordenServicio* ordenes;
    int numOrdenes;
    int capacidadOrdenes;

    // Generador de llaves primarias 'ID' 
    // Se inicializan en 1, garantizan que cada orden tenga un id unico, incluso si se anulan ordenes anteriores
    int siguienteIdVehiculo;
    int siguienteIdMecanico;
    int siguienteIdRepuesto;
    int siguienteIdOrden;
};

#endif // STRUCT_TALLER_H
