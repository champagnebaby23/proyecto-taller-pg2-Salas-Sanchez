#include <iostream>
#include <iomanip>
#include <cstring>
#include "struct_taller.h"
#include "logica.h"
using namespace std;

void menuVehiculos(sysTaller &s);
void menuMecanicos(sysTaller &s);
void menuRepuestos(sysTaller &s);
void menuOrdenes(sysTaller &s);

void mostrarVehiculo(Vehiculo v) {
    cout << "----------------------------------------" << endl;
    cout << "ID          : " << v.id << endl;
    cout << "Placa       : " << v.placa << endl;
    cout << "Vehículo    : " << v.marca << " " << v.modelo << " (" << v.anio << ")" << endl;
    cout << "Kilometraje : " << v.km << " km" << endl;
    cout << "Propietario : " << v.nombrePropietario << " (Cédula: " << v.cedulaPropietario << ")" << endl;
    cout << "Teléfono    : " << v.telefonoPropietario << endl;
    cout << "Registro    : " << v.fechaRegistro << endl;
    cout << "----------------------------------------" << endl;

void menuRegistrarVehiculo(sysTaller &s) {
    Vehiculo v;
    cout << "\n--- REGISTRAR NUEVO VEHÍCULO ---" << endl;
    cout << "Placa (alfanumérica, 6-7 caracteres): ";
    cin >> v.placa;
    
    cout << "Marca: ";
    cin >> ws;
    cin.getline(v.marca, 50);
    
    cout << "Modelo: ";
    cin.getline(v.modelo, 50);
    
    cout << "Año de fabricación (1950-2027): ";
    cin >> v.anio;
    
    cout << "Kilometraje actual: ";
    cin >> v.km;
    
    cout << "Nombre completo del propietario: ";
    cin >> ws;
    cin.getline(v.nombrePropietario, 100);
    
    cout << "Cédula o RIF del propietario: ";
    cin >> v.cedulaPropietario;
    
    cout << "Teléfono del propietario: ";
    cin >> v.telefonoPropietario;
    
    cout << "Fecha de registro (YYYY-MM-DD): ";
    cin >> v.fechaRegistro;

    char confirmacion;
    cout << "¿Desea guardar este vehículo? (S/N): ";
    cin >> confirmacion;

    if (confirmacion == 'S' || confirmacion == 's') {
        Vehiculo* nuevo = agregarVehiculo(s, v);
        if (nuevo != nullptr) {
            cout << "¡Vehículo registrado exitosamente con ID #" << nuevo->id << "!" << endl;
        } else {
            cout << "ERROR: La placa '" << v.placa << "' ya está registrada o los datos son inválidos (año/km)." << endl;
        }
    } else {
        cout << "Registro cancelado." << endl;
    }
}

void menuBuscarVehiculo(sysTaller &s) {
    int opcionBusqueda;
    cout << "\n--- BUSCAR VEHÍCULO ---" << endl;
    cout << "1. Buscar por ID" << endl;
    cout << "2. Buscar por Placa exacta" << endl;
    cout << "3. Buscar por Nombre de Propietario (parcial)" << endl;
    cout << "Seleccione una opción: ";
    cin >> opcionBusqueda;

    if (opcionBusqueda == 1) {
        int id;
        cout << "Ingrese el ID: ";
        cin >> id;
        Vehiculo* v = buscarVehiculoPorID(s, id);
        if (v != nullptr) mostrarVehiculo(*v);
        else cout << "ERROR: Vehículo con ID #" << id << " no encontrado." << endl;
    } 
    else if (opcionBusqueda == 2) {
        char placa[10];
        cout << "Ingrese la placa exacta: ";
        cin >> placa;
        Vehiculo* v = buscarVehiculoPorPlaca(s, placa);
        if (v != nullptr) mostrarVehiculo(*v);
        else cout << "ERROR: Vehículo con placa '" << placa << "' no encontrado." << endl;
    } 
    else if (opcionBusqueda == 3) {
        char sub[100];
        cout << "Ingrese el nombre o parte del propietario: ";
        cin >> ws;
        cin.getline(sub, 100);
        int cantidad = 0;
        Vehiculo** lista = buscarVehiculosPorPropietario(s, sub, cantidad);
        if (cantidad == 0 || lista == nullptr) {
            cout << "No se encontraron vehículos para el criterio \"" << sub << "\"." << endl;
        } else {
            cout << "\nSe encontraron " << cantidad << " vehículo(s):" << endl;
            for (int i = 0; i < cantidad; i++) {
                mostrarVehiculo(*lista[i]);
            }
            delete[] lista; // Liberar arreglo temporal de punteros
        }
    }
}

void menuListarVehiculos(sysTaller &s) {
    int cantidad = 0;
    Vehiculo** lista = listarVehiculos(s, cantidad);
    cout << "\n--- LISTADO GENERAL DE VEHÍCULOS ---" << endl;
    if (cantidad == 0 || lista == nullptr) {
        cout << "No hay vehículos registrados." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarVehiculo(*lista[i]);
    }
    cout << "Total de vehículos: " << cantidad << endl;
    delete[] lista;
}

void menuEliminarVehiculo(sysTaller &s) {
    int id;
    cout << "\n--- ELIMINAR VEHÍCULO ---" << endl;
    cout << "Ingrese el ID del vehículo a eliminar: ";
    cin >> id;

    char conf;
    cout << "¿Está seguro de eliminar físicamente este vehículo? (S/N): ";
    cin >> conf;
    if (conf == 'S' || conf == 's') {
        if (eliminarVehiculo(s, id)) {
            cout << "¡Vehículo eliminado exitosamente!" << endl;
        } else {
            cout << "ERROR: No se pudo eliminar. El vehículo no existe o tiene órdenes de servicio asociadas." << endl;
        }
    } else {
        cout << "Operación cancelada." << endl;
    }
}

void menuVehiculos(sysTaller &s) {
    int op;
    do {
        cout << "\n=== GESTIÓN DE VEHÍCULOS ===" << endl;
        cout << "1. Registrar vehículo" << endl;
        cout << "2. Buscar vehículo" << endl;
        cout << "3. Listar vehículos" << endl;
        cout << "4. Eliminar vehículo" << endl;
        cout << "0. Volver al menú principal" << endl;
        cout << "Seleccione una opción: ";
        cin >> op;
        switch(op) {
            case 1: menuRegistrarVehiculo(s); break;
            case 2: menuBuscarVehiculo(s); break;
            case 3: menuListarVehiculos(s); break;
            case 4: menuEliminarVehiculo(s); break;
            case 0: break;
            default: cout << "Opción inválida." << endl;
        }
    } while(op != 0);
}

void mostrarMecanico(Mecanico m) {
    cout << "----------------------------------------" << endl;
    cout << "ID           : " << m.id << endl;
    cout << "Nombre       : " << m.nombre << endl;
    cout << "Cédula       : " << m.cedula << endl;
    cout << "Especialidad : " << m.especialidad << endl;
    cout << "Fecha Ingreso: " << m.fechaIngreso << endl;
    cout << "----------------------------------------" << endl;
}

void menuRegistrarMecanico(sysTaller &s) {
    Mecanico m;
    cout << "\n--- REGISTRAR MECÁNICO ---" << endl;
    cout << "Nombre completo: ";
    cin >> ws;
    cin.getline(m.nombre, 100);
    cout << "Cédula: ";
    cin >> m.cedula;
    cout << "Especialidad (MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION, GENERAL): ";
    cin >> m.especialidad;
    cout << "Fecha de ingreso (YYYY-MM-DD): ";
    cin >> m.fechaIngreso;

    char conf;
    cout << "¿Guardar mecánico? (S/N): ";
    cin >> conf;
    if (conf == 'S' || conf == 's') {
        Mecanico* nuevo = registrarMecanico(s, m);
        if (nuevo != nullptr) {
            cout << "¡Mecánico registrado con ID #" << nuevo->id << "!" << endl;
        } else {
            cout << "ERROR: Cédula duplicada o especialidad inválida." << endl;
        }
    }
}

void menuListarMecanicos(sysTaller &s) {
    int cantidad = 0;
    Mecanico** lista = listarMecanicos(s, cantidad);
    cout << "\n--- LISTADO DE MECÁNICOS ---" << endl;
    if (cantidad == 0 || lista == nullptr) {
        cout << "No hay mecánicos registrados." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarMecanico(*lista[i]);
    }
    delete[] lista;
}

void menuMecanicos(sysTaller &s) {
    int op;
    do {
        cout << "\n=== GESTIÓN DE MECÁNICOS ===" << endl;
        cout << "1. Registrar mecánico" << endl;
        cout << "2. Listar mecánicos" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opción: ";
        cin >> op;
        if (op == 1) menuRegistrarMecanico(s);
        else if (op == 2) menuListarMecanicos(s);
    } while(op != 0);
}

