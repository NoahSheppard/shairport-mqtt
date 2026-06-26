#include <mqtt/async_client.h>
#include <string>
#include <iostream>
#include <map>

const std::string SERVER_ADDRESS("tcp://192.168.4.169:1883");
const std::string CLIENT_ID("cpp_subscriber");
const std::string TOPIC("shairport-sync/#");

const std::string ANSI_CLEAR = "\033[2J\033[H";

std::map<std::string, std::string> track_info = {
    {"Album", ""},
    {"Artist", ""},
    {"Title", ""}
};

void change() {
    std::cout << ANSI_CLEAR << std::flush;
    std::cout << "Title: " << track_info["Title"] << std::endl;
    std::cout << "Album: " << track_info["Album"] << std::endl;
    std::cout << "Artist: " << track_info["Artist"] << std::endl;
}

class callback : public virtual mqtt::callback {
    void message_arrived(mqtt::const_message_ptr msg) override {
        std::cout << "Message received: " << msg->get_topic() << ", " << msg->get_payload_str() << std::endl; // DEBUG
        // work out what is actually happening 
        /*if (msg->get_topic() == "shairport-sync/title") {
            std::cout << "[Task] Changing title to: " << msg->get_payload_str() << std::endl;
            track_info["Title"] = msg->get_payload_str();
        } else if (msg->get_topic() == "shairport-sync/artist") {
            std::cout << "[Task] Changing Artist to: " << msg->get_payload_str() << std::endl;
            track_info["Artist"] = msg->get_payload_str();
        } else if (msg->get_topic() == "shairport-sync/album") {
            std::cout << "[Task] Changing Album to: " << msg->get_payload_str() << std::endl;
            track_info["Album"] = msg->get_payload_str();
        }

        change();*/
    }
};

int main() {
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    callback cb;
    client.set_callback(cb);

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        // Connect to EMQX broker
        client.connect(connOpts)->wait();
        std::cout << "Connected to EMQX broker" << std::endl;

        // Subscribe to topic
        client.subscribe(TOPIC, 1)->wait();
        std::cout << "Subscribed to topic: " << TOPIC << std::endl;

        // Keep running to receive messages
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();

        // Disconnect
        client.disconnect()->wait();
        std::cout << "Disconnected" << std::endl;
    } catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
