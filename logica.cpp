#include "logica.h"
#include "persistencia.h"
#include <cstring>
#include <cctype>
#include <cstdio>
#include <string>
#include <ctime>
#include <fstream>

#ifdef _WIN32
    #include <direct.h>
#else
    #include <sys/stat.h>
#endif

using namespace std;

// ==================================================
// FUNCIONES AUXILIARES DE CADENAS (NO PERSISTENTES)
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

// ====================================
// CONFIGURACIÓN DEL SISTEMA Y TALLER
// ====================================

void inicializarSistemaArchivos() {
    #ifdef _WIN32
        _mkdir("datos");
    #else
        mkdir("datos", 0777);
    #endif

    inicializarArchivo("datos/vehiculos.bin");
    inicializarArchivo("datos/mecanicos.bin");
    inicializarArchivo("datos/repuestos.bin");
    inicializarArchivo("datos/ordenes.bin");
}

bool cargarTaller(Taller &t) {
    return cargarTallerArchivo(t);
}

void guardarTaller(const Taller &t) {
    guardarTallerArchivo(t);
}

// ====================
// MÓDULO DE VEHÍCULOS
// ==================== 

bool agregarVehiculo(Vehiculo &datos, string &motivoFallo) {
    if (datos.anio < 1950 || datos.anio > 2027) {
        motivoFallo = "El anio de fabricacion debe estar comprendido entre 1950 y 2027.";
        return false;
    }
    if (datos.kilometraje < 0) {
        motivoFallo = "El kilometraje no puede ser un valor negativo.";
        return false;
    }

    // Placa unica entre los vehículos activos
    Vehiculo existente;
    if (buscarVehiculoPorPlaca(datos.placa, existente)) {
        motivoFallo = "Ya existe un vehiculo activo registrado con la placa '" + string(datos.placa) + "'.";
        return false;
    }

    if (!guardarVehiculoCrudo(datos)) {
        motivoFallo = "No se pudo escribir en vehiculos.bin.";
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
        return false; // Según sección 11: La lógica retorna false y reporta si fue eliminado
    }

    return true;
}

