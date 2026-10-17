#include "controllers/UserController.h"

#include <fstream>
#include <stdexcept>

#include "utils/CsvUtils.h"
#include "utils/TimeUtils.h"

UserController& UserController::getInstance() {
    // Singleton de Meyers: la variable local estática se crea una sola vez, en el primer uso.
    static UserController instance;
    return instance;
}

std::shared_ptr<User> UserController::create(const std::string& name,
                                             const std::string& lastName) {
    auto user = std::make_shared<User>(name, lastName);
    users[user->getInternalId()] = user;
    return user;
}

std::shared_ptr<User> UserController::findById(int id) const {
    auto it = users.find(id);
    return it != users.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<User>> UserController::findAll() const {
    std::vector<std::shared_ptr<User>> result;
    for (const auto& [id, user] : users) {
        result.push_back(user);
    }
    return result;
}

bool UserController::update(const std::shared_ptr<User>& user) {
    // Los objetos se comparten con shared_ptr, así que los cambios hechos con los setters ya
    // están en memoria. update() reemplaza la entrada guardada; en un ORM real, aquí se
    // escribiría en la base de datos.
    if (!user || users.count(user->getInternalId()) == 0) {
        return false;
    }
    users[user->getInternalId()] = user;
    return true;
}

bool UserController::remove(int id) { return users.erase(id) > 0; }

void UserController::clear() { users.clear(); }

void UserController::load(const std::string& path) {
    users.clear();
    std::ifstream file(path);
    if (!file.is_open()) {
        return;  // todavía no hay datos guardados
    }

    std::string line;
    std::getline(file, line);  // encabezado
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        // Deserialización: cada columna del CSV vuelve a su atributo.
        std::vector<std::string> fields = CsvUtils::splitLine(line, 5);
        auto user = std::make_shared<User>(std::stoi(fields[0]),   // internal_id
                                           fields[1],              // external_id
                                           fields[2],              // name
                                           fields[3],              // last_name
                                           TimeUtils::fromCsv(fields[4]));  // created_at
        users[user->getInternalId()] = user;
    }
}

void UserController::save(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo escribir " + path);
    }

    file << "internal_id,external_id,name,last_name,created_at\n";
    for (const auto& [id, user] : users) {
        file << user->getInternalId() << ',' << user->getExternalId() << ',' << user->getName()
             << ',' << user->getLastName() << ',' << TimeUtils::toCsv(user->getCreatedAt())
             << '\n';
    }
}
