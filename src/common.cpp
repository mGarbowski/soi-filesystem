#include <chrono>

int64_t getCurrentTimestamp() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}
