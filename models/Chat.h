#ifndef MODELS_CHAT_H
#define MODELS_CHAT_H

#include <memory>
#include <string>
#include <vector>

#include "Message.h"
#include "User.h"
#include "utils/TimeUtils.h"

// Clase base abstracta de los chats

// Guarda los participantes y los mensajes y define la interfaz polimórfica que
// implementan DirectChat (dos participantes fijos) y GroupChat (nombre,
// administrador y participantes que se pueden agregar y quitar).
class Chat {
public:
    virtual ~Chat() = default;

    int getInternalId() const;
    const std::string& getExternalId() const;
    const TimePoint& getCreatedAt() const;
    const std::vector<std::shared_ptr<User>>& getParticipants() const;
    const std::vector<std::shared_ptr<Message>>& getMessages() const;

    bool isParticipant(int userId) const;

    // Envía un mensaje: valida que el remitente participe, lo marca como enviado
    // (markSent) y lo agrega a la lista. Lanza std::invalid_argument si no participa.
    void sendMessage(const std::shared_ptr<Message>& message);

    // Agrega un mensaje que ya fue enviado a este chat. Lo usa MessageController al cargar.
    void loadMessage(const std::shared_ptr<Message>& message);

    // Quita un mensaje de la lista. Devuelve false si no estaba.
    bool removeMessage(int messageId);

    virtual std::string getType() const = 0;
    virtual std::string getTitle() const = 0;

    // Gestión de participantes. Un chat directo no la permite y lanza excepción.
    virtual bool addParticipant(const std::shared_ptr<User>& user) = 0;
    virtual bool removeParticipant(const std::shared_ptr<User>& user) = 0;

    // Datos que solo usan los grupos; el chat directo devuelve vacío / 0.
    virtual std::string getName() const;
    virtual int getAdminId() const;

protected:
    // Constructor nuevo para chats directos (delega en el de participantes).
    Chat(const std::shared_ptr<User>& first, const std::shared_ptr<User>& second);
    // Constructor nuevo a partir de la lista de participantes.
    explicit Chat(std::vector<std::shared_ptr<User>> participants);
    // Constructor de carga: reconstruye un chat leído del CSV.
    Chat(int internalId, const std::string& externalId, const TimePoint& createdAt,
         std::vector<std::shared_ptr<User>> participants);

    // Mutadores de participantes para las subclases (GroupChat).
    void attachParticipant(const std::shared_ptr<User>& user);
    void detachParticipant(int userId);

private:
    static int nextId;

    int internalId;
    std::string externalId;
    TimePoint createdAt;
    std::vector<std::shared_ptr<User>> participants;
    std::vector<std::shared_ptr<Message>> messages;
};

#endif
