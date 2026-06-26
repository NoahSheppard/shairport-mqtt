#include <mqtt/async_client.h>

#include <iostream>

int main() {
    mqtt::async_client client("tcp://localhost:1883", "shairport-mqtt");
    std::cout << "shairport-mqtt build OK, paho client id: " << client.get_client_id() << "\n";
    return 0;
}
