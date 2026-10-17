#ifndef MODELS_TEXT_MESSAGE_H
#define MODELS_TEXT_MESSAGE_H

#include <memory>
#include <string>

#include "Message.h"
#include "MessageMetadata.h"
#include "User.h"

// Mensaje de texto: su único dato propio es el contenido.
class TextMessage : public Message {
public:
    static constexpr const char* TYPE = "text";

    // Crea un mensaje nuevo en estado Pending. Se envía con Chat::sendMessage().
    TextMessage(std::shared_ptr<User> sender, const std::string& content);

    // Constructor de carga: lo usa MessageController para reconstruir un mensaje leído del CSV.
    TextMessage(const MessageMetadata& metadata, const std::string& content);

    // Reconstruye un TextMessage a partir de la columna payload del CSV.
    static std::shared_ptr<TextMessage> fromPayload(const MessageMetadata& metadata,
                                                    const std::string& payload);

    const std::string& getContent() const;
    void setContent(const std::string& content);

    std::string getType() const override;
    std::string getPreview() const override;
    std::string serializePayload() const override;

private:
    std::string content;
};

#endif
