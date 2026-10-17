#include "models/PollMessage.h"

#include <stdexcept>

#include "utils/CsvUtils.h"

// Implementación de PollMessage (encuesta).
// Reglas que aplica esta clase:
//   - al menos 2 opciones, ninguna vacía, y votar por una opción inexistente es error;
//   - un usuario vota una sola vez y puede cambiar su voto;
//   - se puede consultar cuántos votos tiene cada opción.
// Formato del payload en el CSV: pregunta;N;opción1;...;opciónN;votos
// donde votos es id_usuario:índice separados por comas (puede quedar vacío). :U

namespace {

// Valida la pregunta y las opciones comunes a los dos constructores.
void validatePolls(const std::string& question, std::size_t optionCount) {
    if (question.empty()) {
        throw std::invalid_argument("La pregunta de una encuesta no puede estar vacía");
    }
    if (optionCount < 2) {
        throw std::invalid_argument("Una encuesta debe tener al menos 2 opciones");
    }
}

}  // namespace

PollMessage::PollMessage(std::shared_ptr<User> sender, std::string question,
                         std::vector<Option> options)
    : Message(std::move(sender)), question(std::move(question)), options(std::move(options)) {
    // Valida la pregunta, la cantidad de opciones y que ninguna esté vacía.
    validatePolls(this->question, this->options.size());
    for (const auto& option : this->options) {
        if (option.text.empty()) {
            throw std::invalid_argument("Las opciones de una encuesta no pueden estar vacías");
        }
    }
}

// Constructor de carga: reconstruye la encuesta y sus votos desde el payload del CSV.
PollMessage::PollMessage(const MessageMetadata& metadata, const std::string& payload)
    : Message(metadata) {
    // payload = pregunta;N;opción1;...;opciónN;votos
    std::vector<std::string> fields = CsvUtils::split(payload, ';');
    if (fields.size() < 3) {
        throw std::runtime_error("Payload de PollMessage inválido: " + payload);
    }

    question = fields[0];
    std::size_t optionCount = static_cast<std::size_t>(std::stoul(fields[1]));
    validatePolls(question, optionCount);

    if (fields.size() < 2 + optionCount) {
        throw std::runtime_error("Payload de PollMessage con opciones incompletas: " + payload);
    }
    options.reserve(optionCount);
    for (std::size_t i = 0; i < optionCount; ++i) {
        options.push_back({fields[2 + i]});
    }

    // Los votos son opcionales: `id:índice,id:índice`.
    if (fields.size() > 2 + optionCount && !fields[2 + optionCount].empty()) {
        for (const std::string& entry : CsvUtils::split(fields[2 + optionCount], ',')) {
            std::size_t separator = entry.find(':');
            if (separator == std::string::npos) {
                throw std::runtime_error("Voto inválido en el payload: " + entry);
            }
            int userId = std::stoi(entry.substr(0, separator));
            std::size_t optionIndex = static_cast<std::size_t>(std::stoul(entry.substr(separator + 1)));
            if (optionIndex >= options.size()) {
                throw std::runtime_error("Voto por una opción inexistente: " + entry);
            }
            votes[userId] = optionIndex;
        }
    }
}

// Punto de entrada que usa MessageController al cargar el CSV.
std::shared_ptr<PollMessage> PollMessage::fromPayload(const MessageMetadata& metadata,
                                                      const std::string& payload) {
    return std::make_shared<PollMessage>(metadata, payload);
}

const std::string& PollMessage::getQuestion() const { return question; }
const std::vector<PollMessage::Option>& PollMessage::getOptions() const { return options; }

// Cuenta los votos recorriendo el mapa de votos.
int PollMessage::getVoteCount(std::size_t optionIndex) const {
    int count = 0;
    for (const auto& [userId, votedIndex] : votes) {
        (void)userId;
        if (votedIndex == optionIndex) {
            ++count;
        }
    }
    return count;
}

// Devuelve -1 cuando el usuario todavía no votó.
int PollMessage::getOptionIndex(const std::shared_ptr<User>& user) const {
    if (!user) {
        return -1;
    }
    auto it = votes.find(user->getInternalId());
    return it == votes.end() ? -1 : static_cast<int>(it->second);
}

// Registra el primer voto del usuario.
void PollMessage::vote(const std::shared_ptr<User>& user, std::size_t optionIndex) {
    if (!user) {
        throw std::invalid_argument("Un voto necesita un usuario");
    }
    if (optionIndex >= options.size()) {
        throw std::invalid_argument("No existe la opción " + std::to_string(optionIndex));
    }
    if (votes.count(user->getInternalId()) != 0) {
        throw std::logic_error(user->getFullName() + " ya votó; use changeVote()");
    }
    votes[user->getInternalId()] = optionIndex;
}

// Cambia el voto de un usuario que ya había votado.
void PollMessage::changeVote(const std::shared_ptr<User>& user, std::size_t newOptionIndex) {
    if (!user) {
        throw std::invalid_argument("Un voto necesita un usuario");
    }
    if (newOptionIndex >= options.size()) {
        throw std::invalid_argument("No existe la opción " + std::to_string(newOptionIndex));
    }
    auto it = votes.find(user->getInternalId());
    if (it == votes.end()) {
        throw std::logic_error(user->getFullName() + " todavía no ha votado");
    }
    it->second = newOptionIndex;
}

std::string PollMessage::getType() const { return TYPE; }

std::string PollMessage::getPreview() const { return question; }

// Escribe la pregunta, las opciones y los votos en el payload.
std::string PollMessage::serializePayload() const {
    std::string payload = question + ";" + std::to_string(options.size());
    for (const auto& option : options) {
        payload += ";" + option.text;
    }

    payload += ";";
    bool first = true;
    for (const auto& [userId, optionIndex] : votes) {
        if (!first) {
            payload += ",";
        }
        payload += std::to_string(userId) + ":" + std::to_string(optionIndex);
        first = false;
    }
    return payload;
}
