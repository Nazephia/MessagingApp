#ifndef MODELS_MEDIA_MESSAGE_H
#define MODELS_MEDIA_MESSAGE_H

#include <cstddef>
#include <memory>
#include <string>

#include "Message.h"
#include "MessageMetadata.h"
#include "User.h"

// Mensaje con un archivo adjunto. No se sube ningún archivo real: solo se
// guardan la ruta, el tipo MIME, el tamaño en bytes y una descripción opcional.
//
// Formato del payload en el CSV (columna `payload`):
//   ruta|mime_type|tamaño_bytes|descripción
// La descripción va al final, así que puede contener el separador '|' :V
class MediaMessage : public Message {
public:
    static constexpr const char* TYPE = "media";

    // Pre: la ruta y el tipo MIME no pueden estar vacíos.
    MediaMessage(std::shared_ptr<User> sender, std::string filePath, std::string mimeType,
                 std::size_t sizeInBytes, std::string description = "");

    // Constructor de carga: reconstruye un MediaMessage leído del CSV.
    MediaMessage(const MessageMetadata& metadata, const std::string& payload);

    // Reconstruye un MediaMessage a partir de la columna payload del CSV.
    static std::shared_ptr<MediaMessage> fromPayload(const MessageMetadata& metadata,
                                                     const std::string& payload);

    const std::string& getFilePath() const;
    const std::string& getMimeType() const;
    const std::string& getDescription() const;
    std::size_t getSizeInBytes() const;

    std::string getType() const override;
    std::string getPreview() const override;
    std::string serializePayload() const override;

private:
    std::string filePath;
    std::string mimeType;
    std::size_t sizeInBytes;
    std::string description;
};

#endif
