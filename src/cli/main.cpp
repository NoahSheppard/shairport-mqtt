#include "shairport-mqtt/mqtt_client.hpp"
#include "shairport-mqtt/now_playing.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace {

const std::string SERVER_ADDRESS("tcp://192.168.4.169:1883");
const std::string CLIENT_ID("cpp_subscriber");
// Must match the broker credentials shairport-sync.conf's mqtt section uses —
// anonymous clients may be able to subscribe but not publish to /remote.
const std::string MQTT_USERNAME("dev");
const std::string MQTT_PASSWORD("a");
const std::string ANSI_CLEAR = "\033[2J\033[H";

bool changed(const NowPlayingInfo& a, const NowPlayingInfo& b) {
    return a.title != b.title || a.artist != b.artist || a.album != b.album ||
           a.genre != b.genre || a.playing != b.playing;
}

std::string progress_bar(double fraction, int width) {
    int filled = static_cast<int>(fraction * width);
    return "[" + std::string(filled, '#') + std::string(width - filled, '-') + "]";
}

void print_now_playing(const NowPlayingInfo& info) {
    double elapsed = live_elapsed_seconds(info);
    double duration = info.duration_seconds;
    double fraction = duration > 0.0 ? elapsed / duration : 0.0;

    std::cout << ANSI_CLEAR << std::flush;
    std::cout << "Title:  " << info.title << "\n";
    std::cout << "Artist: " << info.artist << "\n";
    std::cout << "Album:  " << info.album << "\n";
    std::cout << "Genre:  " << info.genre << "\n";
    std::cout << "State:  " << (info.playing ? "Playing" : "Paused") << "\n";
    std::cout << format_mm_ss(elapsed) << " " << progress_bar(fraction, 30) << " " << format_mm_ss(duration)
               << std::endl;
}

}  // namespace

int main() {
    NowPlayingStore store;
    ShairportMqttClient client(SERVER_ADDRESS, CLIENT_ID, store, MQTT_USERNAME, MQTT_PASSWORD);

    try {
        client.connect();
    } catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }

    std::cout << "Connected and subscribed. Press Ctrl+C to exit." << std::endl;

    NowPlayingInfo last;
    while (true) {
        NowPlayingInfo current = store.snapshot();
        if (changed(current, last) || current.playing) {
            print_now_playing(current);
            last = current;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
