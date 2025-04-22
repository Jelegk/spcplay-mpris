// Starts an instance of spcplay.exe (given through argv[1]) with the
// song from argv[2] while also subscribing the player as an MPRIS client.

#include "mpris_server.hpp"
#include "id666.h"

#include <string>
#include <filesystem> // exists()
#include <unistd.h>  // sleep()

int spcplayCmd(std::string spcCmd, std::string arg) {
    int ret = system(std::string("wine ").append(spcCmd + " " + arg).c_str());
    if (ret) fprintf(stderr, "\"spcplay.exe %s\" returned with error: %d.", arg.c_str(), ret);
    return ret;
}

int main(int argc, char const *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "usage:  spcplay-mpris <path/to/spcplay.exe> <path/to/file.spc>\n");
        return 1;
    }

    if (!(std::filesystem::exists(argv[1]))) {
        fprintf(stderr, "invalid first argument: doesn't exist.\n");
        return 2;
    }

    auto opt = mpris::Server::make("spcplay.exe");
    if (!opt) {
        fprintf(stderr, "can't connect: someone already there.\n");
        return 3;
    }
    auto &server = *opt;

    id666tag tag = readID666(argv[2]);
    if (strcmp(tag.title, "__i_am_file_error__") == 0 || strcmp(tag.title, "__i_am_no_SPC__") == 0)
        return 2;
    
    server.set_identity("SNES SPC700 Player");
    server.set_supported_uri_schemes({ "file" });                               // (custom mimetype)
    server.set_supported_mime_types({ "application/octet-stream", "text/plain", "audio/spc-dump" });
    server.set_maximum_rate(8.0);
    server.set_minimum_rate(0.1);
    server.set_metadata({ { mpris::Field::Title,   sdbus::Variant(tag.title) },
                          { mpris::Field::Album,   sdbus::Variant(tag.game) },
                          { mpris::Field::Comment, sdbus::Variant(tag.comment) },
                          { mpris::Field::Length,  sdbus::Variant(std::stoi(tag.length) /* + std::stoi(tag.fadeout) * 0.001*/) },
                          { mpris::Field::Artist,  sdbus::Variant(tag.artist) } });

    bool playing = false;
    bool random  = false;

    server.on_shuffle_changed([&](bool shuffle)  { random = shuffle;                                             });
    server.on_open_uri([&](std::string_view uri) { printf("No uri support.\n");                                  }); // TODO: spcplayCmd(std::string(uri));
    server.on_quit([&]                           { spcplayCmd(std::string(argv[1]), "-q"); std::exit(0);         });
    server.on_next([&]                           { spcplayCmd(std::string(argv[1]), random ? "-rand" : "-next"); });
    server.on_previous([&]                       { spcplayCmd(std::string(argv[1]), "-prev");                    });
    server.on_play_pause([&] {
        playing = !playing;
        server.set_playback_status(playing ? mpris::PlaybackStatus::Playing : mpris::PlaybackStatus::Paused);
        spcplayCmd(std::string(argv[1]), "-p");
    });
    server.on_stop([&] {
        playing = false;
        server.set_playback_status(mpris::PlaybackStatus::Stopped);
        spcplayCmd(std::string(argv[1]), "-q");
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
    if (spcplayCmd(std::string(argv[1]), std::string(argv[2]))) return 4;
    server.set_playback_status(mpris::PlaybackStatus::Playing);

    int64_t pos = 0;
    while (true) {
        if (playing) server.set_position(pos++); // TODO: Might lag behind because of next line's overhead
        if (system("[ \"$(pgrep -li spcplay.exe)\" ] && exit 0 || exit 1")) break;
        sleep(1);
    }

    return 0;
}
