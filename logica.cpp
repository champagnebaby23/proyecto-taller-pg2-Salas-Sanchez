#include "logica.h"
#include "persistencia.h"
#include <cstring>
#include <cctype>
#include <string>
#include <ctime>
#include <cmath>
#include <limits>
#include <climits>

using namespace std;

// ==================================================
// FUNCIONES AUXILIARES DE CADENAS
// ==================================================

void aMinusculas(const char* origen, char* destino, size_t tamDestino) {
    size_t i = 0;
    while (origen[i] != '\0' && i + 1 < tamDestino) {
        destino[i] = static_cast<char>(tolower(static_cast<unsigned char>(origen[i])));
        i++;
    }
    destino[i] = '\0';
}

bool contieneSubcadena(const char* texto, const char* subcadena) {
    char t[256], sub[256];
    aMinusculas(texto, t, sizeof(t));
    aMinusculas(subcadena, sub, sizeof(sub));
    return strstr(t, sub) != nullptr;
}

static bool detalleDentroDelLimite(const OrdenServicio &orden) {
    return validarDetalleOrden(orden);
}

template <size_t N>
static bool cadenaTerminada(const char (&campo)[N]) {
    return memchr(campo, '\0', N) != nullptr;
}

static bool camposOrdenTerminados(const OrdenServicio &orden) {
    return cadenaTerminada(orden.fechaApertura) &&
           cadenaTerminada(orden.fechaCierre) &&
           cadenaTerminada(orden.estado) &&
           cadenaTerminada(orden.descripcionFalla);
}

static bool camposVehiculoTerminados(const Vehiculo &vehiculo) {
    return cadenaTerminada(vehiculo.placa) && cadenaTerminada(vehiculo.marca) &&
           cadenaTerminada(vehiculo.modelo) && cadenaTerminada(vehiculo.nombrePropietario) &&
           cadenaTerminada(vehiculo.cedulaPropietario) &&
           cadenaTerminada(vehiculo.telefonoPropietario);
}

static bool camposMecanicoTerminados(const Mecanico &mecanico) {
    return cadenaTerminada(mecanico.nombre) && cadenaTerminada(mecanico.cedula) &&
           cadenaTerminada(mecanico.especialidad) &&
           cadenaTerminada(mecanico.fechaIngreso);
}

static bool camposRepuestoTerminados(const Repuesto &repuesto) {
    return cadenaTerminada(repuesto.codigo) && cadenaTerminada(repuesto.nombre) &&
           cadenaTerminada(repuesto.marca);
}

static bool estadoOrdenValido(const char* estado) {
    return strcmp(estado, "ABIERTA") == 0 ||
           strcmp(estado, "CERRADA") == 0 ||
           strcmp(estado, "ANULADA") == 0;
}

bool fechaValidaISO(const char* fecha) {
    if (fecha == nullptr) return false;
    const char* fin = static_cast<const char*>(memchr(fecha, '\0', 11));
    if (fin == nullptr || fin - fecha != 10) return false;

    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (fecha[i] != '-') return false;
        } else if (!isdigit(static_cast<unsigned char>(fecha[i]))) {
            return false;
        }
    }

    int anio = (fecha[0] - '0') * 1000 + (fecha[1] - '0') * 100 +
               (fecha[2] - '0') * 10 + fecha[3] - '0';
    int mes = (fecha[5] - '0') * 10 + fecha[6] - '0';
    int dia = (fecha[8] - '0') * 10 + fecha[9] - '0';
    if (anio < 1 || mes < 1 || mes > 12) return false;

    const int diasPorMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maximoDia = diasPorMes[mes - 1];
    bool bisiesto = (anio % 4 == 0 && anio % 100 != 0) || anio % 400 == 0;
    if (mes == 2 && bisiesto) maximoDia = 29;
    return dia >= 1 && dia <= maximoDia;
}

// ====================================
// CONFIGURACIÓN DEL SISTEMA Y TALLER
// ====================================

bool inicializarSistemaArchivos(string &motivoFallo) {
    if (!asegurarDirectorioDatos(motivoFallo)) return false;

    // Declara los archivos y el tamano de sus registros
    const struct {
        const char* ruta;
        size_t tamanoRegistro;
    } archivos[] = {
        {"datos/vehiculos.bin", sizeof(Vehiculo)},
        {"datos/mecanicos.bin", sizeof(Mecanico)},
        {"datos/repuestos.bin", sizeof(Repuesto)},
        {"datos/ordenes.bin", sizeof(OrdenServicio)}
    };

    // Valida cada archivo
    for (size_t i = 0; i < sizeof(archivos) / sizeof(archivos[0]); i++) {
        if (!inicializarArchivo(archivos[i].ruta, archivos[i].tamanoRegistro)) {
            motivoFallo = "No se pudo inicializar " + string(archivos[i].ruta) +
                          " (archivo inexistente, ilegible o con metadata/contenido inconsistente).";
            return false;
        }
    }
    return true;
}


bool cargarTaller(Taller &t, bool &primeraEjecucion, string &motivoFallo) {
    bool noExiste = false;
    primeraEjecucion = false;
    if (cargarTallerArchivo(t, noExiste)) {
        if (!cadenaTerminada(t.nombre) || !cadenaTerminada(t.rif) ||
            !cadenaTerminada(t.direccion) || !cadenaTerminada(t.telefono)) {
            motivoFallo = "datos/taller.bin contiene una cadena sin terminador."; // terminador: '\0'
            return false;
        }
        return true;
    }
    if (noExiste) {
        primeraEjecucion = true;
        t = {};
        return true; // Asi main sabe que debe pedir la informacion inicial
    }
    motivoFallo = "taller.bin existe, pero no se pudo leer un registro Taller completo.";
    return false;
}

bool guardarTaller(const Taller &t, string &motivoFallo) {
    if (!cadenaTerminada(t.nombre) || !cadenaTerminada(t.rif) ||
        !cadenaTerminada(t.direccion) || !cadenaTerminada(t.telefono)) {
        motivoFallo = "Los datos del taller contienen cadenas sin terminador.";
        return false;
    }
    if (!guardarTallerArchivo(t)) {
        motivoFallo = "No se pudo escribir datos/taller.bin.";
        return false;
    }
    return true;
}

// ====================
// MÓDULO DE VEHÍCULOS
// ==================== 

bool agregarVehiculo(Vehiculo &datos, string &motivoFallo) {
    if (!camposVehiculoTerminados(datos) || datos.placa[0] == '\0') {
        motivoFallo = "Los datos del vehiculo contienen una placa vacia o cadenas sin terminador.";
        return false;
    }
    if (datos.anio < 1950 || datos.anio > 2027) {
        motivoFallo = "El anio de fabricacion debe estar comprendido entre 1950 y 2027.";
        return false;
    }
    if (datos.kilometraje < 0) {
        motivoFallo = "El kilometraje no puede ser un valor negativo.";
        return false;
    }

    // Placa unica entre los vehiculos activos
    Vehiculo existente;
    string motivoBusqueda;
    if (buscarVehiculoPorPlaca(datos.placa, existente, motivoBusqueda)) {
        motivoFallo = "Ya existe un vehiculo activo registrado con la placa '" + string(datos.placa) + "'.";
        return false;
    }
    if (!motivoBusqueda.empty()) {
        motivoFallo = motivoBusqueda;
        return false;
    }

    if (!guardarVehiculoCrudo(datos, motivoFallo)) {
        return false;
    }
    return true;
}

bool buscarVehiculoPorID(int id, Vehiculo &resultado, bool &fueEliminado, long &posicionBytes) {
    fueEliminado = false;
    posicionBytes = 0;

    if (!leerVehiculoPorID(id, resultado, &posicionBytes)) {
        return false; // El ID no existe en el archivo
    }

    if (resultado.eliminado) {
        fueEliminado = true;
        return false; // La lógica retorna false y reporta si fue eliminado
    }

    return true;
}

