# Práctica 2 — Sistema de mensajería en C++

**Modalidad:** en parejas
**Lenguaje:** C++ estándar (hasta C++17)
**Entrega de la Parte 1:** hoy, 15 de octubre de 2026, al terminar la clase (se hace en el aula)
**Entrega de la Parte 2:** hoy, 15 de octubre de 2026, antes de las 23:59

## Contexto

En esta práctica van a modelar el núcleo de una aplicación de mensajería, al estilo de WhatsApp o Telegram: usuarios que conversan en chats y se envían distintos tipos de mensajes. No se construye una aplicación con pantallas ni red. El objetivo es diseñar bien las clases, y el programa principal es una simulación que crea datos, los guarda en archivos CSV y los vuelve a cargar.

El objetivo central es aplicar los cuatro pilares de la Programación Orientada a Objetos: **abstracción, encapsulamiento, herencia y polimorfismo**. No basta con que el programa funcione. Cada pilar tiene que aparecer en el diseño, y deben poder explicar por qué lo usaron donde lo usaron.

## Código que reciben

La carpeta `version_base/` contiene una versión funcional pero limitada del sistema: solo existen mensajes de texto y chats entre dos personas. Léanla con atención antes de empezar, porque la Parte 1 de la evaluación consiste en explicarla.

| Carpeta / archivo | Contenido |
| --- | --- |
| `models/User` | Usuario con id secuencial, UUID, nombre, apellido y fecha de creación |
| `models/TextMessage` | Mensaje de texto con remitente, estado y fechas de creación, envío, entrega y apertura |
| `models/Chat` | Chat entre exactamente dos usuarios (E2E) |
| `models/MessageStatus` | `enum class` con los estados de un mensaje: `Pending`, `Sent`, `Delivered`, `Opened` |
| `controllers/` | `UserController`, `ChatController` y `MessageController`: Singletons que simulan un ORM, con operaciones CRUD y lectura/escritura de CSV |
| `utils/` | Generación de UUID, manejo de fechas y lectura de CSV |
| `main.cpp` | Simulación mínima: crea dos usuarios, un chat y dos mensajes, guarda, vacía la memoria y vuelve a cargar |
| `tests/test_main.cpp` | Tres pruebas de ejemplo con `<cassert>` |
| `Makefile` | Compilación incremental |

Para compilar y ejecutar, desde la carpeta `version_base/`:

```bash
make        # compila el programa
make run    # compila y ejecuta la simulación (escribe en data/)
make test   # compila y ejecuta las pruebas
make clean  # borra lo compilado
```

El `Makefile` solo recompila los archivos que cambiaron (o los que incluyen una cabecera que cambió), y detecta solo los `.cpp` nuevos que agreguen en `models/`, `controllers/` o `utils/`.

Presten especial atención a los controladores. Muestran cómo se escribe cada objeto en su CSV y cómo se reconstruye el objeto a partir de una línea del archivo (deserialización). Lo van a necesitar para los tipos nuevos.

## Qué deben implementar

### 1. Nuevos tipos de mensaje

Además del mensaje de texto, el sistema debe soportar:

| Tipo | Datos propios | Reglas |
| --- | --- | --- |
| `MediaMessage` | Ruta del archivo, tipo MIME (por ejemplo `image/png`), tamaño en bytes y una descripción opcional | No se sube ningún archivo real: solo se guardan sus datos. Ruta y tipo MIME no pueden estar vacíos; el tamaño no puede ser negativo |
| `PollMessage` | Pregunta, lista de opciones y los votos (qué opción eligió cada usuario) | Al menos 2 opciones. Votar por una opción inexistente es un error. Un usuario puede cambiar su voto. Debe poder obtenerse la cantidad de votos por opción |

Todos los tipos de mensaje comparten lo que hoy tiene `TextMessage`: ids, remitente, estado y fechas. Todos deben cumplir las mismas reglas de estado (ver punto 3).

### 2. Nuevos tipos de chat

| Tipo | Reglas |
| --- | --- |
| `DirectChat` | Lo que hoy es `Chat`: exactamente dos usuarios distintos, fijados al crearlo. No admite agregar participantes |
| `GroupChat` | Tiene nombre y un administrador, que también es participante. Permite agregar y quitar participantes. Nunca puede quedar con menos de 2 participantes, y no se puede quitar al administrador |

En cualquier chat se pueden enviar mensajes de **cualquier tipo**, siempre que el remitente participe del chat.

### 3. Estados de un mensaje

Estas reglas ya existen para `TextMessage` y deben valer para todos los tipos:

```
Pending --enviar--> Sent --entregar--> Delivered --abrir--> Opened
```

- Un mensaje se crea en `Pending`. Pasa a `Sent` cuando un chat lo acepta, y en ese momento se registra su fecha de envío.
- Los estados avanzan en orden y no se puede saltar un paso: no se puede abrir un mensaje que no fue entregado.
- Un mensaje no se puede enviar dos veces.
- Cada paso registra su fecha (`sentAt`, `deliveredAt`, `openedAt`). La fecha de creación (`createdAt`) se registra al crear el objeto y nunca cambia.

### 4. Persistencia en CSV

Extiendan los controladores para que guarden y carguen los tipos nuevos sin cambiar las columnas de los archivos:

- `data/messages.csv`: la columna `type` indica el tipo de mensaje y la columna `payload` guarda sus datos propios. Ustedes definen el formato del `payload` de `MediaMessage` y `PollMessage`, y lo documentan en el informe.
- `data/chats.csv`: la columna `type` indica el tipo de chat. Las columnas `name` y `admin_id`, hoy vacías, se usan para los grupos.

