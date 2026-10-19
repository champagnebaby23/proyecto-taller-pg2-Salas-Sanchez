#include "logica.h"
#include <iostream>
#include <cstring>
#include <cctype>

using namespace std;

// FUNCIONES AUXILIARES
void aMinusculas(const char* origen, char* destino) {
    int i = 0;
    while (origen[i] != '\0') {
        destino[i] = tolower(origen[i]);
        i++;
    }
    destino[i] = '\0';
}

bool contieneSubcadena(const char* texto, const char* subcadena) {
    char t[100], sub[100];
    aMinusculas(texto, t);
    aMinusculas(subcadena, sub);
    return strstr(t, sub) != nullptr;
}

void inicializarSistema(sysTaller &s, const char* nombre, const char* rif, const char* dir, const char* tel) {

    strcpy(s.taller.nombre, nombre);
    strcpy(s.taller.rif, rif);
    strcpy(s.taller.direccion, dir);
    strcpy(s.taller.telefono, tel);

    s.capacidadVehiculos = 4;
    s.numVehiculos = 4;
    s.vehiculos = new Vehiculo[s.capacidadVehiculos];

    s.capacidadMecanicos = 4;
    s.numMecanicos = 4;
    s.mecanicos = new Mecanico[s.capacidadMecanicos];

    s.capacidadRepuestos = 4;
    s.numRepuestos = 4;
    s.repuestos = new Repuesto[s.capacidadRepuestos];

    s.capacidadOrdenes = 4;
    s.numOrdenes = 4;
    s.ordenes = new ordenServicio[s.capacidadOrdenes];

    s.siguienteIdVehiculo = 1;
    s.siguienteIdMecanico = 1;
    s.siguienteIdRepuesto = 1;
    s.siguienteIdOrden = 1;
}

void liberarSistema(sysTaller &s) {

    for (int i=0; i < s.numOrdenes; i++) {
        delete[] s.ordenes[i].detalles;
    }
    delete[] s.ordenes;
    delete[] s.vehiculos;
    delete[] s.mecanicos;
    delete[] s.repuestos;

    s.vehiculos = nullptr;
    s.mecanicos = nullptr;
    s.repuestos = nullptr;
    s.ordenes = nullptr;
}

Vehiculo* buscarVehiculoPorID(sysTaller &s, int id) {
    for (int i=0; i < s.numVehiculos; i++) {
        if (s.vehiculos[i].id == id) {
            return &s.vehiculos[i];
            }
        }
    return nullptr;
    }

Vehiculo* buscarVehiculoPorPlaca(sysTaller &s, const char* placa) {
    for (int i = 0; i < s.numVehiculos; i++) {
        if (strcmp(s.vehiculos[i].placa, placa) == 0) {
            return &s.vehiculos[i];
        }
    }
    return nullptr;
}

Vehiculo** buscarVehiculosPorPropietario(sysTaller &s, const char* subcadena, int &cantidad) {
    cantidad = 0;
    for (int i = 0; i < s.numVehiculos; i++) {
        if (contieneSubcadena(s.vehiculos[i].nombrePropietario, subcadena)) {
            cantidad++;
        }
    }
    if (cantidad == 0) return nullptr;

    Vehiculo** resultado = new Vehiculo*[cantidad];
    int idx = 0;
    for (int i = 0; i < s.numVehiculos; i++) {
        if (contieneSubcadena(s.vehiculos[i].nombrePropietario, subcadena)) {
            resultado[idx++] = &s.vehiculos[i];
        }
    }
    return resultado;
}

Vehiculo** listarVehiculos(sysTaller &s, int &cantidad) {
    cantidad = s.numVehiculos;
    if (cantidad == 0) return nullptr;

    Vehiculo** resultado = new Vehiculo*[cantidad];
    for (int i = 0; i < cantidad; i++) {
        resultado[i] = &s.vehiculos[i];
    }
    return resultado;
}

