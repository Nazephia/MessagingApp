#ifndef CONTROLLERS_CHAT_CONTROLLER_H
#define CONTROLLERS_CHAT_CONTROLLER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "models/Chat.h"
#include "models/User.h"

// Singleton que simula un ORM para los chats. Usa dos archivos:
//   data/chats.csv              internal_id,external_id,type,created_at,name,admin_id
//   data/chat_participants.csv  chat_id,user_id
// La columna type indica el tipo de chat; name y admin_id solo se usan en los grupos.
class ChatController {
public:
    static constexpr const char* DEFAULT_CHATS_PATH = "data/chats.csv";
    static constexpr const char* DEFAULT_PARTICIPANTS_PATH = "data/chat_participants.csv";

    static ChatController& getInstance();

    ChatController(const ChatController&) = delete;
    ChatController& operator=(const ChatController&) = delete;

    // Crea un chat directo entre dos usuarios.
    std::shared_ptr<Chat> create(const std::shared_ptr<User>& first,
                                 const std::shared_ptr<User>& second);
    // Crea un grupo con nombre y administrador (que también es participante).
    std::shared_ptr<Chat> createGroup(const std::shared_ptr<User>& admin,
                                      const std::string& name,
                                      const std::vector<std::shared_ptr<User>>& participants);

    std::shared_ptr<Chat> findById(int id) const;  // nullptr si no existe
    std::vector<std::shared_ptr<Chat>> findAll() const;
    bool update(const std::shared_ptr<Chat>& chat);
    bool remove(int id);
    void clear();

    // Necesita que UserController ya esté cargado, para encontrar a los participantes.
    // Los mensajes de cada chat los agrega MessageController::load().
    void load(const std::string& chatsPath = DEFAULT_CHATS_PATH,
              const std::string& participantsPath = DEFAULT_PARTICIPANTS_PATH);
    void save(const std::string& chatsPath = DEFAULT_CHATS_PATH,
              const std::string& participantsPath = DEFAULT_PARTICIPANTS_PATH) const;

private:
    ChatController() = default;

    std::map<int, std::shared_ptr<Chat>> chats;
};

#endif
