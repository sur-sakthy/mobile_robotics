// -- globals -------------------------------------
// current position and next position
    int pos=4;
    int nextPos=0;

    int i = 0;

// default values
    boolean hold=false;
    boolean destination=false;
    boolean wait=true;
    boolean start=true; // set this false after moving off
    boolean send = false;
   
    int blocked=0;

// -------------------------------------------------

// libraries
    #ifndef __CC3200R1M1RGC__
    #include <SPI.h>
    #endif
    #include <WiFi.h>
    #include <WiFiClient.h>

// network credentials
    char ssid[] = "NETGEAR72";
    char password[] = "littlecello367";
    int keyIndex = 0;
    uint16_t port = 80;     // port number of the server
    IPAddress server(192, 168, 1, 2);   // IP Address of the server
    WiFiClient client;

// used to sync UART com to slave
    const int flagMaster = 37; // P5.6 - P4.7   on slave
    const int flagSlave = A0; // P5.5 - P2.6 -  on slave
    
// LED definitions
    #define LED RED_LED
    #define gLED GREEN_LED
    #define LEDb BLUE_LED




void setup() {

    // set up the flag pins
    pinMode(flagSlave, OUTPUT);
    digitalWrite(flagSlave, LOW);
    
    
    digitalWrite(LED, HIGH);
    startWiFi(); 
    digitalWrite(LED, LOW);

    Serial.begin(115200);

    // allow contact with other MSP432
    establishContact();

   delay(500);
    
    


}



void loop() {

    if(i == 0){
      updateGateway();  
      i++;
    }
    
    if(WiFi.status() != WL_CONNECTED){
      digitalWrite(RED_LED, HIGH);       // turn the LED off (LOW is the voltage level)
    }

    
    
    if(nextPos != -1){
         if(destination){
               
//               nextPos = 0;
//               if (i == 0){
//                  updateGateway(); 
//                  i++;
//               }
               SendToServer(pos);
               nextPos = ReadFromServer();
//               if(i != 0){
                  updateGateway();  
//               }
               miDelay(500);
               hold=false;                  
               destination=false;           
        }
    }
      if(blocked == 255)  updateGateway(); 
    
}




void updateGateway(){

      if(pos==13 || nextPos==10) return;
      
      // used for a loop timeout so wont hang
      unsigned long timer = millis();
      
      //signal to slave
      digitalWrite(flagMaster, HIGH);

      while(digitalRead(flagSlave)==LOW && millis()-timer<6000){digitalWrite(LEDb, HIGH);delay(50);digitalWrite(LEDb, LOW);delay(50);}
  
      // write data 
      Serial.write(nextPos);
      Serial.write(nextPos);
      Serial.write(blocked);

      digitalWrite(flagMaster,LOW);
     
      // Wait forslave flag to go LOW and signal end of transmission
      while(digitalRead(flagSlave)==HIGH && millis()-timer<6000){digitalWrite(LEDb, HIGH);delay(50);digitalWrite(LEDb, LOW);delay(50);}
     
}



void establishContact() {
  while (Serial.available() <= 0) {
      Serial.println('A');
      digitalWrite(LED, HIGH);   
      delay(150);               
      digitalWrite(LED, LOW);    
      delay(150);              
  }
  Serial.flush();
}


//Read reply from Server
  int ReadFromServer(){
      String tmp="";
      char buffer[255] = {0}; while (!client.available()) {}; //Wait for connection to be available...
      if (client.available()) {
        client.read((uint8_t*)buffer, client.available());
      }
      String finalMessage = buffer;
      tmp += String(finalMessage.charAt(178));
      if(tmp.equals("T")){ return -1; }
//      Serial.print("tmp: ");  Serial.println(tmp);
      int pos = tmp.toInt();
//      Serial.print("Next position: "); Serial.println(pos);
      return pos;
  }

//Send message to Server
  void SendToServer(int pos){
      String messageToSend = "POST /mobilerobotics/api/position/tag/?group=2&pos=";  // append string to send to position to server
      messageToSend += String(pos);
      messageToSend += " HTTP/1.1"; 
      client.println(messageToSend);
      client.println("Host: 192.168.1.2");
      client.println();
  }

// prints status of wifi sheild
void printWifiStatus() {
      // print the SSID of the network you're attached to:
//      Serial.print("Network Name: ");
//      Serial.println(WiFi.SSID());
      // print your WiFi shield's IP address:
      IPAddress ip = WiFi.localIP();
//      Serial.print("IP Address: ");
//      Serial.println(ip);
      // print the received signal strength:
      long rssi = WiFi.RSSI();
//      Serial.print("signal strength (RSSI):");
//      Serial.print(rssi);
//      Serial.println(" dBm");
}


void startWiFi(){
      pinMode(RED_LED, OUTPUT);      
     
      WiFi.begin(ssid, password);
      
      // waiting to connect
      while ( WiFi.status() != WL_CONNECTED) { delay(300);}
      
      // waiting fo IP
      while (WiFi.localIP() == INADDR_NONE) {  delay(300);}
      
      uint8_t tries = 0;
      while (client.connect(server, port) == false) {
        if (tries++ > 100) { while(1); }
        delay(100);
      }
}

