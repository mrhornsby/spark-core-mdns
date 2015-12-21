#include "MDNS/MDNS.h"

#define HTTP_PORT 80

MDNS mdns;

TCPServer server = TCPServer(HTTP_PORT);

void setup() {
    server.begin();

    bool success = mdns.setHostname("core-1");

    if (success) {
        success = mdns.setService("tcp", "http", HTTP_PORT, "Core 1");
    }

    if (success) {
        success = mdns.addTXTEntry("coreid", "1");
    }

    if (success) {
        success = mdns.begin();
    }
}

void loop() {
    mdns.processQueries();

    TCPClient client = server.available();

    if (client){
        while (client.read() != -1);
        
        client.write("HTTP/1.1 200 Ok\n\n<html><body><h1>Ok!</h1></body></html>\n\n");
        client.flush();
        delay(5);
        client.stop();
    }
}
