#include "FastLED.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

// WEBSERVER

const char* host = "esp32";
const char* ssid = "SSID-CASEENSITIVE";
const char* password = "PASSWORD";

WebServer server(80);

/*
 * Login page
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
 * Server Index Page
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



// How many leds in your strip(s)?
#define NUM_LEDS 50

// Below is boilerplate code from some of the FastLED examples - see there for more info
#define DATA_PIN 17

// Define the array of leds
CRGB leds[NUM_LEDS];
int sensorInputPin = 13;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int pirValue = 0;                    // variable for reading the pin status
int alert_red = 0;                    // counter to show red leds
int alert_green = 0;                  // counter to show green leds - will override red.


void setup() {
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

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
    while (1) {
      delay(1000);
    }
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

 FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
}
  
  


void loop() 

{
   server.handleClient();
  delay(1);



  pirValue = digitalRead(sensorInputPin);  // read input value
  if (pirValue == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      pirState = HIGH;

    }

    for (int i = 0; i < NUM_LEDS; i++) {
      if (i % 11 == alert_green)
      {
        leds[i] = CRGB::Green;
      }
      else if (i % 3 == alert_red)
      {
        leds[i] = CRGB::Red;
      }
      else
      {
        leds[i] = CRGB::Black;
      }
    }
    //      for (int i = alert_red; i < NUM_LEDS; i+=3) {
    //        leds[i].CRGB::Red;
    //      }
    alert_red++;
    alert_green++;
    alert_red %= 3;
    alert_green %= 11;
    FastLED.show();
    delay(50);
    return;
  }
  if (pirState == HIGH) {
    // we have just turned of
    Serial.println("Motion ended!");
    // We only want to print on the output change, not state
    pirState = LOW;
  }
  //else
  cylon();

}

void setEvery(int step) {
  // TODO make alternating more generic
}

void marquee() {
  // TODO
  int step = NUM_LEDS / 10;
  int half_step = step / 2;
  for (int dot = 0; dot < NUM_LEDS / 10; dot++) {
    leds[dot] = CRGB::Red;
    //leds[dot+ half_step] = CRGB:Green;
    FastLED.show();
    delay(30);
    leds[dot] = CRGB::Black;
    //leds[dot+ half_step] = CRGB:Black;
    FastLED.show();
  }
}




void fadeall_cylon() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = scale8(leds[i].r, 250);
    leds[i].g = scale8(leds[i].g, 250);
    leds[i].b = scale8(qadd8(leds[i].b, 25), 150);
  }
}

void cylon() {
  static uint8_t hue = 0;

  // First slide the led in one direction
  for (int i = 0; i < NUM_LEDS; i++) {

    if ( hue == 150 ) { // Skip the blue range, since we're defaulting them to blue. 
      hue = 190;
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