bool buscarVehiculoPorPlaca(const char* placa, Vehiculo &resultado, string &motivoFallo) {
    motivoFallo.clear();
    if (placa == nullptr) {
        motivoFallo = "La placa de busqueda es nula.";
        return false;
    }
    ArchivoHeader h;
    if (!leerHeader("datos/vehiculos.bin", h)) {
        motivoFallo = "No se pudo leer el header de vehiculos.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Vehiculo v;
        if (!leerVehiculoPorID(i, v)) {
            motivoFallo = "No se pudo leer el vehiculo ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposVehiculoTerminados(v)) {
            motivoFallo = "El vehiculo ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!v.eliminado && strcmp(v.placa, placa) == 0) {
            resultado = v;
            return true;
        }
    }
    return false;
}

bool buscarVehiculosPorPropietario(const char* subcadena, Vehiculo resultados[],
                                   int &cantidad, bool &hayMas, string &motivoFallo) {
    cantidad = 0;
    hayMas = false;
    motivoFallo.clear();

    ArchivoHeader h;
    if (!leerHeader("datos/vehiculos.bin", h)) {
        motivoFallo = "No se pudo leer el header de vehiculos.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Vehiculo v;
        if (!leerVehiculoPorID(i, v)) {
            motivoFallo = "No se pudo leer el vehiculo ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposVehiculoTerminados(v)) {
            motivoFallo = "El vehiculo ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!v.eliminado && contieneSubcadena(v.nombrePropietario, subcadena)) {
            if (cantidad < MAX_RESULTADOS) {
                resultados[cantidad++] = v;
            } else {
                hayMas = true;
                break;
            }
        }
    }
    return cantidad > 0;
}

bool listarVehiculos(Vehiculo resultados[], int &cantidad, bool &hayMas,
                     string &motivoFallo) {
    cantidad = 0;
    hayMas = false;
    motivoFallo.clear();

    ArchivoHeader h;
    if (!leerHeader("datos/vehiculos.bin", h)) {
        motivoFallo = "No se pudo leer el header de vehiculos.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Vehiculo v;
        if (!leerVehiculoPorID(i, v)) {
            motivoFallo = "No se pudo leer el vehiculo ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposVehiculoTerminados(v)) {
            motivoFallo = "El vehiculo ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!v.eliminado) {
            if (cantidad < MAX_RESULTADOS) {
                resultados[cantidad++] = v;
            } else {
                hayMas = true;
                break;
            }
        }
    }
    return cantidad > 0;
}

bool actualizarVehiculo(int id, const Vehiculo &datosNuevos, string &motivoFallo) {
    if (!camposVehiculoTerminados(datosNuevos)) {
        motivoFallo = "Los nuevos datos del vehiculo contienen cadenas sin terminador.";
        return false;
    }
    Vehiculo vActual;
    bool elim = false;
    long pos = 0;
    if (!buscarVehiculoPorID(id, vActual, elim, pos)) {
        if (elim) {
            motivoFallo = "El vehiculo fue eliminado logicamente.";
        } else {
            motivoFallo = "El vehiculo no existe.";
        }
        return false;
    }

    if (datosNuevos.anio < 1950 || datosNuevos.anio > 2027) {
        motivoFallo = "El ano del vehiculo debe estar entre 1950 y 2027.";
        return false;
    }

    // No cambia placa, kilometraje ni ID
    strcpy(vActual.marca, datosNuevos.marca);
    strcpy(vActual.modelo, datosNuevos.modelo);
    vActual.anio = datosNuevos.anio;
    strcpy(vActual.nombrePropietario, datosNuevos.nombrePropietario);
    strcpy(vActual.cedulaPropietario, datosNuevos.cedulaPropietario);
    strcpy(vActual.telefonoPropietario, datosNuevos.telefonoPropietario);

    if (!sobrescribirVehiculo(vActual)) {
        motivoFallo = "Fallo al escribir la actualizacion en el archivo de vehiculos.";
        return false;
    }
    return true;
}

bool eliminarVehiculo(int id, string &motivoFallo) {
    Vehiculo v;
    bool elim = false;
    long pos = 0;
    if (!buscarVehiculoPorID(id, v, elim, pos)) {
        if (elim) {
            motivoFallo = "El vehiculo ya se encuentra eliminado.";
        } else {
            motivoFallo = "El vehiculo no existe.";
        }
        return false;
    }

    // Las ordenes historicas se conservan; solo una orden abierta requiere el vehiculo activo.
    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) {
        motivoFallo = "No se pudo verificar ordenes.bin; no se elimino el vehiculo.";
        return false;
    }
    for (int i = 1; i <= hOrd.cantidadRegistros; i++) {
        OrdenServicio o;
        if (!leerOrdenPorID(i, o)) {
            motivoFallo = "No se pudo leer la orden ID #" + to_string(i) +
                          "; no se elimino el vehiculo.";
            return false;
        }
        if (!validarOrdenServicio(o)) {
            motivoFallo = "La orden ID #" + to_string(i) +
                          " contiene datos invalidos; no se elimino el vehiculo.";
            return false;
        }
        if (!o.eliminado && o.idVehiculo == id && strcmp(o.estado, "ABIERTA") == 0) {
            motivoFallo = "No se puede eliminar el vehiculo porque tiene registrada la orden #" +
                          to_string(o.id) + " en estado ABIERTA.";
            return false;
        }
    }

    if (!marcarVehiculoEliminado(id, motivoFallo)) {
        return false;
    }
    return true;
}

// ====================
// MODULO DE MECANICOS
// ====================

bool especialidadValida(const char* esp) {
    return (strcmp(esp, "MOTOR") == 0 ||
            strcmp(esp, "FRENOS") == 0 ||
            strcmp(esp, "ELECTRICIDAD") == 0 ||
            strcmp(esp, "SUSPENSION") == 0 ||
            strcmp(esp, "GENERAL") == 0);
}

bool registrarMecanico(Mecanico &datos, string &motivoFallo) {
    if (!camposMecanicoTerminados(datos) || datos.nombre[0] == '\0' ||
        datos.cedula[0] == '\0') {
        motivoFallo = "Los datos del mecanico contienen cadenas vacias o sin terminador.";
        return false;
    }
    if (!especialidadValida(datos.especialidad)) {
        motivoFallo = "Especialidad invalida. Debe ser MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION o GENERAL.";
        return false;
    }
    if (!fechaValidaISO(datos.fechaIngreso)) {
        motivoFallo = "La fecha de ingreso debe ser una fecha real con formato YYYY-MM-DD.";
        return false;
    }

    // Cedula única
    ArchivoHeader hMec;
    if (!leerHeader("datos/mecanicos.bin", hMec)) {
        motivoFallo = "No se pudo leer el header de mecanicos.bin.";
        return false;
    }
    for (int i = 1; i <= hMec.cantidadRegistros; i++) {
        Mecanico existente;
        if (!leerMecanicoPorID(i, existente)) {
            motivoFallo = "No se pudo verificar la unicidad de la cedula en mecanicos.bin.";
            return false;
        }
        if (!camposMecanicoTerminados(existente)) {
            motivoFallo = "El mecanico ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (strcmp(existente.cedula, datos.cedula) == 0) {
            motivoFallo = "La cedula '" + string(datos.cedula) + "' ya fue registrada, incluso si el mecanico esta dado de baja.";
            return false;
        }
    }

    if (!guardarMecanicoCrudo(datos, motivoFallo)) {
        return false;
    }
    return true;
}

bool buscarMecanicoPorID(int id, Mecanico &resultado, bool &fueEliminado, long *posicionBytes) {
    fueEliminado = false;
    if (!leerMecanicoPorID(id, resultado, posicionBytes)) {
        return false;
    }

    if (resultado.eliminado) {
        fueEliminado = true;
        return false; // La logica retorna false para mecanicos inactivos en busquedas estandar
    }
    return true;
}

bool obtenerMecanicoHistorico(int id, Mecanico &resultado) {
    // Obtiene un mecanico este activo o dado de baja 
    return leerMecanicoPorID(id, resultado, nullptr);
}

bool buscarMecanicoPorCedula(const char* cedula, Mecanico &resultado,
                             string &motivoFallo) {
    motivoFallo.clear();
    if (cedula == nullptr) {
        motivoFallo = "La cedula de busqueda es nula.";
        return false;
    }
    ArchivoHeader h;
    if (!leerHeader("datos/mecanicos.bin", h)) {
        motivoFallo = "No se pudo leer el header de mecanicos.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Mecanico m;
        if (!leerMecanicoPorID(i, m)) {
            motivoFallo = "No se pudo leer el mecanico ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposMecanicoTerminados(m)) {
            motivoFallo = "El mecanico ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!m.eliminado && strcmp(m.cedula, cedula) == 0) {
            resultado = m;
            return true;
        }
    }
    return false;
}

