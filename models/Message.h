#ifndef MODELS_MESSAGE_H
#define MODELS_MESSAGE_H

#include <memory>
#include <optional>
#include <string>

#include "MessageMetadata.h"
#include "MessageStatus.h"
#include "User.h"
#include "utils/TimeUtils.h"

// Clase base abstracta de todos los mensajes.
//
// Agrupa lo común a cualquier tipo (ids, remitente, estado y fechas) y define la
// interfaz polimórfica (getType, getPreview, serializePayload) que usan Chat y
// MessageController para tratar cualquier mensaje de forma uniforme. Los tipos
// concretos (TextMessage, MediaMessage, PollMessage) solo guardan sus datos propios.
class Message {
public:
    virtual ~Message() = default;

    int getInternalId() const;
    const std::string& getExternalId() const;
    std::shared_ptr<User> getSender() const;
    int getChatId() const;
    MessageStatus getStatus() const;
    const TimePoint& getCreatedAt() const;
    const std::optional<TimePoint>& getSentAt() const;
    const std::optional<TimePoint>& getDeliveredAt() const;
    const std::optional<TimePoint>& getOpenedAt() const;

    bool isSent() const;
    bool isDelivered() const;
    bool isOpened() const;

    // Transiciones de estado. Cada una exige el estado anterior y si no lanza
    // std::logic_error: Pending -> Sent -> Delivered -> Opened, sieguiendo las pautas
    // markSent la llama Chat::sendMessage() al aceptar el mensaje.
    void markSent(int chatId);
    void markDelivered();
    void markOpened();

    // Interfaz polimórfica propia de cada tipo de mensaje.
    virtual std::string getType() const = 0;
    virtual std::string getPreview() const = 0;
    // Lo que se guarda en la columna payload del CSV.
    virtual std::string serializePayload() const = 0;

protected:
    // Crea un mensaje nuevo en estado Pending.
    explicit Message(std::shared_ptr<User> sender);
    // Reconstruye un mensaje desde los datos comunes leídos del CSV.
    explicit Message(const MessageMetadata& metadata);

private:
    static int nextId;

    int internalId;
    std::string externalId;
    std::shared_ptr<User> sender;
    int chatId;  // 0 = aún no se envió a ningún chat
    MessageStatus status;
    TimePoint createdAt;
    std::optional<TimePoint> sentAt;
    std::optional<TimePoint> deliveredAt;
    std::optional<TimePoint> openedAt;
};

#endif
