#ifndef MODELS_USER_H
#define MODELS_USER_H

#include <string>

#include "utils/TimeUtils.h"

class User {
public:
    // Crea un usuario nuevo: asigna el siguiente id secuencial, un UUID y la fecha actual.
    User(const std::string& name, const std::string& lastName);

    // Constructor de carga: lo usa UserController para reconstruir un usuario leído del CSV.
    User(int internalId, const std::string& externalId, const std::string& name,
         const std::string& lastName, const TimePoint& createdAt);

    int getInternalId() const;
    const std::string& getExternalId() const;
    const std::string& getName() const;
    const std::string& getLastName() const;
    std::string getFullName() const;
    const TimePoint& getCreatedAt() const;

    // Los ids y createdAt no tienen setter: no cambian durante la vida del objeto.
    void setName(const std::string& name);
    void setLastName(const std::string& lastName);

private:
    static int nextId;

    int internalId;
    std::string externalId;
    std::string name;
    std::string lastName;
    TimePoint createdAt;
};

#endif
