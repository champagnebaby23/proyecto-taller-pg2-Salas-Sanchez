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

// ========================
// CONFIGURACION DEL TALLER
// ========================

bool cargarTaller(Taller &t) {
    fstream archivo("taller.bin", ios::in | ios::binary);
    if (!archivo.is_open()) {
        return false; // El archivo no existe, es el primer arranque
    }
    
    // Como es la excepción, no tiene ArchivoHeader. Leemos directo la estructura
    archivo.read((char*)&t, sizeof(Taller));
    archivo.close();
    return true;
}

void guardarTaller(Taller t) {
    t.fechaCreacion = time(nullptr);
    t.fechaUltimaModificacion = t.fechaCreacion;
    
    // Se guarda como un único registro sin header
    fstream archivo("taller.bin", ios::out | ios::binary);
    archivo.write((char*)&t, sizeof(Taller));
    archivo.close();
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

bool registrarRepuesto(Repuesto &datos) {
    Repuesto temp;
    // Validamos que el código no exista previamente y que los valores sean positivos[cite: 7]
    if (buscarRepuestoPorCodigo(datos.codigo, temp)) return false; 
    if (datos.precio <= 0 || datos.stock < 0 || datos.stockmin < 0) return false;

    fstream archivo("repuestos.bin", ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    datos.id = h.proximoID;
    datos.eliminado = false;
    datos.fechaCreacion = time(nullptr);
    datos.fechaUltimaModificacion = datos.fechaCreacion;

    archivo.seekp(0, ios::end);
    archivo.write((char*)&datos, sizeof(Repuesto));

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    
    archivo.seekp(0, ios::beg);
    archivo.write((char*)&h, sizeof(ArchivoHeader));
    archivo.close();
    
    return true;
}

bool buscarRepuestoPorID(int id, Repuesto &resultado) {
    fstream archivo("repuestos.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));
    if (id < 1 || id >= h.proximoID) {
        archivo.close();
        return false;
    }

    // ACCESO ALEATORIO
    long posicion = calcularPosicion(id, sizeof(Repuesto));
    archivo.seekg(posicion, ios::beg);
    archivo.read((char*)&resultado, sizeof(Repuesto));
    archivo.close();

    return !resultado.eliminado;
}

bool buscarRepuestoPorCodigo(const char* codigo, Repuesto &resultado) {
    fstream archivo("repuestos.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read((char*)&resultado, sizeof(Repuesto));
        if (!resultado.eliminado && strcmp(resultado.codigo, codigo) == 0) {
            archivo.close();
            return true;
        }
    }
    archivo.close();
    return false;
}

bool registrarEntradaMercancia(int idRepuesto, int cantidadAdicional) {
    if (cantidadAdicional <= 0) return false;
    
    Repuesto r;
    if (!buscarRepuestoPorID(idRepuesto, r)) return false;

    // Actualizamos los datos
    r.stock += cantidadAdicional;
    r.fechaUltimaModificacion = time(nullptr);

    // Sobrescribimos en la misma posición exacta
    fstream archivo("repuestos.bin", ios::in | ios::out | ios::binary);
    long posicion = calcularPosicion(idRepuesto, sizeof(Repuesto));
    archivo.seekp(posicion, ios::beg);
    archivo.write((char*)&r, sizeof(Repuesto));
    archivo.close();

    return true;
}

bool listarRepuestos(Repuesto resultados[], int &cantidad) {
    cantidad = 0;
    fstream archivo("datos/repuestos.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    Repuesto temp;
    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Repuesto));
        if (!temp.eliminado && cantidad < 100) {
            resultados[cantidad] = temp;
            cantidad++;
        }
    }
    archivo.close();
    return cantidad > 0;
}

bool cambiarPrecioRepuesto(int idRepuesto, float nuevoPrecio) {
    if (nuevoPrecio <= 0) return false;

    Repuesto r;
    if (!buscarRepuestoPorID(idRepuesto, r)) return false;

    r.precio = nuevoPrecio;
    r.fechaUltimaModificacion = time(nullptr);

    fstream archivo("repuestos.bin", ios::in | ios::out | ios::binary);
    long posicion = calcularPosicion(idRepuesto, sizeof(Repuesto));
    archivo.seekp(posicion, ios::beg);
    archivo.write((char*)&r, sizeof(Repuesto));
    archivo.close();

    return true;
}

bool eliminarRepuesto(int id) {
    Repuesto r;
    if (!buscarRepuestoPorID(id, r)) return false;

    // Se validara si aparece en una orden cuando se arme ordenes.bin

    r.eliminado = true;
    r.fechaUltimaModificacion = time(nullptr);

    fstream archivo("datos/repuestos.bin", ios::in | ios::out | ios::binary);
    long posicion = calcularPosicion(id, sizeof(Repuesto));
    archivo.seekp(posicion, ios::beg);
    archivo.write((char*)&r, sizeof(Repuesto));

    ArchivoHeader h;
    archivo.seekg(0, ios::beg);
    archivo.read((char*)&h, sizeof(ArchivoHeader));
    h.registrosActivos--;
    archivo.seekp(0, ios::beg);
    archivo.write((char*)&h, sizeof(ArchivoHeader));

    archivo.close();
    return true;
}

// ===========================
// MODULO ORDENES DE SERVICIO
// ===========================

bool abrirOrdenServicio(int idVehiculo, int idMecanico, const char* fechaApertura, const char* descripcionFalla, int kmIngreso, ordenServicio &resultado) {
    Vehiculo v;
    if (!buscarVehiculoPorID(idVehiculo, v)) return false;

    Mecanico m;
    if (!buscarMecanicoPorID(idMecanico, m) || m.eliminado) return false; // Mecánico debe estar activo

    // Validar en el archivo de órdenes las reglas de negocio
    fstream archivo("ordenes.bin", ios::in | ios::out | ios::binary);
    if (!archivo.is_open()) return false;
    
    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    int ordenesAbiertasMecanico = 0;
    ordenServicio temp;
    
    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(ordenServicio));
        if (!temp.eliminado && strcmp(temp.estado, "ABIERTA") == 0) {
            if (temp.idVehiculo == idVehiculo) {
                archivo.close();
                return false; // El vehículo ya tiene una orden abierta
            }
            if (temp.idMecanico == idMecanico) {
                ordenesAbiertasMecanico++;
            }
        }
    }
    
    if (ordenesAbiertasMecanico >= 3) {
        archivo.close();
        return false; // Límite de 3 órdenes por mecánico
    }
    if (kmIngreso < v.km) {
        archivo.close();
        return false;
    }

    // PASO 1: Actualizar kilometraje del vehículo
    v.km = kmIngreso;
    actualizarVehiculo(idVehiculo, v);

    // PASO 2: Construir y guardar la orden
    resultado.id = h.proximoID;
    resultado.idVehiculo = idVehiculo;
    resultado.idMecanico = idMecanico;
    strcpy(resultado.fechaApertura, fechaApertura);
    strcpy(resultado.fechaCierre, "");
    strcpy(resultado.estado, "ABIERTA");
    strcpy(resultado.descripcionFalla, descripcionFalla);
    resultado.kmIngreso = kmIngreso;
    resultado.manoObra = 0.0f;
    resultado.numDetalles = 0; // Inicia vacía
    resultado.eliminado = false;
    resultado.fechaCreacion = time(nullptr);
    resultado.fechaUltimaModificacion = resultado.fechaCreacion;

    archivo.clear();
    archivo.seekp(0, ios::end);
    archivo.write((char*)&resultado, sizeof(ordenServicio));

    h.cantidadRegistros++;
    h.registrosActivos++;
    h.proximoID++;
    archivo.seekp(0, ios::beg);
    archivo.write((char*)&h, sizeof(ArchivoHeader));
    
    archivo.close();
    return true;
}

