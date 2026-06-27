#pragma once

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <string>

struct NowPlayingInfo {
    std::string artist;
    std::string album;
    std::string title;
    std::string genre;
    bool playing = false;

    // Elapsed/duration as of the last shairport-sync/ssnc/prgr update.
    double elapsed_seconds = 0.0;
    double duration_seconds = 0.0;
    std::chrono::steady_clock::time_point progress_updated_at{};
};

// Estimated elapsed playtime right now, accounting for real time passed
// since the last progress update (shairport-sync only sends ssnc/prgr
// occasionally, not every second).
inline double live_elapsed_seconds(const NowPlayingInfo& info) {
    double elapsed = info.elapsed_seconds;
    if (info.playing) {
        elapsed += std::chrono::duration<double>(std::chrono::steady_clock::now() - info.progress_updated_at).count();
    }
    if (info.duration_seconds > 0.0) {
        elapsed = std::min(elapsed, info.duration_seconds);
    }
    return std::max(elapsed, 0.0);
}

inline std::string format_mm_ss(double seconds) {
    int total_seconds = static_cast<int>(seconds < 0.0 ? 0.0 : seconds);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", total_seconds / 60, total_seconds % 60);
    return buf;
}

// Written to from the Paho MQTT callback thread, read from the CLI/GUI thread.
class NowPlayingStore {
public:
    void set_artist(std::string value);
    void set_album(std::string value);
    void set_title(std::string value);
    void set_genre(std::string value);
    void set_playing(bool value);
    void set_progress(double elapsed_seconds, double duration_seconds);

    NowPlayingInfo snapshot() const;

private:
    mutable std::mutex mutex_;
    NowPlayingInfo info_;
};
