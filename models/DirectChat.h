#ifndef MODELS_DIRECT_CHAT_H
#define MODELS_DIRECT_CHAT_H

#include <memory>
#include <string>

#include "Chat.h"
#include "User.h"
#include "utils/TimeUtils.h"

// Chat directo entre exactamente dos usuarios, fijados al crearlo.
// No admite agregar ni quitar participantes.
class DirectChat : public Chat {
public:
    static constexpr const char* TYPE = "direct";

    DirectChat(const std::shared_ptr<User>& first, const std::shared_ptr<User>& second);

    // Constructor de carga: lo usa ChatController para reconstruir un chat leído del CSV.
    DirectChat(int internalId, const std::string& externalId, const TimePoint& createdAt,
               const std::shared_ptr<User>& first, const std::shared_ptr<User>& second);

    std::string getType() const override;
    std::string getTitle() const override;

    // Un chat directo no admite modificar participantes: lanza std::invalid_argument.
    bool addParticipant(const std::shared_ptr<User>& user) override;
    bool removeParticipant(const std::shared_ptr<User>& user) override;
};

#endif