bool contarOrdenesAbiertasMecanico(int idMecanico, int &total, string &motivoFallo) {
    total = 0;
    ArchivoHeader h;
    if (!leerHeader("datos/ordenes.bin", h)) {
        motivoFallo = "No se pudo leer el header de ordenes.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        OrdenServicio o;
        if (!leerOrdenPorID(i, o)) {
            motivoFallo = "No se pudo leer la orden ID #" + to_string(i) + ".";
            return false;
        }
        if (!validarOrdenServicio(o)) {
            motivoFallo = "La orden ID #" + to_string(i) + " contiene datos invalidos.";
            return false;
        }
        if (!o.eliminado && o.idMecanico == idMecanico && strcmp(o.estado, "ABIERTA") == 0) {
            total++;
        }
    }
    return true;
}

bool listarMecanicos(Mecanico resultados[], int &cantidad, bool &hayMas,
                     string &motivoFallo) {
    cantidad = 0;
    hayMas = false;
    motivoFallo.clear();

    ArchivoHeader h;
    if (!leerHeader("datos/mecanicos.bin", h)) {
        motivoFallo = "No se pudo leer el header de mecanicos.bin.";
        return false;
    }

    // Activos y dados de baja
    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Mecanico m;
        if (!leerMecanicoPorID(i, m)) {
            motivoFallo = "No se pudo leer el mecanico ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposMecanicoTerminados(m)) {
            motivoFallo = "El mecanico ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (cantidad < MAX_RESULTADOS) {
            resultados[cantidad++] = m;
        } else {
            hayMas = true;
            break;
        }
    }
    return cantidad > 0;
}

bool eliminarMecanico(int id, string &motivoFallo) {
    Mecanico m;
    bool fueEliminado = false;
    if (!buscarMecanicoPorID(id, m, fueEliminado)) {
        motivoFallo = fueEliminado ? "El mecanico ya fue dado de baja previamente." : "El mecanico no existe.";
        return false;
    }

    // Rechazado si el mecánico tiene ordenes ABIERTAS
    int abiertas = 0;
    if (!contarOrdenesAbiertasMecanico(id, abiertas, motivoFallo)) {
        return false;
    }
    if (abiertas > 0) {
        motivoFallo = "No se puede dar de baja: el mecanico tiene " + to_string(abiertas) + " orden(es) ABIERTA(S).";
        return false;
    }

    if (!marcarMecanicoEliminado(id, motivoFallo)) {
        return false;
    }
    return true;
}

// ====================
// MÓDULO DE REPUESTOS
// ====================

bool registrarRepuesto(Repuesto &datos, string &motivoFallo) {
    if (!camposRepuestoTerminados(datos) || datos.codigo[0] == '\0' ||
        datos.nombre[0] == '\0' || datos.marca[0] == '\0') {
        motivoFallo = "Los datos del repuesto contienen cadenas vacias o sin terminador.";
        return false;
    }
    if (!isfinite(datos.precio) || datos.precio <= 0) {
        motivoFallo = "El precio unitario debe ser mayor a 0.";
        return false;
    }
    if (datos.stock < 0) {
        motivoFallo = "El stock inicial no puede ser negativo.";
        return false;
    }
    if (datos.stockMinimo < 0) {
        motivoFallo = "El stock minimo de alerta no puede ser negativo.";
        return false;
    }

    // Codigo unico entre los repuestos activos
    Repuesto existente;
    string motivoBusqueda;
    if (buscarRepuestoPorCodigo(datos.codigo, existente, motivoBusqueda)) {
        motivoFallo = "Ya existe un repuesto activo registrado con el codigo '" + string(datos.codigo) + "'.";
        return false;
    }
    if (!motivoBusqueda.empty()) {
        motivoFallo = motivoBusqueda;
        return false;
    }

    if (!guardarRepuestoCrudo(datos, motivoFallo)) {
        return false;
    }
    return true;
}

bool buscarRepuestoPorID(int id, Repuesto &resultado, bool &fueEliminado, long *posicionBytes) {
    fueEliminado = false;
    if (!leerRepuestoPorID(id, resultado, posicionBytes)) {
        return false;
    }

    if (resultado.eliminado) {
        fueEliminado = true;
        return false;
    }
    return true;
}

bool obtenerRepuestoHistorico(int id, Repuesto &resultado) {
    return leerRepuestoPorID(id, resultado, nullptr);
}

bool buscarRepuestoPorCodigo(const char* codigo, Repuesto &resultado,
                             string &motivoFallo) {
    motivoFallo.clear();
    if (codigo == nullptr) {
        motivoFallo = "El codigo de busqueda es nulo.";
        return false;
    }
    ArchivoHeader h;
    if (!leerHeader("datos/repuestos.bin", h)) {
        motivoFallo = "No se pudo leer el header de repuestos.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Repuesto r;
        if (!leerRepuestoPorID(i, r)) {
            motivoFallo = "No se pudo leer el repuesto ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposRepuestoTerminados(r)) {
            motivoFallo = "El repuesto ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!camposRepuestoTerminados(r)) {
            motivoFallo = "El repuesto ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!r.eliminado && strcmp(r.codigo, codigo) == 0) {
            resultado = r;
            return true;
        }
    }
    return false;
}

bool buscarRepuestoPorNombreParcial(const char* subcadena, Repuesto resultados[],
                                    int &cantidad, bool &hayMas, string &motivoFallo) {
    cantidad = 0;
    hayMas = false;
    motivoFallo.clear();

    ArchivoHeader h;
    if (!leerHeader("datos/repuestos.bin", h)) {
        motivoFallo = "No se pudo leer el header de repuestos.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Repuesto r;
        if (!leerRepuestoPorID(i, r)) {
            motivoFallo = "No se pudo leer el repuesto ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposRepuestoTerminados(r)) {
            motivoFallo = "El repuesto ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!r.eliminado && contieneSubcadena(r.nombre, subcadena)) {
            if (cantidad < MAX_RESULTADOS) {
                resultados[cantidad++] = r;
            } else {
                hayMas = true;
                break;
            }
        }
    }
    return cantidad > 0;
}

bool registrarEntradaMercancia(int idRepuesto, int cantidadAdicional, string &motivoFallo) {
    if (cantidadAdicional <= 0) {
        motivoFallo = "La cantidad a ingresar debe ser mayor a 0.";
        return false;
    }

    Repuesto r;
    bool elim = false;
    if (!buscarRepuestoPorID(idRepuesto, r, elim)) {
        motivoFallo = elim ? "El repuesto se encuentra dado de baja." : "El repuesto no existe.";
        return false;
    }
    if (r.stock < 0 || cantidadAdicional > numeric_limits<int>::max() - r.stock) {
        motivoFallo = "La entrada excede el stock maximo permitido.";
        return false;
    }
    r.stock += cantidadAdicional;
    if (!sobrescribirRepuesto(r)) {
        motivoFallo = "Error al actualizar el stock en el archivo de repuestos.";
        return false;
    }
    return true;
}