bool agregarRepuestoAOrden(int idOrden, int idRepuesto, int cantidad) {
    ordenServicio o;
    if (!buscarOrdenPorID(idOrden, o) || strcmp(o.estado, "ABIERTA") != 0) return false;
    if (cantidad <= 0) return false;

    Repuesto r;
    if (!buscarRepuestoPorID(idRepuesto, r) || r.stock < cantidad) return false;

    r.stock -= cantidad;
    fstream archivoRepuestos("repuestos.bin", ios::in | ios::out | ios::binary);
    long posRep = calcularPosicion(idRepuesto, sizeof(Repuesto));
    archivoRepuestos.seekp(posRep, ios::beg);
    archivoRepuestos.write((char*)&r, sizeof(Repuesto));
    archivoRepuestos.close();

    bool encontrado = false;
    for (int i = 0; i < o.numDetalles; i++) {
        if (o.detalles[i].idRepuesto == idRepuesto) {
            o.detalles[i].cantidad += cantidad;
            encontrado = true;
            break;
        }
    }

    if (!encontrado) {
        if (o.numDetalles >= 15) return false;
        o.detalles[o.numDetalles].idRepuesto = idRepuesto;
        o.detalles[o.numDetalles].cantidad = cantidad;
        o.detalles[o.numDetalles].precioUnitario = r.precio; 
        o.numDetalles++;
    }

    fstream archivoOrdenes("ordenes.bin", ios::in | ios::out | ios::binary);
    long posOrd = calcularPosicion(idOrden, sizeof(ordenServicio));
    archivoOrdenes.seekp(posOrd, ios::beg);
    archivoOrdenes.write((char*)&o, sizeof(ordenServicio));
    archivoOrdenes.close();

    return true;
}

