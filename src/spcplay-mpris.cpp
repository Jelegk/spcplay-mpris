// Starts an instance of spcplay.exe with the song from argv[1]
// while also registering the player as an MPRIS client.

#include "mpris_server.hpp"
#include "id666.h"

#include <unistd.h>  // sleep()
#include <string>   // concat & substr

// -----------------------------------------------------------------------------

#define SPCPLAY_EXE "/opt/spcplay-2.21.0.8765_pre-release/spcplay.exe"

// -----------------------------------------------------------------------------

int spcplayCmd(const char *arg) {
    int ret = system((std::string("wine ") + SPCPLAY_EXE + " " + arg).c_str());
    if (ret) fprintf(stderr, "\"spcplay.exe %s\" returned with error: %d.", arg, ret);
    return ret;
}

int main(int argc, char const *argv[]) {
    if (argc > 2) {
        fprintf(stdout, "usage:  spcplay-mpris <path/to/file.spc>\n");
        return 1;
    }

    auto opt = mpris::Server::make("spcplay.exe");
    if (!opt) {
        fprintf(stderr, "can't connect: someone already there.\n");
        return 2;
    }
    auto &server = *opt;

    if (strcmp(argv[1], "") == 0) argc = 1;

    if (argc == 2) {
        id666tag tag = readID666(argv[1][0] == '\'' ? std::string(argv[1]).substr(1, strlen(argv[1]) - 2).c_str() : argv[1]); // TODO: won't launch from pcmanfm. the .desktop works, and with this workaround it accepts unescaped spaces, but stil.
        if (strcmp(tag.title, "__i_am_file_error__") == 0 || strcmp(tag.title, "__i_am_no_SPC__") == 0)
            return 3;

        server.set_identity("SNES SPC700 Player");
        server.set_supported_uri_schemes({ "file" });                               // (custom mimetype)
        server.set_supported_mime_types({ "application/octet-stream", "text/plain", "audio/spc-dump" });
        server.set_maximum_rate(8.0);
        server.set_minimum_rate(0.1);
        server.set_metadata({ { mpris::Field::Title,   sdbus::Variant(tag.title)                                                    },
                              { mpris::Field::Album,   sdbus::Variant(tag.game)                                                     },
                              { mpris::Field::Comment, sdbus::Variant(tag.comment)                                                  },
                              { mpris::Field::Length,  sdbus::Variant(std::stoi(tag.length) + int(std::stoi(tag.fadeout) * 0.001f)) },
                              { mpris::Field::Artist,  sdbus::Variant(tag.artist)                                                   } });
    }

    bool playing = false;
    bool random  = false;

    server.on_shuffle_changed([&](bool shuffle) { random = shuffle; });
    server.on_open_uri([&](std::string_view uri) { printf("No uri support.\n"); }); // TODO: spcplayCmd(uri);
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
        spcplayCmd("-q");
    });

    // Must be declared even if empty.
    server.on_play([&]                                          { });
    server.on_pause([&]                                         { });
    server.on_seek([&](int64_t p)                               { });
    server.on_set_position([&](int64_t p)                       { });
    server.on_rate_changed([&](double rate)                     { });
    server.on_volume_changed([&](double vol)                    { });
    server.on_loop_status_changed([&](mpris::LoopStatus status) { });

    server.start_loop_async();
    
    if (spcplayCmd(argv[1])) return 4;
    if (argc == 2) server.set_playback_status(mpris::PlaybackStatus::Playing); // TODO: playerctl returns stopped until play-paused called, then reports reversed Playing/Paused

    int64_t pos = 0;
    while (true) {
        if (playing) server.set_position(pos++); // TODO: Doesn't really. 'playerctl position' reports 0.000000
        if (system("[ \"$(pgrep -li spcplay.exe)\" ] && exit 0 || exit 1")) break;
        sleep(1);
    }

    return 0;
}
