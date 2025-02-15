// V 1.0

// Uses https://www.weatherapi.com Api key to retrieve weather data. create an account and add your own key (FREE)

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>  // #include <Arduino_JSON.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>

// #include <esp_now.h>
// #include <esp_wifi.h>

// #include "ESP32TimerInterrupt.h"

#include "weatherIcons.h" 
// #include "corea.c"
// #include "gvar.h"
// #include "timer.h"
// #include "menu.h"
// #include "button.h"
// #include "cityWeather.h"
// #include "httpGet.h"
// #include "gfunc.h"

// #include "espcom.h"
// #include "init.h"


#define TFT_DC 8
#define TFT_RST 9
#define TFT_CS 10

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#define WIDTH 128
#define HEIGHT 160

// =============== WIFI SETTINGS ===============
struct WiFiCredential {
  const char *ssid;
  const char *password;
};
bool isConnected = false;
bool networkScanFlag = false;
int numNetworks;
unsigned long lastWifiCheck = 0;
WiFiCredential wifiList[] = {
  
};
const String weatherApiKey = "";


// =============== HTTP SETTINGS ===============
const String city[4] = { "BRUSSEL", "ANVERS", "LIEGE", "MONS" };
String const *cityP = city;
const String countryCode = "BE";

typedef struct httpInfo {
  String temp[4];
  String humidity[4];
  String windSpeed[4];
  String all[4];
} httpInfo;

httpInfo httpinfo;
String jsonBuffer;




// =============== TIMER =======================
volatile int interruptCounter = 0;  // Interrupt counter (volatile as it is accessed in ISR)
int totalInterruptCounter = 0;      // Total interrupt counter (for tracking total ISR calls)

unsigned long previousMillis = 0;  // Stores the last time the timer was triggered
unsigned long timerInterval = 0;   // Timer interval in milliseconds (equivalent to the original seconds)
unsigned long currentMillis = 0;   // Variable to store current time (for comparison)


// ============= UI VARIABLES ===============
#define mainView_bannerColor ST77XX_RED
#define cursorColor ST77XX_BLUE  //Dodger blue
#define highlightColor ST77XX_GOLD
#define bootLogColor ST77XX_DARKGREEN

// From ILI9341 - ALL <TFT_BLACK> etc need to be updated as ST77XX_BLACK (same for all colors).
#define ST77XX_NAVY 0x000F         ///<   0,   0, 123
#define ST77XX_DARKGREEN 0x03E0    ///<   0, 125,   0
#define ST77XX_DARKCYAN 0x03EF     ///<   0, 125, 123
#define ST77XX_MAROON 0x7800       ///< 123,   0,   0
#define ST77XX_PURPLE 0x780F       ///< 123,   0, 123
#define ST77XX_OLIVE 0x7BE0        ///< 123, 125,   0
#define ST77XX_LIGHTGREY 0xC618    ///< 198, 195, 198
#define ST77XX_DARKGREY 0x7BEF     ///< 123, 125, 123
#define ST77XX_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ST77XX_PINK 0xFC18         ///< 255, 130, 198

//========Screen=========
int stage = 0;
unsigned long screenUpdateMillis = 0;  
const unsigned long screenUpdateInterval = 5000;



int esp = 0;
int timer = 0;
bool INIT_HTTP = true;

// ============= FUNCTION DECLARATION ===============
void welcome();
void welcomeClose();
void transition();
void connectToWiFi();
void switchScreen();

String httpGETRequest(const char *serverName);
void httpUpdate(int cityNum);
void httpInit();

void cityInfo(String cityName, int cityNum);
void snowIcon(int x, int y, int size, int i);
int KtoC(String k);

bool timerFin();
void timerInit(int sec);

int textWidth(String text, Adafruit_GFX &tft, int textSize);
int textHeight(String text, Adafruit_GFX &tft, int textSize);
void drawText(int x, int y, int textColor, String text, int textSize);
void bootLog(String text, int textColor, int Row);






//===========================================================
//===========================================================
void setup() {
  Serial.begin(115200);
  Serial.println("serialStart");


  // TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);

  welcome();
  connectToWiFi();
}


void loop() {
  if (!isConnected) connectToWiFi();

  if (isConnected && INIT_HTTP) {
    // httpInit();
    switchScreen(); 
    INIT_HTTP = false;
  }


  welcomeClose();
  if(millis() - screenUpdateMillis > screenUpdateInterval)  {
    switchScreen(); 
    screenUpdateMillis=millis();
  }

  // updatehttpESP();

  // loop screens
  // if(millis()-timer > 5000){
  //     Serial.println(stage);
  //     stage = (stage+1)%3;
  //     timer=millis();
  // }

  // Display Memory Usage
  if (millis() - esp > 1000) {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    esp = millis();
  }

  // delay(100);
}