Vehiculo* agregarVehiculo(sysTaller &s, Vehiculo datos) {

    if (buscarVehiculoPorPlaca(s, datos.placa) != nullptr) {
        return nullptr;
    }

    if (datos.anio < 1950 || datos.anio > 2027 || datos.km < 0) {
        return nullptr;
    }

    if (s.numVehiculos >= s.capacidadVehiculos) {
        int nuevaCapacidad = s.capacidadVehiculos * 2;
        Vehiculo* nuevoArreglo = new Vehiculo[nuevaCapacidad];

    for (int i = 0; i < s.numVehiculos; i++) {
            nuevoArreglo[i] = s.vehiculos[i];
    }

    delete[] s.vehiculos;
    s.vehiculos = nuevoArreglo;
    s.capacidadVehiculos = nuevaCapacidad;
    }

datos.id = s.siguienteIdVehiculo++;
s.vehiculos[s.numVehiculos] = datos;

return &s.vehiculos[s.numVehiculos++];
}

bool especialidadValida(const char* esp) {
    return (strcmp(esp, "MOTOR") == 0 || 
            strcmp(esp, "FRENOS") == 0 || 
            strcmp(esp, "ELECTRICIDAD") == 0 || 
            strcmp(esp, "SUSPENSION") == 0 || 
            strcmp(esp, "GENERAL") == 0);
}

Mecanico* buscarMecanicoPorID(sysTaller &s, int id) {
    for (int i = 0; i < s.numMecanicos; i++) {
        if (s.mecanicos[i].id == id) {
            return &s.mecanicos[i];
        }
    }
    return nullptr;
}

Mecanico* registrarMecanico(sysTaller &s, Mecanico datos) {
    if (buscarMecanicoPorCedula(s, datos.cedula) != nullptr) {
        return nullptr; // Ya existe
    }

    if (!especialidadValida(datos.especialidad)) {
        return nullptr;
    }

    if (strlen(datos.nombre) == 0 || strlen(datos.fechaIngreso) == 0) {
        return nullptr;
    }

    if (s.numMecanicos >= s.capacidadMecanicos) {
        int nuevaCapacidad = s.capacidadMecanicos * 2;
        Mecanico* nuevoArreglo = new Mecanico[nuevaCapacidad];

        for (int i = 0; i < s.numMecanicos; i++) {
            nuevoArreglo[i] = s.mecanicos[i];
        }

        delete[] s.mecanicos;
        s.mecanicos = nuevoArreglo;
        s.capacidadMecanicos = nuevaCapacidad;
    }

    datos.id = s.siguienteIdMecanico++;
    s.mecanicos[s.numMecanicos] = datos;

    return &s.mecanicos[s.numMecanicos++];
}

Mecanico** listarMecanicos(sysTaller &s, int &cantidad) {
    cantidad = s.numMecanicos;
    if (cantidad == 0) return nullptr;

    Mecanico** resultado = new Mecanico*[cantidad];
    for (int i = 0; i < cantidad; i++) {
        resultado[i] = &s.mecanicos[i];
    }
    return resultado;
}

Repuesto* buscarRepuestoPorID(sysTaller &s, int id) {
    for (int i = 0; i < s.numRepuestos; i++) {
        if (s.repuestos[i].id == id) {
            return &s.repuestos[i];
        }
    }
    return nullptr;
}

Repuesto* buscarRepuestoPorCodigo(sysTaller &s, const char* codigo) {
    for (int i = 0; i < s.numRepuestos; i++) {
        if (strcmp(s.repuestos[i].codigo, codigo) == 0) {
            return &s.repuestos[i];
        }
    }
    return nullptr;
}

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
    s.repuestos[s.numRepuestos] = datos;

    return &s.repuestos[s.numRepuestos++];
}

bool cambiarPrecioRepuesto(sysTaller &s, int idRepuesto, float nuevoPrecio) {
    if (nuevoPrecio <= 0) return false;

    Repuesto* r = buscarRepuestoPorID(s, idRepuesto);
    if (r == nullptr) return false;

    r->precio = nuevoPrecio;
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

ordenServicio* buscarOrdenPorID(sysTaller &s, int id) {
    for (int i = 0; i < s.numOrdenes; i++) {
        if (s.ordenes[i].id == id) return &s.ordenes[i];
    }
    return nullptr;
}

float calcularTotalOrden(const ordenServicio &o) {
    float totalRepuestos = 0;
    for (int i = 0; i < o.numDetalles; i++) {
        totalRepuestos += (o.detalles[i].cantidad * o.detalles[i].precioUnitario);
    }
    return o.manoObra + totalRepuestos;
}

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