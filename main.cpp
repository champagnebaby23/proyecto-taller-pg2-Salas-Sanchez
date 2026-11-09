#include <iostream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <string>
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
    cout << "Vehiculo    : " << v.marca << " " << v.modelo << " (" << v.anio << ")" << endl;
    cout << "Kilometraje : " << v.km << " km" << endl;
    cout << "Propietario : " << v.nombrePropietario << " (Cedula: " << v.cedulaPropietario << ")" << endl;
    cout << "Telefono    : " << v.telefonoPropietario << endl;
    cout << "Registro    : " << v.fechaRegistro << endl;
    cout << "----------------------------------------" << endl;

}

void menuRegistrarVehiculo(sysTaller &s) {
    Vehiculo v;
    cout << "\n--- REGISTRAR NUEVO VEHICULO ---" << endl;
    leerTextoNoVacio("Placa (alfanumerica): ", v.placa, 10);

    // Se usa cin >> ws para limpiar el bufer antes de usar un cin.getline y evitar que se salte la lectura
    leerTextoNoVacio("Marca: ", v.marca, 50);
    
    leerTextoNoVacio("Modelo: ", v.modelo, 50);
    
    v.anio = leerEnteroSeguro("Ano de Fabricacion (1950-2027): ", 1950, 2027);
    
    v.km = leerEnteroSeguro("Kilometraje actual (>= 0): ", 0, 2000000);
    
    leerTextoSoloLetras("Nombre completo del propietario: ", v.nombrePropietario, 100);
    leerTextoNoVacio("Cedula o RIF del propietario: ", v.cedulaPropietario, 20);
    leerTelefonoSeguro("Telefono del propietario: ", v.telefonoPropietario, 20);

    char confirmacion;
    cout << "¿Desea guardar este vehiculo? (S/N): ";
    cin >> confirmacion;

    if (confirmacion == 'S' || confirmacion == 's') {
        // Se envia la estructura a logica.cpp
        Vehiculo* nuevo = agregarVehiculo(s, v);

        // La presentacion lee el resultado (puntero o nullptr) para imprimir el mensaje correspondiente indicando que pudo fallar
        if (nuevo != nullptr) {
            cout << "¡Vehiculo registrado exitosamente con ID #" << nuevo->id << "!" << endl;
        } else {
            cout << "ERROR: La placa '" << v.placa << "' ya esta registrada o los datos son invalidos (ano/km)." << endl;
        }
    } else {
        cout << "Registro cancelado." << endl;
    }
}

void menuBuscarVehiculo(sysTaller &s) {
    int opcionBusqueda;
    cout << "\n--- BUSCAR VEHICULO ---" << endl;
    cout << "1. Buscar por ID" << endl;
    cout << "2. Buscar por Placa exacta" << endl;
    cout << "3. Buscar por Nombre de Propietario (parcial)" << endl;
    
    opcionBusqueda = leerEnteroSeguro("Seleccione una opcion: ", 1, 3);

    if (opcionBusqueda == 1) {
        int id;
        id = leerEnteroSeguro("Ingrese el ID: ", 1, 999999);
        
        Vehiculo* v = buscarVehiculoPorID(s, id);
        if (v != nullptr) mostrarVehiculo(*v);
        else cout << "ERROR: Vehiculo con ID #" << id << " no encontrado." << endl;
    } 
    else if (opcionBusqueda == 2) {
        char placa[10];
        leerTextoNoVacio("Ingrese la placa exacta: ", placa, 10);
        
        Vehiculo* v = buscarVehiculoPorPlaca(s, placa);
        if (v != nullptr) mostrarVehiculo(*v);
        else cout << "ERROR: Vehiculo con placa '" << placa << "' no encontrado." << endl;
    } 
    else if (opcionBusqueda == 3) {
        char sub[100];
        leerTextoSoloLetras("Ingrese el nombre o parte del propietario: ", sub, 100);
        
        int cantidad = 0;

        Vehiculo** lista = buscarVehiculosPorPropietario(s, sub, cantidad);
        if (cantidad == 0 || lista == nullptr) {
            cout << "No se encontraron vehiculos para el criterio \"" << sub << "\"." << endl;
        } else {
            cout << "\nSe encontraron " << cantidad << " vehiculo(s):" << endl;
            for (int i = 0; i < cantidad; i++) {
                mostrarVehiculo(*lista[i]);
            }
            delete[] lista;
        }
    }
}

