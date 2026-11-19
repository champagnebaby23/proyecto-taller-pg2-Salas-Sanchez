#include "persistencia.h"
#include <fstream>
#include <ctime>
#include <cerrno>
#include <string>
#include <limits>
#include <climits>
#include <cstdio>

#ifdef _WIN32
    #include <io.h>
    #include <direct.h>
    #ifndef F_OK
        #define F_OK 0
    #endif
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
#endif

using namespace std;

// Rutas fijas según especificación (Sección 1)
static const char* RUTA_TALLER     = "datos/taller.bin";
static const char* RUTA_VEHICULOS  = "datos/vehiculos.bin";
static const char* RUTA_MECANICOS  = "datos/mecanicos.bin";
static const char* RUTA_REPUESTOS  = "datos/repuestos.bin";
static const char* RUTA_ORDENES    = "datos/ordenes.bin";

static bool archivoExiste(const char* ruta) {
    #ifdef _WIN32
        return _access(ruta, F_OK) == 0;
    #else
        return access(ruta, F_OK) == 0;
    #endif
}

static bool headerValido(const ArchivoHeader &h) {
    return h.cantidadRegistros >= 0 &&
           static_cast<long long>(h.proximoID) ==
               static_cast<long long>(h.cantidadRegistros) + 1 &&
           h.registrosActivos >= 0 &&
           h.registrosActivos <= h.cantidadRegistros &&
           h.version == 1;
}

static bool reemplazarPorPrefijo(const char* ruta, streamoff tamanoOriginal,
                                 const ArchivoHeader &headerOriginal) {
    if (tamanoOriginal < static_cast<streamoff>(sizeof(ArchivoHeader))) return false;

    const string temporal = string(ruta) + ".rollback.tmp";
    const string respaldo = string(ruta) + ".rollback.bak";
    if (archivoExiste(temporal.c_str()) || archivoExiste(respaldo.c_str())) return false;

    ifstream origen(ruta, ios::binary);
    ofstream destino(temporal.c_str(), ios::binary | ios::trunc);
    if (!origen.is_open() || !destino.is_open()) {
        destino.close();
        remove(temporal.c_str());
        return false;
    }

    destino.write(reinterpret_cast<const char*>(&headerOriginal), sizeof(ArchivoHeader));
    origen.seekg(static_cast<streamoff>(sizeof(ArchivoHeader)), ios::beg);
    streamoff restantes = tamanoOriginal - static_cast<streamoff>(sizeof(ArchivoHeader));
    char buffer[8192];
    bool copiaCorrecta = origen.good() && destino.good();
    while (copiaCorrecta && restantes > 0) {
        streamsize cantidad = static_cast<streamsize>(
            restantes < static_cast<streamoff>(sizeof(buffer))
                ? restantes
                : static_cast<streamoff>(sizeof(buffer)));
        origen.read(buffer, cantidad);
        if (origen.gcount() != cantidad) {
            copiaCorrecta = false;
            break;
        }
        destino.write(buffer, cantidad);
        copiaCorrecta = destino.good();
        restantes -= cantidad;
    }

    origen.close();
    destino.flush();
    copiaCorrecta = copiaCorrecta && !origen.bad() && destino.good();
    destino.close();
    copiaCorrecta = copiaCorrecta && !destino.fail();
    if (!copiaCorrecta) {
        remove(temporal.c_str());
        return false;
    }

    #ifdef _WIN32
        if (rename(ruta, respaldo.c_str()) != 0) {
            remove(temporal.c_str());
            return false;
        }
        if (rename(temporal.c_str(), ruta) != 0) {
            rename(respaldo.c_str(), ruta);
            remove(temporal.c_str());
            return false;
        }
        return remove(respaldo.c_str()) == 0;
    #else
        if (rename(temporal.c_str(), ruta) != 0) {
            remove(temporal.c_str());
            return false;
        }
        return true;
    #endif
}

