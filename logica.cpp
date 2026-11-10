#include "logica.h"
#include <cstring>
#include <cctype>
#include <string>
#include <fstream>
#include <ctime>
using namespace std;

// FUNCIONES AUXILIARES
// Se utilizan arreglos char en C para garantizar memoria fija
void aMinusculas(const char* origen, char* destino) {
    int i = 0;
    while (origen[i] != '\0') {
        destino[i] = tolower(origen[i]);
        i++;
    }
    destino[i] = '\0';
}

// Implementa una busqueda insensible a mayusculas o minusculas 
bool contieneSubcadena(const char* texto, const char* subcadena) {
    char t[100], sub[100];
    aMinusculas(texto, t);
    aMinusculas(subcadena, sub);
    return strstr(t, sub) != nullptr;
}

// ########################
// MOTOR BASE: PERSISTENCIA
// ########################

void inicializarArchivoBase(const char* ruta) {
    fstream archivo(ruta, ios::in | ios::binary);
    if (!archivo.is_open()) {
        archivo.clear();
        archivo.open(ruta, ios::out | ios::binary);
        ArchivoHeader h = {0, 1, 0, 1}; // cant:0, proxID:1, activos:0, ver:1
        archivo.write((char*)&h, sizeof(ArchivoHeader));
        archivo.close();
    } else {
        archivo.close();
    }
}

long calcularPosicion(int id, size_t sizeEstructura) {
    return sizeof(ArchivoHeader) + (id - 1) * sizeEstructura;
}

// =====================
// MODULO DE VEHICULOS
// =====================

bool agregarVehiculo(Vehiculo &datos) {
    Vehiculo temp;
    if (buscarVehiculoPorPlaca(datos.placa, temp)) {
        return false; // Placa ya registrada
    }

    fstream archivo("vehiculos.bin", ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false; // Igual a un cout advirtiendo que no se pudo abrir el archivo

    // Leer Header
    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    // Preparar metadatos
    datos.id = h.proximoID;
    datos.eliminado = false;
    datos.fechaCreacion = time(nullptr);
    datos.fechaUltimaModificacion = datos.fechaCreacion;

    // Escribir al final
    archivo.seekp(0, ios::end);
    archivo.write((char*)&datos, sizeof(Vehiculo));

    // Actualizar Header
    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    
    archivo.seekp(0, ios::beg);
    archivo.write((char*)&h, sizeof(ArchivoHeader));
    archivo.close();
    
    return true;
}

bool buscarVehiculoPorID(int id, Vehiculo &resultado) {
    fstream archivo("vehiculos.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
        archivo.close();
        return false;
    }

    // ACCESO ALEATORIO: Saltamos a la posición exacta (seekg)
    long posicion = calcularPosicion(id, sizeof(Vehiculo));
    archivo.seekg(posicion, ios::beg);
    archivo.read((char*)&resultado, sizeof(Vehiculo));
    archivo.close();

    return !resultado.eliminado;
}

bool buscarVehiculoPorPlaca(const char* placa, Vehiculo &resultado) {
    fstream archivo("vehiculos.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    // Búsqueda secuencial solo para la placa
    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read((char*)&resultado, sizeof(Vehiculo));
        if (!resultado.eliminado && strcmp(resultado.placa, placa) == 0) {
            archivo.close();
            return true; 
        }
    }
    archivo.close();
    return false;
}

bool listarVehiculos(Vehiculo resultados[], int &cantidad) {
    cantidad = 0;
    fstream archivo("datos/vehiculos.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    Vehiculo temp;
    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Vehiculo));
        if (!temp.eliminado && cantidad < 100) { // Limitamos para no desbordar RAM
            resultados[cantidad] = temp;
            cantidad++;
        }
    }
    archivo.close();
    return cantidad > 0;
}

bool actualizarVehiculo(int id, Vehiculo datosNuevos) {
    Vehiculo vActual;
    if (!buscarVehiculoPorID(id, vActual)) return false; 

    // Copiamos datos permitidos
    strcpy(vActual.marca, datosNuevos.marca);
    strcpy(vActual.modelo, datosNuevos.modelo);
    vActual.anio = datosNuevos.anio;
    strcpy(vActual.nombrePropietario, datosNuevos.nombrePropietario);
    strcpy(vActual.cedulaPropietario, datosNuevos.cedulaPropietario);
    strcpy(vActual.telefonoPropietario, datosNuevos.telefonoPropietario);
    
    vActual.fechaUltimaModificacion = time(nullptr);

    // Escribimos en su posición original (seekp)
    fstream archivo("datos/vehiculos.bin", ios::in | ios::out | ios::binary);
    long posicion = calcularPosicion(id, sizeof(Vehiculo));
    archivo.seekp(posicion, ios::beg);
    archivo.write((char*)&vActual, sizeof(Vehiculo));
    archivo.close();

    return true;
}

