#include "Secrets.h"
#include "FastLED.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

// WEBSERVER

// Copy/Paste these into Secrets.h, uncomment, and enter valid values.
// const char* host = "esp32";
// const char* ssid = "SSID-CASEENSITIVE";
// const char* password = "PASSWORD";

WebServer server(80);

// PIR Sensor

#define PIR_DATA_PIN 14

//timer variables:
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
unsigned long pirTimeoutSeconds = 1; // LED will turn off after the number of seconds defined

// LEDs

#define LED_DATA_PIN 26 // NOT CORRECT!
#define NUM_LEDS 50 // How many leds in your strip(s)?

// Define the array of leds
CRGB leds[NUM_LEDS];

// Script Variables
unsigned int pattern = 0; // Indicates which pattern is currently playing on the LEDs
unsigned int startingLed = 0; // For patterns that travel, this is the LED at the start of the pattern

/*
   Login page - TODO extract username and password
*/

const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>ESP32 Login Page</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='admin')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

/*
   Server Index Page
*/

const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

void setup() {
  Serial.begin(115200);//initialise serial communication at 115200 bps

  // Wifi Setup

  Serial.println("");
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wifi Network ");
  Serial.println(ssid);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    //    while (1) {
    //      delay(1000);
    //    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

  // PIR Setup

  pinMode(PIR_DATA_PIN, INPUT_PULLUP); //PIR Motion Sensor mode INPUT_PULLUP
  attachInterrupt(digitalPinToInterrupt(PIR_DATA_PIN), movementDetected, RISING);

  // LED Setup

  FastLED.addLeds<WS2811, LED_DATA_PIN, RGB>(leds, NUM_LEDS);
}
  
//checks if motion was detected, sets a timer until motion no longer detected
void movementDetected() {
  lastTrigger = millis();
  Serial.println("MOTION DETECTED!!! " + lastTrigger); // TODO nice human readable date.
  startTimer = true;
  startingLed = 0; // Reset startingLed
}

void loop() {

  // Do Server stuff
  server.handleClient();
  delay(1);

  now = millis(); //current time

  //turn off the LED after the number of seconds defined in the timeSeconds variable
  if (startTimer && (now - lastTrigger > (pirTimeoutSeconds * 1000))) {
    Serial.println("Motion stopped..." + now);  // TODO nice human readable date.
    startTimer = false;
    startingLed = 0; // Reset startingLed
  }

  if (startTimer) {
    choosePattern();
//    patternDanger();
  } else {
    choosePattern();
  }
}

void setEvery(int step, CRGB set, CRGB unset) {

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i % step == startingLed)
    {
      leds[i] = set;
    }
    else if (unset)
    {
      leds[i] = unset;
    }
  }
  startingLed = (startingLed + 1) % step;
}

void choosePattern(){
  startingLed = 0;
}

void patternDanger()
{
  setEvery(3, CRGB::Red, CRGB::Black);
  FastLED.show();
  delay(50);
}

void fadeall_cylon() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = scale8(leds[i].r, 250);
    leds[i].g = scale8(leds[i].g, 250);
    leds[i].b = scale8(qadd8(leds[i].b, 25), 150);
  }
}

void patternCylon() {
  static uint8_t hue = 0;

  // First slide the led in one direction
  for (int i = 0; i < NUM_LEDS; i++) {

    if ( hue == 144 ) { // Skip the blue range, since we're defaulting them to blue.
      hue = 176;
    }

    // Set the i'th led to the current hue
    leds[i] = CHSV(hue++, 255, 255);

    // Show the leds
    FastLED.show();

    // now that we've shown the leds, fade all of them a little bit;
    fadeall_cylon();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }

  // Now go in the other direction.
  for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
    if ( hue == 150 ) { //Skip the blue range, since we're defaulting them to blue.
      hue = 190;
    }
    // Set the i'th led to the current hue
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();

    // Wait longer in the reverse direction
    fadeall_cylon();
    delay(30);
    //    fadeall_cylon();
    //    delay(10);
    //    fadeall_cylon();
    //    delay(10);
  }
}
