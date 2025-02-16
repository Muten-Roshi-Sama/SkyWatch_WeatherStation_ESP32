// V 2.0

// Uses https://www.weatherapi.com Api key to retrieve weather data. create an account and add your own key (FREE)

// PINOUT:
  // Vcc - 3v3,  GND-GND,  CS-10,  RST-9,  AO/DC-8,  SDA-6,  SCK-4,  LED/BLK-Vcc

// TODO: 
 //      - add more img
 //      - retrieve and print actual weather data
 //      - add way to cycle through cities
 //      - add map of belgium ?
 //      - 



// =============== LIBRARIES ===============
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // #include <Arduino_JSON.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>


// ============== header files ============
#include "settings.h"
#include "weatherIcons.h" 
// #include "corea.c"

// =============== LCD CONFIG ===============
#define TFT_DC 8
#define TFT_RST 9
#define TFT_CS 10
#define WIDTH 128
#define HEIGHT 160
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


// =============== STATE VARIABLES ===============
enum State {INIT, BOOT, CITYVIEW, EXIT };
enum WeatherIcon {SUNNY, SUN_RAIN_CLOUD, NIGHT};
State state = INIT;
WeatherIcon weatherIcon = SUNNY;

bool INIT_HTTP = true;
int stage = 0;


// =============== TIMING VARIABLES ===============
unsigned long screenUpdateMillis = 0;  
const unsigned long screenUpdateInterval = 3000;
int esp = 0;
int timer = 0;





// =================== SETUP FUNCTION ==============================
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

// =================== LOOP FUNCTION ==============================
void loop() {
  if (!isConnected) connectToWiFi();

  if (isConnected && INIT_HTTP) {
    httpInit();
    updateScreen(); 
    INIT_HTTP = false;
  }



    updateScreen(); 
    


  // Display Memory Usage
  if (millis() - esp > 1000) {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    esp = millis();
  }
}





// =================================================
//                   BOOT
// =================================================
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
}








// =================================================
//                   FSM
// =================================================
void updateScreen() {
  switch (state) {
            case INIT :
              welcome();
              state = BOOT;
              break;

            case BOOT:
                if (timerFin() && stage == 0) {
                  // stage = 1;
                  state = CITYVIEW;
                  transition();
                }
                break;

            case CITYVIEW:
              // Update the State Machine
              if(millis() - screenUpdateMillis > screenUpdateInterval)  {
                cityInfo(city[0], 0);
                weatherIcon = static_cast<WeatherIcon>((weatherIcon + 1) % 3);
                screenUpdateMillis=millis();
              }
              
              break;


            case EXIT:
                state = EXIT;
                break;
        }
}



// =================================================
//                   HTTP
// =================================================
void httpUpdate(int cityNum) {  
    bool debug = true;
    String serverPath = "http://api.weatherapi.com/v1/current.json?key=" +
                      String(weatherApiKey) + "&q=" + String(city[cityNum]) + "&aqi=no";
    serverPath.replace(" ", "%20");  // URL-encode spaces

    if (debug) {
        Serial.print(F("Requesting weather data for: "));
        Serial.println(city[cityNum]);
    }

    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverPath.c_str());

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String payload = http.getString();  // Read the raw response
        if (debug) {
            Serial.println(F("Raw API response:"));
            Serial.println(payload);
        }

        StaticJsonDocument<2048> doc;  // Adjust buffer size as needed
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print(F("JSON parsing failed! "));
            Serial.println(error.c_str());
            http.end();
            return;
        }

        // Extract data from JSON
        float temp = doc["current"]["temp_c"].as<float>();
        int humidity = doc["current"]["humidity"].as<int>();
        float windSpeed = doc["current"]["wind_kph"].as<float>();

        // Store data
        httpinfo.temp[cityNum] = String(temp);
        httpinfo.humidity[cityNum] = String(humidity);
        httpinfo.windSpeed[cityNum] = String(windSpeed);

        if (debug) {
            Serial.print(F("Updated weather data for: "));
            Serial.println(city[cityNum]);
            Serial.print(F("  Temperature: "));
            Serial.println(httpinfo.temp[cityNum]);
            Serial.print(F("  Humidity: "));
            Serial.println(httpinfo.humidity[cityNum]);
            Serial.print(F("  Wind Speed: "));
            Serial.println(httpinfo.windSpeed[cityNum]);
        }
    } else {
        Serial.print(F("HTTP request failed! Error code: "));
        Serial.println(httpResponseCode);
    }
    http.end();
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


