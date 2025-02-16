#ifndef settings_h
#define settings_h


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
{ "", "" },

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

// ============= FUNCTION DECLARATION ===============
void welcome();
void welcomeClose();
void transition();
void connectToWiFi();
void updateScreen();

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



#endif
