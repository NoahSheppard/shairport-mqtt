#include "shairport-mqtt/mqtt_client.hpp"
#include "shairport-mqtt/now_playing.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <cstdio>
#include <string>

namespace {
const std::string SERVER_ADDRESS("tcp://192.168.4.169:1883");
const std::string CLIENT_ID("shairport-mqtt-gui");
// Must match the broker credentials shairport-sync.conf's mqtt section uses —
// anonymous clients may be able to subscribe but not publish to /remote.
const std::string MQTT_USERNAME("dev");
const std::string MQTT_PASSWORD("a");

void send_command_safe(ShairportMqttClient& client, const std::string& command) {
    try {
        client.send_command(command);
        std::printf("Sent command '%s' to shairport-sync/remote\n", command.c_str());
    } catch (const mqtt::exception& exc) {
        std::fprintf(stderr, "Failed to send command '%s': %s\n", command.c_str(), exc.what());
    }
}
}  // namespace

int main() {
    NowPlayingStore store;
    ShairportMqttClient mqtt_client(SERVER_ADDRESS, CLIENT_ID, store, MQTT_USERNAME, MQTT_PASSWORD);

    try {
        mqtt_client.connect();
    } catch (const mqtt::exception& exc) {
        std::fprintf(stderr, "MQTT connection failed: %s\n", exc.what());
        return 1;
    }

    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(960, 540, "shairport-mqtt", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        NowPlayingInfo now_playing = store.snapshot();

        ImGui::Begin("shairport-mqtt");
        ImGui::Text("Title:  %s", now_playing.title.c_str());
        ImGui::Text("Artist: %s", now_playing.artist.c_str());
        ImGui::Text("Album:  %s", now_playing.album.c_str());
        ImGui::Text("Genre:  %s", now_playing.genre.c_str());
        ImGui::Text("State:  %s", now_playing.playing ? "Playing" : "Paused");

        double elapsed = live_elapsed_seconds(now_playing);
        double duration = now_playing.duration_seconds;
        float fraction = duration > 0.0 ? static_cast<float>(elapsed / duration) : 0.0f;
        std::string progress_label = format_mm_ss(elapsed) + " / " + format_mm_ss(duration);
        ImGui::ProgressBar(fraction, ImVec2(-1, 0), progress_label.c_str());

        ImGui::Separator();
        if (ImGui::Button("Prev")) {
            send_command_safe(mqtt_client, ShairportCommand::kPrevItem);
        }
        ImGui::SameLine();
        if (ImGui::Button("Play/Pause")) {
            send_command_safe(mqtt_client, ShairportCommand::kPlayPause);
        }
        ImGui::SameLine();
        if (ImGui::Button("Next")) {
            send_command_safe(mqtt_client, ShairportCommand::kNextItem);
        }
        ImGui::End();

        ImGui::Render();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    mqtt_client.disconnect();
    return 0;
}
