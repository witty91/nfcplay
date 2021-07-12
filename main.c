#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <nfc/nfc.h>
#include <unistd.h>
#include <usb.h>

struct idMatchPlaylist {
    uint64_t uid;
    char *plname;
};

typedef struct idMatchPlaylist idMatchPlaylist;

uint64_t hex2int(char *hex) {
    uint64_t val = 0;
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


static uint64_t
get_hex(const uint8_t *pbtData, const size_t szBytes)
{
  size_t  szPos;
  uint64_t uid = 0;
  for (szPos = 0; szPos < szBytes; szPos++) {
   // printf("%02x", pbtData[szPos]);
   //printf("pre shift: %x\n",uid);
    uid = uid * 256;
    //printf("post shift: %x\n",uid);
    uid = uid + pbtData[szPos];
  }

  return uid;
}

int main()
{
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
    uint64_t hex, oldhex = 0;
    bool playing = false;
    while (true){
    if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0) {
        hex = get_hex(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
        printf("uid is: %lx\n", hex);
        for (int i = 0; i < nLines; i++){
                //printf("%014lx\n", matcher[i].uid);
            if (matcher[i].uid == hex && oldhex != hex){
                //printf("Found a match(%014lx), playing %s\n", matcher[i].uid, matcher[i].plname);
                system("mpc clear");
                char *command[100];
                //printf("%s\n", command);
                snprintf(command, sizeof(command), "mpc load %s", matcher[i].plname);
                system(command);
                oldhex = hex;
            }else if ((oldhex == hex) && playing == false){
                system("mpc play");
                playing = 1;
            }
        }
    }else if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) <= 0 && playing == true){
        //printf("got here.\n");
        system("mpc pause");
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
