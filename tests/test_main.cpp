// Pruebas con <cassert>. Ejecutar con `make test`.
// Una función por prueba y un assert por cada cosa que debe cumplirse.

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "controllers/ChatController.h"
#include "controllers/MessageController.h"
#include "controllers/UserController.h"
#include "models/DirectChat.h"
#include "models/GroupChat.h"
#include "models/MediaMessage.h"
#include "models/PollMessage.h"
#include "models/TextMessage.h"

using namespace std;

namespace {

// Devuelve true si `action` lanza una excepción del tipo indicado.
template <typename Exception, typename Action>
bool throws(Action action) {
    try {
        action();
    } catch (const Exception&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

// Un DirectChat rechaza un tercer participante y un GroupChat permite agregar uno.
void testParticipants() {
    auto ana = make_shared<User>("Ana", "Perez");
    auto bruno = make_shared<User>("Bruno", "Diaz");
    auto carla = make_shared<User>("Carla", "Martinez");
    auto diego = make_shared<User>("Diego", "Sanchez");

    DirectChat directo(ana, bruno);
    assert(directo.getParticipants().size() == 2);
    assert(throws<invalid_argument>([&] { directo.addParticipant(carla); }));

    GroupChat grupo(carla, "Equipo", {ana, bruno});
    assert(grupo.getParticipants().size() == 3);
    assert(grupo.addParticipant(diego));
    assert(grupo.getParticipants().size() == 4);
    assert(!grupo.removeParticipant(carla));  // no se puede quitar al administrador
}

// No se puede abrir un mensaje si no fue entregado.
void testMessageCannotBeOpenedBeforeDelivered() {
    auto ana = make_shared<User>("Ana", "Perez");
    auto bruno = make_shared<User>("Bruno", "Diaz");
    DirectChat chat(ana, bruno);
    auto message = make_shared<TextMessage>(ana, "Hola");

    chat.sendMessage(message);
    assert(message->getStatus() == MessageStatus::Sent);
    assert(throws<logic_error>([&] { message->markOpened(); }));
}

// Una encuesta cuenta bien los votos.
void testPollCountsVotes() {
    auto ana = make_shared<User>("Ana", "Perez");
    auto bruno = make_shared<User>("Bruno", "Diaz");
    auto carla = make_shared<User>("Carla", "Martinez");

    PollMessage poll(ana, "¿Lenguaje?", {PollMessage::Option{"C++"}, PollMessage::Option{"C"}});
    poll.vote(ana, 0);
    poll.vote(bruno, 1);
    poll.vote(carla, 0);
    assert(poll.getVoteCount(0) == 2);
    assert(poll.getVoteCount(1) == 1);
    assert(poll.getOptionIndex(bruno) == 1);

    poll.changeVote(bruno, 0);
    assert(poll.getVoteCount(0) == 3);
    assert(poll.getVoteCount(1) == 0);
    assert(throws<invalid_argument>([&] { poll.vote(ana, 5); }));
}

// Después de guardar y cargar, la encuesta y el archivo vuelven con su tipo y sus datos.
void testPersistenceRestoresConcreteTypes() {
    UserController& users = UserController::getInstance();
    ChatController& chats = ChatController::getInstance();
    MessageController& messages = MessageController::getInstance();
    users.clear();
    chats.clear();
    messages.clear();

    auto ana = users.create("Ana", "Perez");
    auto bruno = users.create("Bruno", "Diaz");
    auto carla = users.create("Carla", "Martinez");
    auto grupo = chats.createGroup(carla, "Equipo", {ana, bruno});

    auto media = messages.createMedia(ana, "foto.png", "image/png", 123, "descripcion");
    grupo->sendMessage(media);
    auto poll = messages.createPoll(carla, "¿Lenguaje?",
                                    {PollMessage::Option{"C++"}, PollMessage::Option{"C"}});
    grupo->sendMessage(poll);
    poll->vote(ana, 1);

    const string usersPath = "build/test_data/users.csv";
    const string chatsPath = "build/test_data/chats.csv";
    const string participantsPath = "build/test_data/chat_participants.csv";
    const string messagesPath = "build/test_data/messages.csv";
    users.save(usersPath);
    chats.save(chatsPath, participantsPath);
    messages.save(messagesPath);

    users.clear();
    chats.clear();
    messages.clear();
    users.load(usersPath);
    chats.load(chatsPath, participantsPath);
    messages.load(messagesPath);

    auto reloadedChat = chats.findById(grupo->getInternalId());
    assert(reloadedChat != nullptr);
    assert(reloadedChat->getMessages().size() == 2);

    bool foundMedia = false;
    bool foundPoll = false;
    for (const auto& message : reloadedChat->getMessages()) {
        if (auto loadedMedia = dynamic_pointer_cast<MediaMessage>(message)) {
            foundMedia = true;
            assert(loadedMedia->getFilePath() == "foto.png");
            assert(loadedMedia->getMimeType() == "image/png");
            assert(loadedMedia->getSizeInBytes() == 123);
            assert(loadedMedia->getDescription() == "descripcion");
        } else if (auto loadedPoll = dynamic_pointer_cast<PollMessage>(message)) {
            foundPoll = true;
            assert(loadedPoll->getQuestion() == "¿Lenguaje?");
            assert(loadedPoll->getOptions().size() == 2);
            assert(loadedPoll->getVoteCount(1) == 1);
        }
    }
    assert(foundMedia);
    assert(foundPoll);

    users.clear();
    chats.clear();
    messages.clear();
}

void run(const string& name, void (*test)()) {
    test();
    cout << "OK  " << name << '\n';
}

}  // namespace

int main() {
    run("DirectChat rechaza un tercer participante", testParticipants);
    run("no se abre un mensaje sin entregar", testMessageCannotBeOpenedBeforeDelivered);
    run("la encuesta cuenta bien los votos", testPollCountsVotes);
    run("persistencia reconstruye los tipos concretos", testPersistenceRestoresConcreteTypes);
    cout << "Todas las pruebas pasaron\n";
    return 0;
}
