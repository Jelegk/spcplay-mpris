// Starts an instance of spcplay.exe with the given song,
// while also subscribing it to an MPRIS server.

#include "mpris_server.hpp"
#include <spctag.h>
#include <string>
#include <unistd.h> // sleep()

#define SPCPLAY_CMD "wine /opt/spcplay-2.21.0.8765_pre-release/spcplay.exe "

int main(int argc, char const* argv[])
{
    FILE* pSPC = fopen(argv[1], "r");
    spctag_init(pSPC);
    fclose(pSPC);

    std::string album = spctag_get_gametitle();
    std::string title = spctag_get_songtitle();
    std::string artist = spctag_get_artist();
    std::string length = spctag_get_length();

    spctag_free();

    int64_t pos = 0;
    bool playing = false;
    bool random = false;
    bool repeat = true;

    auto opt = mpris::Server::make("spcplay.exe");
    if (!opt) {
        fprintf(stderr, "can't connect: someone already there.\n");
        return 1;
    }

    auto& server = *opt;

    server.set_identity("SNES SPC700 Player");
    server.set_supported_uri_schemes({ "file" });
    server.set_supported_mime_types({ "application/octet-stream", "text/plain" });
    server.set_metadata({ { mpris::Field::TrackId, "/1" },
        { mpris::Field::Album, album },
        { mpris::Field::Title, title },
        { mpris::Field::Artist, artist },
        { mpris::Field::Length, length } });
    server.set_maximum_rate(8.0);
    server.set_minimum_rate(0.1);

    server.on_quit([&] { system(strcat(SPCPLAY_CMD, "-q")); std::exit(0); });
    server.on_next([&] { system(strcat(SPCPLAY_CMD, (random ? "-rand" : "-next"))); });
    server.on_previous([&] { strcat(SPCPLAY_CMD, "-prev"); });
    // server.on_pause([&] {
    //     playing = false;
    //     server.set_playback_status(mpris::PlaybackStatus::Paused);
    // });
    server.on_play_pause([&] {
        playing = !playing;
        server.set_playback_status(playing ? mpris::PlaybackStatus::Playing
                                           : mpris::PlaybackStatus::Paused);
        system(strcat(SPCPLAY_CMD, "-p"));
    });
    server.on_stop([&] {
        playing = false;
        server.set_playback_status(mpris::PlaybackStatus::Stopped);
        system(strcat(SPCPLAY_CMD, "-s"));
    });
    server.on_play([&] {
        playing = true;
        server.set_playback_status(mpris::PlaybackStatus::Playing);
        system(strcat(SPCPLAY_CMD, "-p"));
    });
    // server.on_seek([&](int64_t p) { pos += p; server.set_position(pos); });
    // server.on_set_position([&](int64_t p) { pos  = p; server.set_position(pos); });
    server.on_open_uri([&](std::string_view uri) { printf("No uri support.\n"); });

    // server.on_loop_status_changed([&](mpris::LoopStatus status) { });
    // server.on_rate_changed([&](double rate) { });
    server.on_shuffle_changed([&](bool shuffle) { random = shuffle; });
    // server.on_volume_changed([&](double vol) { });

    server.start_loop_async();
    system(SPCPLAY_CMD);

    while (true) {
        if (playing)
            server.set_position(pos++);
        sleep(1);
    }

    return 0;
}
