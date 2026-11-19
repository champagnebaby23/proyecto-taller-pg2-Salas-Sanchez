# Proyecto 2 - Taller mecanico con persistencia

## Compilacion y ejecucion

Desde la carpeta del proyecto:

```sh
g++ -std=c++11 -Wall -Wextra -pedantic main.cpp logica.cpp persistencia.cpp -o taller
./taller
```

Los archivos de entidades se guardan en `datos/`. La primera ejecucion solicita los datos del taller; las siguientes los leen desde `datos/taller.bin`.

## Formato binario y acceso aleatorio

Las estructuras persistidas usan arreglos `char` de capacidad fija, tipos numericos, `bool` y `time_t`; no contienen `std::string` ni punteros. En el entorno de compilacion probado (Linux, ABI de 64 bits), `sizeof` reporta:

| Estructura | Bytes |
|---|---:|
| `ArchivoHeader` | 16 |
| `Taller` | 360 |
| `Vehiculo` | 288 |
| `Mecanico` | 176 |
| `Repuesto` | 208 |
| `DetalleOrden` | 12 |
| `OrdenServicio` | 464 |
| `TopRepuesto` | 124 |
| `FilaFacturacionMecanico` | 116 |

Estos valores dependen del compilador, la arquitectura y la ABI; deben comprobarse en el entorno de entrega. `taller.bin` contiene un unico `Taller`, sin header. Los otros cuatro archivos comienzan con `ArchivoHeader`.

Los registros se agregan al final y nunca se compactan ni reordenan. Por eso el ID coincide con el indice fisico mas uno:

```text
indiceFisico = id - 1
offset = sizeof(ArchivoHeader) + indiceFisico * sizeof(Estructura)
```

Una eliminacion es logica y no crea huecos. La relacion dejaria de ser valida si se borraran registros fisicamente, se reordenaran o se insertaran registros con IDs no consecutivos.

## Diseno del detalle de orden

`OrdenServicio` contiene `DetalleOrden detalles[15]` y `numDetalles`. Es un arreglo fijo: todas las ordenes ocupan `sizeof(OrdenServicio) == 464` bytes, tengan 1 o 15 lineas. No se agrega un archivo de detalles. Cada linea conserva ID del repuesto, cantidad y precio unitario historico.

| Lineas utilizadas | Bytes de la orden en disco |
|---:|---:|
| 1 | 464 |
| 15 | 464 |

El espacio no utilizado queda reservado en el registro, a cambio de una lectura directa de una orden completa. Se descarto conservar `DetalleOrden*`: el valor persistido seria una direccion de memoria invalida al reiniciar. Tambien se considero un archivo aparte de lineas; aunque permite crecer sin reservar 15 lineas por orden, exige coordinar IDs, lecturas, backup e integridad en mas archivos.

Para quitar una linea se devuelve al inventario la cantidad completa, se desplazan las lineas siguientes una posicion, se decrementa `numDetalles` y se sobrescribe la orden. Al agregar el mismo repuesto se acumula la cantidad y se conserva el precio capturado cuando se agrego por primera vez.

## Operaciones de persistencia y logica

Las cuatro entidades con header (`Vehiculo`, `Mecanico`, `Repuesto` y `OrdenServicio`) exponen el mismo conjunto de operaciones en `persistencia.h`:

```cpp
bool guardarVehiculoCrudo(Vehiculo&, string&);
bool leerVehiculoPorID(int, Vehiculo&, long* = nullptr);
bool sobrescribirVehiculo(const Vehiculo&);
bool marcarVehiculoEliminado(int, string&);
int contarVehiculosActivos();

bool guardarMecanicoCrudo(Mecanico&, string&);
bool leerMecanicoPorID(int, Mecanico&, long* = nullptr);
bool sobrescribirMecanico(const Mecanico&);
bool marcarMecanicoEliminado(int, string&);
int contarMecanicosActivos();

bool guardarRepuestoCrudo(Repuesto&, string&);
bool leerRepuestoPorID(int, Repuesto&, long* = nullptr);
bool sobrescribirRepuesto(const Repuesto&);
bool marcarRepuestoEliminado(int, string&);
int contarRepuestosActivos();

bool guardarOrdenCruda(OrdenServicio&, string&);
bool leerOrdenPorID(int, OrdenServicio&, long* = nullptr);
bool sobrescribirOrden(const OrdenServicio&);
bool marcarOrdenEliminada(int, string&);
int contarOrdenesActivas();
```

