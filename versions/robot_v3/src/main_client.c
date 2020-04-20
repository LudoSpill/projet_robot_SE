//#include "./telco/client.h"
#include "./telco/remoteui.h"
#include <unistd.h>


int main(){

    RemoteUI_new();
    RemoteUI_start();
    RemoteUI_free();
    return 0;
}   
