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
