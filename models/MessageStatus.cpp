#include "models/MessageStatus.h"

#include <stdexcept>

std::string toString(MessageStatus status) {
    switch (status) {
        case MessageStatus::Pending:   return "pending";
        case MessageStatus::Sent:      return "sent";
        case MessageStatus::Delivered: return "delivered";
        case MessageStatus::Opened:    return "opened";
    }
    throw std::logic_error("Estado de mensaje desconocido");
}

MessageStatus messageStatusFromString(const std::string& text) {
    if (text == "pending")   return MessageStatus::Pending;
    if (text == "sent")      return MessageStatus::Sent;
    if (text == "delivered") return MessageStatus::Delivered;
    if (text == "opened")    return MessageStatus::Opened;
    throw std::invalid_argument("Estado de mensaje desconocido: " + text);
}