Al cargar, cada mensaje y cada chat debe reconstruirse como **su tipo concreto**: un `PollMessage` guardado tiene que volver como `PollMessage`, con sus votos.

### 5. Simulación en `main`

Reemplacen la simulación mínima por una que, en orden:

1. Cree al menos 4 usuarios.
2. Cree un chat directo y un chat grupal con al menos 3 participantes.
3. Envíe al menos un mensaje de cada tipo, y marque algunos como entregados y abiertos.
4. Registre votos en una encuesta e imprima sus resultados.
5. Muestre, capturando las excepciones, que el sistema rechaza al menos dos operaciones inválidas (por ejemplo, agregar un tercer usuario a un chat directo o abrir un mensaje no entregado).
6. Guarde todo, vacíe la memoria, vuelva a cargar desde los CSV e imprima los chats con sus mensajes, para demostrar que los datos persisten.

### 6. Pruebas

Escriban pruebas sencillas que se ejecuten con `make test` y verifiquen lo principal. Alcanza con una prueba corta para cada punto:

- Un `DirectChat` rechaza un tercer participante y un `GroupChat` permite agregar uno.
- Un mensaje no se puede abrir si no fue entregado.
- Una encuesta cuenta bien los votos.
- Después de guardar y cargar, un mensaje de encuesta o de archivo vuelve con su tipo y sus datos.

**Recomendación:** usar `<cassert>`, como en `tests/test_main.cpp`, con una función por prueba. No es obligatorio; pueden organizar las pruebas como prefieran, siempre sin librerías externas.

## Restricciones técnicas

- C++ estándar hasta C++17, sin librerías externas. El código debe compilar con `make` sin errores. Se valorará que no haya advertencias.
- Punteros: solo punteros inteligentes (`std::shared_ptr`, `std::unique_ptr`, `std::weak_ptr`). Nada de `new` ni `delete`.
- STL permitida: `vector`, `list`, `queue`, `stack`, `map`, además de `string`, `optional`, `chrono`, `fstream`, `sstream` y `random`.
- Una clase por par de archivos `.h` / `.cpp`, respetando la estructura de carpetas (`models/`, `controllers/`, `utils/`, `tests/`).
- Ningún atributo público.
- Pueden modificar el código entregado todo lo que necesiten, incluidos `TextMessage`, `Chat` y los controladores.

## Uso de inteligencia artificial

Pueden usar herramientas de IA (ChatGPT, Claude, Copilot, etc.) en ambas partes. Son responsables de todo lo que entregan: deben entender cada clase y cada decisión de diseño, y el docente puede pedirles que expliquen cualquier parte del código o del informe. Indiquen en el README qué herramientas usaron y para qué.

## Entregables y evaluación

### Parte 1 — Informe de análisis y diseño, y primer avance (en el aula, hoy)

La Parte 1 se realiza y se entrega **en el aula, durante la clase de hoy**. Tiene dos componentes.

**a) Informe breve** (entre 1 y 2 páginas) que responda:

1. **Contexto.** Qué entienden del código entregado: qué hace cada clase, cómo se relacionan, cómo funcionan los controladores y cómo se guarda y reconstruye un objeto desde el CSV.
2. **Diseño propuesto.** Cómo van a incorporar los nuevos tipos de mensaje y de chat: qué clases van a crear o modificar y cómo se relacionan. Incluyan un diagrama de clases (UML, Mermaid o a mano).
3. **Los cuatro pilares.** Para cada pilar (abstracción, encapsulamiento, herencia y polimorfismo), indiquen **dónde** lo van a aplicar y **por qué** es la decisión adecuada en ese lugar. Si usan composición, explíquenlo también.
4. **Formato del `payload`** que van a usar para `MediaMessage` y `PollMessage`.

Se evalúa la comprensión y la justificación, no que el diseño coincida con uno "correcto". Si durante la Parte 2 cambian el diseño, explíquenlo brevemente en el README de la entrega final.

**b) Avance en el código.** Junto con el informe, entreguen el estado actual del proyecto. Como mínimo debe:

- Incluir las clases nuevas de su diseño declaradas en sus archivos `.h` (atributos y métodos), aunque algunos métodos todavía no estén implementados.
- Compilar con `make`.

Este avance no se evalúa por completitud. Sirve para verificar que el diseño del informe ya empezó a llevarse al código.

### Parte 2 — Implementación (hoy, antes de las 23:59)

Antes de las **23:59 de hoy**, entreguen la carpeta del proyecto con:

- El código completo, que compile con `make`, ejecute la simulación con `make run` y las pruebas con `make test`.
- Un `README.md` con los integrantes, cómo compilar y ejecutar, las herramientas de IA que usaron, y los cambios de diseño respecto a la Parte 1, si los hubo.
- Sin la carpeta `build/` ni archivos compilados.

Pueden subir el código a un repositorio git (GitHub, GitLab, etc.) si lo consideran útil; en ese caso, incluyan el enlace en el README. No es obligatorio.

### Criterios de evaluación

| Criterio | Peso |
| --- | --- |
| Parte 1 (en el aula): comprensión del código, justificación del diseño con los cuatro pilares y primer avance en el código | 25 % |
| Diseño orientado a objetos en el código: abstracción, encapsulamiento, herencia y polimorfismo aplicados con sentido | 25 % |
| Funcionalidad: tipos de mensaje y chat, reglas de estado, simulación en `main` | 20 % |
| Persistencia: guardar y cargar todos los tipos y reconstruir el tipo concreto | 15 % |
| Pruebas: los cuatro casos pedidos, ejecutables con `make test` | 10 % |
| Calidad del código: legibilidad, organización, compila sin advertencias | 5 % |

Un programa que no compila con `make` no puede obtener puntaje en Funcionalidad, Persistencia ni Pruebas.
