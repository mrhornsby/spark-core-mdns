#include "MDNS.h"

#define HTTP_PORT 80

MDNS mdns;

String html = "<html><body><h1>Ok!</h1></body></html>";

TCPServer server = TCPServer(HTTP_PORT);

void setup() {
    bool success = mdns.setHostname("core-1");
    
    if (success) {
        success = mdns.setService("_http._tcp", HTTP_PORT, "Core 1");
    }
    
    if (success) {
        success = mdns.addTXTEntry("coreid", "1");
    }
    
    if (success) {
        success = mdns.begin();
    }
    
    if (success) {
        Spark.publish("mdns/setup", "success");
    } else {
        Spark.publish("mdns/setup", "error");
    }
}

void loop() {
    mdns.processQueries();
    
    TCPClient client = server.available();
        
    if (client){
        client.println(html);
        client.println();
        client.flush();
        client.stop();
    }
}
