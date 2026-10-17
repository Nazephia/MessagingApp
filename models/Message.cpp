#include "models/Message.h"

#include <algorithm>
#include <stdexcept>

#include "utils/Uuid.h"

int Message::nextId = 1;

Message::Message(std::shared_ptr<User> sender)
    : internalId(nextId++),
      externalId(Uuid::generate()),
      sender(std::move(sender)),
      chatId(0),
      status(MessageStatus::Pending),
      createdAt(TimeUtils::now()) {
    if (!this->sender) {
        throw std::invalid_argument("Un mensaje necesita un remitente");
    }
}

Message::Message(const MessageMetadata& metadata)
    : internalId(metadata.internalId),
      externalId(metadata.externalId),
      sender(metadata.sender),
      chatId(metadata.chatId),
      status(metadata.status),
      createdAt(metadata.createdAt),
      sentAt(metadata.sentAt),
      deliveredAt(metadata.deliveredAt),
      openedAt(metadata.openedAt) {
    if (!sender) {
        throw std::invalid_argument("Un mensaje necesita un remitente");
    }
    // Evita que un mensaje creado después de cargar el CSV repita un id existente.
    nextId = std::max(nextId, internalId + 1);
}

int Message::getInternalId() const { return internalId; }
const std::string& Message::getExternalId() const { return externalId; }
std::shared_ptr<User> Message::getSender() const { return sender; }
int Message::getChatId() const { return chatId; }
MessageStatus Message::getStatus() const { return status; }
const TimePoint& Message::getCreatedAt() const { return createdAt; }
const std::optional<TimePoint>& Message::getSentAt() const { return sentAt; }
const std::optional<TimePoint>& Message::getDeliveredAt() const { return deliveredAt; }
const std::optional<TimePoint>& Message::getOpenedAt() const { return openedAt; }

bool Message::isSent() const { return status != MessageStatus::Pending; }
bool Message::isDelivered() const {
    return status == MessageStatus::Delivered || status == MessageStatus::Opened;
}
bool Message::isOpened() const { return status == MessageStatus::Opened; }

void Message::markSent(int chatId) {
    if (status != MessageStatus::Pending) {
        throw std::logic_error("El mensaje ya fue enviado");
    }
    if (chatId <= 0) {
        throw std::invalid_argument("Id de chat inválido");
    }
    this->chatId = chatId;
    sentAt = TimeUtils::now();
    status = MessageStatus::Sent;
}

void Message::markDelivered() {
    if (status == MessageStatus::Pending) {
        throw std::logic_error("No se puede entregar un mensaje que no fue enviado");
    }
    if (status != MessageStatus::Sent) {
        throw std::logic_error("El mensaje ya fue entregado");
    }
    deliveredAt = TimeUtils::now();
    status = MessageStatus::Delivered;
}

void Message::markOpened() {
    if (status == MessageStatus::Opened) {
        throw std::logic_error("El mensaje ya fue abierto");
    }
    if (status != MessageStatus::Delivered) {
        throw std::logic_error("No se puede abrir un mensaje que no fue entregado");
    }
    openedAt = TimeUtils::now();
    status = MessageStatus::Opened;
}
