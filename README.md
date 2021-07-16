# nfcplay
A small program to load, play and pause mpd playlists through nfc tags.
First of all, the code still needs some cleaning up, but it works as I would want it to.

Building with the Makefile installs into /usr/local/bin. The program itself looks for a file called matchlist in /opt/nfcplay/ and only there. This file should contain tab-separated
uids (without spaces) and mpd playlists.

An nfc-reader that is compatible with libnfc, a running mpd server and mpc are required.
If these requirements are met, the following things should work:
1) Playing a playlist by placing a tag on the reader.
2) Pausing a playlist by removing the tag from the reader.
3) Restarting the playlist from that point by placing the tag on the reader again.
4) Playing any other playlist by placing a different tag on the reader.
