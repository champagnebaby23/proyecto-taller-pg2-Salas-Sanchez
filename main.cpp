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

}

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

void mostrarRepuesto(Repuesto r) {
    cout << "----------------------------------------" << endl;
    cout << "ID           : " << r.id << endl;
    cout << "Código       : " << r.codigo << endl;
    cout << "Nombre       : " << r.nombre << " (" << r.marca << ")" << endl;
    cout << "Precio       : $" << r.precio << endl;
    cout << "Stock        : " << r.stock << " (Mínimo: " << r.stockmin << ")" << endl;
    if (r.stock <= r.stockmin) {
        cout << "ESTADO       : ** BAJO STOCK **" << endl;
    }
    cout << "----------------------------------------" << endl;
}

void menuRegistrarRepuesto(sysTaller &s) {
    Repuesto r;
    cout << "\n--- REGISTRAR REPUESTO ---" << endl;
    cout << "Código único: ";
    cin >> r.codigo;
    cout << "Nombre del repuesto: ";
    cin >> ws;
    cin.getline(r.nombre, 100);
    cout << "Marca: ";
    cin.getline(r.marca, 50);
    cout << "Precio unitario: ";
    cin >> r.precio;
    cout << "Stock inicial: ";
    cin >> r.stock;
    cout << "Stock mínimo de alerta: ";
    cin >> r.stockmin;
    cout << "Fecha registro (YYYY-MM-DD): ";
    cin >> r.fechaRegistro;

    char conf;
    cout << "¿Guardar repuesto? (S/N): ";
    cin >> conf;
    if (conf == 'S' || conf == 's') {
        Repuesto* nuevo = registrarRepuesto(s, r);
        if (nuevo != nullptr) {
            cout << "¡Repuesto registrado con ID #" << nuevo->id << "!" << endl;
        } else {
            cout << "ERROR: Código duplicado o valores inválidos (precio/stock)." << endl;
        }
    }
}

void menuEntradaMercancia(sysTaller &s) {
    int id, cant;
    cout << "\n--- ENTRADA DE MERCANCÍA ---" << endl;
    cout << "ID del repuesto: ";
    cin >> id;
    cout << "Cantidad a sumar al stock: ";
    cin >> cant;
    if (registrarEntradaMercancia(s, id, cant)) {
        cout << "¡Stock actualizado con éxito!" << endl;
    } else {
        cout << "ERROR: Repuesto no encontrado o cantidad inválida." << endl;
    }
}

void menuCambiarPrecio(sysTaller &s) {
    int id;
    float nuevoP;
    cout << "\n--- CAMBIAR PRECIO DE REPUESTO ---" << endl;
    cout << "ID del repuesto: ";
    cin >> id;
    cout << "Nuevo precio unitario: ";
    cin >> nuevoP;
    if (cambiarPrecioRepuesto(s, id, nuevoP)) {
        cout << "¡Precio actualizado con éxito!" << endl;
    } else {
        cout << "ERROR: Repuesto no encontrado o precio inválido." << endl;
    }
}

void menuListarRepuestos(sysTaller &s) {
    int cantidad = 0;
    Repuesto** lista = listarRepuestos(s, cantidad);
    cout << "\n--- INVENTARIO DE REPUESTOS ---" << endl;
    if (cantidad == 0 || lista == nullptr) {
        cout << "Inventario vacío." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarRepuesto(*lista[i]);
    }
    delete[] lista;
}

void menuRepuestos(sysTaller &s) {
    int op;
    do {
        cout << "\n=== INVENTARIO DE REPUESTOS ===" << endl;
        cout << "1. Registrar repuesto" << endl;
        cout << "2. Registrar entrada de mercancía" << endl;
        cout << "3. Cambiar precio" << endl;
        cout << "4. Listar repuestos" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opción: ";
        cin >> op;
        switch(op) {
            case 1: menuRegistrarRepuesto(s); break;
            case 2: menuEntradaMercancia(s); break;
            case 3: menuCambiarPrecio(s); break;
            case 4: menuListarRepuestos(s); break;
        }
    } while(op != 0);
}

void mostrarDetalleOrden(sysTaller &s, const ordenServicio &o) {
    Vehiculo* v = buscarVehiculoPorID(s, o.idVehiculo);
    Mecanico* m = buscarMecanicoPorID(s, o.idMecanico);

    cout << "\n========================================" << endl;
    cout << "         ORDEN DE SERVICIO #" << o.id << endl;
    cout << "========================================" << endl;
    cout << "Estado     : " << o.estado << endl;
    cout << "Apertura   : " << o.fechaApertura << " | Cierre: " << (strlen(o.fechaCierre) > 0 ? o.fechaCierre : "N/A") << endl;
    if (v) cout << "Vehículo   : " << v->placa << " - " << v->marca << " " << v->modelo << " (" << o.kmIngreso << " km)" << endl;
    if (m) cout << "Mecánico   : " << m->nombre << " (" << m->especialidad << ")" << endl;
    cout << "Falla      : " << o.descripcionFalla << endl;
    cout << "----------------------------------------" << endl;
    cout << left << setw(8) << "Código" << setw(18) << "Repuesto" << setw(6) << "Cant" << setw(8) << "P.Unit" << setw(8) << "Total" << endl;
    cout << "----------------------------------------" << endl;

    for (int i = 0; i < o.numDetalles; i++) {
        Repuesto* r = buscarRepuestoPorID(s, o.detalles[i].idRepuesto);
        string nomRep = r ? r->nombre : "Desconocido";
        string codRep = r ? r->codigo : "N/A";
        float subtotal = o.detalles[i].cantidad * o.detalles[i].precioUnitario;
        cout << left << setw(8) << codRep << setw(18) << nomRep.substr(0,16) 
             << setw(6) << o.detalles[i].cantidad << setw(8) << o.detalles[i].precioUnitario 
             << setw(8) << subtotal << endl;
    }
    cout << "----------------------------------------" << endl;
    cout << "Mano de obra: $" << o.manoObra << endl;
    cout << "TOTAL       : $" << calcularTotalOrden(o) << endl;
    cout << "========================================" << endl;
}

