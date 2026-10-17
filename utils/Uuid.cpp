#include "utils/Uuid.h"

#include <random>

namespace Uuid {

std::string generate() {
    static std::random_device device;
    static std::mt19937_64 engine(device());
    std::uniform_int_distribution<int> hexDigit(0, 15);
    std::uniform_int_distribution<int> variantDigit(8, 11);  // 8, 9, a o b

    const char* digits = "0123456789abcdef";
    std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    for (char& c : uuid) {
        if (c == 'x') {
            c = digits[hexDigit(engine)];
        } else if (c == 'y') {
            c = digits[variantDigit(engine)];
        }
    }
    return uuid;
}

}  // namespace Uuid