// Requesting weather data for: BRUSSEL
// Raw API response:
// {"location":{"name":"Brussels","region":"","country":"Belgium","lat":50.8333,"lon":4.3333,"tz_id":"Europe/Brussels","localtime_epoch":1739675464,"localtime":"2025-02-16 04:11"},
// "current":{"last_updated_epoch":1739674800,"last_updated":"2025-02-16 04:00","temp_c":-1.0,"temp_f":30.2,"is_day":0,"condition":{"text":"Clear","icon":"//cdn.weatherapi.com/weather/64x64/night/113.png","code":1000},
//"wind_mph":6.5,"wind_kph":10.4,"wind_degree":57,"wind_dir":"ENE","pressure_mb":1019.0,"pressure_in":30.09,"precip_mm":0.0,"precip_in":0.0,"humidity":86,"cloud":0,"feelslike_c":-4.6,"feelslike_f":23.7,"windchill_c":-4.3,"windchill_f":24.4,"heatindex_c":-0.7,"heatindex_f":30.8,"dewpoint_c":-3.6,"dewpoint_f":25.6,"vis_km":8.0,"vis_miles":4.0,"uv":0.0,"gust_mph":10.9,"gust_kph":17.6}}
// Updated weather data for: BRUSSEL
//   Temperature: -1.00
//   Humidity: 86
//   Wind Speed: 10.40
// Requesting weather data for: ANVERS
// Raw API response:
// {"location":{"name":"Anvers","region":"","country":"Belgium","lat":51.2167,"lon":4.4167,"tz_id":"Europe/Brussels","localtime_epoch":1739675592,"localtime":"2025-02-16 04:13"},
//'current":{"last_updated_epoch":1739674800,"last_updated":"2025-02-16 04:00","temp_c":-0.8,"temp_f":30.6,"is_day":0,"condition":{"text":"Light snow","icon":"//cdn.weatherapi.com/weather/64x64/night/326.png","code":1213},
//"wind_mph":6.7,"wind_kph":10.8,"wind_degree":65,"wind_dir":"ENE","pressure_mb":1020.0,"pressure_in":30.12,"precip_mm":0.0,"precip_in":0.0,"humidity":93,"cloud":0,"feelslike_c":-4.5,"feelslike_f":23.9,"windchill_c":-3.5,"windchill_f":25.7,"heatindex_c":0.0,"heatindex_f":32.0,"dewpoint_c":-2.6,"dewpoint_f":27.4,"vis_km":10.0,"vis_miles":6.0,"uv":0.0,"gust_mph":10.6,"gust_kph":17.1}}
// Updated weather data for: ANVERS
//   Temperature: -0.80
//   Humidity: 93
//   Wind Speed: 10.80
// Requesting weather data for: LIEGE
// Raw API response:
// {"location":{"name":"Liege","region":"","country":"Belgium","lat":50.6333,"lon":5.5667,"tz_id":"Europe/Brussels","localtime_epoch":1739675587,"localtime":"2025-02-16 04:13"},
// "current":{"last_updated_epoch":1739674800,"last_updated":"2025-02-16 04:00","temp_c":-0.6,"temp_f":30.9,"is_day":0,"condition":{"text":"Light snow","icon":"//cdn.weatherapi.com/weather/64x64/night/326.png","code":1213},
// "wind_mph":6.3,"wind_kph":10.1,"wind_degree":64,"wind_dir":"ENE","pressure_mb":1018.0,"pressure_in":30.06,"precip_mm":0.0,"precip_in":0.0,"humidity":80,"cloud":25,"feelslike_c":-4.1,"feelslike_f":24.7,"windchill_c":-4.0,"windchill_f":24.8,"heatindex_c":-0.5,"heatindex_f":31.0,"dewpoint_c":-4.4,"dewpoint_f":24.1,"vis_km":10.0,"vis_miles":6.0,"uv":0.0,"gust_mph":9.6,"gust_kph":15.4}}
// Updated weather data for: LIEGE
//   Temperature: -0.60
//   Humidity: 80
//   Wind Speed: 10.10
// Requesting weather data for: MONS
// Raw API response:
// {"location":{"name":"Mons","region":"","country":"Belgium","lat":50.45,"lon":3.9333,"tz_id":"Europe/Brussels","localtime_epoch":1739675590,"localtime":"2025-02-16 04:13"},
// "current":{"last_updated_epoch":1739674800,"last_updated":"2025-02-16 04:00","temp_c":-0.8,"temp_f":30.6,"is_day":0,"condition":{"text":"Partly cloudy","icon":"//cdn.weatherapi.com/weather/64x64/night/116.png","code":1003},
// "wind_mph":6.9,"wind_kph":11.2,"wind_degree":59,"wind_dir":"ENE","pressure_mb":1019.0,"pressure_in":30.09,"precip_mm":0.0,"precip_in":0.0,"humidity":100,"cloud":75,"feelslike_c":-4.6,"feelslike_f":23.8,"windchill_c":-4.5,"windchill_f":24.0,"heatindex_c":-0.7,"heatindex_f":30.7,"dewpoint_c":-2.5,"dewpoint_f":27.6,"vis_km":10.0,"vis_miles":6.0,"uv":0.0,"gust_mph":11.4,"gust_kph":18.4}}
// Updated weather data for: MONS
//   Temperature: -0.80
//   Humidity: 100
//   Wind Speed: 11.20





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
  String temperature = "27 C";  // for now used to center the temperature along X-axis

  //========= Dynamic display system based on icon dimensions. ==============
  int IconWidth = 50;
  int IconHeight = 50;
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

    int humidity_endY= HEIGHT - margin ;     
    int humidity_startY= humidity_endY - textHeight(headerH, tft, labelSize) - textHeight(humidity, tft, valueSize) - 2*margin; //  Icon_endY + 2*margin;  // add separator ?
    

    int WS_endX= WIDTH - margin;
    int WS_startX= WS_endX - textWidth(headerH, tft, labelSize); //humidity_startX*2 + textWidth(headerH, tft, labelSize);
    
 
  
  


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
  switch (weatherIcon){
    case SUNNY :
      tft.drawRGBBitmap(Icon_startX, Icon_startY, sunny, sunny_width, sunny_height);
      break;

    case SUN_RAIN_CLOUD :
      tft.drawRGBBitmap(Icon_startX, Icon_startY, sun_rain_cloud, sun_rain_cloud_width, sun_rain_cloud_height);
      break;
    
    case NIGHT :
      tft.drawRGBBitmap(Icon_startX, Icon_startY, night, night_width, night_height);
      break;
    
    // case SUNNY :
    //   tft.drawRGBBitmap(Icon_startX, Icon_startY, autumnIcon, IconWidth, IconHeight);
    //   break;
    
  }



  // tft.drawRGBBitmap(Icon_startX, Icon_startY, autumnIcon, IconWidth, IconHeight);




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