void menuAbrirOrden(sysTaller &s) {
    int idV, idM, km;
    char fecha[11], falla[200];
    cout << "\n--- ABRIR ORDEN DE SERVICIO ---" << endl;
    cout << "ID del vehículo: ";
    cin >> idV;
    cout << "ID del mecánico: ";
    cin >> idM;
    cout << "Fecha de apertura (YYYY-MM-DD): ";
    cin >> fecha;
    cout << "Kilometraje de ingreso: ";
    cin >> km;
    cout << "Descripción de la falla: ";
    cin >> ws;
    cin.getline(falla, 200);

    ordenServicio* nueva = abrirOrdenServicio(s, idV, idM, fecha, falla, km);
    if (nueva != nullptr) {
        cout << "¡Orden de servicio #" << nueva->id << " abierta exitosamente!" << endl;
    } else {
        cout << "ERROR: No se pudo abrir la orden. Verifique si el vehículo/mecánico existen, si el vehículo ya tiene otra orden abierta, si el mecánico superó el límite de 3 órdenes o si el kilometraje es menor al registrado." << endl;
    }
}

void menuAgregarRepuestoOrden(sysTaller &s) {
    int idO, idR, cant;
    cout << "\n--- AGREGAR REPUESTO A ORDEN ---" << endl;
    cout << "ID de la orden: ";
    cin >> idO;
    cout << "ID del repuesto: ";
    cin >> idR;
    cout << "Cantidad: ";
    cin >> cant;

    if (agregarRepuestoAOrden(s, idO, idR, cant)) {
        cout << "¡Repuesto agregado a la orden correctamente!" << endl;
    } else {
        cout << "ERROR: Orden no abierta, repuesto no encontrado o stock insuficiente." << endl;
    }
}

void menuQuitarRepuestoOrden(sysTaller &s) {
    int idO, idR;
    cout << "\n--- QUITAR REPUESTO DE ORDEN ---" << endl;
    cout << "ID de la orden: ";
    cin >> idO;
    cout << "ID del repuesto a quitar de la línea: ";
    cin >> idR;

    if (quitarRepuestoDeOrden(s, idO, idR)) {
        cout << "¡Línea de repuesto removida y stock devuelto al inventario!" << endl;
    } else {
        cout << "ERROR: Orden no abierta o repuesto no encontrado en la orden." << endl;
    }
}

void menuCerrarOrden(sysTaller &s) {
    int idO;
    float mano;
    char fecha[11];
    cout << "\n--- CERRAR ORDEN DE SERVICIO ---" << endl;
    cout << "ID de la orden: ";
    cin >> idO;
    cout << "Fecha de cierre (YYYY-MM-DD): ";
    cin >> fecha;
    cout << "Costo de mano de obra: ";
    cin >> mano;

    char conf;
    cout << "¿Desea cerrar esta orden? (Ya no se podrá modificar) (S/N): ";
    cin >> conf;
    if (conf == 'S' || conf == 's') {
        if (cerrarOrdenServicio(s, idO, fecha, mano)) {
            cout << "¡Orden cerrada exitosamente!" << endl;
        } else {
            cout << "ERROR: Orden no encontrada, ya no está abierta o datos de mano de obra inválidos." << endl;
        }
    }
}

void menuAnularOrden(sysTaller &s) {
    int idO;
    cout << "\n--- ANULAR ORDEN DE SERVICIO ---" << endl;
    cout << "ID de la orden a anular: ";
    cin >> idO;

    char conf;
    cout << "¿Confirma la anulación? (Los repuestos volverán al inventario) (S/N): ";
    cin >> conf;
    if (conf == 'S' || conf == 's') {
        if (anularOrdenServicio(s, idO)) {
            cout << "¡Orden anulada y stock devuelto al inventario!" << endl;
        } else {
            cout << "ERROR: Orden no encontrada o estado inválido para anular." << endl;
        }
    }
}

void menuVerDetalleOrden(sysTaller &s) {
    int idO;
    cout << "Ingrese el ID de la orden: ";
    cin >> idO;
    ordenServicio* o = buscarOrdenPorID(s, idO);
    if (o != nullptr) {
        mostrarDetalleOrden(s, *o);
    } else {
        cout << "ERROR: Orden no encontrada." << endl;
    }
}

void menuOrdenes(sysTaller &s) {
    int op;
    do {
        cout << "\n=== ÓRDENES DE SERVICIO ===" << endl;
        cout << "1. Abrir orden" << endl;
        cout << "2. Agregar repuesto a orden" << endl;
        cout << "3. Quitar repuesto de orden" << endl;
        cout << "4. Cerrar orden" << endl;
        cout << "5. Anular orden" << endl;
        cout << "6. Ver detalle de orden" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opción: ";
        cin >> op;
        switch(op) {
            case 1: menuAbrirOrden(s); break;
            case 2: menuAgregarRepuestoOrden(s); break;
            case 3: menuQuitarRepuestoOrden(s); break;
            case 4: menuCerrarOrden(s); break;
            case 5: menuAnularOrden(s); break;
            case 6: menuVerDetalleOrden(s); break;
        }
    } while(op != 0);
}