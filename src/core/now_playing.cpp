#include "shairport-mqtt/now_playing.hpp"

void NowPlayingStore::set_artist(std::string value) {
    std::lock_guard<std::mutex> lock(mutex_);
    info_.artist = std::move(value);
}

void NowPlayingStore::set_album(std::string value) {
    std::lock_guard<std::mutex> lock(mutex_);
    info_.album = std::move(value);
}

void NowPlayingStore::set_title(std::string value) {
    std::lock_guard<std::mutex> lock(mutex_);
    info_.title = std::move(value);
}

void NowPlayingStore::set_genre(std::string value) {
    std::lock_guard<std::mutex> lock(mutex_);
    info_.genre = std::move(value);
}

void NowPlayingStore::set_playing(bool value) {
    std::lock_guard<std::mutex> lock(mutex_);
    info_.playing = value;
}

void NowPlayingStore::set_progress(double elapsed_seconds, double duration_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    info_.elapsed_seconds = elapsed_seconds;
    info_.duration_seconds = duration_seconds;
    info_.progress_updated_at = std::chrono::steady_clock::now();
}

NowPlayingInfo NowPlayingStore::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return info_;
}
