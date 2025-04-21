// Starts an instance of spcplay.exe with the song from argv[1]
// while also subscribing the player as an MPRIS server.

#include "mpris_server.hpp"
#include "id666.h"

#include <string>
#include <unistd.h> // sleep()

void spcplayCmd(std::string arg) {
    int ret = system(std::string("wine /opt/spcplay-2.21.0.8765_pre-release/spcplay.exe ").append(arg).c_str());
    if (ret) fprintf(stderr, "spcplay.exe returned with error %d. Its argument: %s", ret, arg.c_str());
}

int main(int argc, char const *argv[]) {
    auto opt = mpris::Server::make("spcplay.exe");
    if (!opt) {
        fprintf(stderr, "can't connect: someone already there.\n");
        return 1;
    }
    auto &server = *opt;

    id666tag tag = readID666(argv[1]);

    server.set_identity("SNES SPC700 Player");
    server.set_supported_uri_schemes({ "file" });                               // (custom mimetype)
    server.set_supported_mime_types({ "application/octet-stream", "text/plain", "audio/spc-dump" });
    server.set_maximum_rate(8.0);
    server.set_minimum_rate(0.1);
    server.set_metadata({ { mpris::Field::TrackId, sdbus::Variant("/1") },
                          { mpris::Field::Album, sdbus::Variant(tag.game) },
                          { mpris::Field::Title, sdbus::Variant(tag.title) },
                          { mpris::Field::Artist, sdbus::Variant(tag.artist) },
                          { mpris::Field::Length, sdbus::Variant(std::stoi(tag.length)/* + std::stoi(tag.fadeout) * 0.001*/) } });

    bool playing = false;
    bool random  = false;

    server.on_open_uri([&](std::string_view uri) { printf("No uri support.\n"); });
    server.on_shuffle_changed([&](bool shuffle) { random = shuffle; });
    server.on_quit([&] { spcplayCmd("-q"); std::exit(0); });
    server.on_next([&] { spcplayCmd(random ? "-rand" : "-next"); });
    server.on_previous([&] { spcplayCmd("-prev"); });
    server.on_play_pause([&] {
        playing = !playing;
        server.set_playback_status(playing ? mpris::PlaybackStatus::Playing : mpris::PlaybackStatus::Paused);
        spcplayCmd("-p");
    });
    server.on_stop([&] {
        playing = false;
        server.set_playback_status(mpris::PlaybackStatus::Stopped);
        spcplayCmd("-s");
    });
    server.on_play([&] {
        playing = true;
        server.set_playback_status(mpris::PlaybackStatus::Playing);
        spcplayCmd("-p");
    });

    int64_t pos = 0;

    // server.on_pause([&]                   { playing = false; server.set_playback_status(mpris::PlaybackStatus::Paused); });
    // server.on_seek([&](int64_t p)         { pos += p; server.set_position(pos); });
    // server.on_set_position([&](int64_t p) { pos  = p; server.set_position(pos); });
    // server.on_loop_status_changed([&](mpris::LoopStatus status) { });
    // server.on_volume_changed([&](double vol)                    { });
    // server.on_rate_changed([&](double rate)                     { });

    server.start_loop_async();
    spcplayCmd(std::string(argv[1]) + " &");

    while (true) {
        if (playing) server.set_position(pos++);
        sleep(1);
    }

    return 0;
}