// =================================================
//                  BOOT
// =================================================
void welcome() {

  transition();
  timerInit(8);
  

  //==========RAINBOW==============
  // From bootLog();
    // int startY = Row * margin + (Row-1)*textHeight("A", tft, 1); //Row * (textHeight("A", tft, 1) - 2);
    // int endY = startY + textHeight("A", tft, 1) + margin;
    // worst case is row=2
  int bootLogY = 2*(2 + textHeight("A", tft, 1));
  

  int maxRadius = WIDTH/4;  // Maximum radius of the largest circle
  int step = 5;        // Step size for decreasing radii
  int numCircles = 8;  // Total number of circles
  int colors[] = {
      ST77XX_RED, ST77XX_ORANGE, ST77XX_YELLOW, ST77XX_GREEN,
      ST77XX_CYAN, ST77XX_DARKCYAN, ST77XX_PURPLE, ST77XX_BLACK
  };

  int yPos = bootLogY + maxRadius;

  for (int i = 0; i < numCircles; i++) {
      int radius = maxRadius - (i * step);
      tft.fillCircle(WIDTH * 0.5, yPos, radius, colors[i]);
  }


  //========Title Background=========
  tft.fillRect(0, yPos, WIDTH, HEIGHT, ST77XX_BLACK);

  // Text
  String t1 = "Sky-Watch";
  String t2 = "Weather";
  String t3 = "Station";

  int titleSize = 2;
  int titleHeight = textHeight("A", tft, titleSize);
  int margin = 3;
  // int xPos=

  drawText((WIDTH - textWidth(t1, tft, titleSize)) / 2, yPos + margin, ST77XX_WHITE, t1, titleSize);
  drawText((WIDTH - textWidth(t2, tft, titleSize)) / 2, yPos + (titleHeight + margin), ST77XX_WHITE, t2, titleSize);
  drawText((WIDTH - textWidth(t3, tft, titleSize)) / 2, yPos + 2 * (titleHeight + margin), ST77XX_WHITE, t3, titleSize);

  //     tft.setCursor(65, HEIGHT * 0.5);
  //     tft.println(t1);
  //     tft.setCursor(60, HEIGHT * 0.58);
  //     tft.print(t2);
  //     tft.setCursor(60, HEIGHT * 0.66);
  //     tft.print(t3);
}




void welcomeClose() {
  if (timerFin() && stage == 0) {
    stage = 1;
    transition();
  }
}
void transition() {
  uint16_t colors[] = {
    ST77XX_RED, ST77XX_ORANGE, ST77XX_YELLOW, ST77XX_GREEN,
    ST77XX_CYAN, ST77XX_DARKCYAN, ST77XX_PURPLE, ST77XX_BLACK
  };

  for (int i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
    tft.fillScreen(colors[i]);
    delay(150);  // Small delay of 300ms
  }
}

