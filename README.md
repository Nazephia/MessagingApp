"Aplicación de mensajería (estilo WhatsApp o Telegram) modelado con
Programación Orientada a Objetos. No hay red ni interfaz gráfica: el programa
principal es una simulación que crea datos, los guarda en CSV y los vuelve a cargar"

## Integrantes

- Ortega Jeremias 
- Peñaloza José


## Estructura del proyecto

```
models/        Entidades del dominio (User, Message y sus tipos, Chat y sus tipos)
controllers/   Singletons que simulan un ORM (CRUD en memoria + CSV)
utils/         UUID, manejo de fechas y lectura/escritura de CSV
tests/         Pruebas con <cassert>
data/          Archivos CSV generados al ejecutar `make run`
main.cpp       Simulación de la aplicación
Makefile       Compilación incremental
```

## Contexto: cómo funciona el código

- **`User`**: usuario con id secuencial, UUID, nombre, apellido y fecha de creación.
- **`Message`** (clase base abstracta): agrupa lo común a cualquier mensaje: ids,
  remitente, estado y fechas. Define la interfaz polimórfica `getType()`,
  `getPreview()` y `serializePayload()`.
- **`TextMessage`**, **`MediaMessage`** y **`PollMessage`**: tipos concretos que heredan
  de `Message` y solo agregan sus datos propios.
- **`MessageStatus`**: `enum class` con `Pending`, `Sent`, `Delivered`, `Opened`, más su
  conversión a/desde texto para el CSV.
- **`Chat`** (clase base abstracta): guarda participantes y mensajes, y define la
  interfaz `getType()`, `getTitle()`, `addParticipant()` y `removeParticipant()`.
- **`DirectChat`**: chat entre exactamente dos usuarios; no admite agregar ni quitar.
- **`GroupChat`**: nombre y administrador (que siempre participa); permite agregar y
  quitar participantes, nunca por debajo de 2 y nunca al administrador.
- **`MessageMetadata`**: struct que agrupa los datos comunes para reconstruir un
  mensaje leído del CSV sin constructores de diez parámetros.

### Controladores (Singletons)

`UserController`, `ChatController` y `MessageController` son *Singletons* (una única
instancia) que simulan un ORM: mantienen los objetos en memoria y los leen/escriben en
CSV. `save()` escribe cada objeto con sus columnas; `load()` reconstruye el objeto a
partir de una línea (deserialización).

Al cargar, cada mensaje y cada chat se reconstruye **como su tipo concreto**: la columna
`type` decide qué clase construir y la columna `payload` aporta los datos propios. Por
eso un `PollMessage` guardado vuelve como `PollMessage` con sus votos.

Orden de carga (importa, por dependencias): **usuarios → chats → mensajes**.

## Herramientas de IA
Utlizamos Nemotron mayormente para una mejor explicación del código y corrección de errores sintacticos, tambien para optimizar el codigo y corregir nuestra lógica básica. 
Ademas tambien se uso IA para mejorar como se presenta la informacion en el README.md; es decir corregir la redacción.

## Diseño orientado a objetos

### Los cuatro pilares

- **Abstracción**: `Message` y `Chat` son clases base abstractas que exponen solo lo
  común (ids, remitente, estado, fechas / participantes y mensajes) y la interfaz que
  consumen los controladores y `main`. Los detalles de cada tipo quedan en las clases
  concretas.
- **Encapsulamiento**: ningún atributo es público. Las reglas se validan en los
  constructores y en los métodos (`markOpened()` exige `Delivered`, `PollMessage` exige
  2 opciones, `MediaMessage` exige ruta y MIME, etc.).
- **Herencia**: `TextMessage`, `MediaMessage` y `PollMessage` heredan de `Message`;
  `DirectChat` y `GroupChat` heredan de `Chat`.
- **Polimorfismo**: se trabaja con `std::shared_ptr<Message>` y `std::shared_ptr<Chat>`
  apuntando a la base. Las llamadas a `getType()`, `serializePayload()`, `getTitle()`,
  `addParticipant()`, etc. se resuelven en tiempo de ejecución; eso permite tratar
  cualquier tipo de mensaje o de chat de forma uniforme y reconstruir el tipo real al
  cargar el CSV.

### Reglas de estado de un mensaje

```
Pending --enviar--> Sent --entregar--> Delivered --abrir--> Opened
```

- Un mensaje nace en `Pending`. Pasa a `Sent` cuando un chat lo acepta (ahí se registra
  `sentAt`).
- Los pasos no se pueden saltar ni repetir: no se abre un mensaje no entregado y no se
  envía dos veces.
- Cada paso registra su fecha; `createdAt` se fija al crear y nunca cambia.

## Persistencia en CSV

`data/messages.csv`:

```
internal_id,external_id,chat_id,sender_id,type,status,created_at,sent_at,delivered_at,opened_at,payload
```

`data/chats.csv`:

```
internal_id,external_id,type,created_at,name,admin_id
```

`data/chat_participants.csv`: `chat_id,user_id`  ·  `data/users.csv`:
`internal_id,external_id,name,last_name,created_at`.

En `chats.csv`, `type` distingue `direct` y `group`, y `name`/`admin_id` solo se usan en
los grupos.

### Formato del `payload`

- **`TextMessage`**: el propio texto (puede contener comas, porque `payload` es la
  última columna).
- **`MediaMessage`**: `ruta|mime_type|tamaño_bytes|descripción`. La descripción va al
  final, así que puede contener `|`.
- **`PollMessage`**: `pregunta;cantidad_opciones;opción1;...;opciónN;votos`, donde
  `votos` es `id_usuario:índice,id_usuario:índice` (puede quedar vacío). Ni la pregunta
  ni las opciones pueden contener `;`.

## Simulación (`make run`)

La simulación, en orden:

1. Crea 4 usuarios.
2. Crea un chat directo y un grupo de al menos 3 participantes.
3. Envía un mensaje de cada tipo y marca algunos como entregados y abiertos.
4. Registra votos en la encuesta (incluido un cambio de voto) e imprime los resultados.
5. Captura dos operaciones inválidas (agregar a un chat directo y abrir un mensaje no
   entregado).
6. Guarda, vacía la memoria, vuelve a cargar desde los CSV e imprime los chats con sus
   mensajes.

## Pruebas (`make test`)

Con `<cassert>`, una función por caso:

- Un `DirectChat` rechaza un tercer participante y un `GroupChat` permite agregar uno.
- Un mensaje no se puede abrir si no fue entregado.
- Una encuesta cuenta bien los votos.
- Tras guardar y cargar, un mensaje de archivo y uno de encuesta vuelven con su tipo y
  sus datos.

## Cambios de diseño respecto a la Parte 1

- Se extrajo la clase base `Message` para no hacer heredar `MediaMessage` ni
  `PollMessage` de `TextMessage` (un archivo o una encuesta no son un texto).
- Se hizo virtual la interfaz de mensajes y chats, que es justo lo que permite
  reconstruir el tipo concreto al cargar el CSV.

## Notas

- La carpeta `data/` se genera al ejecutar `make run`.
- Para la entrega, borrar `build/` con `make clean`.