bool quitarRepuestoDeOrden(int idOrden, int idRepuesto) {
    ordenServicio o;
    if (!buscarOrdenPorID(idOrden, o) || strcmp(o.estado, "ABIERTA") != 0) return false;

    // Buscar en qué índice de la orden está el repuesto
    int index = -1;
    for (int i = 0; i < o.numDetalles; i++) {
        if (o.detalles[i].idRepuesto == idRepuesto) {
            index = i;
            break;
        }
    }
    if (index == -1) return false;

    // Devolver el stock al inventario
    Repuesto r;
    if (buscarRepuestoPorID(idRepuesto, r)) {
        r.stock += o.detalles[index].cantidad;
        r.fechaUltimaModificacion = time(nullptr);
        
        fstream archRep("datos/repuestos.bin", ios::in | ios::out | ios::binary);
        long posRep = calcularPosicion(idRepuesto, sizeof(Repuesto));
        archRep.seekp(posRep, ios::beg);
        archRep.write((char*)&r, sizeof(Repuesto));
        archRep.close();
    }

    // Eliminar la línea del arreglo estático desplazando los elementos
    for (int i = index; i < o.numDetalles - 1; i++) {
        o.detalles[i] = o.detalles[i + 1];
    }
    o.numDetalles--;
    o.fechaUltimaModificacion = time(nullptr);

    // Guardar la orden actualizada
    fstream archOrd("ordenes.bin", ios::in | ios::out | ios::binary);
    long posOrd = calcularPosicion(idOrden, sizeof(ordenServicio));
    archOrd.seekp(posOrd, ios::beg);
    archOrd.write((char*)&o, sizeof(ordenServicio));
    archOrd.close();

    return true;
}