// int textLength;
// int rowNum;
void connectToWiFi() {
  // lastWifiCheck = millis();
  static int attempts = 0;  // Track connection attempts
  static bool connecting = false;
  static unsigned long lastAttemptTime = 0;
  // loadingBar(); //reload loadingBar()


  // Scan for Networks
  if (!networkScanFlag) {
    bootLog("Scanning for networks...", bootLogColor, 0);
    numNetworks = WiFi.scanNetworks();
    if (numNetworks == 0) {
      bootLog("No networks found.", ST77XX_RED, 0);
      return;
    } else {
      bootLog(String("Found " + String(numNetworks) + " networks."), bootLogColor, 0);
    }

    networkScanFlag = true;
  }
  // loadingBar(); //reload loadingBar()

  // Find and connect to wifi from given list
  if (!connecting) {
    for (int i = 0; i < sizeof(wifiList) / sizeof(wifiList[0]); i++) {
      for (int j = 0; j < numNetworks; j++) {
        if (WiFi.SSID(j) == wifiList[i].ssid) {

          bootLog("Connecting to: ", ST77XX_GREEN, 0);
          bootLog(String(wifiList[i].ssid) + ".", bootLogColor, 1);

          // String message = String("Connecting to: " + String(wifiList[i].ssid) + ".");
          // textLength = textWidth(message, tft);
          // rowNum = (j+1);

          // Serial.printf("Connecting to SSID: %s\n", wifiList[i].ssid);
          // bootLog(message, ST77XX_GREEN, 0);

          // drawText(0,rowNum*6,ST77XX_GREEN ,message);
          WiFi.begin(wifiList[i].ssid, wifiList[i].password);

          connecting = true;
          attempts = 0;
          lastAttemptTime = millis();  // Start connection timer
          return;
        }
      }
    }
  }
  // loadingBar(); //reload loadingBar()

  // Attempt
  if (connecting) {
    // Check connection every 500ms without blocking
    if (millis() - lastAttemptTime >= 2000) {
      lastAttemptTime = millis();
      if (WiFi.status() == WL_CONNECTED) {
        bootLog("Connected!", bootLogColor, 0);

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        isConnected = true;
        connecting = false;

      } else {
        Serial.print(".");
        attempts++;
        if (attempts >= 10) {  // Try for 10 seconds max (10 * 1000ms)
          // Serial.println("\nFailed to connect.");
          bootLog("  Connection Failed.", ST77XX_RED, 2);
          // drawText(textLength ,rowNum*6,ST77XX_RED,"  Failed to connect.");
          connecting = false;
        }
      }
    }
  }

  // Data Fetch when connected (ONLY ONCE)
  if (isConnected) {

    //   updateALLCoins();
    //   fetchHistoricalData(selectedCoinIndex);
  }
}






// =================================================
//                   FSM
// =================================================
void switchScreen() {
  switch (stage) {
    case 1:
      // mapSetup();
      stage = 2;
      break;

    case 2:
      cityInfo(city[0], 0);

      
      // if (xCounter > 0)
      // {
      //     stage = 3;
      //     cityCounter = yCounter;
      // }
      break;

    case 3:
      break;
      // switch (cityCounter)  {
      // case 0:
      //     cityInfo(cityP[SEOUL], SEOUL);

      //     snowIcon(-45);

      //     cityCounter = 3;

      //     break;

      // case 1:
      //     cityInfo(cityP[ANSAN], ANSAN);
      //     snowIcon(-45);

      //     cityCounter = 3;
      //     break;

      // case 2:
      //     cityInfo(cityP[DAEGU], DAEGU);
      //     snowIcon(-45);

      //     cityCounter = 3;
      //     break;

      // case 3:
      //     if (xCounter < 1)
      //     {
      //         stage = 1;
      //     }
      //     break;
      // }
  }
}



// =================================================
//                   HTTP
// =================================================
String httpGETRequest(const char *serverName) {
      bool debug = true;
    HTTPClient http;
    http.begin(serverName);
    
    if (debug) {
        Serial.print(F("Sending HTTP GET request to: "));
        Serial.println(serverName);
    }

    int httpResponseCode = http.GET();
    String payload = "{}"; // Default response in case of failure

    if (httpResponseCode > 0) {
        if (debug) {
            Serial.print(F("HTTP Response code: "));
            Serial.println(httpResponseCode);
        }
        payload = http.getString();
    } else {
        if (debug) {
            Serial.print(F("HTTP request failed! Error code: "));
            Serial.println(httpResponseCode);
        }
    }
    http.end();
    return payload;
}
void httpUpdate(int cityNum) {  
    bool debug = true;
    // Construct the API request URL
    // String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + cityP[cityNum] + "," + countryCode + "&APPID=" + weatherApiKey;
    String serverPath = "http://api.weatherapi.com/v1/current.json?key=" +
                      String(weatherApiKey) + "&q=" + String(city[cityNum]) + "&aqi=no";

    if (debug) {
        Serial.print(F("Requesting weather data for: "));
        Serial.println(city[cityNum]);
    }

    jsonBuffer = httpGETRequest(serverPath.c_str());
       // Debug: Print the received JSON response
      if (debug) {
          Serial.println(F("Received JSON:"));
          Serial.println(jsonBuffer);
      }



    StaticJsonDocument<1024> doc;  // Use static memory allocation to prevent heap fragmentation
    DeserializationError error = deserializeJson(doc, jsonBuffer);
    if (error) {
        Serial.print(F("JSON parsing failed! "));
        Serial.println(error.c_str());
        return;
    }

    httpinfo.temp[cityNum] = doc["current"]["temp_c"].as<String>();
    httpinfo.humidity[cityNum] = doc["current"]["humidity"].as<String>();
    httpinfo.windSpeed[cityNum] = doc["current"]["wind_kph"].as<String>();

    // // Create a JSON document
    // DynamicJsonDocument doc(1024);
    // DeserializationError error = deserializeJson(doc, jsonBuffer);

    // // Check for JSON parsing errors
    // if (error) {
    //     if (debug) {
    //         Serial.print(F("JSON parsing failed for "));
    //         Serial.println(cityP[cityNum]);
    //     }
    //     return;
    // }

    // // Store parsed data in the httpinfo struct
    //   httpinfo.all[cityNum] = jsonBuffer;  // Storing raw JSON (if needed)
    //   httpinfo.temp[cityNum] = doc["current"]["temp_c"].as<String>();  // Extract temperature in Celsius
    //   httpinfo.humidity[cityNum] = doc["current"]["humidity"].as<String>();  // Extract humidity
    //   httpinfo.windSpeed[cityNum] = doc["current"]["wind_kph"].as<String>();  // Extract wind speed in kph

    // Debug: Print extracted values
    if (debug) {
        Serial.print(F("Updated weather data for: "));
        Serial.println(cityP[cityNum]);
        Serial.print(F("  Temperature: "));
        Serial.println(httpinfo.temp[cityNum]);
        Serial.print(F("  Humidity: "));
        Serial.println(httpinfo.humidity[cityNum]);
        Serial.print(F("  Wind Speed: "));
        Serial.println(httpinfo.windSpeed[cityNum]);
    }
}

