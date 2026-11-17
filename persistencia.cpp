#include "persistencia.h"
#include <fstream>
#include <ctime>

using namespace std;

// Rutas fijas según especificación (Sección 1)
static const char* RUTA_TALLER     = "datos/taller.bin";
static const char* RUTA_VEHICULOS  = "datos/vehiculos.bin";
static const char* RUTA_MECANICOS  = "datos/mecanicos.bin";
static const char* RUTA_REPUESTOS  = "datos/repuestos.bin";
static const char* RUTA_ORDENES    = "datos/ordenes.bin";

// ============================================================
// OPERACIONES SOBRE EL HEADER Y CONTROL DE ARCHIVOS (SECCIÓN 4.1)
// ============================================================

bool inicializarArchivo(const char* ruta) {
    // Intenta abrir en modo lectura binaria para verificar si ya existe
    fstream archivo(ruta, ios::in | ios::binary);
    if (!archivo.is_open()) {
        archivo.clear();
        // Si no existe, se crea en modo escritura
        archivo.open(ruta, ios::out | ios::binary);
        if (!archivo.is_open()) {
            return false;
        }
        ArchivoHeader h;
        h.cantidadRegistros = 0;
        h.proximoID = 1;
        h.registrosActivos = 0;
        h.version = 1;
        archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
        archivo.close();
        return true;
    }
    archivo.close();
    return true; // Ya existia y esta listo
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

bool actualizarHeader(const char* ruta, const ArchivoHeader &header) {
    fstream archivo(ruta, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) {
        return false;
    }
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    bool exito = archivo.good();
    archivo.close();
    return exito;
}

long calcularOffset(int id, size_t tamanoRegistro) {
    // posición = sizeof(ArchivoHeader) + i * sizeof(Estructura), donde i = id - 1
    return static_cast<long>(sizeof(ArchivoHeader)) + static_cast<long>(id - 1) * static_cast<long>(tamanoRegistro);
}

// =========================
// PERSISTENCIA DE TALLER
// =========================

bool cargarTallerArchivo(Taller &t) {
    ifstream archivo(RUTA_TALLER, ios::binary);
    if (!archivo.is_open()) {
        return false;
    }
    archivo.read(reinterpret_cast<char*>(&t), sizeof(Taller));
    bool exito = archivo.good();
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
    return exito;
}

// =========================
// PERSISTENCIA: VEHICULOS
// =========================

bool guardarVehiculoCrudo(Vehiculo &v) {
    fstream archivo(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }

    v.id = h.proximoID;
    v.eliminado = false;
    v.fechaCreacion = time(nullptr);
    v.fechaUltimaModificacion = v.fechaCreacion;

    archivo.seekp(0, ios::end);
    archivo.write(reinterpret_cast<const char*>(&v), sizeof(Vehiculo));

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;

    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool leerVehiculoPorID(int id, Vehiculo &v, long *offsetSalida) {
    ifstream archivo(RUTA_VEHICULOS, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
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
    if (v.id < 1 || v.id >= h.proximoID) {
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
    return exito;
}

bool marcarVehiculoEliminado(int id) {
    fstream archivo(RUTA_VEHICULOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(Vehiculo));
    archivo.seekg(pos, ios::beg);
    Vehiculo v;
    archivo.read(reinterpret_cast<char*>(&v), sizeof(Vehiculo));
    if (v.eliminado) {
        archivo.close();
        return false; // Ya estaba eliminado
    }

    v.eliminado = true;
    v.fechaUltimaModificacion = time(nullptr);
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&v), sizeof(Vehiculo));

    h.registrosActivos--;
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

int contarVehiculosActivos() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_VEHICULOS, h)) return 0;
    return h.registrosActivos;
}

// =========================
// PERSISTENCIA: MECANICOS
// =========================