bool eliminarVehiculo(int id) {
    Vehiculo v;
    if (!buscarVehiculoPorID(id, v)) return false;

    // Faltaria verificar si tiene ordenes activas buscando en ordenes.bin

    v.eliminado = true; // Borrado lógico
    v.fechaUltimaModificacion = time(nullptr);

    fstream archivo("datos/vehiculos.bin", ios::in | ios::out | ios::binary);
    long posicion = calcularPosicion(id, sizeof(Vehiculo));
    archivo.seekp(posicion, ios::beg);
    archivo.write((char*)&v, sizeof(Vehiculo));

    // Descontar activos del header
    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read((char*)&h, sizeof(ArchivoHeader));
    h.registrosActivos--;
    archivo.seekp(0, ios::beg);
    archivo.write((char*)&h, sizeof(ArchivoHeader));

    archivo.close();
    return true;
}

// =================
// MODULO MECANICOS
// =================

bool registrarMecanico(Mecanico &datos) {
    Mecanico temp;
    if (buscarMecanicoPorCedula(datos.cedula, temp)) {
        return false; // Cedula duplicada
    }
    if (especialidadValida(datos.especialidad)) {
        return false;
    }
    ArchivoHeader h;
    fstream archivo("mecanicos.bin", ios::in | ios::out |ios::binary);
    if (!archivo.is_open()) return false;
    archivo.seekg(0, ios::beg);
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));

    datos.id = h.proximoID;
    datos.eliminado = false;
    datos.fechaCreacion = time(nullptr);
    datos.fechaUltimaModificacion = datos.fechaCreacion;
    archivo.seekp(0, ios::end);

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    
    archivo.seekp(0, ios::beg);
    archivo.write(reinterpret_cast<char*>(&datos), sizeof(ArchivoHeader));
    return true;
}

bool especialidadValida(const char* esp) {
        return (strcmp(esp, "MOTOR") == 0 || 
                strcmp(esp, "FRENOS") == 0 || 
                strcmp(esp, "ELECTRICIDAD") == 0 || 
                strcmp(esp, "SUSPENSION") == 0 || 
                strcmp(esp, "GENERAL") == 0);
}

bool buscarMecanicoPorID(int id, Mecanico &resultado) {
    ifstream archivo("mecanico.bin", ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));
    if (id >= 1 && id < h.proximoID) {
        return false;
    }

    long posicion = calcularPosicion(id, sizeof(Mecanico));
    archivo.seekg(posicion, ios::beg);
    archivo.read(reinterpret_cast<char*>(&posicion), sizeof(Mecanico));
    archivo.close();
}

bool buscarMecanicoPorCedula(const char* cedula, Mecanico &resultado) {
    fstream archivo("mecanicos.bin", ios::in | ios::binary);
    ArchivoHeader h;
    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&h), sizeof(Mecanico));
        if (!resultado.eliminado && strcmp(resultado.cedula, cedula) == 0) {
            return true;
        }
    }
    archivo.close();
}

bool listarMecanicos(Mecanico resultados[], int &cantidad) {
    cantidad = 0;
    ifstream archivo("mecanicos.bin", ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));

    Mecanico temp;
    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read(reinterpret_cast<char*>(&h), sizeof(Mecanico));
        if (!temp.eliminado && cantidad < 100) {
            resultados[cantidad] = temp;
            cantidad++;
        }   
    }
    archivo.close();
    return cantidad > 0;
}

// =================
// MODULO REPUESTOS
// =================
Repuesto* registrarRepuesto(sysTaller &s, Repuesto datos) {

    if (buscarRepuestoPorCodigo(s, datos.codigo) != nullptr) {
        return nullptr;
    }
    if (datos.precio <= 0 || datos.stock < 0 || datos.stockmin < 0) {
        return nullptr;
    }
    if (strlen(datos.nombre) == 0 || strlen(datos.fechaRegistro) == 0) {
        return nullptr;
    }

    if (s.numRepuestos >= s.capacidadRepuestos) {
        int nuevaCapacidad = s.capacidadRepuestos * 2;
        Repuesto* nuevoArreglo = new Repuesto[nuevaCapacidad];

        for (int i = 0; i < s.numRepuestos; i++) {
            nuevoArreglo[i] = s.repuestos[i];
        }

        delete[] s.repuestos;
        s.repuestos = nuevoArreglo;
        s.capacidadRepuestos = nuevaCapacidad;
    }

    datos.id = s.siguienteIdRepuesto++; 
    
    int indiceActual = s.numRepuestos;  
    s.repuestos[indiceActual] = datos;  
    
    s.numRepuestos++; 

    return &s.repuestos[indiceActual];
}