static bool restaurarAppend(const char* ruta, streamoff tamanoOriginal,
                            const ArchivoHeader &headerOriginal) {
    return reemplazarPorPrefijo(ruta, tamanoOriginal, headerOriginal);
}

template <typename Registro>
static bool guardarRegistroCrudo(const char* ruta, Registro &registro,
                                 string &motivoFallo) {
    motivoFallo.clear();
    ArchivoHeader headerOriginal = {};
    if (!leerHeader(ruta, headerOriginal) || !headerValido(headerOriginal) ||
        headerOriginal.cantidadRegistros == INT_MAX) {
        motivoFallo = "Header invalido o limite de registros alcanzado en " +
                      string(ruta) + ".";
        return false;
    }

    int registrosFisicos = 0;
    bool registroIncompleto = false;
    string motivoInspeccion;
    if (!inspeccionarArchivoEntidad(ruta, sizeof(Registro), registrosFisicos,
                                   registroIncompleto, motivoInspeccion) ||
        registroIncompleto ||
        registrosFisicos != headerOriginal.cantidadRegistros) {
        motivoFallo = "El contenido fisico de " + string(ruta) +
                      " no coincide con su header.";
        return false;
    }

    streamoff tamanoOriginal =
        static_cast<streamoff>(sizeof(ArchivoHeader)) +
        static_cast<streamoff>(registrosFisicos) *
            static_cast<streamoff>(sizeof(Registro));

    time_t ahora = time(nullptr);
    registro.id = headerOriginal.proximoID;
    registro.eliminado = false;
    registro.fechaCreacion = ahora;
    registro.fechaUltimaModificacion = ahora;

    ofstream archivo(ruta, ios::out | ios::binary | ios::app);
    if (!archivo.is_open()) {
        motivoFallo = "No se pudo abrir " + string(ruta) + " para agregar un registro.";
        return false;
    }
    archivo.write(reinterpret_cast<const char*>(&registro), sizeof(Registro));
    archivo.flush();
    bool registroEscrito = archivo.good();
    archivo.close();
    registroEscrito = registroEscrito && !archivo.fail();
    if (!registroEscrito) {
        bool restaurado = restaurarAppend(ruta, tamanoOriginal, headerOriginal);
        motivoFallo = restaurado
            ? "Fallo al escribir el registro; se revirtio el append."
            : "Fallo al escribir el registro y no se pudo revertir completamente el archivo.";
        return false;
    }

    ArchivoHeader headerNuevo = headerOriginal;
    headerNuevo.cantidadRegistros++;
    headerNuevo.registrosActivos++;
    headerNuevo.proximoID++;
    if (!actualizarHeader(ruta, headerNuevo)) {
        bool restaurado = restaurarAppend(ruta, tamanoOriginal, headerOriginal);
        motivoFallo = restaurado
            ? "Fallo al actualizar el header; se revirtio el append."
            : "Fallo al actualizar el header y no se pudo revertir completamente el archivo.";
        return false;
    }
    return true;
}

