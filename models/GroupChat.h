#ifndef MODELS_GROUP_CHAT_H
#define MODELS_GROUP_CHAT_H

#include <memory>
#include <string>
#include <vector>

#include "Chat.h"
#include "User.h"
#include "utils/TimeUtils.h"

// Chat de grupo con nombre y administrador (que siempre es participante).
// Permite agregar y quitar participantes, pero nunca quedar con menos de 2 ni
// quitar al administrador.
class GroupChat : public Chat {
public:
    static constexpr const char* TYPE = "group";

    //  admin no nulo, al menos 2 participantes en total (incluido el admin), 
    GroupChat(const std::shared_ptr<User>& admin, const std::string& name,
              const std::vector<std::shared_ptr<User>>& initialParticipants);

    // Constructor de carga: lo usa ChatController para reconstruir un grupo leído del CSV
    GroupChat(int internalId, const std::string& externalId, const TimePoint& createdAt,
              const std::shared_ptr<User>& admin, const std::string& name,
              std::vector<std::shared_ptr<User>> participants);

    std::shared_ptr<User> getAdmin() const;

    // Agrega un participante. Devuelve false si es nulo o ya participa.
    bool addParticipant(const std::shared_ptr<User>& user) override;
    // Quita un participante. Devuelve false si es el admin, no existe o quedaría menos de 2.
    bool removeParticipant(const std::shared_ptr<User>& user) override;

    std::string getName() const override;
    int getAdminId() const override;
    std::string getType() const override;
    std::string getTitle() const override;

private:
    std::string name;
    std::shared_ptr<User> admin;
};

#endif
