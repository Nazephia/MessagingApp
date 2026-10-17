#include "controllers/MessageController.h"

#include <fstream>
#include <stdexcept>

#include "controllers/ChatController.h"
#include "controllers/UserController.h"
#include "utils/CsvUtils.h"
#include "utils/TimeUtils.h"

MessageController& MessageController::getInstance() {
    // Singleton de Meyers: la variable local estática se crea una sola vez, en el primer uso.
    static MessageController instance;
    return instance;
}

std::shared_ptr<TextMessage> MessageController::createText(const std::shared_ptr<User>& sender,
                                                           const std::string& content) {
    auto message = std::make_shared<TextMessage>(sender, content);
    messages[message->getInternalId()] = message;
    return message;
}

std::shared_ptr<MediaMessage> MessageController::createMedia(const std::shared_ptr<User>& sender,
                                                             const std::string& filePath,
                                                             const std::string& mimeType,
                                                             std::size_t sizeInBytes,
                                                             const std::string& description) {
    auto message = std::make_shared<MediaMessage>(sender, filePath, mimeType, sizeInBytes,
                                                  description);
    messages[message->getInternalId()] = message;
    return message;
}

std::shared_ptr<PollMessage> MessageController::createPoll(const std::shared_ptr<User>& sender,
                                                           const std::string& question,
                                                           std::vector<PollMessage::Option> options) {
    auto message = std::make_shared<PollMessage>(sender, question, std::move(options));
    messages[message->getInternalId()] = message;
    return message;
}

std::shared_ptr<Message> MessageController::findById(int id) const {
    auto it = messages.find(id);
    return it != messages.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<Message>> MessageController::findAll() const {
    std::vector<std::shared_ptr<Message>> result;
    for (const auto& [id, message] : messages) {
        result.push_back(message);
    }
    return result;
}

std::vector<std::shared_ptr<Message>> MessageController::findByChat(int chatId) const {
    std::vector<std::shared_ptr<Message>> result;
    for (const auto& [id, message] : messages) {
        if (message->getChatId() == chatId) {
            result.push_back(message);
        }
    }
    return result;
}

bool MessageController::update(const std::shared_ptr<Message>& message) {
    if (!message || messages.count(message->getInternalId()) == 0) {
        return false;
    }
    messages[message->getInternalId()] = message;
    return true;
}

bool MessageController::remove(int id) {
    auto message = findById(id);
    if (!message) {
        return false;
    }
    if (auto chat = ChatController::getInstance().findById(message->getChatId())) {
        chat->removeMessage(id);
    }
    messages.erase(id);
    return true;
}

void MessageController::clear() { messages.clear(); }

void MessageController::load(const std::string& path) {
    messages.clear();
    std::ifstream file(path);
    if (!file.is_open()) {
        return;  // todavía no hay datos guardados
    }

    UserController& users = UserController::getInstance();
    ChatController& chats = ChatController::getInstance();

    std::string line;
    std::getline(file, line);  // encabezado
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        // payload es la última columna: splitLine le deja el resto de la línea, con sus comas.
        std::vector<std::string> fields = CsvUtils::splitLine(line, 11);

        // Deserialización: primero los datos comunes a cualquier mensaje...
        MessageMetadata metadata;
        metadata.internalId = std::stoi(fields[0]);
        metadata.externalId = fields[1];
        metadata.chatId = std::stoi(fields[2]);
        metadata.sender = users.findById(std::stoi(fields[3]));
        metadata.status = messageStatusFromString(fields[5]);
        metadata.createdAt = TimeUtils::fromCsv(fields[6]);
        metadata.sentAt = TimeUtils::optionalFromCsv(fields[7]);
        metadata.deliveredAt = TimeUtils::optionalFromCsv(fields[8]);
        metadata.openedAt = TimeUtils::optionalFromCsv(fields[9]);
        if (!metadata.sender) {
            throw std::runtime_error("El mensaje " + fields[0] + " tiene un remitente inexistente");
        }

        // ...y después el contenido propio del tipo, según la columna type.
        const std::string& type = fields[4];
        const std::string& payload = fields[10];
        std::shared_ptr<Message> message;
        if (type == TextMessage::TYPE) {
            message = TextMessage::fromPayload(metadata, payload);
        } else if (type == MediaMessage::TYPE) {
            message = MediaMessage::fromPayload(metadata, payload);
        } else if (type == PollMessage::TYPE) {
            message = PollMessage::fromPayload(metadata, payload);
        } else {
            throw std::runtime_error("Tipo de mensaje desconocido: " + type);
        }
        messages[message->getInternalId()] = message;

        // Los mensajes enviados vuelven a su chat; los pendientes (chat_id 0) no tienen chat.
        if (message->getChatId() != 0) {
            auto chat = chats.findById(message->getChatId());
            if (!chat) {
                throw std::runtime_error("El mensaje " + fields[0] + " apunta a un chat inexistente");
            }
            chat->loadMessage(message);
        }
    }
}

void MessageController::save(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo escribir " + path);
    }

    file << "internal_id,external_id,chat_id,sender_id,type,status,created_at,sent_at,"
            "delivered_at,opened_at,payload\n";
    for (const auto& [id, message] : messages) {
        file << id << ',' << message->getExternalId() << ',' << message->getChatId() << ','
             << message->getSender()->getInternalId() << ',' << message->getType() << ','
             << toString(message->getStatus()) << ','
             << TimeUtils::toCsv(message->getCreatedAt()) << ','
             << TimeUtils::toCsv(message->getSentAt()) << ','
             << TimeUtils::toCsv(message->getDeliveredAt()) << ','
             << TimeUtils::toCsv(message->getOpenedAt()) << ',' << message->serializePayload()
             << '\n';
    }
}
