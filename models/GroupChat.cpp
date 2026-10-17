#include "models/GroupChat.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

// Implementación de GroupChat (grupo con nombre y administrador).
// Reglas que aplica esta clase:
//   - el administrador siempre es participante;
//   - se pueden agregar y quitar participantes, pero nunca quedar con menos de 2;
//   - no se puede quitar al administrador.

namespace {

// Construye la lista de participantes del grupo: el admin primero y luego los
// iniciales, sin duplicados. Valida que haya al menos 2.
std::vector<std::shared_ptr<User>> buildParticipants(
    const std::shared_ptr<User>& admin,
    const std::vector<std::shared_ptr<User>>& initialParticipants) {
    if (!admin) {
        throw std::invalid_argument("Un grupo necesita un administrador");
    }
    std::vector<std::shared_ptr<User>> result{admin};
    for (const auto& user : initialParticipants) {
        if (!user) {
            continue;
        }
        bool duplicated = std::any_of(result.begin(), result.end(), [&](const auto& other) {
            return other->getInternalId() == user->getInternalId();
        });
        if (!duplicated) {
            result.push_back(user);
        }
    }
    if (result.size() < 2) {
        throw std::invalid_argument("Un grupo debe tener al menos 2 participantes");
    }
    return result;
}

}  // namespace

GroupChat::GroupChat(const std::shared_ptr<User>& admin, const std::string& name,
                     const std::vector<std::shared_ptr<User>>& initialParticipants)
    : Chat(buildParticipants(admin, initialParticipants)), name(name), admin(admin) {
    if (this->name.empty()) {
        throw std::invalid_argument("Un grupo necesita un nombre");
    }
}

// Constructor de carga: reconstruye el grupo leído del CSV y comprueba que el
// administrador esté entre los participantes.
GroupChat::GroupChat(int internalId, const std::string& externalId, const TimePoint& createdAt,
                     const std::shared_ptr<User>& admin, const std::string& name,
                     std::vector<std::shared_ptr<User>> participants)
    : Chat(internalId, externalId, createdAt, std::move(participants)),
      name(name),
      admin(admin) {
    if (!this->admin) {
        throw std::invalid_argument("Un grupo necesita un administrador");
    }
    if (this->name.empty()) {
        throw std::invalid_argument("Un grupo necesita un nombre");
    }
    if (!isParticipant(this->admin->getInternalId())) {
        throw std::runtime_error("El administrador del grupo no es participante");
    }
}

std::shared_ptr<User> GroupChat::getAdmin() const { return admin; }

// Agrega un participante nuevo; devuelve false si es nulo o ya participaba.
bool GroupChat::addParticipant(const std::shared_ptr<User>& user) {
    if (!user || isParticipant(user->getInternalId())) {
        return false;
    }
    attachParticipant(user);
    return true;
}

// Quita un participante respetando al administrador y el mínimo de 2.
bool GroupChat::removeParticipant(const std::shared_ptr<User>& user) {
    if (!user) {
        return false;
    }
    if (user->getInternalId() == admin->getInternalId()) {
        return false;  // no se puede quitar al administrador
    }
    if (!isParticipant(user->getInternalId())) {
        return false;
    }
    if (getParticipants().size() <= 2) {
        return false;  // nunca puede quedar con menos de 2 participantes
    }
    detachParticipant(user->getInternalId());
    return true;
}

std::string GroupChat::getName() const { return name; }

int GroupChat::getAdminId() const { return admin->getInternalId(); }

std::string GroupChat::getType() const { return TYPE; }

std::string GroupChat::getTitle() const { return name; }
