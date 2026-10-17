#include "models/User.h"

#include <algorithm>
#include <stdexcept>

#include "utils/Uuid.h"

int User::nextId = 1;

User::User(const std::string& name, const std::string& lastName)
    : internalId(nextId++), externalId(Uuid::generate()), createdAt(TimeUtils::now()) {
    setName(name);
    setLastName(lastName);
}

User::User(int internalId, const std::string& externalId, const std::string& name,
           const std::string& lastName, const TimePoint& createdAt)
    : internalId(internalId), externalId(externalId), createdAt(createdAt) {
    setName(name);
    setLastName(lastName);
    // Evita que un usuario creado después de cargar el CSV repita un id existente.
    nextId = std::max(nextId, internalId + 1);
}

int User::getInternalId() const { return internalId; }
const std::string& User::getExternalId() const { return externalId; }
const std::string& User::getName() const { return name; }
const std::string& User::getLastName() const { return lastName; }
std::string User::getFullName() const { return name + " " + lastName; }
const TimePoint& User::getCreatedAt() const { return createdAt; }

void User::setName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("El nombre no puede estar vacío");
    }
    this->name = name;
}

void User::setLastName(const std::string& lastName) {
    if (lastName.empty()) {
        throw std::invalid_argument("El apellido no puede estar vacío");
    }
    this->lastName = lastName;
}