void menuListarVehiculos(sysTaller &s) {
    int cantidad = 0;
    Vehiculo** lista = listarVehiculos(s, cantidad);
    cout << "\n--- LISTADO GENERAL DE VEHICULOS ---" << endl;
    if (cantidad == 0 || lista == nullptr) {
        cout << "No hay vehiculos registrados." << endl;
        return;
    }
    for (int i = 0; i < cantidad; i++) {
        mostrarVehiculo(*lista[i]);
    }
    cout << "Total de vehiculos: " << cantidad << endl;
    delete[] lista;
}

void menuActualizarVehiculo(sysTaller &s) {
    cout << "\n--- ACTUALIZAR DATOS DE VEHICULO ---" << endl;
    
    // Usamos la funcion de validacion para evitar errores al pedir el ID
    int id = leerEnteroSeguro("Ingrese el ID del vehiculo a actualizar: ", 1, 999999);
    
    Vehiculo* actual = buscarVehiculoPorID(s, id);
    
    if (actual == nullptr) {
        cout << "ERROR: No se encontro ningun vehiculo registrado con el ID #" << id << "." << endl;
        return;
    }

    cout << "\nVehiculo encontrado. Ingrese los nuevos datos (Placa, KM y Registro no se pueden modificar)." << endl;
    
    Vehiculo nuevosDatos;
    
    leerTextoNoVacio("Nueva Marca: ", nuevosDatos.marca, 50);
    leerTextoNoVacio("Nuevo Modelo: ", nuevosDatos.modelo, 50);
    
    nuevosDatos.anio = leerEnteroSeguro("Nuevo Ano de Fabricacion (1950-2027): ", 1950, 2027);
    
    leerTextoSoloLetras("Nuevo Nombre completo del propietario: ", nuevosDatos.nombrePropietario, 100);
    leerTextoNoVacio("Nueva Cedula o RIF del propietario: ", nuevosDatos.cedulaPropietario, 20);
    leerTelefonoSeguro("Nuevo Telefono del propietario: ", nuevosDatos.telefonoPropietario, 20);

    char conf;
    cout << "¿Esta seguro de actualizar los datos de este vehiculo? (S/N): ";
    cin >> conf;
    
    if (conf == 'S' || conf == 's') {
        if (actualizarVehiculo(s, id, nuevosDatos)) {
            cout << "¡Vehiculo actualizado exitosamente!" << endl;
        } else {
            cout << "ERROR: Ocurrio un problema al actualizar (datos invalidos)." << endl;
        }
    } else {
        cout << "Operacion cancelada. Los datos originales se mantienen." << endl;
    }
}