bool cerrarOrdenServicio(int idOrden, const char* fechaCierre, float manoObra) {
    ordenServicio o;
    if (!buscarOrdenPorID(idOrden, o) || strcmp(o.estado, "ABIERTA") != 0) return false; 
    if (manoObra < 0) return false;

    // Debe quedar con al menos una linea o mano de obra > 0
    if (o.numDetalles == 0 && manoObra <= 0) return false;

    o.manoObra = manoObra;
    strcpy(o.fechaCierre, fechaCierre);
    strcpy(o.estado, "CERRADA");
    o.fechaUltimaModificacion = time(nullptr);

    // Guardar los cambios
    fstream archivo("ordenes.bin", ios::in | ios::out | ios::binary);
    long posicion = calcularPosicion(idOrden, sizeof(ordenServicio));
    archivo.seekp(posicion, ios::beg);
    archivo.write((char*)&o, sizeof(ordenServicio));
    archivo.close();

    return true;
}

bool anularOrdenServicio(int idOrden) {
    ordenServicio o;
    if (!buscarOrdenPorID(idOrden, o)) return false;
    
    // Solo ABIERTAS o CERRADAS pueden anularse
    if (strcmp(o.estado, "ABIERTA") != 0 && strcmp(o.estado, "CERRADA") != 0) {
        return false;
    }

    // Devolver al inventario las unidades de todas sus lineas
    fstream archRep("datos/repuestos.bin", ios::in | ios::out | ios::binary);
    if (archRep.is_open()) {
        for (int i = 0; i < o.numDetalles; i++) {
            Repuesto r;
            long posRep = calcularPosicion(o.detalles[i].idRepuesto, sizeof(Repuesto));
            
            // Leemos el repuesto específico
            archRep.seekg(posRep, ios::beg);
            archRep.read((char*)&r, sizeof(Repuesto));
            
            // Le devolvemos su stock
            r.stock += o.detalles[i].cantidad;
            r.fechaUltimaModificacion = time(nullptr);
            
            // Sobrescribimos en la misma posición
            archRep.seekp(posRep, ios::beg);
            archRep.write((char*)&r, sizeof(Repuesto));
        }
        archRep.close();
    }

    // Cambiamos el estado de la orden
    strcpy(o.estado, "ANULADA");
    o.fechaUltimaModificacion = time(nullptr);

    // Guardamos la orden anulada
    fstream archOrd("datos/ordenes.bin", ios::in | ios::out | ios::binary);
    long posOrd = calcularPosicion(idOrden, sizeof(ordenServicio));
    archOrd.seekp(posOrd, ios::beg);
    archOrd.write((char*)&o, sizeof(ordenServicio));
    archOrd.close();

    return true;
}

float calcularTotalOrden(const ordenServicio &o) {
    float totalRepuestos = 0;
    for (int i = 0; i < o.numDetalles; i++) {
        totalRepuestos += (o.detalles[i].cantidad * o.detalles[i].precioUnitario);
    }
    return o.manoObra + totalRepuestos;
}

bool buscarOrdenPorID(int id, ordenServicio &resultado) {
    fstream archivo("ordenes.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;
    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));
    
    if (id < 1 || id >= h.proximoID) {
        archivo.close();
        return false;
    }
    
    long posicion = calcularPosicion(id, sizeof(ordenServicio));
    archivo.seekg(posicion, ios::beg);
    archivo.read((char*)&resultado, sizeof(ordenServicio));
    archivo.close();
    return !resultado.eliminado;
}

bool listarOrdenesPorEstado(const char* estado, ordenServicio resultados[], int &cantidad) {
    cantidad = 0;
    fstream archivo("datos/ordenes.bin", ios::in | ios::binary);
    if (!archivo.is_open()) return false;

    ArchivoHeader h;
    archivo.read((char*)&h, sizeof(ArchivoHeader));

    ordenServicio temp;
    for (int i = 0; i < h.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(ordenServicio));
        // Si no esta eliminada, coincide el estado y no superamos el límite del arreglo
        if (!temp.eliminado && strcmp(temp.estado, estado) == 0 && cantidad < 100) {
            resultados[cantidad] = temp;
            cantidad++;
        }
    }
    archivo.close();
    return cantidad > 0;
}