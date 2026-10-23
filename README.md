# Sistema de Gestión de Taller Mecánico

### *1. Descripción del Sistema e Integrantes*

Este proyecto es un sistema integral para la administración de un taller mecánico desarrollado en C++ que permite gestionar un inventario de repuestos, administrar mecánicos y vehículos, y operar un flujo completo de órdenes de servicio. Todo el manejo de datos se realiza de forma temporal en la memoria RAM utilizando arreglos dinámicos, punteros, y gestión manual de memoria (operadores new y delete), garantizando en todo momento la integridad referencial de las entidades.

## Integrantes del equipo:

Luis Diego Salas (C.I: 32.364.203)

Andres Sánchez (C.I: 32.422.856)

### *2. Instrucciones de Compilación y Ejecución*

El proyecto está diseñado para ser compilado usando g++. Se han separado los archivos para modularizar la solución.

*Paso 1: Compilar el código fuente*

Abre una terminal en el directorio raíz del proyecto y ejecuta el siguiente comando para compilar el programa y enlazar la lógica con la presentación:

``g++ main_2.cpp logica.cpp -o main``


*Paso 2: Ejecutar de forma interactiva*

Para iniciar el sistema de menús y usarlo manualmente:

``./main``


*Paso 3: Ejecución mediante casos de prueba*

Para evaluar el programa utilizando el script automatizado, ejecuta:

`./main < pruebas/entrada.txt > pruebas/salida.txt`


### 3. Diagrama de las Estructuras y sus Relaciones

El sistema centraliza todas las entidades en la estructura sysTaller. A continuación se muestra cómo se relacionan lógicamente, haciendo énfasis en la relación de memoria anidada entre ordenServicio y sus detalles.

  ```[sysTaller]
  │
  ├──> (struct) Taller (Datos generales)
  │
  ├──> (Arreglo Dinámico) Vehiculo*
  │     └── [id, placa, datosPropietario...]
  │
  ├──> (Arreglo Dinámico) Mecanico*
  │     └── [id, nombre, especialidad...]
  │
  ├──> (Arreglo Dinámico) Repuesto*
  │     └── [id, codigo, stock, precio...]
  │
  └──> (Arreglo Dinámico) ordenServicio*
        ├── [id, estado, manoObra...]
        ├── Relación 1:1 -> idVehiculo (Refiere a un Vehículo)
        ├── Relación 1:1 -> idMecanico (Refiere a un Mecánico)
        │
        └──> (Arreglo Dinámico Anidado) detalleOrden* detalles
              ├── [idRepuesto, cantidad, precioUnitario]
              ├── [idRepuesto, cantidad, precioUnitario]
              └── Relación N:1 -> idRepuesto (Refiere a Inventario)
```

### 4. Explicación de la Separación de Capas

El proyecto implementa una arquitectura estricta de dos capas para separar responsabilidades y facilitar el mantenimiento:

*Capa de Presentación (`main.cpp`)*: Es la única parte del programa que interactúa con el usuario. Se encarga de mostrar los menús, leer entradas con cin, validar formatos básicos de teclado, e imprimir datos con cout. No toma decisiones de negocio ni modifica datos. Llama a las funciones de lógica y, basándose en lo que estas retornan (nullptr, bool, etc.), imprime mensajes de éxito o error. También es responsable de liberar (con delete[]) los arreglos temporales de punteros que recibe al hacer listados.

*Capa de Lógica (`logica.cpp`, `logica.h`)*: Es el cerebro del sistema, no contiene ningún cin o cout. Se encarga de gestionar la memoria en el Heap, validar reglas de negocio y mantener la integridad referencial. Retorna punteros hacia las entidades modificadas o valores booleanos para confirmar operaciones.

### 5. Firmas Diseñadas y Justificación de Retorno

Siguiendo el patrón del módulo de vehículos, diseñamos las siguientes firmas para los demás módulos:

### Módulo de Mecánicos

`Mecanico* registrarMecanico(sysTaller &s, Mecanico datos);`

`Mecanico* buscarMecanicoPorID(sysTaller &s, int id);`

`Mecanico* buscarMecanicoPorCedula(sysTaller &s, const char* cedula);`

`Mecanico** listarMecanicos(sysTaller &s, int &cantidad);`

