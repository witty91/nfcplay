#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <nfc/nfc.h>
#include <unistd.h>
#include <usb.h>
#include <assert.h>
#include <mpd/client.h>

struct idMatchPlaylist {
    uint32_t uid;
    char *plname;
};

typedef struct idMatchPlaylist idMatchPlaylist;

uint32_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

idMatchPlaylist getidMatchPlaylist(FILE *matchlist){
    idMatchPlaylist imp;
    char *line[250];
    char *uidc;
    fgets(line, 250, matchlist);
    printf("%s\n", line);
    uidc = strtok(line, "\t");
    //printf("read uid %s\n", uid);
    imp.plname = strdup(strtok(0, "\t"));
    //printf("read plname %s\n", plname);
    imp.uid = hex2int(uidc);
    return imp;
}
int getNumberOfLines(FILE *matchlist){
    int lines = 0;
    char ch;
    while(!feof(matchlist)){
        ch = fgetc(matchlist);
        if(ch == '\n')
        {
            lines++;
        }
    }
    return lines;
}


static uint32_t
get_hex(const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;
  uint32_t uid = 0;
  for (szPos = 0; szPos < szBytes; szPos++) {
   // printf("%02x", pbtData[szPos]);
   //printf("pre shift: %x\n",uid);
    uid = uid * 256;
    //printf("post shift: %x\n",uid);
    uid = uid + pbtData[szPos];
  }

  return uid;
}

handle_error(struct mpd_connection *c)
{
	assert(mpd_connection_get_error(c) != MPD_ERROR_SUCCESS);

	fprintf(stderr, "%s\n", mpd_connection_get_error_message(c));
	mpd_connection_free(c);
	return EXIT_FAILURE;
}

int main()
{
	struct mpd_connection *conn;

	conn = mpd_connection_new("127.0.0.1", 0, 30000);

	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
		return handle_error(conn);

	{
		int i;
		for(i=0;i<3;i++) {
			printf("version[%i]: %i\n",i,
			       mpd_connection_get_server_version(conn)[i]);
		}
	}


    nfc_device *pnd;
    nfc_target nt;
    nfc_context *context;
    nfc_init(&context);
    if (context == NULL){
        printf("Unable to init libnfc \n");
        exit(EXIT_FAILURE);
    }
    pnd = nfc_open(context, NULL);

    if (pnd == NULL) {
    printf("ERROR: %s\n", "Unable to open NFC device.");
    exit(EXIT_FAILURE);
  }
  // Set opened NFC device to initiator mode
  if (nfc_initiator_init(pnd) < 0) {
    nfc_perror(pnd, "nfc_initiator_init");
    exit(EXIT_FAILURE);
  }

  printf("NFC reader: %s opened\n", nfc_device_get_name(pnd));
      // Poll for a ISO14443A (MIFARE) tag
  const nfc_modulation nmMifare = {
    .nmt = NMT_ISO14443A,
    .nbr = NBR_106,
  };
    int nLines = 0;
    FILE *matchlist;
    if ((matchlist = fopen("/opt/nfcplay/matchlist", "r")) == NULL){
        printf("your matchlist does not exist.\n");
        exit(1);
    }
    nLines = getNumberOfLines(matchlist);
    idMatchPlaylist matcher[nLines];
    rewind(matchlist);
    for (int i = 0; i < nLines; i++){
        matcher[i] = getidMatchPlaylist(matchlist);
        printf("received: %014lx and %s\n",matcher[i].uid,matcher[i].plname);
    }
    uint32_t hex, oldhex = 0;
    bool playing = false;
    while (true){
    if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0) {
        hex = get_hex(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
        printf("uid is: %lx\n", hex);
        for (int i = 0; i < nLines; i++){
                //printf("%014lx\n", matcher[i].uid);
            if (matcher[i].uid == hex && oldhex != hex){
                //printf("Found a match(%014lx), playing %s\n", matcher[i].uid, matcher[i].plname);
                mpd_run_clear(conn);
                char *plname = strtok(matcher[i].plname, "\n");
                mpd_run_load(conn, plname);
                oldhex = hex;
            }else if ((oldhex == hex) && playing == false){
                mpd_run_play(conn);
                playing = 1;
            }
        }
    }else if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) <= 0 && playing == true){
        //printf("got here.\n");
        mpd_run_pause(conn, true);
        playing = 0;
    }
    //sleep(1);
  }
  // Close NFC device
  nfc_close(pnd);
  // Release the context
  nfc_exit(context);
  exit(EXIT_SUCCESS);
}