bool listarRepuestos(Repuesto resultados[], int &cantidad, bool &hayMas,
                     string &motivoFallo) {
    cantidad = 0;
    hayMas = false;
    motivoFallo.clear();

    ArchivoHeader h;
    if (!leerHeader("datos/repuestos.bin", h)) {
        motivoFallo = "No se pudo leer el header de repuestos.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Repuesto r;
        if (!leerRepuestoPorID(i, r)) {
            motivoFallo = "No se pudo leer el repuesto ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposRepuestoTerminados(r)) {
            motivoFallo = "El repuesto ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }
        if (!r.eliminado) {
            if (cantidad < MAX_RESULTADOS) {
                resultados[cantidad++] = r;
            } else {
                hayMas = true;
                break;
            }
        }
    }
    return cantidad > 0;
}

bool cambiarPrecioRepuesto(int idRepuesto, float nuevoPrecio, string &motivoFallo) {
    if (!isfinite(nuevoPrecio) || nuevoPrecio <= 0) {
        motivoFallo = "El precio debe ser estrictamente mayor a 0.";
        return false;
    }

    Repuesto r;
    bool elim = false;
    if (!buscarRepuestoPorID(idRepuesto, r, elim)) {
        motivoFallo = elim ? "El repuesto fue dado de baja." : "El repuesto no existe.";
        return false;
    }

    r.precio = nuevoPrecio;
    if (!sobrescribirRepuesto(r)) {
        motivoFallo = "Error al actualizar el precio en repuestos.bin.";
        return false;
    }
    return true;
}

bool eliminarRepuesto(int id, string &motivoFallo) {
    Repuesto r;
    bool elim = false;
    if (!buscarRepuestoPorID(id, r, elim)) {
        motivoFallo = elim ? "El repuesto ya fue eliminado previamente." : "El repuesto no existe.";
        return false;
    }

    // Rechazado si el repuesto aparece en el detalle de alguna orden
    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) {
        motivoFallo = "No se pudo verificar ordenes.bin; no se elimino el repuesto.";
        return false;
    }
    for (int i = 1; i <= hOrd.cantidadRegistros; i++) {
        OrdenServicio o;
        if (!leerOrdenPorID(i, o)) {
            motivoFallo = "No se pudo leer la orden ID #" + to_string(i) +
                          "; no se elimino el repuesto.";
            return false;
        }
        if (!validarOrdenServicio(o)) {
            motivoFallo = "La orden #" + to_string(o.id) +
                          " contiene datos invalidos; ejecute la verificacion de integridad.";
            return false;
        }
        for (int j = 0; j < o.numDetalles; j++) {
            if (o.detalles[j].idRepuesto == id) {
                motivoFallo = "No se puede eliminar: el repuesto figura en el detalle de la Orden #" +
                              to_string(o.id) + " (Estado: " + o.estado + ").";
                return false;
            }
        }
    }

    if (!marcarRepuestoEliminado(id, motivoFallo)) {
        return false;
    }
    return true;
}

// ==============================
// MODULO DE ORDENES DE SERVICIO
// ==============================

bool abrirOrdenServicio(int idVehiculo, int idMecanico, const char* fechaApertura,
                       const char* descripcionFalla, int kilometrajeIngreso,
                       OrdenServicio &resultado, string &motivoFallo) {
    const char* finDescripcion = descripcionFalla == nullptr
        ? nullptr
        : static_cast<const char*>(memchr(descripcionFalla, '\0',
                                          sizeof(resultado.descripcionFalla)));
    if (!fechaValidaISO(fechaApertura)) {
        motivoFallo = "La fecha de apertura debe ser una fecha real con formato YYYY-MM-DD.";
        return false;
    }
    if (finDescripcion == nullptr || finDescripcion == descripcionFalla) {
        motivoFallo = "La descripcion de la falla esta vacia o excede el limite permitido.";
        return false;
    }

    // Validaciones previas completas antes de escribir
    Vehiculo v;
    bool elimV = false;
    long posV = 0;
    if (!buscarVehiculoPorID(idVehiculo, v, elimV, posV)) {
        if (elimV) {
            motivoFallo = "El vehiculo ingresado esta eliminado.";
        } else {
            motivoFallo = "El vehiculo con ID #" + to_string(idVehiculo) + " no existe.";
        }
        return false;
    }

    Mecanico m;
    bool elimM = false;
    if (!buscarMecanicoPorID(idMecanico, m, elimM)) {
        if (elimM) {
            motivoFallo = "El mecanico esta dado de baja y no puede recibir nuevas ordenes.";
        } else {
            motivoFallo = "El mecanico con ID #" + to_string(idMecanico) + " no existe.";
        }
        return false;
    }

    if (kilometrajeIngreso < v.kilometraje) {
        motivoFallo = "El kilometraje de ingreso (" + to_string(kilometrajeIngreso) + " km) no puede ser menor al kilometraje actual del vehiculo (" + to_string(v.kilometraje) + " km).";
        return false;
    }

    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) {
        motivoFallo = "Error al leer el archivo de ordenes.";
        return false;
    }

    int abiertasMecanico = 0;
    for (int i = 1; i <= hOrd.cantidadRegistros; i++) {
        OrdenServicio o;
        if (!leerOrdenPorID(i, o)) {
            motivoFallo = "No se pudo leer la orden ID #" + to_string(i) +
                          "; no se pudo validar la apertura.";
            return false;
        }
        if (!validarOrdenServicio(o)) {
            motivoFallo = "La orden ID #" + to_string(i) +
                          " contiene datos invalidos; no se pudo validar la apertura.";
            return false;
        }
        if (!o.eliminado && strcmp(o.estado, "ABIERTA") == 0) {
            if (o.idVehiculo == idVehiculo) {
                motivoFallo = "El vehiculo ya tiene la orden #" + to_string(o.id) + " en estado ABIERTA.";
                return false;
            }
            if (o.idMecanico == idMecanico) {
                abiertasMecanico++;
            }
        }
    }

    if (abiertasMecanico >= 3) {
        motivoFallo = "El mecanico " + string(m.nombre) + " ya tiene 3 ordenes ABIERTAS asignadas (limite maximo alcanzado).";
        return false;
    }

    // Actualizar primero el kilometraje; si falla la creación de la orden se restaura.
    Vehiculo vehiculoOriginal = v;
    v.kilometraje = kilometrajeIngreso;
    if (!sobrescribirVehiculo(v)) {
        if (!sobrescribirVehiculo(vehiculoOriginal)) {
            motivoFallo = "Fallo al actualizar y restaurar el kilometraje. No se creo la orden; verifique la integridad.";
        } else {
            motivoFallo = "No se pudo actualizar el kilometraje; no se creo la orden.";
        }
        return false;
    }

    resultado = {};
    resultado.idVehiculo = idVehiculo;
    resultado.idMecanico = idMecanico;
    strcpy(resultado.fechaApertura, fechaApertura);
    strcpy(resultado.fechaCierre, "");
    strcpy(resultado.estado, "ABIERTA");
    strcpy(resultado.descripcionFalla, descripcionFalla);
    resultado.kilometrajeIngreso = kilometrajeIngreso;
    resultado.manoObra = 0.0f;
    resultado.numDetalles = 0;

    string motivoPersistencia;
    if (!guardarOrdenCruda(resultado, motivoPersistencia)) {
        if (!sobrescribirVehiculo(vehiculoOriginal)) {
            motivoFallo = "Fallo al crear la orden (" + motivoPersistencia +
                          ") y al restaurar el kilometraje. Verifique la integridad del sistema.";
        } else {
            motivoFallo = "Fallo al crear la orden (" + motivoPersistencia +
                          "); el kilometraje fue restaurado.";
        }
        return false;
    }
    return true;
}

