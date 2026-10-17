#ifndef MODELS_MESSAGE_METADATA_H
#define MODELS_MESSAGE_METADATA_H

#include <memory>
#include <optional>
#include <string>

#include "MessageStatus.h"
#include "User.h"
#include "utils/TimeUtils.h"

// Datos comunes a todos los mensajes, agrupados para reconstruir un mensaje leído del CSV
// sSolo lo usan los constructores de carga.
struct MessageMetadata {
    int internalId = 0;
    std::string externalId;
    std::shared_ptr<User> sender;
    int chatId = 0;  // 0 = el mensaje aún no se envió a ningún chat
    MessageStatus status = MessageStatus::Pending;
    TimePoint createdAt;
    std::optional<TimePoint> sentAt;
    std::optional<TimePoint> deliveredAt;
    std::optional<TimePoint> openedAt;
};

#endif