template <typename Registro>
static bool escribirRegistroEnPosicion(const char* ruta, int id,
                                       const Registro &registro) {
    fstream archivo(ruta, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;
    archivo.seekp(calcularOffset(id, sizeof(Registro)), ios::beg);
    archivo.write(reinterpret_cast<const char*>(&registro), sizeof(Registro));
    archivo.flush();
    bool exito = archivo.good();
    archivo.close();
    return exito && !archivo.fail();
}

template <typename Registro>
static bool marcarRegistroEliminado(const char* ruta, int id,
                                   string &motivoFallo) {
    motivoFallo.clear();
    ArchivoHeader headerOriginal = {};
    if (!leerHeader(ruta, headerOriginal) || !headerValido(headerOriginal) ||
        id < 1 || id > headerOriginal.cantidadRegistros ||
        headerOriginal.registrosActivos < 1) {
        motivoFallo = "Header invalido o ID fuera de rango en " + string(ruta) + ".";
        return false;
    }

    int registrosFisicos = 0;
    bool registroIncompleto = false;
    string motivoInspeccion;
    if (!inspeccionarArchivoEntidad(ruta, sizeof(Registro), registrosFisicos,
                                   registroIncompleto, motivoInspeccion) ||
        registroIncompleto ||
        registrosFisicos != headerOriginal.cantidadRegistros) {
        motivoFallo = "El contenido fisico de " + string(ruta) +
                      " no coincide con su header.";
        return false;
    }

    ifstream entrada(ruta, ios::binary);
    Registro original = {};
    if (!entrada.is_open()) {
        motivoFallo = "No se pudo abrir " + string(ruta) + " para leer el registro.";
        return false;
    }
    entrada.seekg(calcularOffset(id, sizeof(Registro)), ios::beg);
    entrada.read(reinterpret_cast<char*>(&original), sizeof(Registro));
    bool lecturaCorrecta = entrada.good();
    entrada.close();
    if (!lecturaCorrecta || original.id != id) {
        motivoFallo = "No se pudo validar el registro ID #" + to_string(id) + ".";
        return false;
    }
    if (original.eliminado) {
        motivoFallo = "El registro ya estaba eliminado.";
        return false;
    }

    Registro actualizado = original;
    actualizado.eliminado = true;
    actualizado.fechaUltimaModificacion = time(nullptr);
    if (!escribirRegistroEnPosicion(ruta, id, actualizado)) {
        bool restaurado = escribirRegistroEnPosicion(ruta, id, original);
        motivoFallo = restaurado
            ? "Fallo al marcar el registro; se restauro el original."
            : "Fallo al marcar y no se pudo restaurar el registro original.";
        return false;
    }

    ArchivoHeader headerNuevo = headerOriginal;
    headerNuevo.registrosActivos--;
    if (!actualizarHeader(ruta, headerNuevo)) {
        bool headerRestaurado = actualizarHeader(ruta, headerOriginal);
        bool registroRestaurado = escribirRegistroEnPosicion(ruta, id, original);
        motivoFallo = headerRestaurado && registroRestaurado
            ? "Fallo al actualizar el header; se revirtio la baja."
            : "Fallo al actualizar el header y no se pudo revertir completamente la baja.";
        return false;
    }
    return true;
}

// ============================================================
// OPERACIONES SOBRE EL HEADER Y CONTROL DE ARCHIVOS (SECCIÓN 4.1)
// ============================================================

bool asegurarDirectorioDatos(string &motivoFallo) {
    #ifdef _WIN32
        if (_mkdir("datos") != 0 && errno != EEXIST) {
            motivoFallo = "No se pudo crear la carpeta datos/.";
            return false;
        }
    #else
        if (mkdir("datos", 0777) != 0 && errno != EEXIST) {
            motivoFallo = "No se pudo crear la carpeta datos/.";
            return false;
        }
    #endif
    return true;
}

bool inicializarArchivo(const char* ruta, size_t tamanoRegistro) {
    ifstream archivo(ruta, ios::in | ios::binary);
    if (!archivo.is_open()) {
        archivo.clear();
        if (archivoExiste(ruta)) {
            return false;
        }
        ofstream nuevo(ruta, ios::out | ios::binary);
        if (!nuevo.is_open()) {
            return false;
        }
        ArchivoHeader h;
        h.cantidadRegistros = 0;
        h.proximoID = 1;
        h.registrosActivos = 0;
        h.version = 1;
        nuevo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
        bool exito = nuevo.good();
        nuevo.close();
        return exito && !nuevo.fail();
    }

    ArchivoHeader h = {};
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    bool valido = archivo.good() && headerValido(h);
    archivo.close();
    if (!valido) return false;
    if (tamanoRegistro == 0) return true;

    int registrosFisicos = 0;
    bool registroIncompleto = false;
    string motivoFallo;
    return inspeccionarArchivoEntidad(ruta, tamanoRegistro, registrosFisicos,
                                      registroIncompleto, motivoFallo) &&
           !registroIncompleto &&
           registrosFisicos == h.cantidadRegistros;
}

bool leerHeader(const char* ruta, ArchivoHeader &header) {
    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) {
        return false;
    }
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    bool exito = archivo.good();
    archivo.close();
    return exito;
}