Al iniciar, cada archivo de entidad se valida contra el tamaño físico: el header debe ser válido, no debe haber registros parciales y el número de registros debe coincidir con `cantidadRegistros`. Los appends repiten esa comprobación; si falla la escritura del registro o del header, intentan restaurar el tamaño y el header originales e informan si la reversión no fue completa.

Al guardar, la persistencia asigna el ID en el objeto de salida. Las lecturas por ID usan el offset calculado y devuelven el registro incluso si esta eliminado; el filtrado por estado pertenece a la logica. Los `bool` permiten distinguir exito de fallo. Las operaciones auxiliares `asegurarDirectorioDatos`, `inicializarArchivo`, `leerHeader`, `inspeccionarArchivoEntidad` y `actualizarHeader` reportan fallo con `bool`; las copias de backup tambien devuelven la causa mediante `string& motivoFallo`.

Las firmas de la capa de logica en `logica.h` se agrupan asi:

```cpp
bool inicializarSistemaArchivos(string&);
bool cargarTaller(Taller&, bool& primeraEjecucion, string&);
bool guardarTaller(const Taller&, string&);
bool fechaValidaISO(const char*);

bool agregarVehiculo(Vehiculo&, string&);
bool buscarVehiculoPorID(int, Vehiculo&, bool& fueEliminado, long& offset);
bool buscarVehiculoPorPlaca(const char*, Vehiculo&, string&);
bool buscarVehiculosPorPropietario(const char*, Vehiculo[], int&, bool& hayMas, string&);
bool listarVehiculos(Vehiculo[], int&, bool& hayMas, string&);
bool actualizarVehiculo(int, const Vehiculo&, string&);
bool eliminarVehiculo(int, string&);

bool registrarMecanico(Mecanico&, string&);
bool buscarMecanicoPorID(int, Mecanico&, bool& fueEliminado, long* offset = nullptr);
bool obtenerMecanicoHistorico(int, Mecanico&);
bool buscarMecanicoPorCedula(const char*, Mecanico&, string&);
bool listarMecanicos(Mecanico[], int&, bool& hayMas, string&);
bool contarOrdenesAbiertasMecanico(int, int& total, string&);
bool eliminarMecanico(int, string&);

bool registrarRepuesto(Repuesto&, string&);
bool buscarRepuestoPorID(int, Repuesto&, bool& fueEliminado, long* offset = nullptr);
bool obtenerRepuestoHistorico(int, Repuesto&);
bool buscarRepuestoPorCodigo(const char*, Repuesto&, string&);
bool buscarRepuestoPorNombreParcial(const char*, Repuesto[], int&, bool& hayMas, string&);
bool registrarEntradaMercancia(int, int, string&);
bool listarRepuestos(Repuesto[], int&, bool& hayMas, string&);
bool cambiarPrecioRepuesto(int, float, string&);
bool eliminarRepuesto(int, string&);

bool abrirOrdenServicio(int, int, const char*, const char*, int, OrdenServicio&, string&);
bool agregarRepuestoAOrden(int, int, int, string&);
bool quitarRepuestoDeOrden(int, int, string&);
bool cerrarOrdenServicio(int, float, string&);
bool anularOrdenServicio(int, string&);
bool validarOrdenServicio(const OrdenServicio&);
bool validarDetalleOrden(const OrdenServicio&);
bool calcularTotalOrden(const OrdenServicio&, float& total);
bool buscarOrdenPorID(int, OrdenServicio&, long* offset = nullptr);
bool listarOrdenesPorEstado(const char*, OrdenServicio[], int&, bool& hayMas, string&);

bool generarReporteHistorialVehiculo(const char*, Vehiculo&, int desdeOrden,
                                    OrdenServicio[], int&, bool& hayMas, float& totalFacturado,
                                    string&);
bool generarReporteTopRepuestos(TopRepuesto[], int&, string&);
bool generarReporteFacturacionMecanicos(FilaFacturacionMecanico[], int&, bool& hayMas, string&);
bool crearBackup(string& ruta, string& motivoFallo);
void verificarIntegridadSistema(vector<string>& errores, int contadores[]);
```

Los `bool` de negocio informan exito/fallo y, en operaciones modificadoras, `string& motivoFallo` explica la causa. Las busquedas por ID entregan una copia; las de vehiculo/mecanico/repuesto tambien distinguen un ID eliminado. Los listados entregan cantidad y `hayMas`. El historial de vehiculo pagina con `desdeOrden` y `hayMas` para recorrer todas las ordenes sin almacenarlas todas a la vez. El cierre calcula su propia fecha actual y ya no recibe una fecha de la interfaz.

Las busquedas/listados devuelven copias en arreglos con una cantidad maxima fija y una bandera `hayMas`. La presentacion muestra el aviso cuando hay mas resultados.

