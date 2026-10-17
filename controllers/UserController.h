#ifndef CONTROLLERS_USER_CONTROLLER_H
#define CONTROLLERS_USER_CONTROLLER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "models/User.h"

// Singleton que simula un ORM para User: CRUD en memoria + persistencia en CSV.
// Formato de data/users.csv:
//   internal_id,external_id,name,last_name,created_at
class UserController {
public:
    static constexpr const char* DEFAULT_PATH = "data/users.csv";

    static UserController& getInstance();

    UserController(const UserController&) = delete;
    UserController& operator=(const UserController&) = delete;

    std::shared_ptr<User> create(const std::string& name, const std::string& lastName);
    std::shared_ptr<User> findById(int id) const;  // nullptr si no existe
    std::vector<std::shared_ptr<User>> findAll() const;
    bool update(const std::shared_ptr<User>& user);  // false si el id no existe
    bool remove(int id);                             // false si el id no existe
    void clear();

    // load() reemplaza lo que hay en memoria por el contenido del archivo.
    // Si el archivo no existe, deja el controlador vacío.
    void load(const std::string& path = DEFAULT_PATH);
    void save(const std::string& path = DEFAULT_PATH) const;

private:
    UserController() = default;

    std::map<int, std::shared_ptr<User>> users;
};

#endif
