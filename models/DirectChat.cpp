#include "models/DirectChat.h"

#include <stdexcept>

// Implementación de DirectChat (chat entre exactamente dos usuarios).
// Los participantes se fijan al crear el chat y no se pueden modificar:
// addParticipant y removeParticipant lanzan std::invalid_argument.

DirectChat::DirectChat(const std::shared_ptr<User>& first, const std::shared_ptr<User>& second)
    : Chat(first, second) {}

// Constructor de carga: reconstruye el chat directo leído del CSV.
DirectChat::DirectChat(int internalId, const std::string& externalId, const TimePoint& createdAt,
                       const std::shared_ptr<User>& first, const std::shared_ptr<User>& second)
    : Chat(internalId, externalId, createdAt,
           std::vector<std::shared_ptr<User>>{first, second}) {
    if (getParticipants().size() != 2) {
        throw std::invalid_argument("Un chat directo necesita exactamente 2 participantes");
    }
}

std::string DirectChat::getType() const { return TYPE; }

// Título legible: los nombres de los dos participantes.
std::string DirectChat::getTitle() const {
    const std::vector<std::shared_ptr<User>>& people = getParticipants();
    return people[0]->getFullName() + " y " + people[1]->getFullName();
}

// Un chat directo no admite agregar participantes.
bool DirectChat::addParticipant(const std::shared_ptr<User>&) {
    throw std::invalid_argument("Un chat directo no admite agregar participantes");
}

// Un chat directo no admite quitar participantes.
bool DirectChat::removeParticipant(const std::shared_ptr<User>&) {
    throw std::invalid_argument("Un chat directo no admite quitar participantes");
}