## Escrituras compuestas y recuperacion

Las validaciones se ejecutan antes de escribir. Las operaciones compuestas tambien intentan compensar las escrituras previas si una escritura posterior falla, y notifican si la compensacion no pudo completarse. Esto no equivale a una transaccion durable: una terminacion abrupta del proceso o del equipo puede ocurrir entre dos escrituras.

- **Agregar repuesto:** primero se descuenta el stock y luego se guarda el detalle. Si la segunda escritura falla mientras el proceso sigue activo, se intenta restaurar el stock. Si el equipo se apaga entre ambas, el stock puede quedar reducido sin linea persistida.
- **Quitar repuesto:** primero se devuelve el stock y luego se guarda la orden sin la linea. Si falla la segunda escritura, se intenta restaurar el stock y la orden original. Una interrupcion entre ambas puede dejar una linea aun visible con el stock ya devuelto.
- **Anular orden:** se devuelven las cantidades, luego se cambia el estado. Si una escritura detecta un fallo, se intenta revertir las anteriores. Una interrupcion abrupta durante la devolucion requiere verificar y reconciliar los archivos antes de reintentar, para no devolver dos veces las mismas unidades.
- **Abrir orden:** se actualiza el kilometraje y luego se crea la orden; si la creacion detecta un fallo, se intenta restaurar el kilometraje.

`fstream` no ofrece atomicidad entre archivos. Una recuperacion automatica ante apagones requeriria un journal/transaccion persistente, fuera del formato solicitado. En agregar, el orden implementado (stock y luego detalle) puede dejar una disminucion de stock sin una linea que la explique; guardar primero el detalle dejaria evidencia directa para detectar el stock no descontado. Despues de una interrupcion se debe ejecutar la verificacion de integridad antes de continuar.

## Preguntas de analisis

1. **Persistir un `DetalleOrden*` del Proyecto 1:** al escribirlo se guardaria la direccion de memoria, no los objetos apuntados. Al cerrar el programa esa direccion deja de ser valida; al leerla en otra ejecucion no se recuperan las lineas y podria apuntar a memoria no asignada.
2. **Interrupcion entre detalle y stock:** con el orden elegido (stock y despues detalle), una interrupcion intermedia deja el stock descontado sin el detalle que explique el consumo. Si se guardara primero el detalle, quedaria la linea y el stock sin descontar; esa linea ofrece evidencia mas directa para detectar y corregir el descuadre. En ambos casos hace falta reconciliacion; la compensacion del programa cubre fallos detectados, no un apagado abrupto.
3. **Eliminar fisicamente un mecanico con ordenes:** las ordenes conservarian un ID que ya no permitiria recuperar su nombre, rompiendo la integridad del historial. La baja logica conserva el registro y permite mostrar el nombre en ordenes cerradas o anuladas.

## Pruebas y transcripciones de los casos de uso

La carpeta `pruebas/` contiene un par `entrada_N*.txt` / `salida_N*.txt` para cada ejecucion. Las letras indican reinicios del programa:

| Caso | Ejecuciones | Comprobacion principal |
|---:|---|---|
| 1 | `1a`, `1b` | Persistencia de vehiculos, mecanicos y repuestos con los mismos IDs |
| 2 | `2a`, `2b` | Acceso directo por ID, offset en bytes y aviso de baja logica |
| 3 | `3a`, `3b`, `3c` | Detalle persistente, acumulacion, devolucion de stock y precio historico |
| 4 | `4a`, `4b` | Persistencia de 15 lineas en una orden |
| 5 | `5a`, `5b`, `5c` | Anulacion de orden cerrada y stock restaurado tras reiniciar |
| 6 | `6a`, `6b` | Baja rechazada con orden abierta, historial conservado y rechazo de nuevas ordenes |
| 7 | `7a`, `7b` | Restauracion parcial de vehiculos y deteccion de referencia rota |
| 8 | `8` | Tres reportes; se listan estados para comprobar que abiertas/anuladas no se facturan |

`pruebas/regresiones.cpp` verifica tambien el limite de 15 lineas, el precio historico, el cierre con fecha actual, la anulacion y devolucion de stock, el backup y la integridad. Para compilar, ejecutar los ocho casos en directorios aislados y regenerar las salidas capturadas:

```sh
./pruebas/ejecutar_casos.sh
```

El caso 7 restaura automaticamente `vehiculos.bin` desde el backup creado durante `7a`; el caso 8 trabaja sobre una copia de la carpeta `datos/` pre-poblada del proyecto. El script no modifica los datos de entrega.
