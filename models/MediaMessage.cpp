#include "models/MediaMessage.h"

#include <stdexcept>
#include <string>
#include <vector>

// Implementación de MediaMessage (mensaje con un archivo adjunto según el enunciado :D)
// No se sube ningún archivo real: solo se guardan sus datos.
// Reglas que aplica esta clase: la ruta y el tipo MIME no pueden estar vacíos y el tamaño se guarda en std::size_t, así que por tipo no puede ser negativo.
// Formato del payload en el CSV: ruta|mime_type|tamaño_bytes|descripción

namespace {

// Divide `text` en `count` campos por `delimiter`; el último campo conserva el
// resto de la cadena, de modo que la descripción puede contener '|'.
std::vector<std::string> splitFields(const std::string& text, char delimiter, std::size_t count) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (fields.size() + 1 < count) {
        std::size_t pos = text.find(delimiter, start);
        if (pos == std::string::npos) {
            throw std::runtime_error("Payload de MediaMessage inválido: " + text);
        }
        fields.push_back(text.substr(start, pos - start));
        start = pos + 1;
    }
    fields.push_back(text.substr(start));
    return fields;
}

}  // namespace

MediaMessage::MediaMessage(std::shared_ptr<User> sender, std::string filePath,
                           std::string mimeType, std::size_t sizeInBytes,
                           std::string description)
    : Message(std::move(sender)),
      filePath(std::move(filePath)),
      mimeType(std::move(mimeType)),
      sizeInBytes(sizeInBytes),
      description(std::move(description)) {
    // Valida los datos propios del mensaje de archivo.
    if (this->filePath.empty()) {
        throw std::invalid_argument("La ruta del archivo no puede estar vacía");
    }
    if (this->mimeType.empty()) {
        throw std::invalid_argument("El tipo MIME no puede estar vacío");
    }
}

// Constructor de carga: reconstruye el mensaje a partir del payload del CSV.
MediaMessage::MediaMessage(const MessageMetadata& metadata, const std::string& payload)
    : Message(metadata) {
    // Separa los 4 campos; la descripción es el último y puede contener '|'.
    std::vector<std::string> fields = splitFields(payload, '|', 4);
    filePath = fields[0];
    mimeType = fields[1];
    sizeInBytes = static_cast<std::size_t>(std::stoull(fields[2]));
    description = fields[3];

    if (filePath.empty()) {
        throw std::invalid_argument("La ruta del archivo no puede estar vacía");
    }
    if (mimeType.empty()) {
        throw std::invalid_argument("El tipo MIME no puede estar vacío");
    }
}

// Punto de entrada que usa MessageController al cargar el CSV.
std::shared_ptr<MediaMessage> MediaMessage::fromPayload(const MessageMetadata& metadata,
                                                        const std::string& payload) {
    return std::make_shared<MediaMessage>(metadata, payload);
}

const std::string& MediaMessage::getFilePath() const { return filePath; }
const std::string& MediaMessage::getMimeType() const { return mimeType; }
const std::string& MediaMessage::getDescription() const { return description; }
std::size_t MediaMessage::getSizeInBytes() const { return sizeInBytes; }

std::string MediaMessage::getType() const { return TYPE; }

// Vista previa: la descripción si existe; si no, el nombre del archivo.
std::string MediaMessage::getPreview() const {
    return description.empty() ? filePath : description;
}

// Serializa los datos propios en el formato del payload.
std::string MediaMessage::serializePayload() const {
    return filePath + "|" + mimeType + "|" + std::to_string(sizeInBytes) + "|" + description;
}
