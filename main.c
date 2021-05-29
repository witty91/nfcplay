#include <stdio.h>
#include <stdlib.h>
#include <nfc/nfc.h>
#include <usb.h>

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
    return 0;
}