Repuesto* buscarRepuestoPorID(sysTaller &s, int id) {
    for (int i = 0; i < s.numRepuestos; i++) {
        if (s.repuestos[i].id == id) {
            return &s.repuestos[i];
        }
    }
    return nullptr;
}

Repuesto* buscarRepuestoPorCodigo(sysTaller s, const char* codigo) {
    for (int i = 0; i < s.numRepuestos; i++) {
        if (strcmp(s.repuestos[i].codigo, codigo) == 0) {
            return &s.repuestos[i];
        }
    }
    return nullptr;
}

bool registrarEntradaMercancia(sysTaller &s, int idRepuesto, int cantidadAdicional) {
    if (cantidadAdicional <= 0) return false;
    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r == nullptr) return false;
    r->stock += cantidadAdicional;
    return true;
}

Repuesto** listarRepuestos(sysTaller &s, int &cantidad) {
    cantidad = s.numRepuestos;
    if (cantidad == 0) return nullptr;

    Repuesto** resultado = new Repuesto*[cantidad];
    for (int i = 0; i < cantidad; i++) {
        resultado[i] = &s.repuestos[i];
    }
    return resultado;
}

bool cambiarPrecioRepuesto(sysTaller &s, int idRepuesto, float nuevoPrecio) {
    if (nuevoPrecio <= 0) return false;

    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r == nullptr) return false;

    r->precio = nuevoPrecio;
    return true;
}

bool eliminarRepuesto(sysTaller &s, int id) {
    for (int i = 0; i < s.numOrdenes; i++) {
        for (int j = 0; j < s.ordenes[i].numDetalles; j++) {
            if (s.ordenes[i].detalles[j].idRepuesto == id) {
                return false; // Rechazado: Romperia el historial del taller
            }
        }
    }

    int index = -1;
    for (int i = 0; i < s.numRepuestos; i++) {
        if (s.repuestos[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) return false;

    for (int i = index; i < s.numRepuestos - 1; i++) {
        s.repuestos[i] = s.repuestos[i + 1];
    }
    
    s.numRepuestos--;
    return true;
}

// ===========================
// MODULO ORDENES DE SERVICIO
// ===========================

ordenServicio* abrirOrdenServicio(sysTaller &s, int idVehiculo, int idMecanico, const char* fechaApertura, const char* descripcionFalla, int kmIngreso) {
    Vehiculo* v = buscarVehiculoPorID(s, idVehiculo);
    if (v == nullptr) return nullptr;

    Mecanico* m = buscarMecanicoPorID(s, idMecanico);
    if (m == nullptr) return nullptr;

    for (int i = 0; i < s.numOrdenes; i++) {
        if (s.ordenes[i].idVehiculo == idVehiculo && strcmp(s.ordenes[i].estado, "ABIERTA") == 0) {
            return nullptr; 
        }
    }

    int ordenesAbiertasMecanico = 0;
    for (int i = 0; i < s.numOrdenes; i++) {    
        if (s.ordenes[i].idMecanico == idMecanico && strcmp(s.ordenes[i].estado, "ABIERTA") == 0) {
            ordenesAbiertasMecanico++;
        }
    }
    if (ordenesAbiertasMecanico >= 3) return nullptr;

    if (kmIngreso < v->km) return nullptr;

    v->km = kmIngreso;

    if (s.numOrdenes >= s.capacidadOrdenes) {
        int nuevaCapacidad = s.capacidadOrdenes * 2;
        ordenServicio* nuevoArreglo = new ordenServicio[nuevaCapacidad];
        for (int i = 0; i < s.numOrdenes; i++) {
            nuevoArreglo[i] = s.ordenes[i];
        }
        delete[] s.ordenes;
        s.ordenes = nuevoArreglo;
        s.capacidadOrdenes = nuevaCapacidad;
    }

    ordenServicio nueva;
    nueva.id = s.siguienteIdOrden++;
    nueva.idVehiculo = idVehiculo;
    nueva.idMecanico = idMecanico;
    strcpy(nueva.fechaApertura, fechaApertura);
    strcpy(nueva.fechaCierre, "");
    strcpy(nueva.estado, "ABIERTA");
    strcpy(nueva.descripcionFalla, descripcionFalla);
    nueva.kmIngreso = kmIngreso;
    nueva.manoObra = 0.0f;

    nueva.capacidadDetalles = 2;
    nueva.numDetalles = 0;
    nueva.detalles = new detalleOrden[nueva.capacidadDetalles];

    s.ordenes[s.numOrdenes] = nueva;
    return &s.ordenes[s.numOrdenes++];
}

bool agregarRepuestoAOrden(sysTaller &s, int idOrden, int idRepuesto, int cantidad) {
    ordenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr || strcmp(o->estado, "ABIERTA") != 0) return false; 
    if (cantidad <= 0) return false;

    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r == nullptr || r->stock < cantidad) return false; 

    bool encontrado = false;
    for (int i = 0; i < o->numDetalles; i++) {
        if (o->detalles[i].idRepuesto == idRepuesto) {
            o->detalles[i].cantidad += cantidad;
            encontrado = true;
            break;
        }
    }

    if (!encontrado) {
        if (o->numDetalles >= o->capacidadDetalles) {
            int nuevaCapacidad = o->capacidadDetalles * 2;
            detalleOrden* nuevosDetalles = new detalleOrden[nuevaCapacidad];
            for (int i = 0; i < o->numDetalles; i++) {
                nuevosDetalles[i] = o->detalles[i];
            }
            delete[] o->detalles;
            o->detalles = nuevosDetalles;
            o->capacidadDetalles = nuevaCapacidad;
        }

        o->detalles[o->numDetalles].idRepuesto = idRepuesto;
        o->detalles[o->numDetalles].cantidad = cantidad;
        o->detalles[o->numDetalles].precioUnitario = r->precio;
        o->numDetalles++;
    }

    r->stock -= cantidad;
    return true;
}