bool agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad, string &motivoFallo) {
    if (cantidad <= 0) {
        motivoFallo = "La cantidad debe ser mayor a 0.";
        return false;
    }

    OrdenServicio o;
    if (!leerOrdenPorID(idOrden, o)) {
        motivoFallo = "La orden #" + to_string(idOrden) + " no existe.";
        return false;
    }
    if (!validarOrdenServicio(o)) {
        motivoFallo = "La orden contiene datos invalidos; ejecute la verificacion de integridad.";
        return false;
    }
    if (strcmp(o.estado, "ABIERTA") != 0) {
        motivoFallo = "La orden se encuentra " + string(o.estado) + ". Solo se pueden agregar repuestos a ordenes ABIERTAS.";
        return false;
    }
    if (!detalleDentroDelLimite(o)) {
        motivoFallo = "La orden tiene un numero de lineas de detalle invalido; ejecute la verificacion de integridad.";
        return false;
    }

    Repuesto r;
    bool elimR = false;
    if (!buscarRepuestoPorID(idRepuesto, r, elimR)) {
        motivoFallo = elimR ? "El repuesto seleccionado esta dado de baja." : "El repuesto no existe.";
        return false;
    }
    if (r.stock < cantidad) {
        motivoFallo = "Stock insuficiente en inventario. Stock disponible: " + to_string(r.stock) + " unidades.";
        return false;
    }

    int idxExistente = -1;
    for (int i = 0; i < o.numDetalles; i++) {
        if (o.detalles[i].idRepuesto == idRepuesto) {
            idxExistente = i;
            if (cantidad > numeric_limits<int>::max() - o.detalles[i].cantidad) {
                motivoFallo = "La cantidad acumulada excede el limite numerico permitido.";
                return false;
            }
            break;
        }
    }

    if (idxExistente == -1 && o.numDetalles >= MAX_LINEAS_DETALLE) {
        motivoFallo = "Limite de " + to_string(MAX_LINEAS_DETALLE) + " lineas de detalle alcanzado para esta orden.";
        return false;
    }

    Repuesto repuestoOriginal = r;
    r.stock -= cantidad;
    if (!sobrescribirRepuesto(r)) {
        if (!sobrescribirRepuesto(repuestoOriginal)) {
            motivoFallo = "Fallo al descontar el stock y al restaurar repuestos.bin. Verifique la integridad.";
        } else {
            motivoFallo = "Error al descontar el stock en repuestos.bin; no se modifico la orden.";
        }
        return false;
    }

    OrdenServicio ordenOriginal = o;
    if (idxExistente != -1) {
        o.detalles[idxExistente].cantidad += cantidad;
    } else {
        o.detalles[o.numDetalles].idRepuesto = idRepuesto;
        o.detalles[o.numDetalles].cantidad = cantidad;
        o.detalles[o.numDetalles].precioUnitario = r.precio; // Precio historico vigente
        o.numDetalles++;
    }

    if (!sobrescribirOrden(o)) {
        bool stockRestaurado = sobrescribirRepuesto(repuestoOriginal);
        bool ordenRestaurada = sobrescribirOrden(ordenOriginal);
        if (!stockRestaurado || !ordenRestaurada) {
            motivoFallo = "Fallo al guardar la orden y al compensar una o mas escrituras. Verifique la integridad.";
        } else {
            motivoFallo = "Fallo al guardar la orden; el stock fue restaurado.";
        }
        return false;
    }

    return true;
}

bool quitarRepuestoDeOrden(int idOrden, int idRepuesto, string &motivoFallo) {
    OrdenServicio o;
    if (!leerOrdenPorID(idOrden, o)) {
        motivoFallo = "La orden #" + to_string(idOrden) + " no existe.";
        return false;
    }
    if (!validarOrdenServicio(o)) {
        motivoFallo = "La orden contiene datos invalidos; ejecute la verificacion de integridad.";
        return false;
    }
    if (strcmp(o.estado, "ABIERTA") != 0) {
        motivoFallo = "La orden se encuentra " + string(o.estado) + ". Solo se pueden modificar ordenes ABIERTAS.";
        return false;
    }
    if (!detalleDentroDelLimite(o)) {
        motivoFallo = "La orden tiene un numero de lineas de detalle invalido; ejecute la verificacion de integridad.";
        return false;
    }

    int idx = -1;
    for (int i = 0; i < o.numDetalles; i++) {
        if (o.detalles[i].idRepuesto == idRepuesto) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        motivoFallo = "El repuesto ID #" + to_string(idRepuesto) + " no se encuentra en el detalle de esta orden.";
        return false;
    }

    Repuesto r;
    if (!leerRepuestoPorID(idRepuesto, r) || r.eliminado) {
        motivoFallo = "No se pudo leer un repuesto activo para devolver las unidades al inventario.";
        return false;
    }
    if (r.stock < 0 || o.detalles[idx].cantidad <= 0 ||
        o.detalles[idx].cantidad > numeric_limits<int>::max() - r.stock) {
        motivoFallo = "La cantidad a devolver produciria un stock invalido.";
        return false;
    }

    Repuesto repuestoOriginal = r;
    r.stock += o.detalles[idx].cantidad;
    if (!sobrescribirRepuesto(r)) {
        if (!sobrescribirRepuesto(repuestoOriginal)) {
            motivoFallo = "Fallo al devolver el stock y al restaurar repuestos.bin. Verifique la integridad.";
        } else {
            motivoFallo = "No se pudo devolver el stock; la orden no fue modificada.";
        }
        return false;
    }

    OrdenServicio ordenOriginal = o;
    // Eliminar la línea desplazando los detalles
    for (int i = idx; i < o.numDetalles - 1; i++) {
        o.detalles[i] = o.detalles[i + 1];
    }
    o.numDetalles--;

    if (!sobrescribirOrden(o)) {
        bool stockRestaurado = sobrescribirRepuesto(repuestoOriginal);
        bool ordenRestaurada = sobrescribirOrden(ordenOriginal);
        if (!stockRestaurado || !ordenRestaurada) {
            motivoFallo = "Fallo al quitar la linea y al compensar una o mas escrituras. Verifique la integridad.";
        } else {
            motivoFallo = "Fallo al guardar la orden; el stock fue restaurado.";
        }
        return false;
    }
    return true;
}

bool cerrarOrdenServicio(int idOrden, float manoObra, string &motivoFallo) {
    OrdenServicio o;
    if (!leerOrdenPorID(idOrden, o)) {
        motivoFallo = "La orden #" + to_string(idOrden) + " no existe.";
        return false;
    }
    if (!validarOrdenServicio(o)) {
        motivoFallo = "La orden contiene datos invalidos; ejecute la verificacion de integridad.";
        return false;
    }
    if (strcmp(o.estado, "ABIERTA") != 0) {
        motivoFallo = "La orden ya se encuentra " + string(o.estado) + ". Solo se pueden cerrar ordenes ABIERTAS.";
        return false;
    }
    if (!isfinite(manoObra) || manoObra < 0) {
        motivoFallo = "El costo de mano de obra debe ser finito y mayor o igual a cero.";
        return false;
    }
    if (!detalleDentroDelLimite(o)) {
        motivoFallo = "La orden tiene un numero de lineas de detalle invalido; ejecute la verificacion de integridad.";
        return false;
    }
    if (o.numDetalles == 0 && manoObra <= 0) {
        motivoFallo = "Una orden no puede cerrarse vacia: debe incluir al menos un repuesto o mano de obra > 0.";
        return false;
    }

    time_t ahora = time(nullptr);
    tm* fechaLocal = localtime(&ahora);
    if (fechaLocal == nullptr || strftime(o.fechaCierre, sizeof(o.fechaCierre), "%Y-%m-%d", fechaLocal) == 0) {
        motivoFallo = "No se pudo obtener la fecha actual para cerrar la orden.";
        return false;
    }
    o.manoObra = manoObra;
    strcpy(o.estado, "CERRADA");

    if (!sobrescribirOrden(o)) {
        motivoFallo = "Error al actualizar la orden en disco.";
        return false;
    }
    return true;
}