bool buscarVehiculoPorPlaca(const char* placa, Vehiculo &resultado) {
    ArchivoHeader h;
    if (!leerHeader("datos/vehiculos.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Vehiculo v;
        if (leerVehiculoPorID(i, v)) {
            if (!v.eliminado && strcmp(v.placa, placa) == 0) {
                resultado = v;
                return true;
            }
        }
    }
    return false;
}

bool buscarVehiculosPorPropietario(const char* subcadena, Vehiculo resultados[], int &cantidad, bool &hayMas) {
    cantidad = 0;
    hayMas = false;

    ArchivoHeader h;
    if (!leerHeader("datos/vehiculos.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Vehiculo v;
        if (leerVehiculoPorID(i, v)) {
            if (!v.eliminado && contieneSubcadena(v.nombrePropietario, subcadena)) {
                if (cantidad < MAX_RESULTADOS) {
                    resultados[cantidad++] = v;
                } else {
                    hayMas = true;
                    break;
                }
            }
        }
    }
    return cantidad > 0;
}

bool listarVehiculos(Vehiculo resultados[], int &cantidad, bool &hayMas) {
    cantidad = 0;
    hayMas = false;

    ArchivoHeader h;
    if (!leerHeader("datos/vehiculos.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Vehiculo v;
        if (leerVehiculoPorID(i, v)) {
            if (!v.eliminado) {
                if (cantidad < MAX_RESULTADOS) {
                    resultados[cantidad++] = v;
                } else {
                    hayMas = true;
                    break;
                }
            }
        }
    }
    return cantidad > 0;
}

bool actualizarVehiculo(int id, const Vehiculo &datosNuevos, string &motivoFallo) {
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

    // Rechazado si el vehículo tiene ordenes en cualquier estado
    ArchivoHeader hOrd;
    if (leerHeader("datos/ordenes.bin", hOrd)) {
        for (int i = 1; i <= hOrd.cantidadRegistros; i++) {
            OrdenServicio o;
            if (leerOrdenPorID(i, o)) {
                if (o.idVehiculo == id) {
                    motivoFallo = "No se puede eliminar el vehiculo porque tiene registrada la orden #" +
                                  to_string(o.id) + " en estado " + o.estado + ".";
                    return false;
                }
            }
        }
    }

    if (!marcarVehiculoEliminado(id)) {
        motivoFallo = "Error al marcar el vehiculo como eliminado en disco.";
        return false;
    }
    return true;
}

// ====================
// MÓDULO DE MECÁNICOS
// ====================

bool especialidadValida(const char* esp) {
    return (strcmp(esp, "MOTOR") == 0 ||
            strcmp(esp, "FRENOS") == 0 ||
            strcmp(esp, "ELECTRICIDAD") == 0 ||
            strcmp(esp, "SUSPENSION") == 0 ||
            strcmp(esp, "GENERAL") == 0);
}

bool registrarMecanico(Mecanico &datos, string &motivoFallo) {
    if (!especialidadValida(datos.especialidad)) {
        motivoFallo = "Especialidad invalida. Debe ser MOTOR, FRENOS, ELECTRICIDAD, SUSPENSION o GENERAL.";
        return false;
    }

    // Regla 5.3: Cédula única
    Mecanico existente;
    if (buscarMecanicoPorCedula(datos.cedula, existente)) {
        motivoFallo = "Ya existe un mecanico registrado con la cedula '" + string(datos.cedula) + "'.";
        return false;
    }

    if (!guardarMecanicoCrudo(datos)) {
        motivoFallo = "Error de persistencia: no se pudo escribir en mecanicos.bin.";
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
        return false; // La logica retorna false para mecánicos inactivos en búsquedas estándar
    }
    return true;
}

bool obtenerMecanicoHistorico(int id, Mecanico &resultado) {
    // Obtiene un mecanico este activo o dado de baja 
    return leerMecanicoPorID(id, resultado, nullptr);
}

bool buscarMecanicoPorCedula(const char* cedula, Mecanico &resultado) {
    ArchivoHeader h;
    if (!leerHeader("datos/mecanicos.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Mecanico m;
        if (leerMecanicoPorID(i, m)) {
            if (!m.eliminado && strcmp(m.cedula, cedula) == 0) {
                resultado = m;
                return true;
            }
        }
    }
    return false;
}

int contarOrdenesAbiertasMecanico(int idMecanico) {
    int total = 0;
    ArchivoHeader h;
    if (!leerHeader("datos/ordenes.bin", h)) return 0;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        OrdenServicio o;
        if (leerOrdenPorID(i, o)) {
            if (!o.eliminado && o.idMecanico == idMecanico && strcmp(o.estado, "ABIERTA") == 0) {
                total++;
            }
        }
    }
    return total;
}

bool listarMecanicos(Mecanico resultados[], int &cantidad, bool &hayMas) {
    cantidad = 0;
    hayMas = false;

    ArchivoHeader h;
    if (!leerHeader("datos/mecanicos.bin", h)) return false;

    // Regla 5.3: Activos y dados de baja (marcados como tales)
    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Mecanico m;
        if (leerMecanicoPorID(i, m)) {
            if (cantidad < MAX_RESULTADOS) {
                resultados[cantidad++] = m;
            } else {
                hayMas = true;
                break;
            }
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

    // Regla 5.3: Rechazado si el mecánico tiene órdenes ABIERTAS
    int abiertas = contarOrdenesAbiertasMecanico(id);
    if (abiertas > 0) {
        motivoFallo = "No se puede dar de baja: el mecanico tiene " + to_string(abiertas) + " orden(es) ABIERTA(S).";
        return false;
    }

    if (!marcarMecanicoEliminado(id)) {
        motivoFallo = "Error al marcar la baja del mecanico en disco.";
        return false;
    }
    return true;
}

// ====================
// MÓDULO DE REPUESTOS
// ====================

bool registrarRepuesto(Repuesto &datos, string &motivoFallo) {
    if (datos.precio <= 0) {
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

    // Código único entre los repuestos activos
    Repuesto existente;
    if (buscarRepuestoPorCodigo(datos.codigo, existente)) {
        motivoFallo = "Ya existe un repuesto activo registrado con el codigo '" + string(datos.codigo) + "'.";
        return false;
    }

    if (!guardarRepuestoCrudo(datos)) {
        motivoFallo = "Error de persistencia: no se pudo escribir en repuestos.bin.";
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

bool buscarRepuestoPorCodigo(const char* codigo, Repuesto &resultado) {
    ArchivoHeader h;
    if (!leerHeader("datos/repuestos.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Repuesto r;
        if (leerRepuestoPorID(i, r)) {
            if (!r.eliminado && strcmp(r.codigo, codigo) == 0) {
                resultado = r;
                return true;
            }
        }
    }
    return false;
}

bool buscarRepuestoPorNombreParcial(const char* subcadena, Repuesto resultados[], int &cantidad, bool &hayMas) {
    cantidad = 0;
    hayMas = false;

    ArchivoHeader h;
    if (!leerHeader("datos/repuestos.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Repuesto r;
        if (leerRepuestoPorID(i, r)) {
            if (!r.eliminado && contieneSubcadena(r.nombre, subcadena)) {
                if (cantidad < MAX_RESULTADOS) {
                    resultados[cantidad++] = r;
                } else {
                    hayMas = true;
                    break;
                }
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

    r.stock += cantidadAdicional;
    if (!sobrescribirRepuesto(r)) {
        motivoFallo = "Error al actualizar el stock en el archivo de repuestos.";
        return false;
    }
    return true;
}

bool listarRepuestos(Repuesto resultados[], int &cantidad, bool &hayMas) {
    cantidad = 0;
    hayMas = false;

    ArchivoHeader h;
    if (!leerHeader("datos/repuestos.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        Repuesto r;
        if (leerRepuestoPorID(i, r)) {
            if (!r.eliminado) {
                if (cantidad < MAX_RESULTADOS) {
                    resultados[cantidad++] = r;
                } else {
                    hayMas = true;
                    break;
                }
            }
        }
    }
    return cantidad > 0;
}

bool cambiarPrecioRepuesto(int idRepuesto, float nuevoPrecio, string &motivoFallo) {
    if (nuevoPrecio <= 0) {
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
    if (leerHeader("datos/ordenes.bin", hOrd)) {
        for (int i = 1; i <= hOrd.cantidadRegistros; i++) {
            OrdenServicio o;
            if (leerOrdenPorID(i, o)) {
                for (int j = 0; j < o.numDetalles; j++) {
                    if (o.detalles[j].idRepuesto == id) {
                        motivoFallo = "No se puede eliminar: el repuesto figura en el detalle de la Orden #" +
                                      to_string(o.id) + " (Estado: " + o.estado + ").";
                        return false;
                    }
                }
            }
        }
    }

    if (!marcarRepuestoEliminado(id)) {
        motivoFallo = "Error al marcar el repuesto como eliminado en disco.";
        return false;
    }
    return true;
}

// ==============================
// MÓDULO DE ÓRDENES DE SERVICIO
// ==============================

bool abrirOrdenServicio(int idVehiculo, int idMecanico, const char* fechaApertura,
                       const char* descripcionFalla, int kilometrajeIngreso,
                       OrdenServicio &resultado, string &motivoFallo) {
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
        if (leerOrdenPorID(i, o)) {
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
    }

    if (abiertasMecanico >= 3) {
        motivoFallo = "El mecanico " + string(m.nombre) + " ya tiene 3 ordenes ABIERTAS asignadas (limite maximo alcanzado).";
        return false;
    }

    // Coordinación de escrituras: Orden primero luego Vehículo
    resultado.idVehiculo = idVehiculo;
    resultado.idMecanico = idMecanico;
    strcpy(resultado.fechaApertura, fechaApertura);
    strcpy(resultado.fechaCierre, "");
    strcpy(resultado.estado, "ABIERTA");
    strcpy(resultado.descripcionFalla, descripcionFalla);
    resultado.kilometrajeIngreso = kilometrajeIngreso;
    resultado.manoObra = 0.0f;
    resultado.numDetalles = 0;

    if (!guardarOrdenCruda(resultado)) {
        motivoFallo = "Error de persistencia: fallo al crear la orden en ordenes.bin.";
        return false;
    }

    // Actualizar kilometraje del vehiculo en vehiculos.bin
    v.kilometraje = kilometrajeIngreso;
    if (!sobrescribirVehiculo(v)) {
        motivoFallo = "Aviso: La orden fue creada pero hubo un fallo al actualizar el kilometraje en vehiculos.bin.";
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
    if (strcmp(o.estado, "ABIERTA") != 0) {
        motivoFallo = "La orden se encuentra " + string(o.estado) + ". Solo se pueden agregar repuestos a ordenes ABIERTAS.";
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
            break;
        }
    }

    if (idxExistente == -1 && o.numDetalles >= MAX_LINEAS_DETALLE) {
        motivoFallo = "Limite de " + to_string(MAX_LINEAS_DETALLE) + " lineas de detalle alcanzado para esta orden.";
        return false;
    }

    // Primero descontar repuesto del inventario, luego registrar en la orden
    r.stock -= cantidad;
    if (!sobrescribirRepuesto(r)) {
        motivoFallo = "Error de persistencia al descontar el stock en repuestos.bin.";
        return false;
    }

    if (idxExistente != -1) {
        o.detalles[idxExistente].cantidad += cantidad;
    } else {
        o.detalles[o.numDetalles].idRepuesto = idRepuesto;
        o.detalles[o.numDetalles].cantidad = cantidad;
        o.detalles[o.numDetalles].precioUnitario = r.precio; // Precio historico vigente
        o.numDetalles++;
    }

    if (!sobrescribirOrden(o)) {
        motivoFallo = "Error de persistencia al actualizar las lineas de la orden en ordenes.bin.";
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
    if (strcmp(o.estado, "ABIERTA") != 0) {
        motivoFallo = "La orden se encuentra " + string(o.estado) + ". Solo se pueden modificar ordenes ABIERTAS.";
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

    // Devolver unidades al stock en repuestos.bin
    Repuesto r;
    if (leerRepuestoPorID(idRepuesto, r)) {
        r.stock += o.detalles[idx].cantidad;
        sobrescribirRepuesto(r);
    }

    // Eliminar la línea desplazando los detalles
    for (int i = idx; i < o.numDetalles - 1; i++) {
        o.detalles[i] = o.detalles[i + 1];
    }
    o.numDetalles--;

    if (!sobrescribirOrden(o)) {
        motivoFallo = "Error al guardar los cambios en la orden.";
        return false;
    }
    return true;
}

bool cerrarOrdenServicio(int idOrden, const char* fechaCierre, float manoObra, string &motivoFallo) {
    OrdenServicio o;
    if (!leerOrdenPorID(idOrden, o)) {
        motivoFallo = "La orden #" + to_string(idOrden) + " no existe.";
        return false;
    }
    if (strcmp(o.estado, "ABIERTA") != 0) {
        motivoFallo = "La orden ya se encuentra " + string(o.estado) + ". Solo se pueden cerrar ordenes ABIERTAS.";
        return false;
    }
    if (manoObra < 0) {
        motivoFallo = "El costo de mano de obra no puede ser negativo.";
        return false;
    }
    if (o.numDetalles == 0 && manoObra <= 0) {
        motivoFallo = "Una orden no puede cerrarse vacia: debe incluir al menos un repuesto o mano de obra > 0.";
        return false;
    }

    o.manoObra = manoObra;
    strcpy(o.fechaCierre, fechaCierre);
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
    if (strcmp(o.estado, "ANULADA") == 0) {
        motivoFallo = "La orden ya se encuentra ANULADA.";
        return false;
    }
    if (strcmp(o.estado, "ABIERTA") != 0 && strcmp(o.estado, "CERRADA") != 0) {
        motivoFallo = "Estado de orden invalido para anulacion.";
        return false;
    }

    // Devolver al inventario las unidades de todas sus líneas
    for (int i = 0; i < o.numDetalles; i++) {
        Repuesto r;
        if (leerRepuestoPorID(o.detalles[i].idRepuesto, r)) {
            r.stock += o.detalles[i].cantidad;
            sobrescribirRepuesto(r);
        }
    }

    strcpy(o.estado, "ANULADA");
    if (!sobrescribirOrden(o)) {
        motivoFallo = "Error al cambiar el estado de la orden en disco.";
        return false;
    }
    return true;
}

float calcularTotalOrden(const OrdenServicio &o) {
    float totalRepuestos = 0.0f;
    for (int i = 0; i < o.numDetalles; i++) {
        totalRepuestos += (o.detalles[i].cantidad * o.detalles[i].precioUnitario);
    }
    return o.manoObra + totalRepuestos;
}

bool buscarOrdenPorID(int id, OrdenServicio &resultado, long *posicionBytes) {
    if (!leerOrdenPorID(id, resultado, posicionBytes)) {
        return false;
    }
    return !resultado.eliminado;
}

bool listarOrdenesPorEstado(const char* estado, OrdenServicio resultados[], int &cantidad, bool &hayMas) {
    cantidad = 0;
    hayMas = false;

    ArchivoHeader h;
    if (!leerHeader("datos/ordenes.bin", h)) return false;

    for (int i = 1; i <= h.cantidadRegistros; i++) {
        OrdenServicio o;
        if (leerOrdenPorID(i, o)) {
            if (!o.eliminado && strcmp(o.estado, estado) == 0) {
                if (cantidad < MAX_RESULTADOS) {
                    resultados[cantidad++] = o;
                } else {
                    hayMas = true;
                    break;
                }
            }
        }
    }
    return cantidad > 0;
}

// ===================================
// MÓDULO DE REPORTES Y MANTENIMIENTO
// ===================================

bool generarReporteHistorialVehiculo(const char* placa, Vehiculo &vEncontrado,
                                    OrdenServicio ordenesVehiculo[], int &cantOrdenes, float &totalFacturado) {
    if (!buscarVehiculoPorPlaca(placa, vEncontrado)) {
        return false;
    }

    cantOrdenes = 0;
    totalFacturado = 0.0f;

    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) return false;

    for (int i = 1; i <= hOrd.cantidadRegistros; i++) {
        OrdenServicio o;
        if (leerOrdenPorID(i, o)) {
            if (!o.eliminado && o.idVehiculo == vEncontrado.id) {
                if (cantOrdenes < 100) {
                    ordenesVehiculo[cantOrdenes++] = o;
                }
                if (strcmp(o.estado, "CERRADA") == 0) {
                    totalFacturado += calcularTotalOrden(o);
                }
            }
        }
    }
    return true;
}

bool generarReporteTopRepuestos(TopRepuesto top10[], int &cantEncontrados) {
    cantEncontrados = 0;
    for (int i = 0; i < 10; i++) {
        top10[i].unidades = 0;
        top10[i].codigo[0] = '\0';
        top10[i].nombre[0] = '\0';
    }

    ArchivoHeader hRep;
    if (!leerHeader("datos/repuestos.bin", hRep)) return false;

    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) return false;

    for (int i = 1; i <= hRep.cantidadRegistros; i++) {
        Repuesto r;
        if (!leerRepuestoPorID(i, r)) continue;

        int consumos = 0;
        for (int j = 1; j <= hOrd.cantidadRegistros; j++) {
            OrdenServicio o;
            if (leerOrdenPorID(j, o)) {
                if (!o.eliminado && strcmp(o.estado, "CERRADA") == 0) {
                    for (int k = 0; k < o.numDetalles; k++) {
                        if (o.detalles[k].idRepuesto == r.id) {
                            consumos += o.detalles[k].cantidad;
                        }
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

bool generarReporteFacturacionMecanicos(FilaFacturacionMecanico filas[], int &cantFilas, bool &hayMas) {
    cantFilas = 0;
    hayMas = false;

    ArchivoHeader hMec;
    if (!leerHeader("datos/mecanicos.bin", hMec)) return false;

    ArchivoHeader hOrd;
    if (!leerHeader("datos/ordenes.bin", hOrd)) return false;

    for (int i = 1; i <= hMec.cantidadRegistros; i++) {
        Mecanico m;
        if (!leerMecanicoPorID(i, m)) continue;

        FilaFacturacionMecanico fila;
        strcpy(fila.nombre, m.nombre);
        fila.dadoDeBaja = m.eliminado;
        fila.ordenesCerradas = 0;
        fila.totalManoObra = 0.0f;
        fila.totalRepuestos = 0.0f;

        for (int j = 1; j <= hOrd.cantidadRegistros; j++) {
            OrdenServicio o;
            if (leerOrdenPorID(j, o)) {
                if (!o.eliminado && o.idMecanico == m.id && strcmp(o.estado, "CERRADA") == 0) {
                    fila.ordenesCerradas++;
                    fila.totalManoObra += o.manoObra;
                    for (int d = 0; d < o.numDetalles; d++) {
                        fila.totalRepuestos += (o.detalles[d].cantidad * o.detalles[d].precioUnitario);
                    }
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

bool crearBackup(string &rutaGenerada) {
    time_t ahora = time(nullptr);
    tm* t = localtime(&ahora);
    char carpeta[128];
    sprintf(carpeta, "datos/backup_%d-%02d-%02d_%02d-%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
    rutaGenerada = carpeta;

    #ifdef _WIN32
        _mkdir(carpeta);
    #else
        mkdir(carpeta, 0777);
    #endif

    const char* archivos[] = {"taller.bin", "vehiculos.bin", "mecanicos.bin", "repuestos.bin", "ordenes.bin"};
    for (int i = 0; i < 5; i++) {
        string origen = "datos/" + string(archivos[i]);
        string destino = rutaGenerada + "/" + string(archivos[i]);

        ifstream src(origen, ios::binary);
        if (src.is_open()) {
            ofstream dst(destino, ios::binary);
            if (dst.is_open()) {
                dst << src.rdbuf();
                dst.close();
            }
            src.close();
        }
    }
    return true;
}

void verificarIntegridadSistema(string errores[], int &cantErrores, int contadores[]) {
    cantErrores = 0;
    for (int i = 0; i < 4; i++) contadores[i] = 0;

    // 1. Vehiculos
    int maxVehID = 0;
    ifstream fv("datos/vehiculos.bin", ios::binary);
    if (fv.is_open()) {
        fv.seekg(0, ios::end);
        long tamArchivo = fv.tellg();
        fv.seekg(0, ios::beg);

        ArchivoHeader h;
        fv.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));

        long bytesEsperados = static_cast<long>(sizeof(ArchivoHeader)) +
                              static_cast<long>(h.cantidadRegistros) * static_cast<long>(sizeof(Vehiculo));

        int reales = (tamArchivo >= static_cast<long>(sizeof(ArchivoHeader))) ?
                     static_cast<int>((tamArchivo - static_cast<long>(sizeof(ArchivoHeader))) / sizeof(Vehiculo)) : 0;
        contadores[0] = reales;
        maxVehID = h.proximoID - 1;

        if (h.cantidadRegistros != reales || tamArchivo != bytesEsperados) {
            errores[cantErrores++] = "ROTO: Header de vehiculos.bin indica " + to_string(h.cantidadRegistros) +
                                     " registros; el archivo contiene " + to_string(reales);
        }

        int activos = 0;
        for (int i = 0; i < reales; i++) {
            Vehiculo v;
            fv.read(reinterpret_cast<char*>(&v), sizeof(Vehiculo));
            if (!v.eliminado) activos++;
        }
        if (h.registrosActivos != activos) {
            errores[cantErrores++] = "ROTO: Header de vehiculos.bin indica " + to_string(h.registrosActivos) +
                                     " activos; se contaron " + to_string(activos);
        }
        fv.close();
    }

    // 2. Mecanicos
    int maxMecID = 0;
    ifstream fm("datos/mecanicos.bin", ios::binary);
    if (fm.is_open()) {
        fm.seekg(0, ios::end);
        long tamArchivo = fm.tellg();
        fm.seekg(0, ios::beg);

        ArchivoHeader h;
        fm.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));

        long bytesEsperados = static_cast<long>(sizeof(ArchivoHeader)) +
                              static_cast<long>(h.cantidadRegistros) * static_cast<long>(sizeof(Mecanico));

        int reales = (tamArchivo >= static_cast<long>(sizeof(ArchivoHeader))) ?
                     static_cast<int>((tamArchivo - static_cast<long>(sizeof(ArchivoHeader))) / sizeof(Mecanico)) : 0;
        contadores[1] = reales;
        maxMecID = h.proximoID - 1;

        if (h.cantidadRegistros != reales || tamArchivo != bytesEsperados) {
            errores[cantErrores++] = "ROTO: Header de mecanicos.bin indica " + to_string(h.cantidadRegistros) +
                                     " registros; el archivo contiene " + to_string(reales);
        }

        int activos = 0;
        for (int i = 0; i < reales; i++) {
            Mecanico m;
            fm.read(reinterpret_cast<char*>(&m), sizeof(Mecanico));
            if (!m.eliminado) activos++;
        }
        if (h.registrosActivos != activos) {
            errores[cantErrores++] = "ROTO: Header de mecanicos.bin indica " + to_string(h.registrosActivos) +
                                     " activos; se contaron " + to_string(activos);
        }
        fm.close();
    }

    // 3. Repuestos
    int maxRepID = 0;
    ifstream fr("datos/repuestos.bin", ios::binary);
    if (fr.is_open()) {
        fr.seekg(0, ios::end);
        long tamArchivo = fr.tellg();
        fr.seekg(0, ios::beg);

        ArchivoHeader h;
        fr.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));

        long bytesEsperados = static_cast<long>(sizeof(ArchivoHeader)) +
                              static_cast<long>(h.cantidadRegistros) * static_cast<long>(sizeof(Repuesto));

        int reales = (tamArchivo >= static_cast<long>(sizeof(ArchivoHeader))) ?
                     static_cast<int>((tamArchivo - static_cast<long>(sizeof(ArchivoHeader))) / sizeof(Repuesto)) : 0;
        contadores[2] = reales;
        maxRepID = h.proximoID - 1;

        if (h.cantidadRegistros != reales || tamArchivo != bytesEsperados) {
            errores[cantErrores++] = "ROTO: Header de repuestos.bin indica " + to_string(h.cantidadRegistros) +
                                     " registros; el archivo contiene " + to_string(reales);
        }

        int activos = 0;
        for (int i = 0; i < reales; i++) {
            Repuesto r;
            fr.read(reinterpret_cast<char*>(&r), sizeof(Repuesto));
            if (!r.eliminado) activos++;
            if (r.stock < 0) {
                errores[cantErrores++] = "ROTO: Repuesto ID " + to_string(r.id) +
                                         " tiene stock negativo (" + to_string(r.stock) + ")";
            }
        }
        if (h.registrosActivos != activos) {
            errores[cantErrores++] = "ROTO: Header de repuestos.bin indica " + to_string(h.registrosActivos) +
                                     " activos; se contaron " + to_string(activos);
        }
        fr.close();
    }

    // 4. Ordenes de servicio
    ifstream fo("datos/ordenes.bin", ios::binary);
    if (fo.is_open()) {
        fo.seekg(0, ios::end);
        long tamArchivo = fo.tellg();
        fo.seekg(0, ios::beg);

        ArchivoHeader h;
        fo.read(reinterpret_cast<char*>(&h), sizeof(ArchivoHeader));

        long bytesEsperados = static_cast<long>(sizeof(ArchivoHeader)) +
                              static_cast<long>(h.cantidadRegistros) * static_cast<long>(sizeof(OrdenServicio));

        int reales = (tamArchivo >= static_cast<long>(sizeof(ArchivoHeader))) ?
                     static_cast<int>((tamArchivo - static_cast<long>(sizeof(ArchivoHeader))) / sizeof(OrdenServicio)) : 0;
        contadores[3] = reales;

        if (h.cantidadRegistros != reales || tamArchivo != bytesEsperados) {
            errores[cantErrores++] = "ROTO: Header de ordenes.bin indica " + to_string(h.cantidadRegistros) +
                                     " registros; el archivo contiene " + to_string(reales);
        }

        int activos = 0;
        for (int i = 0; i < reales; i++) {
            OrdenServicio o;
            fo.read(reinterpret_cast<char*>(&o), sizeof(OrdenServicio));
            if (!o.eliminado) activos++;

            // Vehículo existe
            Vehiculo v;
            bool elimV = false;
            long posV = 0;
            if (o.idVehiculo < 1 || o.idVehiculo > maxVehID || !buscarVehiculoPorID(o.idVehiculo, v, elimV, posV)) {
                if (elimV) {
                    if (strcmp(o.estado, "ABIERTA") == 0) {
                        errores[cantErrores++] = "ROTO: Orden #" + to_string(o.id) + " esta ABIERTA pero su vehiculo ID " +
                                                 to_string(o.idVehiculo) + " no esta activo";
                    }
                } else {
                    errores[cantErrores++] = "ROTO: Orden #" + to_string(o.id) + " apunta al vehiculo ID " +
                                             to_string(o.idVehiculo) + " (no existe en vehiculos.bin)";
                }
            }

            // Mecánico existe y activo si la orden está abierta
            Mecanico m;
            if (o.idMecanico < 1 || o.idMecanico > maxMecID || !leerMecanicoPorID(o.idMecanico, m)) {
                errores[cantErrores++] = "ROTO: Orden #" + to_string(o.id) + " apunta al mecanico ID " +
                                         to_string(o.idMecanico) + " (no existe en mecanicos.bin)";
            } else if (strcmp(o.estado, "ABIERTA") == 0 && m.eliminado) {
                errores[cantErrores++] = "ROTO: Orden #" + to_string(o.id) + " esta ABIERTA pero su mecanico ID " +
                                         to_string(o.idMecanico) + " no esta activo";
            }

            // Lineas de detalle cuyo repuesto no existe
            for (int j = 0; j < o.numDetalles; j++) {
                int idR = o.detalles[j].idRepuesto;
                Repuesto r;
                if (idR < 1 || idR > maxRepID || !leerRepuestoPorID(idR, r)) {
                    errores[cantErrores++] = "ROTO: Orden #" + to_string(o.id) + " contiene en su detalle el repuesto ID " +
                                             to_string(idR) + " (no existe en repuestos.bin)";
                }
            }
        }
        if (h.registrosActivos != activos) {
            errores[cantErrores++] = "ROTO: Header de ordenes.bin indica " + to_string(h.registrosActivos) +
                                     " activos; se contaron " + to_string(activos);
        }
        fo.close();
    }
}
