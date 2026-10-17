#ifndef MODELS_POLL_MESSAGE_H
#define MODELS_POLL_MESSAGE_H

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Message.h"
#include "MessageMetadata.h"
#include "User.h"

// Encuesta: una pregunta con al menos dos opciones y los votos emitidos.
//
// Formato del payload en el CSV (columna `payload`):
//   pregunta;cantidad_opciones;opción1;...;opciónN;votos
// donde `votos` es una lista `id_usuario:índice,id_usuario:índice` (puede estar
// vacía). Ni la pregunta ni las opciones pueden contener ';'.
class PollMessage : public Message {
public:
    struct Option {
        std::string text;
    };

    static constexpr const char* TYPE = "poll";

    // Pre: al menos 2 opciones, ninguna vacía.
    PollMessage(std::shared_ptr<User> sender, std::string question, std::vector<Option> options);

    // Constructor de carga: reconstruye un PollMessage (con sus votos) leído del CSV.
    PollMessage(const MessageMetadata& metadata, const std::string& payload);

    // Reconstruye un PollMessage a partir de la columna payload del CSV.
    static std::shared_ptr<PollMessage> fromPayload(const MessageMetadata& metadata,
                                                    const std::string& payload);

    const std::string& getQuestion() const;
    const std::vector<Option>& getOptions() const;

    int getVoteCount(std::size_t optionIndex) const;              // votos de una opción
    int getOptionIndex(const std::shared_ptr<User>& user) const;  // -1 si no votó

    // Votar por una opción inexistente lanza std::invalid_argument.
    void vote(const std::shared_ptr<User>& user, std::size_t optionIndex);
    // Cambia el voto previo del usuario; lanza std::logic_error si aún no votó.
    void changeVote(const std::shared_ptr<User>& user, std::size_t newOptionIndex);

    std::string getType() const override;
    std::string getPreview() const override;
    std::string serializePayload() const override;

private:
    std::string question;
    std::vector<Option> options;
    // Voto de cada usuario: id interno del usuario -> índice de la opción elegida.
    std::map<int, std::size_t> votes;
};

#endif
