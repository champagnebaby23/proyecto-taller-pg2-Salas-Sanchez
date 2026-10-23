#include <iostream>
#include <iomanip>
#include <cstring>
#include "struct_taller.h"
#include "logica.h"
using namespace std;

// ====================
// CAPA DE PRESENTACION
// ====================
//Prototipos de los menus principales
void menuVehiculos(sysTaller &s);
void menuMecanicos(sysTaller &s);
void menuRepuestos(sysTaller &s);
void menuOrdenes(sysTaller &s);

// ====================
// MODULO DE VEHICULOS
// ====================
void mostrarVehiculo(Vehiculo v) {
    // Los ID nunca se muestran solos, siempre van acompa;ados de datos descriptivos de la entidad
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

    // Se usa cin >> ws para limpiar el bufer antes de usar un cin.getline y evitar que se salte la lectura
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
        // Se envia la estructura a logica.cpp
        Vehiculo* nuevo = agregarVehiculo(s, v);

        // La presentacion lee el resultado (puntero o nullptr) para imprimir el mensaje correspondiente indicando que pudo fallar
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
        // Recibiendo arreglos temporales: Retorna un Vehiculo**
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

    char confirmacion;
    cout << "¿Está seguro de eliminar físicamente este vehículo? (S/N): ";
    cin >> confirmacion;
    if (confirmacion == 'S' || confirmacion == 's') {
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
    int opcion;
    do {
        cout << "\n=== GESTION DE VEHICULOS ===" << endl;
        cout << "1. Registrar vehiculo" << endl;
        cout << "2. Buscar vehiculo" << endl;
        cout << "3. Listar vehiculos" << endl;
        cout << "4. Eliminar vehiculo" << endl;
        cout << "0. Volver al menu principal" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        switch(opcion) {
            case 1: menuRegistrarVehiculo(s); break;
            case 2: menuBuscarVehiculo(s); break;
            case 3: menuListarVehiculos(s); break;
            case 4: menuEliminarVehiculo(s); break;
            case 0: break;
            default: cout << "Opcion invalida." << endl;
        }
    } while(opcion != 0);
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

    char confirmacion;
    cout << "¿Guardar mecánico? (S/N): ";
    cin >> confirmacion;
    if (confirmacion == 'S' || confirmacion == 's') {
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
    cout << "\n--- LISTADO DE MECANICOS ---" << endl;
    if (cantidad == 0 || lista == nullptr) {
        cout << "No hay mecanicos registrados." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarMecanico(*lista[i]);
    }
    delete[] lista;
}

void menuMecanicos(sysTaller &s) {
    int opcion;
    do {
        cout << "\n=== GESTION DE MECANICOS ===" << endl;
        cout << "1. Registrar mecanico" << endl;
        cout << "2. Listar mecanicos" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opcion: ";
        cin >> opcion;
        if (opcion == 1) menuRegistrarMecanico(s);
        else if (opcion == 2) menuListarMecanicos(s);
    } while(opcion != 0);
}

void mostrarRepuesto(Repuesto r) {
    cout << "----------------------------------------" << endl;
    cout << "ID           : " << r.id << endl;
    cout << "Codigo       : " << r.codigo << endl;
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
    cout << "Codigo unico: ";
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
    cout << "Stock minimo de alerta: ";
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
            cout << "ERROR: Codigo duplicado o valores invalidos (precio/stock)." << endl;
        }
    }
}

void menuEntradaMercancia(sysTaller &s) {
    int id, cant;
    cout << "\n--- ENTRADA DE MERCANCIA ---" << endl;
    cout << "ID del repuesto: ";
    cin >> id;
    cout << "Cantidad a sumar al stock: ";
    cin >> cant;
    if (registrarEntradaMercancia(s, id, cant)) {
        cout << "¡Stock actualizado con exito!" << endl;
    } else {
        cout << "ERROR: Repuesto no encontrado o cantidad invalida." << endl;
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
        cout << "¡Precio actualizado con exito!" << endl;
    } else {
        cout << "ERROR: Repuesto no encontrado o precio invalido." << endl;
    }
}

void menuListarRepuestos(sysTaller &s) {
    int cantidad = 0;
    Repuesto** lista = listarRepuestos(s, cantidad);
    cout << "\n--- INVENTARIO DE REPUESTOS ---" << endl;
    if (cantidad == 0 || lista == nullptr) {
        cout << "Inventario vacio." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarRepuesto(*lista[i]);
    }
    delete[] lista;
}

void menuRepuestos(sysTaller &s) {
    int opcion;
    do {
        cout << "\n=== INVENTARIO DE REPUESTOS ===" << endl;
        cout << "1. Registrar repuesto" << endl;
        cout << "2. Registrar entrada de mercancia" << endl;
        cout << "3. Cambiar precio" << endl;
        cout << "4. Listar repuestos" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opcion: ";
        cin >> opcion;
        switch(opcion) {
            case 1: menuRegistrarRepuesto(s); break;
            case 2: menuEntradaMercancia(s); break;
            case 3: menuCambiarPrecio(s); break;
            case 4: menuListarRepuestos(s); break;
        }
    } while(opcion != 0);
}