bool quitarRepuestoDeOrden(sysTaller &s, int idOrden, int idRepuesto) {
    ordenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr || strcmp(o->estado, "ABIERTA") != 0) return false;

    int index = -1;
    for (int i = 0; i < o->numDetalles; i++) {
        if (o->detalles[i].idRepuesto == idRepuesto) {
            index = i;
            break;
        }
    }
    if (index == -1) return false;

    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r != nullptr) {
        r->stock += o->detalles[index].cantidad;
    }

    for (int i = index; i < o->numDetalles - 1; i++) {
        o->detalles[i] = o->detalles[i + 1];
    }
    o->numDetalles--;
    return true;
}

bool cerrarOrdenServicio(sysTaller &s, int idOrden, const char* fechaCierre, float manoObra) {
    ordenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr || strcmp(o->estado, "ABIERTA") != 0) return false; 
    if (manoObra < 0) return false;

    if (o->numDetalles == 0 && manoObra <= 0) return false;

    o->manoObra = manoObra;
    strcpy(o->fechaCierre, fechaCierre);
    strcpy(o->estado, "CERRADA");
    return true;
}

bool anularOrdenServicio(sysTaller &s, int idOrden) {
    ordenServicio* o = buscarOrdenPorID(s, idOrden);
    if (o == nullptr || (strcmp(o->estado, "ABIERTA") != 0 && strcmp(o->estado, "CERRADA") != 0)) {
        return false;
    }

    for (int i = 0; i < o->numDetalles; i++) {
        Repuesto* r = buscarRepuestoPorID(s, o->detalles[i].idRepuesto);
        if (r != nullptr) {
            r->stock += o->detalles[i].cantidad;
        }
    }

    strcpy(o->estado, "ANULADA");
    return true;
}

float calcularTotalOrden(const ordenServicio &o) {
    float totalRepuestos = 0;
    for (int i = 0; i < o.numDetalles; i++) {
        totalRepuestos += (o.detalles[i].cantidad * o.detalles[i].precioUnitario);
    }
    return o.manoObra + totalRepuestos;
}

ordenServicio* buscarOrdenPorID(sysTaller &s, int id) {
    for (int i = 0; i < s.numOrdenes; i++) {
        if (s.ordenes[i].id == id) return &s.ordenes[i];
    }
    return nullptr;
}

ordenServicio** listarOrdenesPorEstado(sysTaller &s, const char* estado, int &cantidad) {
    cantidad = 0;
    for (int i = 0; i < s.numOrdenes; i++) {
        if (strcmp(s.ordenes[i].estado, estado) == 0) {
            cantidad++;
        }
    }
    if (cantidad == 0) return nullptr;

    ordenServicio** resultado = new ordenServicio*[cantidad];
    int idx = 0;
    for (int i = 0; i < s.numOrdenes; i++) {
        if (strcmp(s.ordenes[i].estado, estado) == 0) {
            resultado[idx++] = &s.ordenes[i];
        }
    }
    return resultado;
}