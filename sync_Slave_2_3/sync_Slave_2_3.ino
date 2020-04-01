
#include <SPI.h>
#include <WiFi.h>
#include <PubNub.h>
#include <aJSON.h>
#include <Temboo.h>
#include "TembooAccount.h"

String twt;

// used to sync UART com to master
const int flagSlave = 39; // P2.6
const int flagMaster = A6;   // P4.7


#define LED RED_LED
#define LEDg GREEN_LED
#define LEDb BLUE_LED

//wifi credentials
char ssid[] = "DCU-guest-WiFi";
char password[] = "";
String tmp = "Taxi Stand";


////wifi credentials
//char ssid[] = "AndroidHotspot2844";
//char password[] = "cdd4ff73d3e5";

//char ssid[] = "3Wireless-Modem-dac1";
//char password[] = "42387227";

// your network key Index number (needed only for WEP)
static int keyIndex = 0;


// pubnub credentials
const static char pubkey[] = "pub-c-eddb4434-2478-47de-a4c9-2e3d097c19c9";
const static char subkey[] = "sub-c-b216c686-f395-11e6-a0e8-02ee2ddab7fe";
const static char channel[] = "Data1"; // applicaion specific channell



#define NUM_CHANNELS 9

int analogValues[NUM_CHANNELS];
char buffer[3];


boolean setupComplete = false;
unsigned long wdt;

int ct = 0;

void setup()
{


  // set up the flag pins
  pinMode(flagMaster, OUTPUT);
  //pinMode(flagSlave,INPUT);
  digitalWrite(flagMaster, LOW);



  // spin lock if no sheild present
  if (WiFi.status() == WL_NO_SHIELD)
    while (true) {
      digitalWrite(LED, HIGH);
      delay(150);
      digitalWrite(LED, LOW);
      delay(150);
    }

  digitalWrite(LED, HIGH);

  // attempt to connect to Wifi network-> red LED off when connected
  digitalWrite(LED, HIGH);
  digitalWrite(LEDg, LOW);
  int status;
  do {
    status = WiFi.begin(ssid);
    //status = WiFi.begin(ssid, password);
  } while (status != WL_CONNECTED);
  digitalWrite(LED, LOW);

  digitalWrite(LEDb, HIGH);
  Serial.begin(115200);
  char buffer[1];
  do {
    Serial.readBytes(buffer, 1);
  } while (buffer[0] != 'A');

  for (int i = 0; i < 5; i++)
    Serial.print('A');
  Serial.flush();
  digitalWrite(LEDb, LOW);

  PubNub.begin(pubkey, subkey);

  wdt = millis();

  analogValues[0] = 0;
  analogValues[1] = 0;
  analogValues[2] = 0;  // blocked indicator

  analogValues[3] = 255;

  for (int i = 4; i < 9; i++)
    analogValues[i] = 0;

  publishToCloud();
}






void loop() {

  readMaster();

  if (millis() - wdt > 500) {
    publishToCloud();
    wdt = millis();
  }

  digitalWrite(LED, HIGH);
  delay(50);
  digitalWrite(LED, LOW);
  delay(50);


}


void readMaster() {

  // execute read if signaled
  if (digitalRead(flagMaster) == HIGH) {

    // used for a loop timeout so wont hang
    unsigned long timer = millis();

    digitalWrite(flagSlave, HIGH);

    while (digitalRead(flagMaster) == HIGH && millis() - timer < 6000) {
      digitalWrite(LEDb, HIGH);
      delay(50);
      digitalWrite(LEDb, LOW);
      delay(50);
    }

    //read from master
    Serial.readBytes(buffer, 3);
    analogValues[0] = buffer[0];
    analogValues[1] = buffer[1];
    analogValues[2] = buffer[2];  // blocked indicator

    digitalWrite(flagSlave, LOW);


    // update the freeboard dashboard
    for (int i = 3; i < 9; i++) {

      if (analogValues[0] == i - 3)
        analogValues[i] = 255;
      else
        analogValues[i] = 0;
    }

    if (analogValues[0] != 13 && analogValues[0] != 255) {
//      if (i = 0) {
//        i++;
//        twt = "Position: 0, Destination: " + String(analogValues[1], DEC) + " Stamp: " + String(random(rand()));
//        tweet(twt);
//      }
      twt = "Position: ";
      twt +=  tmp;
      twt += ", Destination: ";
      twt += String(analogValues[0], DEC);
      twt += "    Stamp: ";
      twt += String(random(rand())); // random seed
      tweet(twt);
      tmp = String(analogValues[0], DEC);
    }
     
    
  }
}





// format sensor values into aJson object and publish to cloud
void publishToCloud() {

  WiFiClient *client;

  // formatted ajson object to be sent
  aJsonObject *msg = aJson.createObject();

  // create sender object and add to message object
  aJsonObject *sender = aJson.createObject();
  aJson.addStringToObject(sender, "name", "Arduino");
  aJson.addItemToObject(msg, "sender", sender);

  //ajson object which holds values read from previous function call
  aJsonObject *analog = aJson.createIntArray(analogValues, NUM_CHANNELS);

  // add reading to message object
  aJson.addItemToObject(msg, "analog", analog);

  // convert json object to charr array which pubnub api will understand
  char *msgStr = aJson.print(msg);

  // delete formatted json object
  aJson.deleteItem(msg);

  // trim buffer to size
  msgStr = (char *) realloc(msgStr, strlen(msgStr) + 1);

  // publish message to pubnub to cloud over wifi connection
  client = PubNub.publish(channel, msgStr);

  // free up memory assigned to msgStr
  free(msgStr);

  // flash red LED if client disconnects
  if (!client) {
    digitalWrite(LED, HIGH);
    delay(1250);
    digitalWrite(LED, LOW);
    delay(250);
  }

  client->stop();
}



void tweet(String tweet) {

  WiFiClient tweetClient;

  //Serial.println("Tweeting");
  TembooChoreo StatusesUpdateChoreo(tweetClient);

  // Invoke the Temboo client
  StatusesUpdateChoreo.begin();

  // Set Temboo account credentials
  StatusesUpdateChoreo.setAccountName(TEMBOO_ACCOUNT);
  StatusesUpdateChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  StatusesUpdateChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs

  StatusesUpdateChoreo.addInput("StatusUpdate", tweet);
  String ConsumerKeyValue = "YejB2vm1QXJYxwNg5pS9WEF70";
  StatusesUpdateChoreo.addInput("ConsumerKey", ConsumerKeyValue);
  String AccessTokenValue = "836348481337950209-1AX4CbEPEhUNFghfpgdA93GgHqu18Vy";
  StatusesUpdateChoreo.addInput("AccessToken", AccessTokenValue);
  String ConsumerSecretValue = "7OpIYZktDTRXurmDlKeEEAm9rNjX4FOGWlPHccdC7atMV9wLTc";
  StatusesUpdateChoreo.addInput("ConsumerSecret", ConsumerSecretValue);
  String AccessTokenSecretValue = "3lVjlUZopIpDk7VovQjNABZ8erkdPA9RQnmBGjf1wOP4m";
  StatusesUpdateChoreo.addInput("AccessTokenSecret", AccessTokenSecretValue);

  // Identify the Choreo to run
  StatusesUpdateChoreo.setChoreo("/Library/Twitter/Tweets/StatusesUpdate");

  // Run the Choreo; when results are available, print them to Serial
  // Boolean input to tell library to use HTTPS
  StatusesUpdateChoreo.run();

  StatusesUpdateChoreo.close();
  tweetClient.stop();
}