`bool especialidadValida(const char* esp);`
`
### Módulo de Repuestos

`Repuesto* registrarRepuesto(sysTaller &s, Repuesto datos);`

`Repuesto* buscarRepuestoPorID(sysTaller &s, int id);`

`Repuesto* buscarRepuestoPorCodigo(sysTaller &s, const char* codigo);`

`bool registrarEntradaMercancia(sysTaller &s, int idRepuesto, int cantidadAdicional);`

`Repuesto** listarRepuestos(sysTaller &s, int &cantidad);`

`bool cambiarPrecioRepuesto(sysTaller &s, int idRepuesto, float nuevoPrecio);`

`bool eliminarRepuesto(sysTaller &s, int id);`

### Módulo de Órdenes de Servicio

`ordenServicio* abrirOrdenServicio(sysTaller &s, int idVehiculo, int idMecanico, const char* fechaApertura, const char* descripcionFalla, int kmIngreso);`

`bool agregarRepuestoAOrden(sysTaller &s, int idOrden, int idRepuesto, int cantidad);`

`bool quitarRepuestoDeOrden(sysTaller &s, int idOrden, int idRepuesto);`

`bool cerrarOrdenServicio(sysTaller &s, int idOrden, const char* fechaCierre, float manoObra);`

`bool anularOrdenServicio(sysTaller &s, int idOrden);`

`float calcularTotalOrden(const ordenServicio &o);`

`ordenServicio* buscarOrdenPorID(sysTaller &s, int id);`

`ordenServicio** listarOrdenesPorEstado(sysTaller &s, const char* estado, int &cantidad);`
`

### Justificación de los tipos de retorno elegidos:

`Retorno Entidad*`: Permite saber rápidamente si la operación falló (retornando nullptr por validaciones de negocio) o triunfó (devolviendo la dirección de memoria para que la presentación muestre el ID autogenerado).

`Retorno bool`: Como la presentación solo necesita saber si el proceso fue exitoso o violó una regla de negocio (ej. borrar un repuesto que está en una orden), un booleano es el flag perfecto para activar el mensaje de error correspondiente.

`Retorno Entidad**`: Retornamos un arreglo de punteros (doble puntero) para entregar una lista de referencias a los registros en lugar de duplicar las estructuras pesadas en la RAM. La presentación lo usa para iterar y luego destruye solo la lista temporal.

`Retorno float`: Utilizado en calcularTotalOrden porque el total no se almacena, sino que se calcula bajo demanda.

### 6. Respuestas a las Preguntas de Análisis

1. `agregarVehiculo` retorna un puntero a un elemento de `s->vehiculos`. Si guardan ese puntero en una variable, registran varios vehículos más y luego lo usan, ¿qué puede ocurrir y por qué?
Al registrar varios vehículos más, el arreglo dinámico alcanzará su capacidad máxima y se activará la función de redimensionamiento. Esta función asigna un nuevo bloque de memoria, copia los datos y ejecuta `delete[]` sobre el arreglo viejo. El puntero que habíamos guardado originalmente quedará apuntando a esa zona de memoria antigua que ya fue destruida y devuelta al sistema operativo, por lo que intentar acceder a ella corromperá el programa.

2. ¿Por qué `DetalleOrden` guarda `precioUnitario` en lugar de consultar el precio del repuesto cada vez que se calcula el total?
Porque el precio de los repuestos fluctúa con el tiempo, pero una orden de servicio es una factura. Si no se guardara el precio unitario exacto en el momento de agregar el repuesto, un incremento posterior en el inventario modificaría automáticamente el total de las órdenes cerradas en el pasado.

3. Cuando se redimensiona el arreglo de órdenes, ¿qué ocurre con el arreglo de detalles de cada orden? ¿Se copia, se comparte o se pierde? Justifiquen con su implementación.
El arreglo de detalles de cada orden se comparte (se transfiere), no se pierde ni se duplica en profundidad. se realiza una copia superficial de la estructura de la orden. Esto significa que el puntero detalles se copia exactamente igual a la nueva orden. Al hacer delete[] s.ordenes;, se destruye el contenedor superficial antiguo de las órdenes, pero no se ejecuta un delete[] sobre sus miembros internos. Como resultado, la nueva orden redimensionada conserva intacta la conexión con el mismo arreglo de detalles original que ya existía en la memoria RAM, manteniendo los datos a salvo y sin causar fugas de memoria.
