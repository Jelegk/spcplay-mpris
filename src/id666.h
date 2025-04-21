// NOTE: Handles the most important parts of id666 tags only. (no such thing as xid6)
// TODO: Add extended id666 capabilities, maybe.
// TODO: inline functions?

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define TITLE_ADDR      0x2E // 0002Eh
#define LENGTH_ADDR     0xA9 // 000A9h
#define ARTIST_ADDR_TXT 0xB1 // 000B1h
#define ARTIST_ADDR_BIN 0xB0 // 000B0h, see below.

#define MAGIC_BYTES "SNES-SPC700 Sound File Data v0.30"


#define PRINT_ID666(tag)                   \
    printf(" Title:   %s\n", tag.title);   \
    printf(" Game:    %s\n", tag.game);    \
    printf(" Dumper:  %s\n", tag.dumper);  \
    printf(" Comment: %s\n", tag.comment); \
    printf(" Length:  %s\n", tag.length);  \
    printf(" Fadeout: %s\n", tag.fadeout); \
    printf(" Artist:  %s\n", tag.artist)


typedef struct id666tag {
    char title[33];
    char game[33];
    char dumper[17];
    char comment[33];
    char length[4];   // in seconds.
    char fadeout[6];  // in milliseconds.
    char artist[33];
} id666tag;


// -------------------------------- READING ------------------------------------


/// @brief Returns true if the block of memory pointed to by s (and l bytes long) appears
///        to be textual. Every byte in the block is checked, even if a \0 is encountered.
static int _isTagText(const char *s, const int l) {
    int i = 0;
    while (i < l && (s[i] == 0 || (s[i] >= 0x30 && s[i] <= 0x39) || s[i] == '/'))
        ++i;
    return s[i] == 0 || i == l;
} // Appears in Alpha-II's ID666.cpp and KFileMetaData's kfile_spc.cpp.


/// @param file the path of the file to extract information
/// @return a struct of char*, all empty if an error occur
id666tag readID666(const char *file) {
    id666tag tag            = { "", "", "", "", "", "", "" };
    char     magicBytes[34] = "";
    int      isTextFormat   = 0;

    FILE *fSPC = fopen(file, "r");
    if (fSPC == NULL) {
        fprintf(stderr, "Failed to open %s: %s\n", file, strerror(errno));
        goto id666_end;
    }
    fgets(magicBytes, 34, fSPC);
    if (strcmp(magicBytes, MAGIC_BYTES) != 0) {
        fprintf(stderr, "%s is no SPC file\n", file);
        goto id666_end;
    }
    fseek(fSPC, 0x23, SEEK_SET);
    if (fgetc(fSPC) != 26) {
        fprintf(stderr, "No ID666 tag in %s\n", file);
        goto id666_end;
    }

    fseek(fSPC, TITLE_ADDR, SEEK_SET);
    fgets(tag.title,   33, fSPC);
    fgets(tag.game,    33, fSPC);
    fgets(tag.dumper,  17, fSPC);
    fgets(tag.comment, 33, fSPC);
    fseek(fSPC, LENGTH_ADDR, SEEK_SET);
    fgets(tag.length,   4, fSPC);
    fgets(tag.fadeout,  6, fSPC);
    isTextFormat = _isTagText(tag.length, 3) || _isTagText(tag.fadeout, 5);
    fseek(fSPC, isTextFormat ? ARTIST_ADDR_TXT : ARTIST_ADDR_BIN, SEEK_SET);
    fgets(tag.artist,  33, fSPC);

id666_end:
    fclose(fSPC);
    return tag;
}


// -------------------------------- WRITING ------------------------------------


/// @brief Zero pads (NUL char) to the right 'src' until it reaches 'len' size
/// @return dest, the right padded string
static char *_zeroPad(char *dest, const char *src, const size_t len) {
    memset(dest, '\0', len);
    memcpy(dest, src, strlen(src));
    return dest;
} // Derived from jim_mcnamara's answer https://community.unix.com/t/how-to-right-pad-with-zeros-using-sprintf/273667/6.


/// @brief Warning: overrides anything already present without additional confirmation.
///        Writes in text format and only the informations from the struct.
/// @param tag a struct of char*
/// @param file the path of the file to write to
/// @return 0: success, 1: file inaccessible, 2: file isn't SPC, 3: your computer is fried
int writeID666(id666tag tag, const char *file) {
    char magicBytes[34];
    char entry[32];

    FILE *fSPC = fopen(file, "r+");
    if (fSPC == NULL) {
        fprintf(stderr, "Failed to open %s: %s\n", file, strerror(errno));
        fclose(fSPC);
        return 1;
    }
    fgets(magicBytes, 34, fSPC);
    if (strcmp(magicBytes, MAGIC_BYTES) != 0) {
        fprintf(stderr, "%s is no SPC file\n", file);
        fclose(fSPC);
        return 2;
    }

    fseek(fSPC, TITLE_ADDR, SEEK_SET);
    fwrite(_zeroPad(entry, tag.title,   32), sizeof(char), 32, fSPC);
    fwrite(_zeroPad(entry, tag.game,    32), sizeof(char), 32, fSPC);
    fwrite(_zeroPad(entry, tag.dumper,  32), sizeof(char), 16, fSPC);
    fwrite(_zeroPad(entry, tag.comment, 32), sizeof(char), 32, fSPC);
    fseek(fSPC, LENGTH_ADDR, SEEK_SET);
    fwrite(_zeroPad(entry, tag.length,  32), sizeof(char),  3, fSPC);
    fwrite(_zeroPad(entry, tag.fadeout, 32), sizeof(char),  5, fSPC);
    fseek(fSPC, ARTIST_ADDR_TXT, SEEK_SET);
    fwrite(_zeroPad(entry, tag.artist,  32), sizeof(char), 32, fSPC);

    fclose(fSPC);
    return 0;
}



#ifdef __cplusplus
}
#endif




/*           https://wiki.superfamicom.org/spc-and-rsn-file-format

SPC File Format v0.30
=====================

Offset Size  Description
------ ----- ------------------------------------------------------------------
00000h    33 File header "SNES-SPC700 Sound File Data v0.30"
00021h     2 26,26
00023h     1 26 = header contains ID666 information
             27 = header contains no ID666 tag
00024h     1 Version minor (i.e. 30)

                                     ...

ID666 Tag (text format):
------------------------
0002Eh    32 Song title
0004Eh    32 Game title
0006Eh    16 Name of dumper
0007Eh    32 Comments
0009Eh    11 Date SPC was dumped (MM/DD/YYYY)
000A9h     3 Number of seconds to play song before fading out
000ACh     5 Length of fade in milliseconds
000B1h    32 Artist of song
000D1h     1 Default channel disables (0 = enable, 1 = disable)
000D2h     1 Emulator used to dump SPC:
              0 = unknown
              1 = ZSNES
              2 = Snes9x
000D3h    45 reserved (set to all 0's)

ID666 Tag (binary format):
--------------------------
0002Eh    32 Song title
0004Eh    32 Game title
0006Eh    16 Name of dumper
0007Eh    32 Comments
0009Eh     4 Date SPC was dumped (YYYYMMDD)
000A2h     7 unused
000A9h     3 Number of seconds to play song before fading out
000ACh     4 Length of fade in milliseconds
000B0h    32 Artist of song
000D0h     1 Default channel disables (0 = enable, 1 = disable)
000D1h     1 Emulator used to dump SPC:
              0 = unknown
              1 = ZSNES
              2 = Snes9x
000D2h    46 reserved (set to all 0's)

                                     ...                                      */
