#include <iostream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include "struct_taller.h"
#include "logica.h"

using namespace std;

// ==========================
// DECLARACIONES DE SUBMENUS
// ==========================
void menuVehiculos();
void menuMecanicos();
void menuRepuestos();
void menuOrdenes();
void menuReportes();
void menuMantenimiento();

// =======================================
// FUNCIONES AUXILIARES DE ENTRADA SEGURA
// =======================================

static bool leerLineaCompleta(string &entrada) {
    cin >> ws;
    if (getline(cin, entrada)) return true;
    if (cin.eof()) {
        cout << "\nFin de entrada. Saliendo del sistema." << endl;
        exit(EXIT_SUCCESS);
    }
    cin.clear();
    cin.ignore(10000, '\n');
    return false;
}

template <size_t N>
static bool cadenaTerminada(const char (&campo)[N]) {
    return memchr(campo, '\0', N) != nullptr;
}

void leerTextoNoVacio(const string &mensaje, char destino[], int tamMax) {
    string entrada;
    do {
        cout << mensaje;
        if (!leerLineaCompleta(entrada)) continue;

        if (entrada.empty()) {
            cout << "[!] Este campo no puede estar vacio. Intente nuevamente." << endl;
        } else if (entrada.length() >= static_cast<size_t>(tamMax)) {
            cout << "[!] El texto es demasiado largo (maximo " << tamMax - 1 << " caracteres). Intente nuevamente." << endl;
            entrada = "";
        }
    } while (entrada.empty());

    strcpy(destino, entrada.c_str());
}

int leerEnteroSeguro(const string& mensaje, int min, int max) {
    int valor;
    while (true) {
        cout << mensaje;
        string entrada;
        if (!leerLineaCompleta(entrada)) continue;
        istringstream lectura(entrada);

        char sobrante;
        if (!(lectura >> valor) || (lectura >> sobrante)) {
            cout << "[!] Entrada invalida. Ingrese un valor numerico entero." << endl;
        } else if (valor < min || valor > max) {
            cout << "[!] Error: El valor debe estar entre " << min << " y " << max << "." << endl;
        } else {
            return valor;
        }
    }
}

bool contieneNumeros(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit(static_cast<unsigned char>(str[i]))) {
            return true;
        }
    }
    return false;
}

void leerTextoSoloLetras(const string& mensaje, char destino[], int tamMax) {
    while (true) {
        string entrada;
        cout << mensaje;
        if (!leerLineaCompleta(entrada)) continue;

        if (entrada.empty()) {
            cout << "[!] El campo no puede estar vacio. Intente de nuevo." << endl;
        } else if (entrada.length() >= static_cast<size_t>(tamMax)) {
            cout << "[!] El texto es demasiado largo (maximo " << tamMax - 1
                 << " caracteres). Intente nuevamente." << endl;
        } else if (contieneNumeros(entrada.c_str())) {
            cout << "[!] Entrada invalida. No se permiten numeros en este campo." << endl;
        } else {
            strcpy(destino, entrada.c_str());
            return;
        }
    }
}

void leerFormatoFijo(const string& mensaje, char destino[], int longitudEsperada) {
    string entrada;
    while (true) {
        cout << mensaje;
        if (!leerLineaCompleta(entrada)) continue;
        bool formatoValido = longitudEsperada == 10 &&
                             entrada.length() == 10 &&
                             fechaValidaISO(entrada.c_str());

        if (formatoValido) {
            strcpy(destino, entrada.c_str());
            return;
        }

        cout << "[!] Formato invalido. Debe tener exactamente " << longitudEsperada
             << " caracteres en formato YYYY-MM-DD (Ej: 2026-11-03).\n";
    }
}

void leerTelefonoSeguro(const string& mensaje, char destino[], int tamMax) {
    while (true) {
        string entrada;
        cout << mensaje;
        if (!leerLineaCompleta(entrada)) continue;

        if (entrada.empty()) {
            cout << "[!] Este campo no puede estar vacio. Intente nuevamente.\n";
        } else if (entrada.length() >= static_cast<size_t>(tamMax)) {
            cout << "[!] El texto es demasiado largo. Intente nuevamente.\n";
        } else {
            bool valido = true;
            for (char c : entrada) {
                if (!isdigit(static_cast<unsigned char>(c)) && c != '-') {
                    cout << "[!] Error: El telefono solo debe contener numeros o guiones (-).\n";
                    valido = false;
                    break;
                }
            }
            if (valido) {
                strcpy(destino, entrada.c_str());
                return;
            }
        }
    }
}

float leerFlotanteSeguro(const string& mensaje, float min, float max) {
    while (true) {
        cout << mensaje;
        string entrada;
        if (!leerLineaCompleta(entrada)) continue;
        istringstream lectura(entrada);
        float valor;
        char sobrante;

        if (!(lectura >> valor) || (lectura >> sobrante) ||
            !isfinite(valor) || valor < min || valor > max) {
            cout << "[!] Entrada invalida. Ingrese un valor numerico valido (" << min << " a " << max << ")." << endl;
        } else {
            return valor;
        }
    }
}

char leerConfirmacionSiNo(const string& mensaje) {
    while (true) {
        cout << mensaje;
        string entrada;
        if (!leerLineaCompleta(entrada)) continue;
        if (entrada.length() == 1) {
            char respuesta = static_cast<char>(toupper(static_cast<unsigned char>(entrada[0])));
            if (respuesta == 'S' || respuesta == 'N') return respuesta;
        }
        cout << "[!] Entrada invalida. Ingrese S o N." << endl;
    }
}

// ======================
// MÓDULO DE VEHÍCULOS 
// ======================

void mostrarVehiculo(const Vehiculo &v) {
    if (!cadenaTerminada(v.placa) || !cadenaTerminada(v.marca) ||
        !cadenaTerminada(v.modelo) || !cadenaTerminada(v.nombrePropietario) ||
        !cadenaTerminada(v.cedulaPropietario) ||
        !cadenaTerminada(v.telefonoPropietario)) {
        cout << "ERROR: El vehiculo contiene una cadena sin terminador." << endl;
        return;
    }
    cout << "----------------------------------------" << endl;
    cout << "ID          : " << v.id << endl;
    cout << "Placa       : " << v.placa << endl;
    cout << "Vehiculo    : " << v.marca << " " << v.modelo << " (" << v.anio << ")" << endl;
    cout << "Kilometraje : " << v.kilometraje << " km" << endl;
    cout << "Propietario : " << v.nombrePropietario << " (Cedula: " << v.cedulaPropietario << ")" << endl;
    cout << "Telefono    : " << v.telefonoPropietario << endl;
    cout << "----------------------------------------" << endl;
}

