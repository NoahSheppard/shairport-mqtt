#include "shairport-mqtt/mqtt_client.hpp"

#include <exception>

namespace {
constexpr const char* kTopicFilter = "shairport-sync/#";
constexpr const char* kRemoteTopic = "shairport-sync/remote";

// shairport-sync always sends AirPlay audio at 44100 samples/sec, regardless
// of the source's original sample rate.
constexpr double kSampleRate = 44100.0;

// Payload format: "<start>/<current>/<end>", all RTP timestamps (sample
// counts). Returns false if the payload doesn't have the expected shape.
bool parse_progress(const std::string& payload, double& elapsed_seconds, double& duration_seconds) {
    size_t first_slash = payload.find('/');
    if (first_slash == std::string::npos) {
        return false;
    }
    size_t second_slash = payload.find('/', first_slash + 1);
    if (second_slash == std::string::npos) {
        return false;
    }

    try {
        double start = std::stod(payload.substr(0, first_slash));
        double current = std::stod(payload.substr(first_slash + 1, second_slash - first_slash - 1));
        double end = std::stod(payload.substr(second_slash + 1));
        elapsed_seconds = (current - start) / kSampleRate;
        duration_seconds = (end - start) / kSampleRate;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
}  // namespace

ShairportMqttClient::ShairportMqttClient(std::string server_address, std::string client_id, NowPlayingStore& store,
                                         std::string username, std::string password)
    : store_(store),
      client_(std::move(server_address), std::move(client_id)),
      username_(std::move(username)),
      password_(std::move(password)) {
    client_.set_callback(*this);
}

void ShairportMqttClient::connect() {
    mqtt::connect_options conn_opts;
    conn_opts.set_keep_alive_interval(20);
    conn_opts.set_clean_session(true);
    if (!username_.empty()) {
        conn_opts.set_user_name(username_);
        conn_opts.set_password(password_);
    }

    client_.connect(conn_opts)->wait();
    client_.subscribe(kTopicFilter, 1)->wait();
}

void ShairportMqttClient::disconnect() {
    client_.disconnect()->wait();
}

void ShairportMqttClient::send_command(const std::string& command) {
    client_.publish(kRemoteTopic, command.data(), command.size(), 1, false)->wait();
}

void ShairportMqttClient::message_arrived(mqtt::const_message_ptr msg) {
    const std::string& topic = msg->get_topic();

    if (topic == "shairport-sync/artist") {
        store_.set_artist(msg->get_payload_str());
    } else if (topic == "shairport-sync/album") {
        store_.set_album(msg->get_payload_str());
    } else if (topic == "shairport-sync/title") {
        store_.set_title(msg->get_payload_str());
    } else if (topic == "shairport-sync/genre") {
        store_.set_genre(msg->get_payload_str());
    } else if (topic == "shairport-sync/playing") {
        store_.set_playing(msg->get_payload_str() == "1");
    } else if (topic == "shairport-sync/ssnc/prgr") {
        double elapsed_seconds = 0.0;
        double duration_seconds = 0.0;
        if (parse_progress(msg->get_payload_str(), elapsed_seconds, duration_seconds)) {
            store_.set_progress(elapsed_seconds, duration_seconds);
        }
    }
    // The rest of shairport-sync/core/* (raw DACP codes) and shairport-sync/ssnc/*
    // (other sync markers, binary cover art) are intentionally ignored —
    // the topics above already carry the decoded values we need.
}
