#pragma once

#include "shairport-mqtt/now_playing.hpp"

#include <mqtt/async_client.h>

#include <string>

// Remote-control command payloads accepted by shairport-sync on its
// "<topic>/remote" command topic. Requires `enable_remote = "yes";` in the
// mqtt section of shairport-sync.conf — it's "no" by default.
namespace ShairportCommand {
constexpr const char* kPlay = "play";
constexpr const char* kPause = "pause";
constexpr const char* kPlayPause = "playpause";
constexpr const char* kNextItem = "nextitem";
constexpr const char* kPrevItem = "previtem";
constexpr const char* kVolumeUp = "volumeup";
constexpr const char* kVolumeDown = "volumedown";
}  // namespace ShairportCommand

// Subscribes to shairport-sync's MQTT metadata topics and updates a
// NowPlayingStore as messages arrive. shairport-sync publishes both
// human-readable topics (artist/album/title/genre/playing) and raw DACP/sync
// topics (core/*, ssnc/*); only the human-readable ones are parsed here.
//
// Also publishes remote-control commands (see ShairportCommand) to
// shairport-sync's "<topic>/remote" topic.
class ShairportMqttClient : public virtual mqtt::callback {
public:
    // username/password should match whatever the broker requires for
    // publishing to "<topic>/remote" — anonymous clients are often allowed to
    // subscribe/read metadata but blocked from publishing to control topics.
    ShairportMqttClient(std::string server_address, std::string client_id, NowPlayingStore& store,
                        std::string username = "", std::string password = "");

    void connect();
    void disconnect();
    void send_command(const std::string& command);

private:
    void message_arrived(mqtt::const_message_ptr msg) override;

    NowPlayingStore& store_;
    mqtt::async_client client_;
    std::string username_;
    std::string password_;
};
