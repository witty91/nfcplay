#include <stdio.h>
#include <stdlib.h>
#include <nfc/nfc.h>
#include <usb.h>

static void
print_hex(const uint8_t *pbtData, const size_t szBytes)
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
  printf("%x", uid);
  printf("\n");

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
  while (true){
  if (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0) {
    print_hex(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
//    print_hex(&nt.nti.nai.btSak, 1);
    if (nt.nti.nai.szAtsLen) {
      printf("          ATS (ATR): ");

      //print_hex(nt.nti.nai.abtAts, nt.nti.nai.szAtsLen);
    }
  }
  }
  // Close NFC device
  nfc_close(pnd);
  // Release the context
  nfc_exit(context);
  exit(EXIT_SUCCESS);
}