bool inspeccionarArchivoEntidad(const char* ruta, size_t tamanoRegistro,
                                int &registrosFisicos, bool &registroIncompleto,
                                string &motivoFallo) {
    registrosFisicos = 0;
    registroIncompleto = false;
    if (tamanoRegistro == 0) {
        motivoFallo = "El tamano de registro no puede ser cero.";
        return false;
    }

    ifstream archivo(ruta, ios::binary);
    if (!archivo.is_open()) {
        motivoFallo = "No se pudo abrir " + string(ruta) + ".";
        return false;
    }
    archivo.seekg(0, ios::end);
    streamoff tamArchivo = archivo.tellg();
    if (!archivo.good() || tamArchivo < static_cast<streamoff>(sizeof(ArchivoHeader))) {
        motivoFallo = string(ruta) + " no contiene un header completo.";
        return false;
    }

    streamoff bytesRegistros = tamArchivo - static_cast<streamoff>(sizeof(ArchivoHeader));
    streamoff cantidad = bytesRegistros / static_cast<streamoff>(tamanoRegistro);
    if (cantidad > numeric_limits<int>::max()) {
        motivoFallo = string(ruta) + " excede la cantidad de registros soportada.";
        return false;
    }
    registrosFisicos = static_cast<int>(cantidad);
    registroIncompleto = bytesRegistros % static_cast<streamoff>(tamanoRegistro) != 0;
    return true;
}

bool actualizarHeader(const char* ruta, const ArchivoHeader &header) {
    fstream archivo(ruta, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) {
        return false;
    }
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    bool exito = archivo.good();
    archivo.close();
    return exito && !archivo.fail();
}

long calcularOffset(int id, size_t tamanoRegistro) {
    // posición = sizeof(ArchivoHeader) + i * sizeof(Estructura), donde i = id - 1
    return static_cast<long>(sizeof(ArchivoHeader)) + static_cast<long>(id - 1) * static_cast<long>(tamanoRegistro);
}

// =========================
// PERSISTENCIA DE TALLER
// =========================

bool cargarTallerArchivo(Taller &t, bool &noExiste) {
    noExiste = false;
    ifstream archivo(RUTA_TALLER, ios::binary);
    if (!archivo.is_open()) {
        noExiste = !archivoExiste(RUTA_TALLER) && errno == ENOENT;
        return false;
    }
    archivo.read(reinterpret_cast<char*>(&t), sizeof(Taller));
    bool exito = archivo.good();
    if (exito) {
        archivo.seekg(0, ios::end);
        exito = archivo.good() && archivo.tellg() == static_cast<streamoff>(sizeof(Taller));
    }
    archivo.close();
    return exito;
}

bool guardarTallerArchivo(const Taller &t) {
    Taller copia = t;
    time_t ahora = time(nullptr);
    if (copia.fechaCreacion == 0) {
        copia.fechaCreacion = ahora;
    }
    copia.fechaUltimaModificacion = ahora;

    ofstream archivo(RUTA_TALLER, ios::binary | ios::trunc);
    if (!archivo.is_open()) {
        return false;
    }
    archivo.write(reinterpret_cast<const char*>(&copia), sizeof(Taller));
    bool exito = archivo.good();
    archivo.close();
    return exito && !archivo.fail();
}