void menuEliminarVehiculo(sysTaller &s) {
    int id;
    cout << "\n--- ELIMINAR VEHICULO ---" << endl;
    id = leerEnteroSeguro("Ingrese la ID: ", 1, 999999);

    char confirmacion;
    do {
        cout << "¿Esta seguro de eliminar fisicamente este vehiculo? (S/N): ";
        cin >> confirmacion;
        
        if (confirmacion != 'S' && confirmacion != 's' && confirmacion != 'N' && confirmacion != 'n') {
            cout << "[!] Entrada invalida. Por favor ingrese S para Si o N para No.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    } while (confirmacion != 'S' && confirmacion != 's' && confirmacion != 'N' && confirmacion != 'n');

    cin.ignore(10000, '\n');

    if (confirmacion == 'S' || confirmacion == 's') {
        if (eliminarVehiculo(s, id)) {
            cout << "¡Vehiculo eliminado exitosamente!" << endl;
        } else {
            cout << "ERROR: No se pudo eliminar. El vehiculo no existe o tiene ordenes de servicio asociadas." << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
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
        cout << "5. Actualizar vehiculo" << endl;
        cout << "0. Volver al menu principal" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        switch(opcion) {
            case 1: menuRegistrarVehiculo(s); break;
            case 2: menuBuscarVehiculo(s); break;
            case 3: menuListarVehiculos(s); break;
            case 4: menuEliminarVehiculo(s); break;
            case 5: menuActualizarVehiculo(s); break;
            case 0: break;
            default: cout << "Opcion invalida." << endl;
        }
    } while(opcion != 0);
}

void mostrarMecanico(Mecanico m) {
    cout << "----------------------------------------" << endl;
    cout << "ID           : " << m.id << endl;
    cout << "Nombre       : " << m.nombre << endl;
    cout << "Cedula       : " << m.cedula << endl;
    cout << "Especialidad : " << m.especialidad << endl;
    cout << "Fecha Ingreso: " << m.fechaIngreso << endl;
    cout << "----------------------------------------" << endl;
}

void menuRegistrarMecanico(sysTaller &s) {
    Mecanico m;
    cout << "\n--- REGISTRAR MECANICO ---" << endl;
    
    leerTextoSoloLetras("Nombre completo: ", m.nombre, 100);
    
    leerTextoNoVacio("Cedula: ", m.cedula, 20);
    
    do {
        leerTextoNoVacio("Especialidad (MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION, GENERAL): ", m.especialidad, sizeof(m.especialidad));
        for (int i = 0; m.especialidad[i] != '\0'; i++) {
            m.especialidad[i] = static_cast<char>(toupper(static_cast<unsigned char>(m.especialidad[i])));
        }
        if (!especialidadValida(m.especialidad)) {
            cout << "[!] Especialidad invalida. Seleccione una de las opciones indicadas.\n";
        }
    } while (!especialidadValida(m.especialidad));
    
    leerFormatoFijo("Fecha de ingreso (YYYY-MM-DD): ", m.fechaIngreso, 10);

    char confirmacion;
    do {
        cout << "¿Guardar mecanico? (S/N): ";
        cin >> confirmacion;
        
        if (confirmacion != 'S' && confirmacion != 's' && confirmacion != 'N' && confirmacion != 'n') {
            cout << "[!] Entrada invalida. Ingrese S para Si o N para No.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    } while (confirmacion != 'S' && confirmacion != 's' && confirmacion != 'N' && confirmacion != 'n');
    
    cin.ignore(10000, '\n');

    if (confirmacion == 'S' || confirmacion == 's') {
        Mecanico* nuevo = registrarMecanico(s, m);
        if (nuevo != nullptr) {
            cout << "¡Mecanico registrado con ID #" << nuevo->id << "!" << endl;
        } else {
            cout << "ERROR: Cedula duplicada o especialidad invalida." << endl;
        }
    } else {
        cout << "Registro cancelado." << endl;
    }
}

void menuBuscarMecanicoPorID(sysTaller &s) {
    cout << "\n--- BUSCAR MECANICO POR ID ---" << endl;
    
    int idBuscado = leerEnteroSeguro("Ingrese el ID del mecanico a buscar: ", 1, 999999);
    
    Mecanico* m = buscarMecanicoPorID(s, idBuscado);
    
    if (m != nullptr) {
        cout << "\n¡Mecanico encontrado!" << endl;
        mostrarMecanico(*m);
    } else {
        cout << "ERROR: No se encontro ningun mecanico registrado con el ID #" << idBuscado << "." << endl;
    }
}

void menuBuscarMecanicoPorCedula(sysTaller &s) {
    cout << "\n--- BUSCAR MECANICO POR CEDULA ---" << endl;
    
    char cedula[20];
    leerTextoNoVacio("Ingrese la cedula del mecanico a buscar: ", cedula, 20);
    
    Mecanico* m = buscarMecanicoPorCedula(s, cedula);
    
    if (m != nullptr) {
        cout << "\n¡Mecanico encontrado!" << endl;
        mostrarMecanico(*m);
    } else {
        cout << "ERROR: No se encontro ningun mecanico registrado con la cedula '" << cedula << "'." << endl;
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
        cout << "2. Buscar mecanico por ID" << endl;
        cout << "3. Buscar mecanico por cedula" << endl;
        cout << "4. Listar mecanicos" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opcion: ";
        cin >> opcion;
        if (opcion == 1) menuRegistrarMecanico(s);
        else if (opcion == 2) menuBuscarMecanicoPorID(s);
        else if (opcion == 3) menuBuscarMecanicoPorCedula(s);
        else if (opcion == 4) menuListarMecanicos(s);
        else cout << "Opcion invalida, intente nuevamente" << endl;
    } while(opcion != 0);
}

void mostrarRepuesto(Repuesto r) {
    cout << "----------------------------------------" << endl;
    cout << "ID           : " << r.id << endl;
    cout << "Codigo       : " << r.codigo << endl;
    cout << "Nombre       : " << r.nombre << " (" << r.marca << ")" << endl;
    cout << "Precio       : $" << r.precio << endl;
    cout << "Stock        : " << r.stock << " (Minimo: " << r.stockmin << ")" << endl;
    if (r.stock <= r.stockmin) {
        cout << "ESTADO       : ** BAJO STOCK **" << endl;
    }
    cout << "----------------------------------------" << endl;
}

void menuRegistrarRepuesto(sysTaller &s) {
    Repuesto r;
    cout << "\n--- REGISTRAR REPUESTO ---" << endl;
    
    leerTextoNoVacio("Codigo unico: ", r.codigo, 20);
    
    leerTextoNoVacio("Nombre del repuesto: ", r.nombre, 100);
    
    leerTextoNoVacio("Marca: ", r.marca, 50);
    
    // Precio unitario validado de forma segura (> 0)
    r.precio = leerFlotanteSeguro("Precio unitario (> 0): ", 0.01f, 1000000.0f);
    
    // Stock inicial validado con entero seguro (>= 0)
    r.stock = leerEnteroSeguro("Stock inicial (>= 0): ", 0, 1000000);
    
    // Stock minimo de alerta validado con entero seguro (>= 0)
    r.stockmin = leerEnteroSeguro("Stock minimo de alerta (>= 0): ", 0, 1000000);
    
    // Fecha de registro con formato fijo exacto (YYYY-MM-DD)[cite: 3]
    leerFormatoFijo("Fecha registro (YYYY-MM-DD): ", r.fechaRegistro, 10);

    char conf;
    do {
        cout << "¿Guardar repuesto? (S/N): ";
        cin >> conf;
        
        if (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n') {
            cout << "[!] Entrada invalida. Ingrese S para Si o N para No." << endl;
            cin.clear();
            cin.ignore(10000, '\n');
        }
    } while (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n');
    
    cin.ignore(10000, '\n');

    if (conf == 'S' || conf == 's') {
        Repuesto* nuevo = registrarRepuesto(s, r);
        if (nuevo != nullptr) {
            cout << "¡Repuesto registrado con ID #" << nuevo->id << "!" << endl;
        } else {
            cout << "ERROR: Codigo duplicado o valores invalidos (precio/stock)." << endl;
        }
    } else {
        cout << "Registro cancelado." << endl;
    }
}

void menuEntradaMercancia(sysTaller &s) {
    cout << "\n--- ENTRADA DE MERCANCIA ---" << endl;
    
    int id = leerEnteroSeguro("ID del repuesto: ", 1, 999999);
    
    int cant = leerEnteroSeguro("Cantidad a sumar al stock (> 0): ", 1, 1000000);
    
    if (registrarEntradaMercancia(s, id, cant)) {
        cout << "¡Stock actualizado con exito!" << endl;
    } else {
        cout << "ERROR: Repuesto no encontrado o cantidad invalida." << endl;
    }
}

void menuBuscarRepuestoPorID(sysTaller &s) {
    cout << "\n--- BUSCAR REPUESTO POR ID ---" << endl;
    
    int id = leerEnteroSeguro("Ingrese el ID del repuesto a buscar: ", 1, 999999);
    
    Repuesto* r = buscarRepuestoPorID(s, id);
    
    if (r != nullptr) {
        cout << "\n¡Repuesto encontrado!" << endl;
        mostrarRepuesto(*r);
    } else {
        cout << "ERROR: No se encontro ningun repuesto registrado con el ID #" << id << "." << endl;
    }
}

void menuCambiarPrecio(sysTaller &s) {
    cout << "\n--- CAMBIAR PRECIO DE REPUESTO ---" << endl;
    
    int id = leerEnteroSeguro("ID del repuesto: ", 1, 999999);
    
    float nuevoP = leerFlotanteSeguro("Nuevo precio unitario (> 0): ", 0.01f, 1000000.0f);
    
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

void menuEliminarRepuesto(sysTaller &s) {
    cout << "\n--- ELIMINAR REPUESTO ---" << endl;
    
    int id = leerEnteroSeguro("Ingrese el ID del repuesto a eliminar: ", 1, 999999);

    char conf;
    do {
        cout << "¿Esta seguro de eliminar fisicamente este repuesto? (S/N): ";
        cin >> conf;
        
        if (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n') {
            cout << "[!] Entrada invalida. Ingrese S para Si o N para No." << endl;
            cin.clear();
            cin.ignore(10000, '\n');
        }
    } while (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n');
    
    cin.ignore(10000, '\n');

    if (conf == 'S' || conf == 's') {
        if (eliminarRepuesto(s, id)) {
            cout << "¡Repuesto eliminado exitosamente!" << endl;
        } else {
            cout << "ERROR: No se pudo eliminar. El repuesto no existe o ya fue usado en una orden de servicio." << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuRepuestos(sysTaller &s) {
    int opcion;
    do {
        cout << "\n=== INVENTARIO DE REPUESTOS ===" << endl;
        cout << "1. Registrar repuesto" << endl;
        cout << "2. Registrar entrada de mercancia" << endl;
        cout << "3. Cambiar precio" << endl;
        cout << "4. Buscar repuesto por ID" << endl;
        cout << "5. Listar repuestos" << endl;
        cout << "6. Eliminar repuesto" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione opcion: ";
        cin >> opcion;
        switch(opcion) {
            case 1: menuRegistrarRepuesto(s); break;
            case 2: menuEntradaMercancia(s); break;
            case 3: menuCambiarPrecio(s); break;
            case 4: menuBuscarRepuestoPorID(s); break;
            case 5: menuListarRepuestos(s); break;
            case 6: menuEliminarRepuesto(s); break;
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
    cout << "\n--- ABRIR ORDEN DE SERVICIO ---" << endl;
    
    int idV = leerEnteroSeguro("ID del vehiculo: ", 1, 999999);
    
    int idM = leerEnteroSeguro("ID del mecanico: ", 1, 999999);
    
    char fecha[11];
    leerFormatoFijo("Fecha de apertura (YYYY-MM-DD): ", fecha, 10);
    
    int km = leerEnteroSeguro("Kilometraje de ingreso (>= 0): ", 0, 2000000);
    
    char falla[200];
    leerTextoNoVacio("Descripcion de la falla: ", falla, 200);

    ordenServicio* nueva = abrirOrdenServicio(s, idV, idM, fecha, falla, km);
    if (nueva != nullptr) {
        cout << "¡Orden de servicio #" << nueva->id << " abierta exitosamente!" << endl;
    } else {
        cout << "ERROR: No se pudo abrir la orden. Verifique si el vehiculo/mecanico existen, si el vehiculo ya tiene otra orden abierta, si el mecanico supero el limite de 3 ordenes o si el kilometraje es menor al registrado." << endl;
    }
}



void menuAgregarRepuestoOrden(sysTaller &s) {
    cout << "\n--- AGREGAR REPUESTO A ORDEN ---" << endl;
    
    int idO = leerEnteroSeguro("ID de la orden: ", 1, 999999);
    
    int idR = leerEnteroSeguro("ID del repuesto: ", 1, 999999);
    
    int cant = leerEnteroSeguro("Cantidad (> 0): ", 1, 1000000);

    if (agregarRepuestoAOrden(s, idO, idR, cant)) {
        cout << "¡Repuesto agregado a la orden correctamente!" << endl;
    } else {
        cout << "ERROR: Orden no abierta, repuesto no encontrado o stock insuficiente." << endl;
    }
}

void menuListarOrdenesPorEstado(sysTaller &s) {
    cout << "\n--- LISTAR ORDENES POR ESTADO ---" << endl;
    cout << "Seleccione el estado a filtrar:" << endl;
    cout << "1. ABIERTA" << endl;
    cout << "2. CERRADA" << endl;
    cout << "3. ANULADA" << endl;
    
    // Protegemos la seleccion usando tu funcion de validacion segura
    int opcionEstado = leerEnteroSeguro("Seleccione una opcion: ", 1, 3);
    
    const char* estadoFiltro = "";
    if (opcionEstado == 1) {
        estadoFiltro = "ABIERTA";
    } else if (opcionEstado == 2) {
        estadoFiltro = "CERRADA";
    } else if (opcionEstado == 3) {
        estadoFiltro = "ANULADA";
    }
    
    int cantidad = 0;
    // Llamamos a la funcion de logica existente que retorna un ordenServicio**
    ordenServicio** lista = listarOrdenesPorEstado(s, estadoFiltro, cantidad);
    
    if (cantidad == 0 || lista == nullptr) {
        cout << "No se encontraron ordenes registradas con el estado " << estadoFiltro << "." << endl;
        return;
    }
    
    cout << "\nSe encontraron " << cantidad << " orden(es) en estado " << estadoFiltro << ":" << endl;
    for (int i = 0; i < cantidad; i++) {
        mostrarDetalleOrden(s, *lista[i]); // Reutilizamos tu formato de detalles completo
    }
    
    // Importante: Liberar el arreglo temporal de punteros para evitar fugas de memoria
    delete[] lista;
}

void menuQuitarRepuestoOrden(sysTaller &s) {
    cout << "\n--- QUITAR REPUESTO DE ORDEN ---" << endl;
    
    int idO = leerEnteroSeguro("ID de la orden: ", 1, 999999);
    
    int idR = leerEnteroSeguro("ID del repuesto a quitar de la linea: ", 1, 999999);

    if (quitarRepuestoDeOrden(s, idO, idR)) {
        cout << "¡Linea de repuesto removida y stock devuelto al inventario!" << endl;
    } else {
        cout << "ERROR: Orden no abierta o repuesto no encontrado en la orden." << endl;
    }
}

void menuCerrarOrden(sysTaller &s) {
    cout << "\n--- CERRAR ORDEN DE SERVICIO ---" << endl;
    
    int idO = leerEnteroSeguro("ID de la orden: ", 1, 999999);
    
    char fecha[11];
    leerFormatoFijo("Fecha de cierre (YYYY-MM-DD): ", fecha, 10);
    
    float mano = leerFlotanteSeguro("Costo de mano de obra (>= 0): ", 0.0f, 1000000.0f);

    char conf;
    do {
        cout << "¿Desea cerrar esta orden? (Ya no se podra modificar) (S/N): ";
        cin >> conf;
        
        if (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n') {
            cout << "[!] Entrada invalida. Ingrese S para Si o N para No." << endl;
            cin.clear();
            cin.ignore(10000, '\n');
        }
    } while (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n');
    
    cin.ignore(10000, '\n');

    if (conf == 'S' || conf == 's') {
        if (cerrarOrdenServicio(s, idO, fecha, mano)) {
            cout << "¡Orden cerrada exitosamente!" << endl;
        } else {
            cout << "ERROR: Orden no encontrada, ya no esta abierta o datos de mano de obra invalidos." << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuAnularOrden(sysTaller &s) {
    cout << "\n--- ANULAR ORDEN DE SERVICIO ---" << endl;
    
    int idO = leerEnteroSeguro("ID de la orden a anular: ", 1, 999999);

    char conf;
    do {
        cout << "¿Confirma la anulacion? (Los repuestos volveran al inventario) (S/N): ";
        cin >> conf;
        
        if (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n') {
            cout << "[!] Entrada invalida. Ingrese S para Si o N para No." << endl;
            cin.clear();
            cin.ignore(10000, '\n');
        }
    } while (conf != 'S' && conf != 's' && conf != 'N' && conf != 'n');
    
    cin.ignore(10000, '\n');

    if (conf == 'S' || conf == 's') {
        if (anularOrdenServicio(s, idO)) {
            cout << "¡Orden anulada y stock devuelto al inventario!" << endl;
        } else {
            cout << "ERROR: Orden no encontrada o estado invalido para anular." << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuVerDetalleOrden(sysTaller &s) {
    cout << "\n--- VER DETALLE DE ORDEN ---" << endl;
    
    int idO = leerEnteroSeguro("Ingrese el ID de la orden: ", 1, 999999);
    
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
        cout << "7. Listar ordenes por estado" << endl; // <--- NUEVA OPCION
        cout << "0. Volver" << endl;
        
        // Protegemos la opcion del menu
        opcion = leerEnteroSeguro("Seleccione opcion: ", 0, 7);
        
        switch(opcion) {
            case 1: menuAbrirOrden(s); break;
            case 2: menuAgregarRepuestoOrden(s); break;
            case 3: menuQuitarRepuestoOrden(s); break;
            case 4: menuCerrarOrden(s); break;
            case 5: menuAnularOrden(s); break;
            case 6: menuVerDetalleOrden(s); break;
            case 7: menuListarOrdenesPorEstado(s); break; // <--- LLAMADA A LA FUNCION
            case 0: break;
            default: cout << "Opcion invalida." << endl;
        }
    } while(opcion != 0);
}

int main() {
    sysTaller miTaller;

    inicializarSistema(miTaller, "Taller Mecanico URDANETA", "J-00000000-0", "Av. Principal del Taller", "0261-5550000");

    int opcion;
    do {
        cout << "\n==========================================" << endl;
        cout << "         SISTEMA DE GESTION DE TALLER     " << endl;
        cout << "  Taller: " << miTaller.taller.nombre << endl;
        cout << "==========================================" << endl;
        cout << "1. Gestion de Vehiculos" << endl;
        cout << "2. Gestion de Mecanicos" << endl;
        cout << "3. Inventario de Repuestos" << endl;
        cout << "4. Ordenes de Servicio" << endl;
        cout << "0. Salir" << endl;
        
        // AQUI ESTA LA MAGIA: Reemplazamos el 'cin >> opcion' por tu validacion segura
        opcion = leerEnteroSeguro("Seleccione una opcion: ", 0, 4);
        
        switch (opcion) {
            case 1: 
                menuVehiculos(miTaller); 
                break;
            case 2: 
                menuMecanicos(miTaller); 
                break;
            case 3: 
                menuRepuestos(miTaller); 
                break;
            case 4: 
                menuOrdenes(miTaller); 
                break;
            case 0: {
                char cont;
                do {
                    cout << "¿Esta seguro de salir? (Se perderan los datos en memoria) (S/N): ";
                    cin >> cont;
                    if (cont != 'S' && cont != 's' && cont != 'N' && cont != 'n') {
                        cout << "[!] Entrada invalida. Ingrese S para Si o N para No." << endl;
                        cin.clear();
                        cin.ignore(10000, '\n');
                    }
                } while (cont != 'S' && cont != 's' && cont != 'N' && cont != 'n');
                
                cin.ignore(10000, '\n');
                
                if (cont != 'S' && cont != 's') {
                    opcion = -1;
                } else {
                    cout << "Saliendo del sistema... ¡Hasta luego!" << endl;
                }
                break;
            }
            default: 
                cout << "Opcion invalida." << endl;
        }
    } while (opcion != 0);
}