void httpInit() {
  bool debug = true;
    if (WiFi.status() == WL_CONNECTED) {
        int cityArraySize = sizeof(city) / sizeof(city[0]);

        if (debug) Serial.println(F("Starting weather data updates..."));
        for (int i = 0; i < cityArraySize; i++) {
            httpUpdate(i);
        }
        if (debug) Serial.println(F("All cities updated successfully."));
        
    } else {
        if (debug) {
            Serial.println(F("WiFi not connected! Skipping weather update."));
        }
    }
}



// =================================================
//                 CITY WEATHER
// =================================================










void cityInfo(String cityName, int cityNum) {
  // Clear screen
  tft.fillScreen(ST77XX_BLACK);

  // Define layout variables for easy adjustments
  int margin = 2;                        // General margin spacing
  int titleSize = 2;                      // Font size for the city name
  int tempSize = 3;                       // Font size for the temperature
  int labelSize = 1;                      // Font size for labels
  int valueSize = 2;                      // Font size for values
  int titleHeight = textHeight("A", tft, titleSize) + HEIGHT * 0.05; // Height of title
  int tempHeight = textHeight("A", tft, tempSize); // Height of title

  
  //TEMP
  String temperature = "27 °C";  // for now used to center the temperature along X-axis

  //========= Dynamic display system based on icon dimensions. ==============
  int IconWidth = autumnIcon_width;
  int IconHeight = autumnIcon_height;
  // 1. Title (x: margin/WIDTH,      y: margin/titleHeight+margin)
  // 2. Icon (x: margin/SizeX+margin,  y: y1 + margin/y1+SizeY)
  // 3. Temp (x:  x2+margin / WIDTH-margin ,   y: (y2 -tempHeight/2) / Size)        !! tft writes at the coordinates at the top of the letter height
  // 3. 
  // 3. 
  

  // Title
    int title_startX= margin;
    int title_endX= WIDTH;

    int title_startY= margin;
    int title_endY= titleHeight+margin;
  // Icon
    int Icon_startX= margin;
    int Icon_endX= margin+IconWidth+margin;

    int Icon_startY= (title_startY+title_endY) + margin;
    int Icon_endY= Icon_startY + IconHeight + margin;

    int Icon_centerX = Icon_startX + IconWidth/2;
    int Icon_centerY = Icon_startY + IconHeight/2;
  // Temp
    int Temp_endX= WIDTH - margin; // box coordinates, NOT coordinates of end of text
    int Temp_startX= (Icon_endX + margin);// + (Temp_endX -Icon_endX - textWidth(temperature, tft, tempSize));
    
    int Temp_startY= Icon_centerY - tempHeight/2;             // To be centered with the image center, remember !! tft writes at the coordinates at the top of the letter height
    int Temp_endY= Temp_startY+tempHeight;
  // Humidity
    String headerH = "HUMIDITY";
    String humidity = String(65);
        humidity.concat(" %");
  // Wind Speed
    String headerW = "WIND SPEED";
    String windSpeed = String(12.5);
        // windSpeed.concat(" m/s");

    int humidity_startX= (WIDTH - textWidth(headerH, tft, labelSize) - textWidth(headerW, tft, labelSize))/3;
    int humidity_endX= humidity_startX + textWidth(headerH, tft, labelSize);

    int humidity_startY= Icon_endY + 2*margin;  // add separator ?
    int humidity_endY= humidity_startY ;     // TODO

    int WS_startX= humidity_startX*2 + textWidth(headerH, tft, labelSize);
    int WS_endX= humidity_startX + textWidth(headerH, tft, labelSize);

  
  


  // ========== CITY NAME ==========
  char cityNameList[10]; // Create a character array to store the city name
  strcpy(cityNameList, cityName.c_str()); // Convert String to C-string
  tft.setTextSize(titleSize);
  tft.setTextColor(ST77XX_WHITE);
  for (int i = 0; i < 5; i++) {
      float x = 0.08 + i * 0.2;  // Calculate x position dynamically
      tft.setCursor(WIDTH * x, HEIGHT * 0.05);
      tft.print(cityNameList[i]);  // Print each letter of the city name
  }

  //=========== WEATHER ICON =======
  // Dimensions 
  // int size=45; // 1/2.75cm x 128px
  // int margin_snow=3;
  // int x = size/2 + margin_snow;
  // int y = size/2 + titleHeight;
  // snowIcon(x, y, size, -45);

  tft.drawRGBBitmap(Icon_startX, Icon_startY, autumnIcon, IconWidth, IconHeight);




  // ========== TEMPERATURE ==========
  // tft.setTextColor(ST77XX_WHITE);
  // tft.setTextSize(tempSize);

  // int tempC = KtoC(httpinfo.temp[cityNum]);  // Convert temperature from Kelvin to Celsius
  // int tempX = (tempC < 10) ? WIDTH * 0.4 : WIDTH * 0.35;  // Adjust X position if single/double digit
  // int tempY = titleHeight + margin;  // Position under the title

  // tft.setCursor(tempX, tempY);
  // tft.print(tempC);
  
  // tft.setTextSize(1);  // Smaller font for degree symbol
  // tft.print(" °C");

  drawText(Temp_startX, Temp_startY, ST77XX_WHITE, temperature, tempSize);

  // ========== SEPARATOR LINE ==========
  // int lineY = tempY + textHeight("A", tft, tempSize) + margin;
  // tft.drawLine(WIDTH * 0.30, lineY+25, WIDTH * 0.70, lineY+25, ST77XX_DARKCYAN);


  // ========== HUMIDITY ==========
  drawText(humidity_startX, humidity_startY, ST77XX_DARKGREY, headerH, labelSize);
  drawText(humidity_startX, humidity_startY + textHeight(headerH, tft, labelSize) + margin, ST77XX_DARKGREY, humidity, valueSize);

  


  // int humidityLabelX = WIDTH * 0.1;
  // int humidityLabelY = lineY + margin * 2;

  // tft.setTextColor(ST77XX_DARKGREY);
  // tft.setTextSize(labelSize);
  // tft.setCursor(humidityLabelX, humidityLabelY);
  // tft.println("HUMIDITY");

  // tft.setTextSize(valueSize);
  // tft.setCursor(humidityLabelX, humidityLabelY + textHeight("A", tft, labelSize));
  // tft.print(httpinfo.humidity[cityNum]);
  // tft.print(" %");

  // ========== WIND SPEED ==========
  drawText(WS_startX, humidity_startY, ST77XX_DARKGREY, headerW, labelSize);
  drawText(WS_startX, humidity_startY + textHeight(headerW, tft, labelSize) + margin, ST77XX_DARKGREY, windSpeed, valueSize);


  // int windLabelX = WIDTH * 0.6;
  // int windLabelY = humidityLabelY;

  // tft.setTextColor(ST77XX_DARKGREY);
  // tft.setTextSize(labelSize);
  // tft.setCursor(windLabelX, windLabelY);
  // tft.println("WIND SPEED");

  // tft.setTextSize(valueSize);
  // tft.setCursor(windLabelX, windLabelY + textHeight("A", tft, labelSize));
  // tft.print(httpinfo.windSpeed[cityNum]);

  // tft.setTextSize(1);
  // tft.print(" SSE");  // Wind direction (assumed constant)
}


