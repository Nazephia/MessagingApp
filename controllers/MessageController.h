#ifndef CONTROLLERS_MESSAGE_CONTROLLER_H
#define CONTROLLERS_MESSAGE_CONTROLLER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "models/MediaMessage.h"
#include "models/Message.h"
#include "models/PollMessage.h"
#include "models/TextMessage.h"

// Singleton que simula un ORM para los mensajes. Formato de data/messages.csv:
//   internal_id,external_id,chat_id,sender_id,type,status,created_at,sent_at,delivered_at,opened_at,payload
// La columna type dice qué clase de mensaje es, y payload guarda su contenido
// propio (serializePayload() al guardar, fromPayload() al cargar).
class MessageController {
public:
    static constexpr const char* DEFAULT_PATH = "data/messages.csv";

    static MessageController& getInstance();

    MessageController(const MessageController&) = delete;
    MessageController& operator=(const MessageController&) = delete;

    // Los mensajes se crean en estado Pending; se envían con Chat::sendMessage().
    std::shared_ptr<TextMessage> createText(const std::shared_ptr<User>& sender,
                                            const std::string& content);
    std::shared_ptr<MediaMessage> createMedia(const std::shared_ptr<User>& sender,
                                              const std::string& filePath,
                                              const std::string& mimeType,
                                              std::size_t sizeInBytes,
                                              const std::string& description = "");
    std::shared_ptr<PollMessage> createPoll(const std::shared_ptr<User>& sender,
                                            const std::string& question,
                                            std::vector<PollMessage::Option> options);

    std::shared_ptr<Message> findById(int id) const;  // nullptr si no existe
    std::vector<std::shared_ptr<Message>> findAll() const;
    std::vector<std::shared_ptr<Message>> findByChat(int chatId) const;
    bool update(const std::shared_ptr<Message>& message);
    // Quita el mensaje del controlador y, si estaba enviado, también de su chat.
    bool remove(int id);
    void clear();

    // Necesita que UserController y ChatController ya estén cargados.
    void load(const std::string& path = DEFAULT_PATH);
    void save(const std::string& path = DEFAULT_PATH) const;

private:
    MessageController() = default;

    std::map<int, std::shared_ptr<Message>> messages;
};

#endif
