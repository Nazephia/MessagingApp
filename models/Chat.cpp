#include "models/Chat.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "utils/Uuid.h"

int Chat::nextId = 1;

Chat::Chat(const std::shared_ptr<User>& first, const std::shared_ptr<User>& second)
    : Chat(std::vector<std::shared_ptr<User>>{first, second}) {}

Chat::Chat(std::vector<std::shared_ptr<User>> participants)
    : internalId(nextId++), externalId(Uuid::generate()), createdAt(TimeUtils::now()) {
    std::vector<std::shared_ptr<User>> installed;
    for (const auto& participant : participants) {
        if (!participant) {
            throw std::invalid_argument("Un chat no puede tener participantes nulos");
        }
        bool duplicated = std::any_of(installed.begin(), installed.end(), [&](const auto& other) {
            return other->getInternalId() == participant->getInternalId();
        });
        if (!duplicated) {
            installed.push_back(participant);
        }
    }
    if (installed.size() < 2) {
        throw std::invalid_argument("Un chat necesita al menos 2 participantes distintos");
    }
    this->participants = std::move(installed);
}

Chat::Chat(int internalId, const std::string& externalId, const TimePoint& createdAt,
           std::vector<std::shared_ptr<User>> participants)
    : internalId(internalId), externalId(externalId), createdAt(createdAt) {
    std::vector<std::shared_ptr<User>> installed;
    for (const auto& participant : participants) {
        if (!participant) {
            throw std::invalid_argument("Un chat no puede tener participantes nulos");
        }
        bool duplicated = std::any_of(installed.begin(), installed.end(), [&](const auto& other) {
            return other->getInternalId() == participant->getInternalId();
        });
        if (!duplicated) {
            installed.push_back(participant);
        }
    }
    if (installed.size() < 2) {
        throw std::invalid_argument("Un chat necesita al menos 2 participantes distintos");
    }
    this->participants = std::move(installed);
    nextId = std::max(nextId, internalId + 1);
}

int Chat::getInternalId() const { return internalId; }
const std::string& Chat::getExternalId() const { return externalId; }
const TimePoint& Chat::getCreatedAt() const { return createdAt; }
const std::vector<std::shared_ptr<User>>& Chat::getParticipants() const { return participants; }
const std::vector<std::shared_ptr<Message>>& Chat::getMessages() const { return messages; }

bool Chat::isParticipant(int userId) const {
    for (const auto& participant : participants) {
        if (participant->getInternalId() == userId) {
            return true;
        }
    }
    return false;
}

void Chat::sendMessage(const std::shared_ptr<Message>& message) {
    if (!message) {
        throw std::invalid_argument("El mensaje no puede ser nulo");
    }
    if (!isParticipant(message->getSender()->getInternalId())) {
        throw std::invalid_argument(message->getSender()->getFullName() +
                                    " no participa en este chat");
    }
    message->markSent(internalId);  // lanza std::logic_error si ya fue enviado
    messages.push_back(message);
}

void Chat::loadMessage(const std::shared_ptr<Message>& message) {
    if (!message || message->getChatId() != internalId) {
        throw std::invalid_argument("El mensaje no pertenece a este chat");
    }
    messages.push_back(message);
}

bool Chat::removeMessage(int messageId) {
    auto it = std::find_if(messages.begin(), messages.end(), [messageId](const auto& message) {
        return message->getInternalId() == messageId;
    });
    if (it == messages.end()) {
        return false;
    }
    messages.erase(it);
    return true;
}

void Chat::attachParticipant(const std::shared_ptr<User>& user) {
    if (!user || isParticipant(user->getInternalId())) {
        return;
    }
    participants.push_back(user);
}

void Chat::detachParticipant(int userId) {
    participants.erase(
        std::remove_if(participants.begin(), participants.end(), [userId](const auto& participant) {
            return participant->getInternalId() == userId;
        }),
        participants.end());
}

std::string Chat::getName() const { return ""; }

int Chat::getAdminId() const { return 0; }