bool anularOrdenServicio(int idOrden, string &motivoFallo) {
    OrdenServicio o;
    if (!leerOrdenPorID(idOrden, o)) {
        motivoFallo = "La orden #" + to_string(idOrden) + " no existe.";
        return false;
    }
    if (!validarOrdenServicio(o)) {
        motivoFallo = "La orden contiene datos invalidos; ejecute la verificacion de integridad.";
        return false;
    }
    if (strcmp(o.estado, "ANULADA") == 0) {
        motivoFallo = "La orden ya se encuentra ANULADA.";
        return false;
    }
    if (strcmp(o.estado, "ABIERTA") != 0 && strcmp(o.estado, "CERRADA") != 0) {
        motivoFallo = "Estado de orden invalido para anulacion.";
        return false;
    }
    if (!detalleDentroDelLimite(o)) {
        motivoFallo = "La orden tiene un numero de lineas de detalle invalido; ejecute la verificacion de integridad.";
        return false;
    }

    vector<int> idsRepuestos;
    vector<int> cantidades;
    for (int i = 0; i < o.numDetalles; i++) {
        const DetalleOrden &detalle = o.detalles[i];
        if (detalle.idRepuesto < 1 || detalle.cantidad <= 0) {
            motivoFallo = "La orden contiene una linea de detalle invalida; no se modifico el inventario.";
            return false;
        }
        size_t indice = 0;
        while (indice < idsRepuestos.size() && idsRepuestos[indice] != detalle.idRepuesto) {
            indice++;
        }
        if (indice == idsRepuestos.size()) {
            idsRepuestos.push_back(detalle.idRepuesto);
            cantidades.push_back(detalle.cantidad);
        } else {
            if (detalle.cantidad > numeric_limits<int>::max() - cantidades[indice]) {
                motivoFallo = "La cantidad total a devolver excede el limite numerico permitido.";
                return false;
            }
            cantidades[indice] += detalle.cantidad;
        }
    }

    vector<Repuesto> originales;
    vector<Repuesto> actualizados;
    for (size_t i = 0; i < idsRepuestos.size(); i++) {
        Repuesto r;
        bool eliminado = false;
        if (!buscarRepuestoPorID(idsRepuestos[i], r, eliminado)) {
            motivoFallo = eliminado
                ? "Un repuesto de la orden esta dado de baja; no se modifico el inventario."
                : "No se pudo leer un repuesto de la orden; no se modifico el inventario.";
            return false;
        }
        if (r.stock < 0 || cantidades[i] > numeric_limits<int>::max() - r.stock) {
            motivoFallo = "La devolucion produciria un stock fuera de rango.";
            return false;
        }
        originales.push_back(r);
        r.stock += cantidades[i];
        actualizados.push_back(r);
    }

    size_t repuestosEscritos = 0;
    for (size_t i = 0; i < actualizados.size(); i++) {
        if (!sobrescribirRepuesto(actualizados[i])) {
            repuestosEscritos = i + 1;
            bool compensacionCorrecta = true;
            for (size_t j = 0; j < repuestosEscritos; j++) {
                if (!sobrescribirRepuesto(originales[j])) compensacionCorrecta = false;
            }
            motivoFallo = compensacionCorrecta
                ? "No se pudo actualizar un repuesto; los cambios anteriores fueron revertidos."
                : "Fallo al actualizar y revertir el inventario. Verifique la integridad del sistema.";
            return false;
        }
        repuestosEscritos = i + 1;
    }

    OrdenServicio ordenOriginal = o;
    strcpy(o.estado, "ANULADA");
    if (!sobrescribirOrden(o)) {
        bool compensacionCorrecta = true;
        for (size_t i = 0; i < repuestosEscritos; i++) {
            if (!sobrescribirRepuesto(originales[i])) compensacionCorrecta = false;
        }
        if (!sobrescribirOrden(ordenOriginal)) compensacionCorrecta = false;
        motivoFallo = compensacionCorrecta
            ? "No se pudo anular la orden; las devoluciones de stock fueron revertidas."
            : "Fallo al anular y compensar las escrituras. Verifique la integridad del sistema.";
        return false;
    }
    return true;
}

bool validarDetalleOrden(const OrdenServicio &o) {
    if (o.numDetalles < 0 || o.numDetalles > MAX_LINEAS_DETALLE) return false;
    for (int i = 0; i < o.numDetalles; i++) {
        const DetalleOrden &detalle = o.detalles[i];
        if (detalle.idRepuesto < 1 || detalle.cantidad <= 0 ||
            !isfinite(detalle.precioUnitario) || detalle.precioUnitario <= 0.0f) {
            return false;
        }
    }
    return true;
}

bool validarOrdenServicio(const OrdenServicio &o) {
    return camposOrdenTerminados(o) &&
           fechaValidaISO(o.fechaApertura) &&
           (o.fechaCierre[0] == '\0' || fechaValidaISO(o.fechaCierre)) &&
           estadoOrdenValido(o.estado) &&
           isfinite(o.manoObra) && o.manoObra >= 0.0f &&
           validarDetalleOrden(o);
}

bool calcularTotalOrden(const OrdenServicio &o, float &total) {
    if (!validarOrdenServicio(o)) return false;

    double suma = o.manoObra;
    for (int i = 0; i < o.numDetalles; i++) {
        suma += static_cast<double>(o.detalles[i].cantidad) *
                static_cast<double>(o.detalles[i].precioUnitario);
        if (!isfinite(suma) || suma > numeric_limits<float>::max()) return false;
    }
    total = static_cast<float>(suma);
    return true;
}

bool buscarOrdenPorID(int id, OrdenServicio &resultado, long *posicionBytes) {
    if (!leerOrdenPorID(id, resultado, posicionBytes)) {
        return false;
    }
    return !resultado.eliminado;
}

bool listarOrdenesPorEstado(const char* estado, OrdenServicio resultados[], int &cantidad,
                            bool &hayMas, string &motivoFallo) {
    cantidad = 0;
    hayMas = false;
    motivoFallo.clear();

    if (estado == nullptr ||
        (strcmp(estado, "ABIERTA") != 0 && strcmp(estado, "CERRADA") != 0 &&
         strcmp(estado, "ANULADA") != 0)) {
        motivoFallo = "El estado solicitado no es valido.";
        return false;
    }

    ArchivoHeader h;
    if (!leerHeader("datos/ordenes.bin", h)) {
        motivoFallo = "No se pudo leer el header de ordenes.bin.";
        return false;
    }

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        OrdenServicio o;
        if (!leerOrdenPorID(i, o)) {
            motivoFallo = "No se pudo leer la orden ID #" + to_string(i) + ".";
            return false;
        }
        if (!validarOrdenServicio(o)) {
            motivoFallo = "La orden #" + to_string(o.id) + " contiene datos invalidos.";
            return false;
        }
        if (!o.eliminado && strcmp(o.estado, estado) == 0) {
            if (cantidad < MAX_RESULTADOS) {
                resultados[cantidad++] = o;
            } else {
                hayMas = true;
                break;
            }
        }
    }
    return cantidad > 0;
}

// ===================================
// MÓDULO DE REPORTES Y MANTENIMIENTO
// ===================================

bool generarReporteHistorialVehiculo(const char* placa, Vehiculo &vEncontrado,
                                    int desdeOrden, OrdenServicio ordenesVehiculo[],
                                    int &cantOrdenes, bool &hayMas, float &totalFacturado,
                                    string &motivoFallo) {
    motivoFallo.clear();
    if (!buscarVehiculoPorPlaca(placa, vEncontrado, motivoFallo)) {
        return false;
    }
    if (desdeOrden < 0) {
        motivoFallo = "El indice inicial del historial no puede ser negativo.";
        return false;
    }

    cantOrdenes = 0;
    hayMas = false;
    totalFacturado = 0.0f;

    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) {
        motivoFallo = "No se pudo leer el header de ordenes.bin.";
        return false;
    }

    int coincidencias = 0;
    for (int i = 1; i <= hOrd.cantidadRegistros; i++) {
        OrdenServicio o;
        if (!leerOrdenPorID(i, o)) {
            motivoFallo = "No se pudo leer la orden ID #" + to_string(i) + ".";
            return false;
        }
        if (!o.eliminado && o.idVehiculo == vEncontrado.id) {
            if (!validarOrdenServicio(o)) {
                motivoFallo = "La orden #" + to_string(o.id) + " contiene datos invalidos.";
                return false;
            }
            if (coincidencias >= desdeOrden && cantOrdenes < 100) {
                ordenesVehiculo[cantOrdenes++] = o;
            } else if (coincidencias - desdeOrden >= 100) {
                hayMas = true;
            }
            coincidencias++;
            if (strcmp(o.estado, "CERRADA") == 0) {
                float totalOrden = 0.0f;
                if (!calcularTotalOrden(o, totalOrden) ||
                    !isfinite(totalFacturado + totalOrden)) {
                    motivoFallo = "El total facturado contiene un valor invalido o excede el limite numerico.";
                    return false;
                }
                totalFacturado += totalOrden;
            }
        }
    }
    return true;
}