bool crearBackupArchivos(string &rutaGenerada, string &motivoFallo) {
    time_t ahora = time(nullptr);
    tm* t = localtime(&ahora);
    if (t == nullptr) {
        motivoFallo = "No se pudo obtener la fecha y hora para nombrar el respaldo.";
        return false;
    }
    char marcaTiempo[32];
    if (strftime(marcaTiempo, sizeof(marcaTiempo), "%Y-%m-%d_%H-%M-%S", t) == 0) {
        motivoFallo = "No se pudo formatear la fecha y hora del respaldo.";
        return false;
    }
    rutaGenerada = string("datos/backup_") + marcaTiempo;

    #ifdef _WIN32
        if (_mkdir(rutaGenerada.c_str()) != 0) {
            motivoFallo = "No se pudo crear " + rutaGenerada + ".";
            return false;
        }
    #else
        if (mkdir(rutaGenerada.c_str(), 0777) != 0) {
            motivoFallo = "No se pudo crear " + rutaGenerada + ".";
            return false;
        }
    #endif

    const char* archivos[] = {"taller.bin", "vehiculos.bin", "mecanicos.bin", "repuestos.bin", "ordenes.bin"};
    for (size_t i = 0; i < sizeof(archivos) / sizeof(archivos[0]); i++) {
        string origen = "datos/" + string(archivos[i]);
        string destino = rutaGenerada + "/" + string(archivos[i]);
        ifstream src(origen, ios::binary);
        if (!src.is_open()) {
            motivoFallo = "No se pudo abrir el archivo de origen " + origen + ".";
            return false;
        }
        ofstream dst(destino, ios::binary | ios::trunc);
        if (!dst.is_open()) {
            motivoFallo = "No se pudo crear el archivo de respaldo " + destino + ".";
            return false;
        }
        dst << src.rdbuf();
        bool copiaCorrecta = !src.bad() && dst.good();
        dst.close();
        if (!copiaCorrecta || dst.fail()) {
            motivoFallo = "Fallo al copiar " + origen + " a " + destino + ".";
            return false;
        }
    }
    return true;
}

// =========================
// PERSISTENCIA: VEHICULOS
// =========================

bool guardarVehiculoCrudo(Vehiculo &v, string &motivoFallo) {
    return guardarRegistroCrudo(RUTA_VEHICULOS, v, motivoFallo);
}

bool leerVehiculoPorID(int id, Vehiculo &v, long *offsetSalida) {
    ifstream archivo(RUTA_VEHICULOS, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || id < 1 || id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(Vehiculo));
    if (offsetSalida) *offsetSalida = pos;

    archivo.seekg(pos, ios::beg);
    archivo.read(reinterpret_cast<char*>(&v), sizeof(Vehiculo));
    bool exito = archivo.good();
    archivo.close();
    return exito;
}

bool sobrescribirVehiculo(const Vehiculo &v) {
    fstream archivo(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || v.id < 1 || v.id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    Vehiculo copia = v;
    copia.fechaUltimaModificacion = time(nullptr);

    long pos = calcularOffset(copia.id, sizeof(Vehiculo));
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&copia), sizeof(Vehiculo));
    bool exito = archivo.good();
    archivo.close();
    return exito && !archivo.fail();
}

bool marcarVehiculoEliminado(int id, string &motivoFallo) {
    return marcarRegistroEliminado<Vehiculo>(RUTA_VEHICULOS, id, motivoFallo);
}

int contarVehiculosActivos() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, h)) return 0;
    return h.registrosActivos;
}

// =========================
// PERSISTENCIA: MECANICOS
// =========================

bool guardarMecanicoCrudo(Mecanico &m, string &motivoFallo) {
    return guardarRegistroCrudo(RUTA_MECANICOS, m, motivoFallo);
}