void mostrarDetalleOrden(sysTaller &s, const ordenServicio &o) {
    Vehiculo* v = buscarVehiculoPorID(s, o.idVehiculo);
    Mecanico* m = buscarMecanicoPorID(s, o.idMecanico);

    cout << "\n========================================" << endl;
    cout << "         ORDEN DE SERVICIO #" << o.id << endl;
    cout << "========================================" << endl;
    cout << "Estado     : " << o.estado << endl;
    cout << "Apertura   : " << o.fechaApertura << " | Cierre: " << (strlen(o.fechaCierre) > 0 ? o.fechaCierre : "N/A") << endl;
    if (v) cout << "Vehiculo   : " << v->placa << " - " << v->marca << " " << v->modelo << " (" << o.kmIngreso << " km)" << endl;
    if (m) cout << "Mecanico   : " << m->nombre << " (" << m->especialidad << ")" << endl;
    cout << "Falla      : " << o.descripcionFalla << endl;
    cout << "----------------------------------------" << endl;
    cout << left << setw(8) << "Codigo" << setw(18) << "Repuesto" << setw(6) << "Cant" << setw(8) << "P.Unit" << setw(8) << "Total" << endl;
    cout << "----------------------------------------" << endl;

    for (int i = 0; i < o.numDetalles; i++) {
        Repuesto* r = buscarRepuestoPorID(s, o.detalles[i].idRepuesto);
        string nomRep = r ? r->nombre : "Desconocido";
        string codRep = r ? r->codigo : "Ninguno";
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
    cout << "ID del vehiculo: ";
    cin >> idV;
    cout << "ID del mecanico: ";
    cin >> idM;
    cout << "Fecha de apertura (YYYY-MM-DD): ";
    cin >> fecha;
    cout << "Kilometraje de ingreso: ";
    cin >> km;
    cout << "Descripcion de la falla: ";
    cin >> ws;
    cin.getline(falla, 200);

    ordenServicio* nueva = abrirOrdenServicio(s, idV, idM, fecha, falla, km);
    if (nueva != nullptr) {
        cout << "¡Orden de servicio #" << nueva->id << " abierta exitosamente!" << endl;
    } else {
        cout << "ERROR: No se pudo abrir la orden. Verifique si el vehículo/mecanico existen, si el vehículo ya tiene otra orden abierta, si el mecánico supero el limite de 3 ordenes o si el kilometraje es menor al registrado." << endl;
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
    cout << "ID del repuesto a quitar de la linea: ";
    cin >> idR;

    if (quitarRepuestoDeOrden(s, idO, idR)) {
        cout << "¡Linea de repuesto removida y stock devuelto al inventario!" << endl;
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
    cout << "¿Desea cerrar esta orden? (Ya no se podra modificar) (S/N): ";
    cin >> conf;
    if (conf == 'S' || conf == 's') {
        if (cerrarOrdenServicio(s, idO, fecha, mano)) {
            cout << "¡Orden cerrada exitosamente!" << endl;
        } else {
            cout << "ERROR: Orden no encontrada, ya no esta abierta o datos de mano de obra invalidos." << endl;
        }
    }
}

void menuAnularOrden(sysTaller &s) {
    int idO;
    cout << "\n--- ANULAR ORDEN DE SERVICIO ---" << endl;
    cout << "ID de la orden a anular: ";
    cin >> idO;

    char conf;
    cout << "¿Confirma la anulacion? (Los repuestos volveran al inventario) (S/N): ";
    cin >> conf;
    if (conf == 'S' || conf == 's') {
        if (anularOrdenServicio(s, idO)) {
            cout << "¡Orden anulada y stock devuelto al inventario!" << endl;
        } else {
            cout << "ERROR: Orden no encontrada o estado invalido para anular." << endl;
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
    int opcion;
    do {
        cout << "\n=== ORDENES DE SERVICIO ===" << endl;
        cout << "1. Abrir orden" << endl;
        cout << "2. Agregar repuesto a orden" << endl;
        cout << "3. Quitar repuesto de orden" << endl;
        cout << "4. Cerrar orden" << endl;
        cout << "5. Anular orden" << endl;
        cout << "6. Ver detalle de orden" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opcion: ";
        cin >> opcion;
        switch(opcion) {
            case 1: menuAbrirOrden(s); break;
            case 2: menuAgregarRepuestoOrden(s); break;
            case 3: menuQuitarRepuestoOrden(s); break;
            case 4: menuCerrarOrden(s); break;
            case 5: menuAnularOrden(s); break;
            case 6: menuVerDetalleOrden(s); break;
        }
    } while(opcion != 0);
}

int main() {
    sysTaller miTaller;

    inicializarSistema(miTaller, "Taller Mecánico URDANETA", "J-00000000-0", "Av. Principal del Taller", "0261-5550000");

    int opcion;
    do {
        cout << "\n========================================" << endl;
        cout << "     SISTEMA DE GESTION DE TALLER       " << endl;
        cout << "     Taller: " << miTaller.taller.nombre << endl;
        cout << "========================================" << endl;
        cout << "1. Gestión de Vehiculos" << endl;
        cout << "2. Gestión de Mecanicos" << endl;
        cout << "3. Inventario de Repuestos" << endl;
        cout << "4. Ordenes de Servicio" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1: menuVehiculos(miTaller); break;
            case 2: menuMecanicos(miTaller); break;
            case 3: menuRepuestos(miTaller); break;
            case 4: menuOrdenes(miTaller); break;
            case 0: {
                char conf;
                cout << "¿Esta seguro de salir? (Se perderán los datos en memoria) (S/N): ";
                cin >> conf;
                if (conf != 'S' && conf != 's') {
                    opcion = -1; // Evita que salga si cancela
                }
                break;
            }
            default:
                cout << "Opcion invalida. Intente nuevamente." << endl;
        }
    } while (opcion != 0);

    liberarSistema(miTaller);
    cout << "\nMemoria liberada exitosamente. ¡Programa finalizado con exito!" << endl;

    return 0;
}