bool generarReporteTopRepuestos(TopRepuesto top10[], int &cantEncontrados,
                                string &motivoFallo) {
    cantEncontrados = 0;
    motivoFallo.clear();
    for (int i = 0; i < 10; i++) {
        top10[i].unidades = 0;
        top10[i].codigo[0] = '\0';
        top10[i].nombre[0] = '\0';
    }

    ArchivoHeader hRep;
    if (!leerHeader("datos/repuestos.bin", hRep)) {
        motivoFallo = "No se pudo leer el header de repuestos.bin.";
        return false;
    }

    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) {
        motivoFallo = "No se pudo leer el header de ordenes.bin.";
        return false;
    }

    for (int i = 1; i <= hRep.cantidadRegistros; i++) {
        Repuesto r;
        if (!leerRepuestoPorID(i, r)) {
            motivoFallo = "No se pudo leer el repuesto ID #" + to_string(i) + ".";
            return false;
        }

        int consumos = 0;
        for (int j = 1; j <= hOrd.cantidadRegistros; j++) {
            OrdenServicio o;
            if (!leerOrdenPorID(j, o)) {
                motivoFallo = "No se pudo leer la orden ID #" + to_string(j) + ".";
                return false;
            }
            if (!validarOrdenServicio(o)) {
                motivoFallo = "La orden #" + to_string(o.id) + " contiene datos invalidos.";
                return false;
            }
            if (!o.eliminado && strcmp(o.estado, "CERRADA") == 0) {
                for (int k = 0; k < o.numDetalles; k++) {
                    if (o.detalles[k].idRepuesto == r.id) {
                        if (o.detalles[k].cantidad > INT_MAX - consumos) {
                            motivoFallo = "El total de unidades consumidas excede el limite numerico.";
                            return false;
                        }
                        consumos += o.detalles[k].cantidad;
                    }
                }
            }
        }

        if (consumos > 0) {
            TopRepuesto nuevo;
            strcpy(nuevo.codigo, r.codigo);
            strcpy(nuevo.nombre, r.nombre);
            nuevo.unidades = consumos;

            for (int k = 0; k < 10; k++) {
                if (nuevo.unidades > top10[k].unidades) {
                    for (int m = 9; m > k; m--) {
                        top10[m] = top10[m - 1];
                    }
                    top10[k] = nuevo;
                    if (cantEncontrados < 10) cantEncontrados++;
                    break;
                }
            }
        }
    }
    return cantEncontrados > 0;
}

bool generarReporteFacturacionMecanicos(FilaFacturacionMecanico filas[], int &cantFilas,
                                       bool &hayMas, string &motivoFallo) {
    cantFilas = 0;
    hayMas = false;
    motivoFallo.clear();

    ArchivoHeader hMec;
    if (!leerHeader("datos/mecanicos.bin", hMec)) {
        motivoFallo = "No se pudo leer el header de mecanicos.bin.";
        return false;
    }

    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) {
        motivoFallo = "No se pudo leer el header de ordenes.bin.";
        return false;
    }

    for (int i = 1; i <= hMec.cantidadRegistros; i++) {
        Mecanico m;
        if (!leerMecanicoPorID(i, m)) {
            motivoFallo = "No se pudo leer el mecanico ID #" + to_string(i) + ".";
            return false;
        }
        if (!camposMecanicoTerminados(m)) {
            motivoFallo = "El mecanico ID #" + to_string(i) +
                          " contiene una cadena sin terminador.";
            return false;
        }

        FilaFacturacionMecanico fila;
        strcpy(fila.nombre, m.nombre);
        fila.dadoDeBaja = m.eliminado;
        fila.ordenesCerradas = 0;
        fila.totalManoObra = 0.0f;
        fila.totalRepuestos = 0.0f;

        for (int j = 1; j <= hOrd.cantidadRegistros; j++) {
            OrdenServicio o;
            if (!leerOrdenPorID(j, o)) {
                motivoFallo = "No se pudo leer la orden ID #" + to_string(j) + ".";
                return false;
            }
            if (!validarOrdenServicio(o)) {
                motivoFallo = "La orden #" + to_string(o.id) + " contiene datos invalidos.";
                return false;
            }
            if (!o.eliminado && o.idMecanico == m.id && strcmp(o.estado, "CERRADA") == 0) {
                float totalOrden = 0.0f;
                if (!calcularTotalOrden(o, totalOrden)) {
                    motivoFallo = "La orden #" + to_string(o.id) + " contiene importes invalidos.";
                    return false;
                }
                fila.ordenesCerradas++;
                fila.totalManoObra += o.manoObra;
                for (int d = 0; d < o.numDetalles; d++) {
                    fila.totalRepuestos += (o.detalles[d].cantidad * o.detalles[d].precioUnitario);
                }
                if (!isfinite(fila.totalManoObra) || !isfinite(fila.totalRepuestos)) {
                    motivoFallo = "La facturacion del mecanico excede el limite numerico.";
                    return false;
                }
            }
        }

        if (cantFilas < MAX_RESULTADOS) {
            filas[cantFilas++] = fila;
        } else {
            hayMas = true;
            break;
        }
    }
    return cantFilas > 0;
}

bool crearBackup(string &rutaGenerada, string &motivoFallo) {
    return crearBackupArchivos(rutaGenerada, motivoFallo);
}

