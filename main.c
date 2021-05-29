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
    return 0;
}
