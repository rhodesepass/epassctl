#include <stdio.h>
#include "ipc_client.h"
#include "log.h"


int main(int argc, char *argv[]) {
    ipc_client_t client;
    
    log_info("srgn");
    ipc_client_init(&client);
    ipc_client_destroy(&client);
    return 0;
}