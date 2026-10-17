#include "models/TextMessage.h"

#include <stdexcept>

TextMessage::TextMessage(std::shared_ptr<User> sender, const std::string& content)
    : Message(std::move(sender)) {
    setContent(content);
}

TextMessage::TextMessage(const MessageMetadata& metadata, const std::string& content)
    : Message(metadata) {
    setContent(content);
}

std::shared_ptr<TextMessage> TextMessage::fromPayload(const MessageMetadata& metadata,
                                                      const std::string& payload) {
    // El texto es todo el payload: es la última columna, así que puede contener comas.
    return std::make_shared<TextMessage>(metadata, payload);
}

const std::string& TextMessage::getContent() const { return content; }

void TextMessage::setContent(const std::string& content) {
    if (content.empty()) {
        throw std::invalid_argument("El texto del mensaje no puede estar vacío");
    }
    this->content = content;
}

std::string TextMessage::getType() const { return TYPE; }

std::string TextMessage::getPreview() const { return content; }

std::string TextMessage::serializePayload() const { return content; }