void verificarIntegridadSistema(vector<string> &errores, int contadores[]) {

    // Prepara los resultados
    errores.clear();
    for (int i = 0; i < 4; i++) contadores[i] = 0;

    // Comprueba datos del taller
    Taller taller = {};
    bool tallerNoExiste = false;
    if (!cargarTallerArchivo(taller, tallerNoExiste)) {
        errores.push_back(tallerNoExiste
            ? "ROTO: Falta datos/taller.bin."
            : "ROTO: datos/taller.bin no contiene un registro Taller completo.");
    } else if (!cadenaTerminada(taller.nombre) || !cadenaTerminada(taller.rif) ||
               !cadenaTerminada(taller.direccion) || !cadenaTerminada(taller.telefono)) {
        errores.push_back("ROTO: datos/taller.bin contiene una cadena sin terminador.");
    }

    // Declara un header y un conteo fisico para cada entidad
    ArchivoHeader hVeh = {};
    ArchivoHeader hMec = {};
    ArchivoHeader hRep = {};
    ArchivoHeader hOrd = {};
    int realesVeh = 0, realesMec = 0, realesRep = 0, realesOrd = 0;

    // Define lambda local "inspeccionar"
    // El [&] significa que puede acceder por referencia a variables del ambio exterior como errores
    auto inspeccionar = [&](const char* ruta, size_t tamRegistro, ArchivoHeader &h, int &reales) {

        //Para cada archivo:
        // Lee el header. Si falla, agrega un error y devuelve false
        // Inspecciona los bytes para contar registros completos y detectar un registro parcial, si la inspección falla, agrega el motivo y devuelve false
        // Compara los registros fisicos con h.cantidadRegistros
        // Verifica los valores del header: total no negativo, proximoID igual al total más uno, activos entre cero y el total, y versión 1
        // Si pudo leer e inspeccionar el archivo, devuelve true, aunque haya encontrado inconsistencias de conteo o metadata
        string motivoFallo;
        bool registroIncompleto = false;
        if (!leerHeader(ruta, h)) {
            errores.push_back("ROTO: No se pudo leer el header de " + string(ruta));
            return false;
        }
        if (!inspeccionarArchivoEntidad(ruta, tamRegistro, reales, registroIncompleto, motivoFallo)) {
            errores.push_back("ROTO: " + motivoFallo);
            return false;
        }
        if (registroIncompleto) {
            errores.push_back("ROTO: " + string(ruta) + " termina con un registro incompleto");
        }
        if (h.cantidadRegistros != reales) {
            errores.push_back("ROTO: Header de " + string(ruta) + " indica " +
                              to_string(h.cantidadRegistros) + " registros; el archivo contiene " +
                              to_string(reales));
        }
        if (h.cantidadRegistros < 0 ||
            static_cast<long long>(h.proximoID) != static_cast<long long>(h.cantidadRegistros) + 1 ||
            h.registrosActivos < 0 || h.registrosActivos > h.cantidadRegistros ||
            h.version != 1) {
            errores.push_back("ROTO: Metadata invalida en el header de " + string(ruta));
        }
        return true;
    };

    // Las cuatro llamadas ejecutan esa comprobacion para las 4 entidades
    bool vehiculosDisponibles = inspeccionar("datos/vehiculos.bin", sizeof(Vehiculo), hVeh, realesVeh);
    bool mecanicosDisponibles = inspeccionar("datos/mecanicos.bin", sizeof(Mecanico), hMec, realesMec);
    bool repuestosDisponibles = inspeccionar("datos/repuestos.bin", sizeof(Repuesto), hRep, realesRep);
    bool ordenesDisponibles = inspeccionar("datos/ordenes.bin", sizeof(OrdenServicio), hOrd, realesOrd);
    contadores[0] = realesVeh;
    contadores[1] = realesMec;
    contadores[2] = realesRep;
    contadores[3] = realesOrd;

    int activosVeh = 0, activosMec = 0, activosRep = 0, activosOrd = 0;
    if (vehiculosDisponibles) {
        int limite = hVeh.cantidadRegistros < realesVeh ? hVeh.cantidadRegistros : realesVeh;
        for (int i = 1; i <= limite; i++) {
            Vehiculo v = {};
            if (!leerVehiculoPorID(i, v)) {
                errores.push_back("ROTO: Fallo al leer el vehiculo ID " + to_string(i));
                continue;
            }
            if (!v.eliminado) activosVeh++;
            if (v.id != i) {
                errores.push_back("ROTO: El registro fisico de vehiculo " + to_string(i) +
                                  " declara el ID " + to_string(v.id));
            }
            if (!cadenaTerminada(v.placa) || !cadenaTerminada(v.marca) ||
                !cadenaTerminada(v.modelo) || !cadenaTerminada(v.nombrePropietario) ||
                !cadenaTerminada(v.cedulaPropietario) ||
                !cadenaTerminada(v.telefonoPropietario)) {
                errores.push_back("ROTO: El vehiculo ID " + to_string(i) +
                                  " contiene una cadena sin terminador.");
            }
        }
        if (hVeh.registrosActivos != activosVeh) {
            errores.push_back("ROTO: Header de vehiculos.bin indica " + to_string(hVeh.registrosActivos) +
                              " activos; se contaron " + to_string(activosVeh));
        }
    }

    if (mecanicosDisponibles) {
        int limite = hMec.cantidadRegistros < realesMec ? hMec.cantidadRegistros : realesMec;
        for (int i = 1; i <= limite; i++) {
            Mecanico m = {};
            if (!leerMecanicoPorID(i, m)) {
                errores.push_back("ROTO: Fallo al leer el mecanico ID " + to_string(i));
                continue;
            }
            if (!m.eliminado) activosMec++;
            if (m.id != i) {
                errores.push_back("ROTO: El registro fisico de mecanico " + to_string(i) +
                                  " declara el ID " + to_string(m.id));
            }
            if (!cadenaTerminada(m.nombre) || !cadenaTerminada(m.cedula) ||
                !cadenaTerminada(m.especialidad) || !cadenaTerminada(m.fechaIngreso) ||
                !fechaValidaISO(m.fechaIngreso)) {
                errores.push_back("ROTO: El mecanico ID " + to_string(i) +
                                  " contiene cadenas o fecha de ingreso invalidas.");
            }
        }
        if (hMec.registrosActivos != activosMec) {
            errores.push_back("ROTO: Header de mecanicos.bin indica " + to_string(hMec.registrosActivos) +
                              " activos; se contaron " + to_string(activosMec));
        }
    }

    if (repuestosDisponibles) {
        int limite = hRep.cantidadRegistros < realesRep ? hRep.cantidadRegistros : realesRep;
        for (int i = 1; i <= limite; i++) {
            Repuesto r = {};
            if (!leerRepuestoPorID(i, r)) {
                errores.push_back("ROTO: Fallo al leer el repuesto ID " + to_string(i));
                continue;
            }
            if (!r.eliminado) activosRep++;
            if (r.id != i) {
                errores.push_back("ROTO: El registro fisico de repuesto " + to_string(i) +
                                  " declara el ID " + to_string(r.id));
            }
            if (!cadenaTerminada(r.codigo) || !cadenaTerminada(r.nombre) ||
                !cadenaTerminada(r.marca)) {
                errores.push_back("ROTO: El repuesto ID " + to_string(i) +
                                  " contiene una cadena sin terminador.");
            }
            if (!isfinite(r.precio) || r.precio <= 0.0f || r.stockMinimo < 0) {
                errores.push_back("ROTO: El repuesto ID " + to_string(i) +
                                  " contiene precio o stock minimo invalido.");
            }
            if (r.stock < 0) {
                errores.push_back("ROTO: Repuesto ID " + to_string(r.id) +
                                  " tiene stock negativo (" + to_string(r.stock) + ")");
            }
        }
        if (hRep.registrosActivos != activosRep) {
            errores.push_back("ROTO: Header de repuestos.bin indica " + to_string(hRep.registrosActivos) +
                              " activos; se contaron " + to_string(activosRep));
        }
    }

    if (ordenesDisponibles) {
        int limite = hOrd.cantidadRegistros < realesOrd ? hOrd.cantidadRegistros : realesOrd;
        for (int i = 1; i <= limite; i++) {
            OrdenServicio o = {};
            if (!leerOrdenPorID(i, o)) {
                errores.push_back("ROTO: Fallo al leer la orden ID " + to_string(i));
                continue;
            }
            if (!o.eliminado) activosOrd++;
            else errores.push_back("ROTO: La orden #" + to_string(o.id) + " esta marcada como eliminada");
            if (o.id != i) {
                errores.push_back("ROTO: El registro fisico de orden " + to_string(i) +
                                  " declara el ID " + to_string(o.id));
            }
            if (!camposOrdenTerminados(o) || !fechaValidaISO(o.fechaApertura) ||
                (o.fechaCierre[0] != '\0' && !fechaValidaISO(o.fechaCierre))) {
                errores.push_back("ROTO: La orden #" + to_string(i) +
                                  " contiene cadenas o fechas invalidas.");
                continue;
            }
            if (!estadoOrdenValido(o.estado)) {
                errores.push_back("ROTO: La orden #" + to_string(i) + " tiene un estado invalido.");
                continue;
            }
            if (!isfinite(o.manoObra) || o.manoObra < 0.0f) {
                errores.push_back("ROTO: La orden #" + to_string(i) +
                                  " contiene un importe de mano de obra invalido.");
            }

            Vehiculo v;
            long offsetV = 0;
            if (!vehiculosDisponibles || o.idVehiculo < 1 || o.idVehiculo > realesVeh ||
                !leerVehiculoPorID(o.idVehiculo, v, &offsetV)) {
                errores.push_back("ROTO: Orden #" + to_string(o.id) + " apunta al vehiculo ID " +
                                  to_string(o.idVehiculo) + " (no existe o no se pudo leer en vehiculos.bin)");
            } else if (v.eliminado) {
                if (strcmp(o.estado, "ABIERTA") == 0) {
                    errores.push_back("ROTO: Orden #" + to_string(o.id) + " esta ABIERTA pero su vehiculo ID " +
                                      to_string(o.idVehiculo) + " no esta activo");
                }
            }
            Mecanico m;
            if (!mecanicosDisponibles || o.idMecanico < 1 || o.idMecanico > realesMec ||
                !leerMecanicoPorID(o.idMecanico, m)) {
                errores.push_back("ROTO: Orden #" + to_string(o.id) + " apunta al mecanico ID " +
                                  to_string(o.idMecanico) + " (no existe o no se pudo leer en mecanicos.bin)");
            } else if (strcmp(o.estado, "ABIERTA") == 0 && m.eliminado) {
                errores.push_back("ROTO: Orden #" + to_string(o.id) + " esta ABIERTA pero su mecanico ID " +
                                  to_string(o.idMecanico) + " no esta activo");
            }

            if (!detalleDentroDelLimite(o)) {
                errores.push_back("ROTO: Orden #" + to_string(o.id) +
                                  " tiene un conteo o contenido de detalle invalido.");
                continue;
            }
            for (int j = 0; j < o.numDetalles; j++) {
                int idR = o.detalles[j].idRepuesto;
                Repuesto r;
                if (!repuestosDisponibles || idR < 1 || idR > realesRep || !leerRepuestoPorID(idR, r)) {
                    errores.push_back("ROTO: Orden #" + to_string(o.id) + " contiene en su detalle el repuesto ID " +
                                      to_string(idR) + " (no existe o no se pudo leer en repuestos.bin)");
                } else if (r.eliminado) {
                    errores.push_back("ROTO: Orden #" + to_string(o.id) +
                                      " referencia un repuesto dado de baja.");
                }
            }
        }
        if (hOrd.registrosActivos != activosOrd) {
            errores.push_back("ROTO: Header de ordenes.bin indica " + to_string(hOrd.registrosActivos) +
                              " activos; se contaron " + to_string(activosOrd));
        }
    }
}