bool guardarMecanicoCrudo(Mecanico &m) {
    fstream archivo(RUTA_MECANICOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }

    m.id = h.proximoID;
    m.eliminado = false;
    m.fechaCreacion = time(nullptr);
    m.fechaUltimaModificacion = m.fechaCreacion;

    archivo.seekp(0, ios::end);
    archivo.write(reinterpret_cast<const char*>(&m), sizeof(Mecanico));

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;

    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool leerMecanicoPorID(int id, Mecanico &m, long *offsetSalida) {
    ifstream archivo(RUTA_MECANICOS, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
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
    if (m.id < 1 || m.id >= h.proximoID) {
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
    return exito;
}

bool marcarMecanicoEliminado(int id) {
    fstream archivo(RUTA_MECANICOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(Mecanico));
    archivo.seekg(pos, ios::beg);
    Mecanico m;
    archivo.read(reinterpret_cast<char*>(&m), sizeof(Mecanico));
    if (m.eliminado) {
        archivo.close();
        return false;
    }

    m.eliminado = true;
    m.fechaUltimaModificacion = time(nullptr);
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&m), sizeof(Mecanico));

    h.registrosActivos--;
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

int contarMecanicosActivos() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_MECANICOS, h)) return 0;
    return h.registrosActivos;
}

// =========================
// PERSISTENCIA: REPUESTOS
// =========================

bool guardarRepuestoCrudo(Repuesto &r) {
    fstream archivo(RUTA_REPUESTOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }

    r.id = h.proximoID;
    r.eliminado = false;
    r.fechaCreacion = time(nullptr);
    r.fechaUltimaModificacion = r.fechaCreacion;

    archivo.seekp(0, ios::end);
    archivo.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;

    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool leerRepuestoPorID(int id, Repuesto &r, long *offsetSalida) {
    ifstream archivo(RUTA_REPUESTOS, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
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
    if (r.id < 1 || r.id >= h.proximoID) {
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
    return exito;
}

bool marcarRepuestoEliminado(int id) {
    fstream archivo(RUTA_REPUESTOS, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(Repuesto));
    archivo.seekg(pos, ios::beg);
    Repuesto r;
    archivo.read(reinterpret_cast<char*>(&r), sizeof(Repuesto));
    if (r.eliminado) {
        archivo.close();
        return false;
    }

    r.eliminado = true;
    r.fechaUltimaModificacion = time(nullptr);
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&r), sizeof(Repuesto));

    h.registrosActivos--;
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

int contarRepuestosActivos() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_REPUESTOS, h)) return 0;
    return h.registrosActivos;
}

// ===================================
// PERSISTENCIA: ORDENES DE SERVICIO
// ===================================

bool guardarOrdenCruda(OrdenServicio &o) {
    fstream archivo(RUTA_ORDENES, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (!archivo.good()) {
        archivo.close();
        return false;
    }

    o.id = h.proximoID;
    o.eliminado = false;
    o.fechaCreacion = time(nullptr);
    o.fechaUltimaModificacion = o.fechaCreacion;

    archivo.seekp(0, ios::end);
    archivo.write(reinterpret_cast<const char*>(&o), sizeof(OrdenServicio));

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;

    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool leerOrdenPorID(int id, OrdenServicio &o, long *offsetSalida) {
    ifstream archivo(RUTA_ORDENES, ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
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
    if (o.id < 1 || o.id >= h.proximoID) {
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
    return exito;
}

bool marcarOrdenEliminada(int id) {
    fstream archivo(RUTA_ORDENES, ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
        archivo.close();
        return false;
    }

    long pos = calcularOffset(id, sizeof(OrdenServicio));
    archivo.seekg(pos, ios::beg);
    OrdenServicio o;
    archivo.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));
    if (o.eliminado) {
        archivo.close();
        return false;
    }

    o.eliminado = true;
    o.fechaUltimaModificacion = time(nullptr);
    archivo.seekp(pos, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&o), sizeof(OrdenServicio));

    h.registrosActivos--;
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<const char*>(&h), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

int contarOrdenesActivas() {
    ArchivoHeader h;
    if (!leerHeader(RUTA_ORDENES, h)) return 0;
    return h.registrosActivos;
}
