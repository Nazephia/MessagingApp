#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "controllers/ChatController.h"
#include "controllers/MessageController.h"
#include "controllers/UserController.h"
#include "models/Chat.h"
#include "models/MediaMessage.h"
#include "models/Message.h"
#include "models/PollMessage.h"
#include "models/TextMessage.h"
#include "models/User.h"

using namespace std;

namespace {

void printPoll(const shared_ptr<PollMessage>& poll) {
    cout << "  Pregunta: " << poll->getQuestion() << '\n';
    const auto& options = poll->getOptions();
    for (size_t i = 0; i < options.size(); ++i) {
        cout << "    " << i << ") " << options[i].text << " -> " << poll->getVoteCount(i)
             << " voto(s)\n";
    }
}

}  // namespace

int main() {
    try {
        UserController& users = UserController::getInstance();
        ChatController& chats = ChatController::getInstance();
        MessageController& messages = MessageController::getInstance();

        // 1. Al menos 4 usuarios.
        cout << "=== 1. Usuarios ===\n";
        auto ana = users.create("Ana", "Perez");
        auto bruno = users.create("Bruno", "Diaz");
        auto carla = users.create("Carla", "Martinez");
        auto diego = users.create("Diego", "Sanchez");
        for (const auto& user : users.findAll()) {
            cout << "  #" << user->getInternalId() << " " << user->getFullName() << '\n';
        }

        // 2. Un chat directo y un grupo con al menos 3 participantes.
        cout << "\n=== 2. Chats ===\n";
        auto directo = chats.create(ana, bruno);
        auto grupo = chats.createGroup(carla, "Equipo POO", {ana, bruno, diego});
        cout << "  Directo: " << directo->getTitle() << " (" << directo->getParticipants().size()
             << " participantes)\n";
        cout << "  Grupo:   " << grupo->getTitle() << " (" << grupo->getParticipants().size()
             << " participantes, admin " << carla->getName() << ")\n";

        // 3. Un mensaje de cada tipo; algunos entregados y abiertos.
        cout << "\n=== 3. Mensajes ===\n";
        auto texto = messages.createText(ana, "Hola Bruno, como va la practica?");
        directo->sendMessage(texto);
        texto->markDelivered();
        texto->markOpened();

        auto media = messages.createMedia(bruno, "foto_perfil.png", "image/png", 2048,
                                          "Foto de perfil");
        directo->sendMessage(media);
        media->markDelivered();

        auto encuesta = messages.createPoll(
            carla, "Que lenguaje usamos?", {PollMessage::Option{"C++"}, PollMessage::Option{"C"}});
        grupo->sendMessage(encuesta);
        encuesta->markDelivered();

        for (const auto& message : messages.findAll()) {
            cout << "  [" << message->getType() << "] " << message->getSender()->getFullName()
                 << " -> " << toString(message->getStatus()) << " (\"" << message->getPreview()
                 << "\")\n";
        }

        // 4. Votos en la encuesta y resultados.
        cout << "\n=== 4. Votos de la encuesta ===\n";
        encuesta->vote(ana, 0);
        encuesta->vote(bruno, 1);
        encuesta->vote(diego, 0);
        cout << "  Votos iniciales:\n";
        printPoll(encuesta);

        encuesta->changeVote(bruno, 0);  // Bruno cambia su voto
        cout << "  Tras cambiar el voto de Bruno:\n";
        printPoll(encuesta);

        // 5. Operaciones inválidas capturadas.
        cout << "\n=== 5. Operaciones invalidas ===\n";
        try {
            directo->addParticipant(diego);
            cout << "  ERROR: se esperaba una excepcion\n";
        } catch (const invalid_argument& e) {
            cout << "  Correcto: el chat directo no admite participantes (" << e.what() << ")\n";
        }

        auto pendiente = messages.createText(ana, "Mensaje que no se entrego");
        directo->sendMessage(pendiente);  // queda en estado Sent
        try {
            pendiente->markOpened();
            cout << "  ERROR: se esperaba una excepcion\n";
        } catch (const logic_error& e) {
            cout << "  Correcto: no se abre un mensaje no entregado (" << e.what() << ")\n";
        }

        // 6. Guardar, vaciar, volver a cargar y mostrar.
        cout << "\n=== 6. Persistencia ===\n";
        users.save();
        chats.save();
        messages.save();
        cout << "  Guardado en data/*.csv. Vaciando memoria...\n";

        users.clear();
        chats.clear();
        messages.clear();

        // El orden importa: los mensajes necesitan a los usuarios y a los chats.
        users.load();
        chats.load();
        messages.load();
        cout << "  Recargado: " << users.findAll().size() << " usuarios, "
             << chats.findAll().size() << " chats, " << messages.findAll().size()
             << " mensajes.\n";

        cout << "\n=== Chats y mensajes tras recargar ===\n";
        for (const auto& chat : chats.findAll()) {
            cout << '[' << chat->getType() << "] " << chat->getTitle() << '\n';
            for (const auto& message : chat->getMessages()) {
                cout << "    - (" << message->getType() << ") "
                     << message->getSender()->getFullName() << ": " << message->getPreview()
                     << " [" << toString(message->getStatus()) << "]\n";
            }
        }

        cout << "\n=== Fin de la simulacion ===\n";
        return 0;

    } catch (const exception& e) {
        cerr << "Error inesperado: " << e.what() << '\n';
        return 1;
    }
}
