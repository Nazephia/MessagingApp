#ifndef MODELS_MESSAGE_STATUS_H
#define MODELS_MESSAGE_STATUS_H

#include <string>

// Estados de un mensaje. Avanzan en un solo sentido:
// Pending -> Sent -> Delivered -> Opened
enum class MessageStatus { Pending, Sent, Delivered, Opened };

// Conversión a/desde el texto que se guarda en el CSV ("pending", "sent", "delivered", "opened").
std::string toString(MessageStatus status);
MessageStatus messageStatusFromString(const std::string& text);

#endif