void snowIcon(int x, int y, int size, int i) {
    int centerX = x;
    int centerY = y + i;
    int halfSize = size / 2;
    int quarterSize = size / 4;

    // Main vertical line (spine)
    tft.drawLine(centerX, centerY - halfSize, centerX, centerY + halfSize, ST77XX_CYAN);

    // Diagonal cross lines
    tft.drawLine(centerX - halfSize, centerY + quarterSize, centerX + halfSize, centerY - quarterSize, ST77XX_CYAN);
    tft.drawLine(centerX - halfSize, centerY - quarterSize, centerX + halfSize, centerY + quarterSize, ST77XX_CYAN);

    // Top branches
    tft.drawLine(centerX, centerY - quarterSize, centerX - quarterSize, centerY - halfSize, ST77XX_CYAN);
    tft.drawLine(centerX, centerY - quarterSize, centerX + quarterSize, centerY - halfSize, ST77XX_CYAN);

    // Bottom branches
    tft.drawLine(centerX, centerY + quarterSize, centerX - quarterSize, centerY + halfSize, ST77XX_CYAN);
    tft.drawLine(centerX, centerY + quarterSize, centerX + quarterSize, centerY + halfSize, ST77XX_CYAN);

    // Left-top branches
    tft.drawLine(centerX - quarterSize, centerY - quarterSize, centerX - quarterSize, centerY - halfSize, ST77XX_CYAN);
    tft.drawLine(centerX - quarterSize, centerY - quarterSize, centerX - halfSize, centerY, ST77XX_CYAN);

    // Left-bottom branches
    tft.drawLine(centerX - quarterSize, centerY + quarterSize, centerX - halfSize, centerY, ST77XX_CYAN);
    tft.drawLine(centerX - quarterSize, centerY + quarterSize, centerX - quarterSize, centerY + halfSize, ST77XX_CYAN);

    // Right-top branches
    tft.drawLine(centerX + quarterSize, centerY - quarterSize, centerX + quarterSize, centerY - halfSize, ST77XX_CYAN);
    tft.drawLine(centerX + quarterSize, centerY - quarterSize, centerX + halfSize, centerY, ST77XX_CYAN);

    // Right-bottom branches
    tft.drawLine(centerX + quarterSize, centerY + quarterSize, centerX + halfSize, centerY, ST77XX_CYAN);
    tft.drawLine(centerX + quarterSize, centerY + quarterSize, centerX + quarterSize, centerY + halfSize, ST77XX_CYAN);
}