void menuRegistrarVehiculo() {
    Vehiculo v = {};
    cout << "\n--- REGISTRAR NUEVO VEHICULO ---" << endl;
    leerTextoNoVacio("Placa (alfanumerica): ", v.placa, 10);
    leerTextoNoVacio("Marca: ", v.marca, 50);
    leerTextoNoVacio("Modelo: ", v.modelo, 50);
    v.anio = leerEnteroSeguro("Ano de Fabricacion (1950-2027): ", 1950, 2027);
    v.kilometraje = leerEnteroSeguro("Kilometraje actual (>= 0): ", 0, 2000000);
    leerTextoSoloLetras("Nombre completo del propietario: ", v.nombrePropietario, 100);
    leerTextoNoVacio("Cedula o RIF del propietario: ", v.cedulaPropietario, 20);
    leerTelefonoSeguro("Telefono del propietario: ", v.telefonoPropietario, 20);

    char confirmacion = leerConfirmacionSiNo("¿Desea guardar este vehiculo? (S/N): ");

    if (confirmacion == 'S') {
        string motivoFallo;
        if (agregarVehiculo(v, motivoFallo)) {
            cout << "¡Vehiculo registrado exitosamente con ID #" << v.id << "!" << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Registro cancelado." << endl;
    }
}

void menuBuscarVehiculo() {
    cout << "\n--- BUSCAR VEHICULO ---" << endl;
    cout << "1. Buscar por ID" << endl;
    cout << "2. Buscar por Placa exacta" << endl;
    cout << "3. Buscar por Propietario (parcial)" << endl;
    int opcionBusqueda = leerEnteroSeguro("Seleccione una opcion: ", 1, 3);

    Vehiculo vEncontrado;

    if (opcionBusqueda == 1) {
        int id = leerEnteroSeguro("Ingrese el ID: ", 1, 999999);
        bool fueEliminado = false;
        long posBytes = 0;

        if (buscarVehiculoPorID(id, vEncontrado, fueEliminado, posBytes)) {
            cout << "Leido en offset: " << posBytes << " bytes" << endl;
            mostrarVehiculo(vEncontrado);
        } else if (fueEliminado) {
            cout << "Leido en offset: " << posBytes << " bytes" << endl;
            cout << "AVISO: El vehiculo con ID #" << id << " fue ELIMINADO logicamente del sistema." << endl;
        } else {
            cout << "ERROR: No existe ningun vehiculo registrado con el ID #" << id << "." << endl;
        }
    } else if (opcionBusqueda == 2) {
        char placa[10];
        leerTextoNoVacio("Ingrese la placa exacta: ", placa, 10);
        string motivoFallo;
        if (buscarVehiculoPorPlaca(placa, vEncontrado, motivoFallo)) {
            mostrarVehiculo(vEncontrado);
        } else if (!motivoFallo.empty()) {
            cout << "ERROR: " << motivoFallo << endl;
        } else {
            cout << "ERROR: Vehiculo con placa '" << placa << "' no encontrado entre los activos." << endl;
        }
    } else if (opcionBusqueda == 3) {
        char sub[100];
        leerTextoNoVacio("Ingrese nombre o fragmento del propietario: ", sub, 100);
        Vehiculo lista[MAX_RESULTADOS];
        int cant = 0;
        bool hayMas = false;
        string motivoFallo;
        if (buscarVehiculosPorPropietario(sub, lista, cant, hayMas, motivoFallo)) {
            for (int i = 0; i < cant; i++) {
                mostrarVehiculo(lista[i]);
            }
            cout << "Coincidencias mostradas: " << cant << endl;
            if (hayMas) {
                cout << "AVISO: Existen mas de " << MAX_RESULTADOS << " coincidencias en el archivo. Se muestran los primeros " << MAX_RESULTADOS << "." << endl;
            }
        } else if (!motivoFallo.empty()) {
            cout << "ERROR: " << motivoFallo << endl;
        } else {
            cout << "No se encontraron vehiculos activos para ese propietario." << endl;
        }
    }
}

void menuListarVehiculos() {
    Vehiculo lista[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    string motivoFallo;

    if (listarVehiculos(lista, cantidad, hayMas, motivoFallo)) {
        cout << "\n--- LISTADO GENERAL DE VEHICULOS (ACTIVOS) ---" << endl;
        for (int i = 0; i < cantidad; i++) {
            mostrarVehiculo(lista[i]);
        }
        cout << "Total mostrados: " << cantidad << endl;
        if (hayMas) {
            cout << "AVISO: Existen mas registros en el archivo. Se muestran los primeros " << MAX_RESULTADOS << "." << endl;
        }
    } else if (!motivoFallo.empty()) {
        cout << "ERROR: " << motivoFallo << endl;
    } else {
        cout << "No hay vehiculos activos registrados." << endl;
    }
}

void menuActualizarVehiculo() {
    cout << "\n--- ACTUALIZAR DATOS DE VEHICULO ---" << endl;
    int id = leerEnteroSeguro("Ingrese el ID del vehiculo a actualizar: ", 1, 999999);

    Vehiculo actual;
    bool elim = false;
    long pos = 0;
    if (!buscarVehiculoPorID(id, actual, elim, pos)) {
        if (elim) cout << "ERROR: El vehiculo con ID #" << id << " fue eliminado logicamente." << endl;
        else cout << "ERROR: No se encontro ningun vehiculo activo con el ID #" << id << "." << endl;
        return;
    }

    cout << "\nVehiculo actual encontrado. Ingrese los nuevos datos." << endl;

    Vehiculo datosNuevos = actual;
    leerTextoNoVacio("Nueva Marca: ", datosNuevos.marca, 50);
    leerTextoNoVacio("Nuevo Modelo: ", datosNuevos.modelo, 50);
    datosNuevos.anio = leerEnteroSeguro("Nuevo Ano de Fabricacion (1950-2027): ", 1950, 2027);
    leerTextoSoloLetras("Nuevo Nombre completo del propietario: ", datosNuevos.nombrePropietario, 100);
    leerTextoNoVacio("Nueva Cedula o RIF del propietario: ", datosNuevos.cedulaPropietario, 20);
    leerTelefonoSeguro("Nuevo Telefono del propietario: ", datosNuevos.telefonoPropietario, 20);

    char conf = leerConfirmacionSiNo(
        "¿Esta seguro de actualizar los datos de este vehiculo? (S/N): ");

    if (conf == 'S') {
        string motivoFallo;
        if (actualizarVehiculo(id, datosNuevos, motivoFallo)) {
            cout << "¡Vehiculo actualizado exitosamente en el archivo binario!" << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuEliminarVehiculo() {
    cout << "\n--- ELIMINAR VEHICULO ---" << endl;
    int id = leerEnteroSeguro("Ingrese el ID del vehiculo a eliminar: ", 1, 999999);

    char confirmacion =
        leerConfirmacionSiNo("¿Esta seguro de eliminar este vehiculo? (S/N): ");

    if (confirmacion == 'S') {
        string motivoFallo;
        if (eliminarVehiculo(id, motivoFallo)) {
            cout << "¡Vehiculo eliminado exitosamente!" << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuVehiculos() {
    int opcion;
    do {
        cout << "\n=== GESTION DE VEHICULOS ===" << endl;
        cout << "1. Registrar vehiculo" << endl;
        cout << "2. Buscar vehiculo" << endl;
        cout << "3. Listar vehiculos" << endl;
        cout << "4. Eliminar vehiculo" << endl;
        cout << "5. Actualizar vehiculo" << endl;
        cout << "0. Volver al menu principal" << endl;
        opcion = leerEnteroSeguro("Seleccione una opcion: ", 0, 5);
        switch (opcion) {
            case 1: menuRegistrarVehiculo(); break;
            case 2: menuBuscarVehiculo(); break;
            case 3: menuListarVehiculos(); break;
            case 4: menuEliminarVehiculo(); break;
            case 5: menuActualizarVehiculo(); break;
            case 0: break;
        }
    } while (opcion != 0);
}

// =====================
// MODULO DE MECANICOS
// =====================

void mostrarMecanico(const Mecanico &m) {
    if (!cadenaTerminada(m.nombre) || !cadenaTerminada(m.cedula) ||
        !cadenaTerminada(m.especialidad) || !cadenaTerminada(m.fechaIngreso)) {
        cout << "ERROR: El mecanico contiene una cadena sin terminador." << endl;
        return;
    }
    cout << "----------------------------------------" << endl;
    cout << "ID           : " << m.id;
    if (m.eliminado) cout << " **(DADO DE BAJA)**";
    cout << endl;
    cout << "Nombre       : " << m.nombre << endl;
    cout << "Cedula       : " << m.cedula << endl;
    cout << "Especialidad : " << m.especialidad << endl;
    cout << "Fecha Ingreso: " << m.fechaIngreso << endl;
    cout << "----------------------------------------" << endl;
}

void menuRegistrarMecanico() {
    Mecanico m = {};
    cout << "\n--- REGISTRAR MECANICO ---" << endl;

    leerTextoSoloLetras("Nombre completo: ", m.nombre, 100);
    leerTextoNoVacio("Cedula: ", m.cedula, 20);

    do {
        leerTextoNoVacio("Especialidad (MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION, GENERAL): ", m.especialidad, sizeof(m.especialidad));
        for (int i = 0; m.especialidad[i] != '\0'; i++) {
            m.especialidad[i] = static_cast<char>(toupper(static_cast<unsigned char>(m.especialidad[i])));
        }
        if (!especialidadValida(m.especialidad)) {
            cout << "[!] Especialidad invalida. Seleccione una de las permitidas.\n";
        }
    } while (!especialidadValida(m.especialidad));

    leerFormatoFijo("Fecha de ingreso (YYYY-MM-DD): ", m.fechaIngreso, 10);

    char confirmacion = leerConfirmacionSiNo("¿Guardar mecanico? (S/N): ");

    if (confirmacion == 'S') {
        string motivoFallo;
        if (registrarMecanico(m, motivoFallo)) {
            cout << "¡Mecanico registrado con ID #" << m.id << "!" << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Registro cancelado." << endl;
    }
}

void menuBuscarMecanicoPorID() {
    cout << "\n--- BUSCAR MECANICO POR ID ---" << endl;
    int idBuscado = leerEnteroSeguro("Ingrese el ID del mecanico a buscar: ", 1, 999999);

    Mecanico m;
    bool fueEliminado = false;
    long posBytes = 0;
    if (buscarMecanicoPorID(idBuscado, m, fueEliminado, &posBytes)) {
        cout << "\n¡Mecanico activo encontrado! (Offset: " << posBytes << " bytes)" << endl;
        mostrarMecanico(m);
        int abiertas = 0;
        string motivoFallo;
        if (contarOrdenesAbiertasMecanico(m.id, abiertas, motivoFallo)) {
            cout << "Ordenes ABIERTAS actuales: " << abiertas << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else if (fueEliminado) {
        cout << "AVISO: El mecanico con ID #" << idBuscado << " fue DADO DE BAJA del sistema." << endl;
    } else {
        cout << "ERROR: No se encontro ningun mecanico registrado con el ID #" << idBuscado << "." << endl;
    }
}

void menuBuscarMecanicoPorCedula() {
    cout << "\n--- BUSCAR MECANICO POR CEDULA ---" << endl;
    char cedula[20];
    leerTextoNoVacio("Ingrese la cedula del mecanico: ", cedula, 20);

    Mecanico m;
    string motivoFallo;
    if (buscarMecanicoPorCedula(cedula, m, motivoFallo)) {
        cout << "\n¡Mecanico encontrado!" << endl;
        mostrarMecanico(m);
    } else if (!motivoFallo.empty()) {
        cout << "ERROR: " << motivoFallo << endl;
    } else {
        cout << "ERROR: No se encontro ningun mecanico activo con la cedula '" << cedula << "'." << endl;
    }
}

void menuListarMecanicos() {
    Mecanico lista[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    string motivoFallo;

    if (listarMecanicos(lista, cantidad, hayMas, motivoFallo)) {
        cout << "\n--- LISTADO DE MECANICOS (ACTIVOS Y DADOS DE BAJA) ---" << endl;
        for (int i = 0; i < cantidad; i++) {
            mostrarMecanico(lista[i]);
            int abiertas = 0;
            string motivoFallo;
            if (contarOrdenesAbiertasMecanico(lista[i].id, abiertas, motivoFallo)) {
                cout << "Ordenes Abiertas: " << abiertas << endl;
            } else {
                cout << "ERROR al contar ordenes abiertas: " << motivoFallo << endl;
            }
        }
        cout << "Total listados: " << cantidad << endl;
        if (hayMas) {
            cout << "AVISO: Existen mas registros en el archivo. Se muestran los primeros " << MAX_RESULTADOS << "." << endl;
        }
    } else if (!motivoFallo.empty()) {
        cout << "ERROR: " << motivoFallo << endl;
    } else {
        cout << "No hay mecanicos registrados en el sistema." << endl;
    }
}

void menuEliminarMecanico() {
    cout << "\n--- DAR DE BAJA A MECANICO ---" << endl;
    int id = leerEnteroSeguro("Ingrese el ID del mecanico a dar de baja: ", 1, 999999);

    char confirmacion = leerConfirmacionSiNo(
        "¿Esta seguro de dar de baja a este mecanico? (Borrado logico) (S/N): ");

    if (confirmacion == 'S') {
        string motivoFallo;
        if (eliminarMecanico(id, motivoFallo)) {
            cout << "¡Mecanico dado de baja exitosamente!" << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuMecanicos() {
    int opcion;
    do {
        cout << "\n=== GESTION DE MECANICOS ===" << endl;
        cout << "1. Registrar mecanico" << endl;
        cout << "2. Buscar mecanico por ID" << endl;
        cout << "3. Buscar mecanico por cedula" << endl;
        cout << "4. Listar mecanicos" << endl;
        cout << "5. Dar de baja mecanico" << endl;
        cout << "0. Volver" << endl;

        opcion = leerEnteroSeguro("Seleccione opcion: ", 0, 5);
        switch (opcion) {
            case 1: menuRegistrarMecanico(); break;
            case 2: menuBuscarMecanicoPorID(); break;
            case 3: menuBuscarMecanicoPorCedula(); break;
            case 4: menuListarMecanicos(); break;
            case 5: menuEliminarMecanico(); break;
            case 0: break;
        }
    } while (opcion != 0);
}

// =====================
// MÓDULO DE REPUESTOS
// =====================

void mostrarRepuesto(const Repuesto &r) {
    if (!cadenaTerminada(r.codigo) || !cadenaTerminada(r.nombre) ||
        !cadenaTerminada(r.marca)) {
        cout << "ERROR: El repuesto contiene una cadena sin terminador." << endl;
        return;
    }
    cout << "----------------------------------------" << endl;
    cout << "ID           : " << r.id << endl;
    cout << "Codigo       : " << r.codigo << endl;
    cout << "Nombre       : " << r.nombre << " (" << r.marca << ")" << endl;
    cout << "Precio       : $" << fixed << setprecision(2) << r.precio << endl;
    cout << "Stock        : " << r.stock << " (Minimo: " << r.stockMinimo << ")" << endl;
    if (r.stock <= r.stockMinimo) {
        cout << "ESTADO       : ** BAJO STOCK **" << endl;
    }
    cout << "----------------------------------------" << endl;
}

void menuRegistrarRepuesto() {
    Repuesto r = {};
    cout << "\n--- REGISTRAR REPUESTO ---" << endl;

    leerTextoNoVacio("Codigo unico: ", r.codigo, 20);
    leerTextoNoVacio("Nombre del repuesto: ", r.nombre, 100);
    leerTextoNoVacio("Marca: ", r.marca, 50);

    r.precio = leerFlotanteSeguro("Precio unitario (> 0): ", 0.01f, 1000000.0f);
    r.stock = leerEnteroSeguro("Stock inicial (>= 0): ", 0, 1000000);
    r.stockMinimo = leerEnteroSeguro("Stock minimo de alerta (>= 0): ", 0, 1000000);

    char conf = leerConfirmacionSiNo("¿Guardar repuesto? (S/N): ");

    if (conf == 'S') {
        string motivoFallo;
        if (registrarRepuesto(r, motivoFallo)) {
            cout << "¡Repuesto registrado exitosamente con ID #" << r.id << "!" << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Registro cancelado." << endl;
    }
}

void menuEntradaMercancia() {
    cout << "\n--- ENTRADA DE MERCANCIA ---" << endl;
    int id = leerEnteroSeguro("ID del repuesto: ", 1, 999999);
    int cant = leerEnteroSeguro("Cantidad a sumar al stock (> 0): ", 1, 1000000);

    string motivoFallo;
    if (registrarEntradaMercancia(id, cant, motivoFallo)) {
        cout << "¡Stock actualizado con exito en el archivo de repuestos!" << endl;
    } else {
        cout << "ERROR: " << motivoFallo << endl;
    }
}

void menuBuscarRepuesto() {
    cout << "\n--- BUSCAR REPUESTO ---" << endl;
    cout << "1. Buscar por ID" << endl;
    cout << "2. Buscar por Codigo exacto" << endl;
    cout << "3. Buscar por Nombre parcial" << endl;
    int op = leerEnteroSeguro("Seleccione una opcion: ", 1, 3);

    Repuesto r;
    if (op == 1) {
        int id = leerEnteroSeguro("Ingrese el ID del repuesto: ", 1, 999999);
        bool elim = false;
        long pos = 0;
        if (buscarRepuestoPorID(id, r, elim, &pos)) {
            mostrarRepuesto(r);
        } else if (elim) {
            cout << "AVISO: El repuesto con ID #" << id << " fue ELIMINADO logicamente del inventario." << endl;
        } else {
            cout << "ERROR: No existe ningun repuesto con ID #" << id << "." << endl;
        }
    } else if (op == 2) {
        char cod[20];
        leerTextoNoVacio("Ingrese el codigo exacto: ", cod, 20);
        string motivoFallo;
        if (buscarRepuestoPorCodigo(cod, r, motivoFallo)) {
            mostrarRepuesto(r);
        } else if (!motivoFallo.empty()) {
            cout << "ERROR: " << motivoFallo << endl;
        } else {
            cout << "ERROR: No se encontro ningun repuesto activo con el codigo '" << cod << "'." << endl;
        }
    } else if (op == 3) {
        char sub[100];
        leerTextoNoVacio("Ingrese nombre o fragmento: ", sub, 100);
        Repuesto lista[MAX_RESULTADOS];
        int cant = 0;
        bool hayMas = false;
        string motivoFallo;
        if (buscarRepuestoPorNombreParcial(sub, lista, cant, hayMas, motivoFallo)) {
            for (int i = 0; i < cant; i++) {
                mostrarRepuesto(lista[i]);
            }
            cout << "Coincidencias encontradas: " << cant << endl;
            if (hayMas) {
                cout << "AVISO: Existen mas coincidencias. Se muestran las primeras " << MAX_RESULTADOS << "." << endl;
            }
        } else if (!motivoFallo.empty()) {
            cout << "ERROR: " << motivoFallo << endl;
        } else {
            cout << "No se encontraron repuestos activos con ese nombre." << endl;
        }
    }
}

void menuCambiarPrecio() {
    cout << "\n--- CAMBIAR PRECIO DE REPUESTO ---" << endl;
    int id = leerEnteroSeguro("ID del repuesto: ", 1, 999999);
    float nuevoP = leerFlotanteSeguro("Nuevo precio unitario (> 0): ", 0.01f, 1000000.0f);

    string motivoFallo;
    if (cambiarPrecioRepuesto(id, nuevoP, motivoFallo)) {
        cout << "¡Precio actualizado con exito! Las ordenes antiguas conservaran su precio historico." << endl;
    } else {
        cout << "ERROR: " << motivoFallo << endl;
    }
}

void menuListarRepuestos() {
    Repuesto lista[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    string motivoFallo;

    if (listarRepuestos(lista, cantidad, hayMas, motivoFallo)) {
        cout << "\n--- INVENTARIO DE REPUESTOS (SOLO ACTIVOS) ---" << endl;
        for (int i = 0; i < cantidad; i++) {
            mostrarRepuesto(lista[i]);
        }
        cout << "Total listados: " << cantidad << endl;
        if (hayMas) {
            cout << "AVISO: Existen mas registros en el inventario. Se muestran los primeros " << MAX_RESULTADOS << "." << endl;
        }
    } else if (!motivoFallo.empty()) {
        cout << "ERROR: " << motivoFallo << endl;
    } else {
        cout << "El inventario de repuestos esta vacio." << endl;
    }
}

void menuEliminarRepuesto() {
    cout << "\n--- ELIMINAR REPUESTO ---" << endl;
    int id = leerEnteroSeguro("Ingrese el ID del repuesto a eliminar: ", 1, 999999);

    char conf = leerConfirmacionSiNo("¿Esta seguro de eliminar este repuesto? (S/N): ");

    if (conf == 'S') {
        string motivoFallo;
        if (eliminarRepuesto(id, motivoFallo)) {
            cout << "¡Repuesto eliminado exitosamente!" << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuRepuestos() {
    int opcion;
    do {
        cout << "\n=== INVENTARIO DE REPUESTOS ===" << endl;
        cout << "1. Registrar repuesto" << endl;
        cout << "2. Registrar entrada de mercancia" << endl;
        cout << "3. Cambiar precio" << endl;
        cout << "4. Buscar repuesto" << endl;
        cout << "5. Listar repuestos" << endl;
        cout << "6. Eliminar repuesto" << endl;
        cout << "0. Volver" << endl;

        opcion = leerEnteroSeguro("Seleccione opcion: ", 0, 6);
        switch (opcion) {
            case 1: menuRegistrarRepuesto(); break;
            case 2: menuEntradaMercancia(); break;
            case 3: menuCambiarPrecio(); break;
            case 4: menuBuscarRepuesto(); break;
            case 5: menuListarRepuestos(); break;
            case 6: menuEliminarRepuesto(); break;
            case 0: break;
        }
    } while (opcion != 0);
}

// ==============================
// MÓDULO DE ÓRDENES DE SERVICIO
// ==============================

void mostrarDetalleOrden(const OrdenServicio &o) {
    float totalOrden = 0.0f;
    if (!calcularTotalOrden(o, totalOrden)) {
        cout << "ERROR: La orden contiene datos de detalle o importes invalidos." << endl;
        return;
    }

    Vehiculo v = {};
    bool elimV = false;
    long posV = 0;
    if (!buscarVehiculoPorID(o.idVehiculo, v, elimV, posV) && !elimV) {
        cout << "ERROR: No se pudo recuperar el vehiculo de la orden." << endl;
        return;
    }

    Mecanico m = {};
    if (!obtenerMecanicoHistorico(o.idMecanico, m)) {
        cout << "ERROR: No se pudo recuperar el mecanico de la orden." << endl;
        return;
    }
    if (!cadenaTerminada(v.placa) || !cadenaTerminada(v.marca) ||
        !cadenaTerminada(v.modelo) || !cadenaTerminada(m.nombre) ||
        !cadenaTerminada(m.especialidad)) {
        cout << "ERROR: La orden referencia datos con cadenas sin terminador." << endl;
        return;
    }

    cout << "\n========================================" << endl;
    cout << "         ORDEN DE SERVICIO #" << o.id << endl;
    cout << "========================================" << endl;
    cout << "Estado     : " << o.estado << endl;
    cout << "Apertura   : " << o.fechaApertura << " | Cierre: " << (strlen(o.fechaCierre) > 0 ? o.fechaCierre : "N/A") << endl;
    cout << "Vehiculo   : " << v.placa << " - " << v.marca << " " << v.modelo << " (" << o.kilometrajeIngreso << " km)" << endl;
    cout << "Mecanico   : " << m.nombre << " (" << m.especialidad << ")" << (m.eliminado ? " (BAJA)" : "") << endl;
    cout << "Falla      : " << o.descripcionFalla << endl;
    cout << "----------------------------------------" << endl;
    cout << left << setw(8) << "Codigo" << setw(18) << "Repuesto" << setw(6) << "Cant" << setw(8) << "P.Unit" << setw(8) << "Total" << endl;
    cout << "----------------------------------------" << endl;

    for (int i = 0; i < o.numDetalles; i++) {
        Repuesto r = {};
        bool elimR = false;
        if (!buscarRepuestoPorID(o.detalles[i].idRepuesto, r, elimR)) {
            if (!obtenerRepuestoHistorico(o.detalles[i].idRepuesto, r)) {
                cout << "ERROR: No se pudo recuperar el repuesto ID #"
                     << o.detalles[i].idRepuesto << " de la orden." << endl;
                return;
            }
        }
        if (!cadenaTerminada(r.codigo) || !cadenaTerminada(r.nombre)) {
            cout << "ERROR: El repuesto de la orden contiene una cadena sin terminador." << endl;
            return;
        }
        float subtotal = o.detalles[i].cantidad * o.detalles[i].precioUnitario;

        cout << left << setw(8) << r.codigo << setw(18) << string(r.nombre).substr(0, 16)
             << setw(6) << o.detalles[i].cantidad << fixed << setprecision(2) << setw(8) << o.detalles[i].precioUnitario
             << setw(8) << subtotal << endl;
    }
    cout << "----------------------------------------" << endl;
    cout << "Mano de obra: $" << fixed << setprecision(2) << o.manoObra << endl;
    cout << "TOTAL       : $" << fixed << setprecision(2) << totalOrden << endl;
    cout << "========================================" << endl;
}

void menuAbrirOrden() {
    cout << "\n--- ABRIR ORDEN DE SERVICIO ---" << endl;
    int idV = leerEnteroSeguro("ID del vehiculo: ", 1, 999999);
    int idM = leerEnteroSeguro("ID del mecanico: ", 1, 999999);

    char fecha[11];
    leerFormatoFijo("Fecha de apertura (YYYY-MM-DD): ", fecha, 10);
    int km = leerEnteroSeguro("Kilometraje de ingreso (>= 0): ", 0, 2000000);

    char falla[200];
    leerTextoNoVacio("Descripcion de la falla: ", falla, 200);

    OrdenServicio nueva = {};
    string motivoFallo;
    if (abrirOrdenServicio(idV, idM, fecha, falla, km, nueva, motivoFallo)) {
        cout << "¡Orden de servicio #" << nueva.id << " abierta exitosamente y kilometraje del vehiculo actualizado!" << endl;
    } else {
        cout << "ERROR: " << motivoFallo << endl;
    }
}

void menuAgregarRepuestoOrden() {
    cout << "\n--- AGREGAR REPUESTO A ORDEN ---" << endl;
    int idO = leerEnteroSeguro("ID de la orden: ", 1, 999999);
    int idR = leerEnteroSeguro("ID del repuesto: ", 1, 999999);
    int cant = leerEnteroSeguro("Cantidad (> 0): ", 1, 1000000);

    string motivoFallo;
    if (agregarRepuestoAOrden(idO, idR, cant, motivoFallo)) {
        cout << "¡Repuesto agregado a la orden y stock descontado del inventario exitosamente!" << endl;
    } else {
        cout << "ERROR: " << motivoFallo << endl;
    }
}

void menuVerDetalleOrden() {
    cout << "\n--- VER DETALLE DE ORDEN ---" << endl;
    int idO = leerEnteroSeguro("Ingrese el ID de la orden: ", 1, 999999);

    OrdenServicio o;
    if (buscarOrdenPorID(idO, o)) {
        mostrarDetalleOrden(o);
    } else {
        cout << "ERROR: Orden no encontrada o fue eliminada." << endl;
    }
}

void menuQuitarRepuestoOrden() {
    cout << "\n--- QUITAR REPUESTO DE ORDEN ---" << endl;
    int idO = leerEnteroSeguro("ID de la orden: ", 1, 999999);
    int idR = leerEnteroSeguro("ID del repuesto a quitar de la linea: ", 1, 999999);

    string motivoFallo;
    if (quitarRepuestoDeOrden(idO, idR, motivoFallo)) {
        cout << "¡Linea removida exitosamente! El stock ha sido devuelto al inventario de forma segura." << endl;
    } else {
        cout << "ERROR: " << motivoFallo << endl;
    }
}

void menuCerrarOrden() {
    cout << "\n--- CERRAR ORDEN DE SERVICIO ---" << endl;
    int idO = leerEnteroSeguro("ID de la orden: ", 1, 999999);

    float mano = leerFlotanteSeguro("Costo de mano de obra (>= 0): ", 0.0f, 1000000.0f);

    char conf = leerConfirmacionSiNo(
        "¿Desea cerrar esta orden? (Ya no se podran agregar repuestos) (S/N): ");

    if (conf == 'S') {
        string motivoFallo;
        if (cerrarOrdenServicio(idO, mano, motivoFallo)) {
            cout << "¡Orden cerrada exitosamente! Ya esta lista para facturar." << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuAnularOrden() {
    cout << "\n--- ANULAR ORDEN DE SERVICIO ---" << endl;
    int idO = leerEnteroSeguro("ID de la orden a anular: ", 1, 999999);

    char conf = leerConfirmacionSiNo(
        "¿Confirma la anulacion? (El stock de repuestos volvera automaticamente al inventario) (S/N): ");

    if (conf == 'S') {
        string motivoFallo;
        if (anularOrdenServicio(idO, motivoFallo)) {
            cout << "¡Orden anulada exitosamente! El stock fue devuelto al inventario de repuestos." << endl;
        } else {
            cout << "ERROR: " << motivoFallo << endl;
        }
    } else {
        cout << "Operacion cancelada." << endl;
    }
}

void menuListarOrdenesPorEstado() {
    cout << "\n--- LISTAR ORDENES POR ESTADO ---" << endl;
    cout << "1. ABIERTA" << endl;
    cout << "2. CERRADA" << endl;
    cout << "3. ANULADA" << endl;

    int opcionEstado = leerEnteroSeguro("Seleccione una opcion: ", 1, 3);
    const char* estadoFiltro;
    if (opcionEstado == 1) {
        estadoFiltro = "ABIERTA";
    } else if (opcionEstado == 2) {
        estadoFiltro = "CERRADA";
    } else {
        estadoFiltro = "ANULADA";
    }

    OrdenServicio lista[MAX_RESULTADOS];
    int cantidad = 0;
    bool hayMas = false;
    string motivoFallo;

    if (listarOrdenesPorEstado(estadoFiltro, lista, cantidad, hayMas, motivoFallo)) {
        cout << "\nSe encontraron " << cantidad << " orden(es) en estado " << estadoFiltro << ":" << endl;
        for (int i = 0; i < cantidad; i++) {
            mostrarDetalleOrden(lista[i]);
        }
        if (hayMas) {
            cout << "AVISO: Existen mas ordenes en este estado. Se muestran las primeras " << MAX_RESULTADOS << "." << endl;
        }
    } else if (!motivoFallo.empty()) {
        cout << "ERROR: " << motivoFallo << endl;
    } else {
        cout << "No se encontraron ordenes registradas con el estado " << estadoFiltro << "." << endl;
    }
}

void menuOrdenes() {
    int opcion;
    do {
        cout << "\n=== ORDENES DE SERVICIO ===" << endl;
        cout << "1. Abrir orden" << endl;
        cout << "2. Agregar repuesto a orden" << endl;
        cout << "3. Quitar repuesto de orden" << endl;
        cout << "4. Cerrar orden" << endl;
        cout << "5. Anular orden" << endl;
        cout << "6. Ver detalle de orden" << endl;
        cout << "7. Listar ordenes por estado" << endl;
        cout << "0. Volver" << endl;

        opcion = leerEnteroSeguro("Seleccione opcion: ", 0, 7);
        switch (opcion) {
            case 1: menuAbrirOrden(); break;
            case 2: menuAgregarRepuestoOrden(); break;
            case 3: menuQuitarRepuestoOrden(); break;
            case 4: menuCerrarOrden(); break;
            case 5: menuAnularOrden(); break;
            case 6: menuVerDetalleOrden(); break;
            case 7: menuListarOrdenesPorEstado(); break;
            case 0: break;
        }
    } while (opcion != 0);
}

// ====================
// MÓDULO DE REPORTES
// ====================

void menuReportes() {
    int opcion;
    do {
        cout << "\n=== MODULO DE REPORTES ===" << endl;
        cout << "1. Historial de vehiculo por placa" << endl;
        cout << "2. Repuestos mas utilizados (Top 10)" << endl;
        cout << "3. Facturacion por mecanico" << endl;
        cout << "0. Volver" << endl;
        opcion = leerEnteroSeguro("Seleccione una opcion: ", 0, 3);

        if (opcion == 1) {
            char placa[10];
            leerTextoNoVacio("Ingrese la placa exacta del vehiculo: ", placa, 10);
            Vehiculo v;
            OrdenServicio ords[100];
            int inicio = 0;
            int cant = 0;
            bool hayMas = false;
            float total = 0.0f;
            string motivoFallo;

            if (generarReporteHistorialVehiculo(placa, v, inicio, ords, cant, hayMas, total,
                                                motivoFallo)) {
                cout << "\n--- HISTORIAL DEL VEHICULO: " << v.placa << " ---" << endl;
                cout << "Propietario: " << v.nombrePropietario << " | " << v.marca << " " << v.modelo << " (" << v.anio << ")" << endl;
                do {
                    for (int i = 0; i < cant; i++) {
                        float totalOrden = 0.0f;
                        if (!calcularTotalOrden(ords[i], totalOrden)) {
                            cout << "ERROR: La orden #" << ords[i].id
                                 << " contiene datos de detalle o importes invalidos." << endl;
                            break;
                        }
                        Mecanico m = {};
                        if (!obtenerMecanicoHistorico(ords[i].idMecanico, m)) {
                            cout << "ERROR: No se pudo recuperar el mecanico de la orden #"
                                 << ords[i].id << "." << endl;
                            break;
                        }
                        cout << " - Orden #" << ords[i].id << " | Fecha: " << ords[i].fechaApertura
                             << " | Mecanico: " << m.nombre << (m.eliminado ? " (BAJA)" : "")
                             << " | Estado: " << ords[i].estado
                             << " | Total: $" << fixed << setprecision(2) << totalOrden << endl;
                    }
                    inicio += cant;
                    if (hayMas) {
                        if (!generarReporteHistorialVehiculo(placa, v, inicio, ords, cant,
                                                             hayMas, total, motivoFallo)) {
                            cout << "ERROR: " << (motivoFallo.empty()
                                ? "No se pudo continuar leyendo el historial."
                                : motivoFallo) << endl;
                            break;
                        }
                    }
                } while (hayMas);
                cout << "Total Facturado (Ordenes CERRADAS): $" << fixed << setprecision(2) << total << endl;
            } else if (!motivoFallo.empty()) {
                cout << "ERROR: " << motivoFallo << endl;
            } else {
                cout << "ERROR: Vehiculo con placa '" << placa << "' no encontrado." << endl;
            }
        } else if (opcion == 2) {
            TopRepuesto top10[10];
            int cantEncontrados = 0;
            string motivoFallo;
            cout << "\n--- TOP 10: REPUESTOS MAS UTILIZADOS (ORDENES CERRADAS) ---" << endl;
            if (generarReporteTopRepuestos(top10, cantEncontrados, motivoFallo)) {
                cout << left << setw(12) << "Codigo" << setw(25) << "Nombre" << "Unidades" << endl;
                cout << "-----------------------------------------------" << endl;
                for (int i = 0; i < cantEncontrados; i++) {
                    cout << left << setw(12) << top10[i].codigo
                         << setw(25) << string(top10[i].nombre).substr(0, 24)
                         << top10[i].unidades << endl;
                }
            } else if (!motivoFallo.empty()) {
                cout << "ERROR: " << motivoFallo << endl;
            } else {
                cout << "No hay repuestos consumidos en ordenes cerradas." << endl;
            }
        } else if (opcion == 3) {
            FilaFacturacionMecanico filas[MAX_RESULTADOS];
            int cantFilas = 0;
            bool hayMas = false;
            string motivoFallo;
            cout << "\n--- FACTURACION POR MECANICO ---" << endl;
            if (generarReporteFacturacionMecanicos(filas, cantFilas, hayMas, motivoFallo)) {
                cout << "╔══════════════════════╦═══════════╦═══════════╦═══════════╗" << endl;
                cout << "║ Mecanico             ║  Ordenes  ║  M. obra  ║ Repuestos ║" << endl;
                cout << "╠══════════════════════╬═══════════╬═══════════╬═══════════╣" << endl;
                for (int i = 0; i < cantFilas; i++) {
                    string nom = string(filas[i].nombre) + (filas[i].dadoDeBaja ? " (BAJA)" : "");
                    cout << "║ " << left << setw(21) << nom.substr(0, 21)
                         << "║ " << right << setw(9) << filas[i].ordenesCerradas
                         << " ║ " << fixed << setprecision(2) << setw(9) << filas[i].totalManoObra
                         << " ║ " << setw(9) << filas[i].totalRepuestos << " ║" << endl;
                }
                cout << "╚══════════════════════╩═══════════╩═══════════╩═══════════╝" << endl;
                if (hayMas) {
                    cout << "AVISO: Existen mas mecanicos en el archivo. Se muestran los primeros " << MAX_RESULTADOS << "." << endl;
                }
            } else if (!motivoFallo.empty()) {
                cout << "ERROR: " << motivoFallo << endl;
            } else {
                cout << "No hay mecanicos registrados." << endl;
            }
        }
    } while (opcion != 0);
}

// =========================
// MÓDULO DE MANTENIMIENTO
// =========================

void menuMantenimiento() {
    int opcion;
    do {
        cout << "\n=== MODULO DE MANTENIMIENTO ===" << endl;
        cout << "1. Crear respaldo (Backup)" << endl;
        cout << "2. Verificar integridad del sistema" << endl;
        cout << "0. Volver" << endl;
        opcion = leerEnteroSeguro("Seleccione una opcion: ", 0, 2);

        if (opcion == 1) {
            string rutaBackup;
            string motivoFallo;
            if (crearBackup(rutaBackup, motivoFallo)) {
                cout << "Respaldo creado exitosamente en: " << rutaBackup << endl;
            } else {
                cout << "ERROR: " << motivoFallo << endl;
            }
        } else if (opcion == 2) {
            vector<string> errores;
            int contadores[4] = {0};

            verificarIntegridadSistema(errores, contadores);

            for (size_t i = 0; i < errores.size(); i++) {
                cout << errores[i] << endl;
            }
            if (!errores.empty()) cout << endl;

            cout << "╔═══════════════════════════════════════════════════════╗" << endl;
            cout << "║          REPORTE DE INTEGRIDAD DEL SISTEMA            ║" << endl;
            cout << "╠═══════════════════════════════════════════════════════╣" << endl;
            cout << "║ Vehiculos verificados : " << left << setw(30) << contadores[0] << "║" << endl;
            cout << "║ Mecanicos verificados : " << left << setw(30) << contadores[1] << "║" << endl;
            cout << "║ Repuestos verificados : " << left << setw(30) << contadores[2] << "║" << endl;
            cout << "║ Ordenes verificadas   : " << left << setw(30) << contadores[3] << "║" << endl;
            cout << "║                                                       ║" << endl;
            cout << "║ Problemas encontrados : " << left << setw(30) << errores.size() << "║" << endl;
            cout << "║ Estado: " << left << setw(46) << (errores.empty() ? "SISTEMA INTEGRO" : "REQUIERE ATENCION") << "║" << endl;
            cout << "╚═══════════════════════════════════════════════════════╝" << endl;
        }
    } while (opcion != 0);
}

// ==================================
// MENU PRINCIPAL Y PUNTO DE ENTRADA 
// ==================================

int main() {
    string motivoFallo;
    if (!inicializarSistemaArchivos(motivoFallo)) {
        cerr << "ERROR al inicializar los archivos del sistema: " << motivoFallo << endl;
        return 1;
    }

    Taller miTaller = {};
    bool primeraEjecucion = false;
    if (!cargarTaller(miTaller, primeraEjecucion, motivoFallo)) {
        cerr << "ERROR al cargar los datos del taller: " << motivoFallo << endl;
        return 1;
    }
    if (primeraEjecucion) {
        cout << "==========================================" << endl;
        cout << "   PRIMER ARRANQUE - CONFIGURAR TALLER    " << endl;
        cout << "==========================================" << endl;
        leerTextoNoVacio("Nombre del taller: ", miTaller.nombre, 100);
        leerTextoNoVacio("RIF: ", miTaller.rif, 20);
        leerTextoNoVacio("Direccion: ", miTaller.direccion, 200);
        leerTelefonoSeguro("Telefono: ", miTaller.telefono, 20);

        if (!guardarTaller(miTaller, motivoFallo)) {
            cerr << "ERROR al guardar los datos del taller: " << motivoFallo << endl;
            return 1;
        }
        cout << "\n¡Taller configurado exitosamente!\n";
    }

    int opcion;
    do {
        cout << "\n╔═══════════════════════════════════════════╗" << endl;
        cout << "║       SISTEMA DE GESTION DE TALLER        ║" << endl;
        cout << "║ Taller: " << left << setw(33) << string(miTaller.nombre).substr(0, 33) << " ║" << endl;
        cout << "╠═══════════════════════════════════════════╣" << endl;
        cout << "║ 1. Gestion de Vehiculos                   ║" << endl;
        cout << "║ 2. Gestion de Mecanicos                   ║" << endl;
        cout << "║ 3. Inventario de Repuestos                ║" << endl;
        cout << "║ 4. Ordenes de Servicio                    ║" << endl;
        cout << "║ 5. Reportes                               ║" << endl;
        cout << "║ 6. Mantenimiento                          ║" << endl;
        cout << "║ 0. Salir                                  ║" << endl;
        cout << "╚═══════════════════════════════════════════╝" << endl;

        opcion = leerEnteroSeguro("Seleccione una opcion: ", 0, 6);

        switch (opcion) {
            case 1: menuVehiculos(); break;
            case 2: menuMecanicos(); break;
            case 3: menuRepuestos(); break;
            case 4: menuOrdenes(); break;
            case 5: menuReportes(); break;
            case 6: menuMantenimiento(); break;
            case 0: {
                char cont = leerConfirmacionSiNo(
                    "¿Esta seguro de salir del sistema? (S/N): ");

                if (cont != 'S') {
                    opcion = -1;
                } else {
                    cout << "\nSaliendo del sistema... ¡Hasta luego!" << endl;
                }
                break;
            }
        }
    } while (opcion != 0);

    return 0;
}
