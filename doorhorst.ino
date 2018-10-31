/**
 * This Arduino Sketch is used to adjust the door status on the homepage using a single board computer.
 * 
 * @author Lars Riße
*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

String doorState = ""; // the currently set door status

char wlanSSID[] = "";
char wlanPassword[] = "";
char doorStateCheckUrl[] = ""; // The current state of the door must be found under this path.
char doorStateSetClosedUrl[] = ""; // This path is called to set the door status to closed.
char doorStateSetInternUrl[] = ""; // This path is called to set the door status to internal.
char doorStateSetOpenUrl[] = ""; // This path is called to set the door status to open.

int ledClosed = D7;
int ledIntern = D6;
int ledOpen = D5;

int buttonClosed = D3;
int buttonIntern = D2;
int buttonOpen = D1;

/**
 * In this method the program is prepared. It is connected to the configured WLAN and the current status is queried.
 */
void setup() {
  Serial.begin(9600);
  pinMode(ledClosed, OUTPUT);
  pinMode(ledIntern, OUTPUT);
  pinMode(ledOpen, OUTPUT);
  pinMode(buttonClosed, INPUT);
  pinMode(buttonIntern, INPUT);
  pinMode(buttonOpen, INPUT);

  WiFi.begin(wlanSSID, wlanPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  HTTPClient http;
  http.begin(doorStateCheckUrl);
  int code = http.GET();
  if (code > 0 ) {
    doorState = http.getString();
    if (doorState == "open") {
      switchLed(ledOpen);
    } else if (doorState == "internal") {
      switchLed(ledIntern);
    } else if (doorState == "closed") {
      switchLed(ledClosed);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(code).c_str());
  }
}

void loop() {
  if (checkNewState()) {
    if (doorState == "open") {
      switchLed(ledOpen);
    } else if (doorState == "internal") {
      switchLed(ledIntern);
    } else if (doorState == "closed") {
      switchLed(ledClosed);
    }
  } else {
    int buttonStateClosed = digitalRead(buttonClosed);
    int buttonStateIntern = digitalRead(buttonIntern);
    int buttonStateOpen = digitalRead(buttonOpen);

    if (buttonStateClosed == HIGH) {
      switchLed(ledClosed);
    } else if (buttonStateIntern == HIGH) {
      switchLed(ledIntern);
    } else if (buttonStateOpen == HIGH) {
      switchLed(ledOpen);
    }
  }
  delay(1000);
}

/**
 * This method is responsible for setting the statute and using the correct LED. This method calls the corresponding HTTP request, depending on which LED is to be used.
 * 
 * @param led LED OUTPUT which should be used
 */
void switchLed(int led) {
  HTTPClient http;

  switch (led) {
    case D7:
      digitalWrite(ledIntern, LOW);
      digitalWrite(ledOpen, LOW);
      digitalWrite(led, HIGH);
      http.begin(doorStateSetClosedUrl);
      break;
    case D6:
      digitalWrite(ledClosed, LOW);
      digitalWrite(ledOpen, LOW);
      digitalWrite(led, HIGH);
      http.begin(doorStateSetInternUrl);
      break;
    case D5:
      digitalWrite(ledIntern, LOW);
      digitalWrite(ledClosed, LOW);
      digitalWrite(led, HIGH);
      http.begin(doorStateSetOpenUrl);
      break;
  }

  int code = http.GET();
  if (code > 0 ) {
    Serial.println("done");
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(code).c_str());
  }
}

/**
 * This method checks every second at the beginning of the loop whether the door status has changed manually and then changes the LED.
 * 
 * @return If the status has changed manually, a true is returned. Otherwise a false is returned.
 */
boolean checkNewState() {
  HTTPClient http;
  http.begin(doorStateCheckUrl);
  int code = http.GET();
  if (code > 0 ) {
    String newDoorState = http.getString();
    if (newDoorState != doorState) {
      doorState = newDoorState;
      return true;
    }
    return false;
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(code).c_str());
    return false;
  }
}
