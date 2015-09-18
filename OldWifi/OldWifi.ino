#include <WiFly.h>
#include <SPI.h>

//WiFly
WiFlyServer server(80);
void setup() {
  //WiFly
  Serial.begin(9600);
  WiFly.begin();
  Serial.println(WiFly.ip());
  server.begin();

}

String testmsg = "test";
void loop() {
   //WiFly
  WiFlyClient client = server.available();

 if(client) {
    while(client.connected()) {
      if(client.available()){

        //Webpage
        //client.println("HTTP/1.1 200 OK");
        //client.println("Content-Type: text/html");
        //client.println();

        client.println("<html>");
        client.println("<center>");
        client.println("<p>");
        client.println(testmsg);
        client.println("</p>");
        client.println("<center>");
        client.println("</html>");
      }
    }
    delay(4);
    client.stop();
  }
 

}