int KtoC(String k) {
  //convert kelvin to celcius
  float tempK = atoi(k.c_str());
  float tempC = tempK - 273.15;
  int tempC2 = tempC;

  //반올림
  if (tempC - tempC2 > 0.5) {
    tempC2 += 1;
  }
  return tempC2;
}






// =================================================
//                  TIMER
// =================================================
bool timerFin() {
  currentMillis = millis();  // Get the current time

  // Check if the interval has passed
  if (currentMillis - previousMillis >= timerInterval) {
    // Interval has passed, trigger the timer action
    previousMillis = currentMillis;  // Reset the timer
    interruptCounter++;              // Increment the interrupt counter
    totalInterruptCounter++;         // Increment the total interrupt counter
    return true;                     // Timer finished, return true
  }

  return false;  // Timer still running, return false
}
void timerInit(int sec) {
  timerInterval = sec * 1000;  // Convert seconds to milliseconds
  previousMillis = millis();   // Initialize the previousMillis to the current time
}


// =================================================
//                 DISPLAY FUNCTIONS
// =================================================
//------Helpers-------
int textWidth(String text, Adafruit_GFX &tft, int textSize) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(textSize);
  tft.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);
  return w;
}
int textHeight(String text, Adafruit_GFX &tft, int textSize) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(textSize);
  tft.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);
  return h;  // Return the computed text height
}

void drawText(int x, int y, int textColor, String text, int textSize) {
  tft.setTextSize(textSize);
  tft.setCursor(x, y);
  tft.setTextColor(textColor);
  tft.println(text);
}
void bootLog(String text, int textColor, int Row) {
  Serial.println(text);
  tft.setTextSize(1);
  int margin = 2;

  int startY = Row * margin + (Row-1)*textHeight("A", tft, 1); //Row * (textHeight("A", tft, 1) - 2);
  int endY = startY + textHeight("A", tft, 1) + margin;

  // CLEAR
  tft.fillRect(0, startY, WIDTH, endY, ST77XX_BLACK);

  // WRITE
  drawText(0, startY, textColor, text, 1);

}  // MAX 4 ROWS