bool leerMecanicoPorID(int id, Mecanico &m, long *offsetSalida) {
    ifstream archivo(RUTA_MECANICOS, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || id < 1 || id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(Mecanico));
    if (offsetSalida) *offsetSalida = pos;

    archivo.seekg(pos, ios::beg);
    archivo.read(reinterpret_cast<char*>(&m), sizeof(Mecanico));
    bool exito = archivo.good();
    archivo.close();
    return exito;
}

bool sobrescribirMecanico(const Mecanico &m) {
    fstream archivo(RUTA_MECANICOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || m.id < 1 || m.id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    Mecanico copia = m;
    copia.fechaUltimaModificacion = time(nullptr);

    long pos = calcularOffset(copia.id, sizeof(Mecanico));
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&copia), sizeof(Mecanico));
    bool exito = archivo.good();
    archivo.close();
    return exito && !archivo.fail();
}

bool marcarMecanicoEliminado(int id, string &motivoFallo) {
    return marcarRegistroEliminado<Mecanico>(RUTA_MECANICOS, id, motivoFallo);
}

int contarMecanicosActivos() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, h)) return 0;
    return h.registrosActivos;
}

// =========================
// PERSISTENCIA: REPUESTOS
// =========================

bool guardarRepuestoCrudo(Repuesto &r, string &motivoFallo) {
    return guardarRegistroCrudo(RUTA_REPUESTOS, r, motivoFallo);
}

bool leerRepuestoPorID(int id, Repuesto &r, long *offsetSalida) {
    ifstream archivo(RUTA_REPUESTOS, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || id < 1 || id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(Repuesto));
    if (offsetSalida) *offsetSalida = pos;

    archivo.seekg(pos, ios::beg);
    archivo.read(reinterpret_cast<char*>(&r), sizeof(Repuesto));
    bool exito = archivo.good();
    archivo.close();
    return exito;
}

bool sobrescribirRepuesto(const Repuesto &r) {
    fstream archivo(RUTA_REPUESTOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || r.id < 1 || r.id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    Repuesto copia = r;
    copia.fechaUltimaModificacion = time(nullptr);

    long pos = calcularOffset(copia.id, sizeof(Repuesto));
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&copia), sizeof(Repuesto));
    bool exito = archivo.good();
    archivo.close();
    return exito && !archivo.fail();
}

bool marcarRepuestoEliminado(int id, string &motivoFallo) {
    return marcarRegistroEliminado<Repuesto>(RUTA_REPUESTOS, id, motivoFallo);
}

int contarRepuestosActivos() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, h)) return 0;
    return h.registrosActivos;
}

// ===================================
// PERSISTENCIA: ORDENES DE SERVICIO
// ===================================

bool guardarOrdenCruda(OrdenServicio &o, string &motivoFallo) {
    return guardarRegistroCrudo(RUTA_ORDENES, o, motivoFallo);
}

bool leerOrdenPorID(int id, OrdenServicio &o, long *offsetSalida) {
    ifstream archivo(RUTA_ORDENES, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || id < 1 || id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(OrdenServicio));
    if (offsetSalida) *offsetSalida = pos;

    archivo.seekg(pos, ios::beg);
    archivo.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));
    bool exito = archivo.good();
    archivo.close();
    return exito;
}

bool sobrescribirOrden(const OrdenServicio &o) {
    fstream archivo(RUTA_ORDENES, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good() || o.id < 1 || o.id > h.cantidadRegistros) {
        archivo.close();
        return false;
    }

    OrdenServicio copia = o;
    copia.fechaUltimaModificacion = time(nullptr);

    long pos = calcularOffset(copia.id, sizeof(OrdenServicio));
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&copia), sizeof(OrdenServicio));
    bool exito = archivo.good();
    archivo.close();
    return exito && !archivo.fail();
}

bool marcarOrdenEliminada(int id, string &motivoFallo) {
    return marcarRegistroEliminado<OrdenServicio>(RUTA_ORDENES, id, motivoFallo);
}

int contarOrdenesActivas() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, h)) return 0;
    return h.registrosActivos;
}
