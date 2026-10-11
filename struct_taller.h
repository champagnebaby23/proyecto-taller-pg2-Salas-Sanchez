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
