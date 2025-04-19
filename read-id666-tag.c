/*
 * read-id666-tag.c
 * Reads ID666 tags from SPC files.  Hasn't been extremely well tested, but
 * seems to work well here.
 *
 * Copyright (c) 2004 Michael Pyne <michael.pyne@kdemail.net>
 *
 * ID666 is documented at http://www.snesmusic.org/files/spc_file_format.txt,
 * although the docs are a little confusing.  For example, there are text and
 * binary ID666 tags, but the difference is only in the date, which most SPCs
 * I've seen don't bother doing anything with anyways.
 *
 * This will become a KFileMetaInfo plugin later, after much code cleanup.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void display_info(FILE *f)
{
    char buffer[210];
    char info[33] = { 0 };
    int time;
    fseek (f, 0x2E, SEEK_SET);

    if(fread(buffer, 210, 1, f) < 1) {
	fprintf (stderr, "Couldn't read from file: %s\n", strerror(errno));
	return;
    }

    if(buffer[0x9E - 0x2E + 2] == '/')
	printf("Text ID666 tag\n");
    else {
	printf("Binary ID666 tag\n");

	memcpy(info, buffer, 32);
	printf ("Song: %s\n", info);
	memcpy(info, buffer + 32, 32);
	printf ("Game: %s\n", info);
	memcpy(info, buffer + 64, 16);
	info[16] = 0;
	printf ("Dumped by: %s\n", info);
	memcpy(info, buffer + 0x7E - 0x2E, 32);
	printf ("Comments: %s\n", info);
	memcpy(info, buffer + 0x9E - 0x2E, 4);
	time = *((int*)info);
	info[4] = 0;
	printf("Date: %d\n", time);

	memcpy(info, buffer + 0xA9 - 0x2E, 3);
	info[3] = 0;
	time = atoi(info);

	printf ("Time to play: %d seconds\n", time);

	memcpy(info, buffer + 0xAC - 0x2E, 4);
	info[4] = 0;
	time = atoi(info);

	printf ("Time to fadeout: %d ms\n", time);

	memcpy(info, buffer + 0xB0 - 0x2E, 32);
	info[32] = 0;
	printf ("Song artist: %s\n", info);

	if(buffer[0xD1 - 0x2E] == 1)
	    printf ("Dumped by: zSNES\n");
	else if(buffer[0xD1 - 0x2E] == 2)
	    printf ("Dumped by: Snes9x\n");
	else
	    printf ("Dumped by: Unknown\n");
    }
}

// Here is a comment so you can't accuse me of not having comments in my
// code. :P
int main(int argc, char**argv)
{
    int i;
    FILE *f;
    char buffer[38];

    // The docs have a longer label, but I don't want to be too restrictive.
    char *header = "SNES-SPC700 Sound File Data";

    for (i = 1; i < argc; ++i) {
	f = fopen(argv[i], "r");
	if(!f) {
	    fprintf (stderr, "Failed: %s due to %s\n", argv[i], strerror(errno));
	    continue;
	}

	// Read in a block of the .spc file to determine its info.
	if(fread(buffer, 37, 1, f) < 1) {
	    fprintf (stderr, "Failed to read from %s: %s\n", argv[i], strerror(errno));
	    fclose(f);
	    continue;
	}

	if(memcmp(buffer, header, strlen(header)) != 0) {
	    fprintf (stderr, "%s doesn't appear to be an SPC file.\n", argv[i]);
	    fclose(f);
	    continue;
	}

	if(buffer[0x23] == 26) {
	    printf ("%s has an ID666 tag\n", argv[i]);
	}
	else if(buffer[0x23] == 27) {
	    printf ("%s has no ID666 tag\n", argv[i]);
	}
	else {
	    printf ("Not sure about %s bub.\n", argv[i]);
	}

	printf ("Version minor: %d\n", (int) buffer[0x24]);
	display_info(f);
    }

    return 0;
}
