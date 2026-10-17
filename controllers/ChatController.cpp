#include "controllers/ChatController.h"

#include <fstream>
#include <stdexcept>

#include "controllers/UserController.h"
#include "models/DirectChat.h"
#include "models/GroupChat.h"
#include "utils/CsvUtils.h"
#include "utils/TimeUtils.h"

ChatController& ChatController::getInstance() {
    // Singleton de Meyers: la variable local estática se crea una sola vez, en el primer uso.
    static ChatController instance;
    return instance;
}

std::shared_ptr<Chat> ChatController::create(const std::shared_ptr<User>& first,
                                             const std::shared_ptr<User>& second) {
    auto chat = std::make_shared<DirectChat>(first, second);
    chats[chat->getInternalId()] = chat;
    return chat;
}

std::shared_ptr<Chat> ChatController::createGroup(
    const std::shared_ptr<User>& admin, const std::string& name,
    const std::vector<std::shared_ptr<User>>& participants) {
    auto chat = std::make_shared<GroupChat>(admin, name, participants);
    chats[chat->getInternalId()] = chat;
    return chat;
}

std::shared_ptr<Chat> ChatController::findById(int id) const {
    auto it = chats.find(id);
    return it != chats.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<Chat>> ChatController::findAll() const {
    std::vector<std::shared_ptr<Chat>> result;
    for (const auto& [id, chat] : chats) {
        result.push_back(chat);
    }
    return result;
}

bool ChatController::update(const std::shared_ptr<Chat>& chat) {
    // Los objetos se comparten con shared_ptr, así que los cambios ya están en memoria.
    // update() reemplaza la entrada guardada; en un ORM real, aquí se escribiría en la base.
    if (!chat || chats.count(chat->getInternalId()) == 0) {
        return false;
    }
    chats[chat->getInternalId()] = chat;
    return true;
}

bool ChatController::remove(int id) { return chats.erase(id) > 0; }

void ChatController::clear() { chats.clear(); }

void ChatController::load(const std::string& chatsPath, const std::string& participantsPath) {
    chats.clear();
    UserController& users = UserController::getInstance();

    // 1. Participantes de cada chat: chat_id -> lista de usuarios.
    std::map<int, std::vector<std::shared_ptr<User>>> participantsByChat;
    std::ifstream participantsFile(participantsPath);
    std::string line;
    if (participantsFile.is_open()) {
        std::getline(participantsFile, line);  // encabezado
        while (std::getline(participantsFile, line)) {
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields = CsvUtils::splitLine(line, 2);
            int chatId = std::stoi(fields[0]);
            auto user = users.findById(std::stoi(fields[1]));
            if (!user) {
                throw std::runtime_error("El chat " + fields[0] + " tiene un usuario inexistente: " +
                                         fields[1]);
            }
            participantsByChat[chatId].push_back(user);
        }
    }

    // 2. Chats. La columna type indica qué clase de chat hay que construir.
    std::ifstream chatsFile(chatsPath);
    if (!chatsFile.is_open()) {
        return;
    }
    std::getline(chatsFile, line);  // encabezado
    while (std::getline(chatsFile, line)) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> fields = CsvUtils::splitLine(line, 6);
        int id = std::stoi(fields[0]);
        const std::string& externalId = fields[1];
        const std::string& type = fields[2];
        TimePoint createdAt = TimeUtils::fromCsv(fields[3]);
        const auto& participants = participantsByChat[id];

        if (type == DirectChat::TYPE) {
            if (participants.size() != 2) {
                throw std::runtime_error("El chat " + fields[0] + " no tiene 2 participantes");
            }
            chats[id] = std::make_shared<DirectChat>(id, externalId, createdAt, participants[0],
                                                     participants[1]);
        } else if (type == GroupChat::TYPE) {
            if (fields[5].empty()) {
                throw std::runtime_error("El grupo " + fields[0] + " no tiene administrador");
            }
            auto admin = users.findById(std::stoi(fields[5]));
            if (!admin) {
                throw std::runtime_error("El grupo " + fields[0] + " tiene un administrador inexistente: " +
                                         fields[5]);
            }
            if (participants.size() < 2) {
                throw std::runtime_error("El grupo " + fields[0] + " tiene menos de 2 participantes");
            }
            chats[id] = std::make_shared<GroupChat>(id, externalId, createdAt, admin, fields[4],
                                                    participants);
        } else {
            throw std::runtime_error("Tipo de chat desconocido: " + type);
        }
    }
}

void ChatController::save(const std::string& chatsPath,
                          const std::string& participantsPath) const {
    std::ofstream chatsFile(chatsPath);
    std::ofstream participantsFile(participantsPath);
    if (!chatsFile.is_open() || !participantsFile.is_open()) {
        throw std::runtime_error("No se pudo escribir " + chatsPath + " o " + participantsPath);
    }

    chatsFile << "internal_id,external_id,type,created_at,name,admin_id\n";
    participantsFile << "chat_id,user_id\n";
    for (const auto& [id, chat] : chats) {
        std::string adminField =
            chat->getAdminId() > 0 ? std::to_string(chat->getAdminId()) : "";
        chatsFile << id << ',' << chat->getExternalId() << ',' << chat->getType() << ','
                  << TimeUtils::toCsv(chat->getCreatedAt()) << ',' << chat->getName() << ','
                  << adminField << '\n';

        for (const auto& participant : chat->getParticipants()) {
            participantsFile << id << ',' << participant->getInternalId() << '\n';
        }
    }
}
