#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MQTTClient.h>

//Your Wifi
const char* ssid = "*****";
const char* password = "*****";
//Your MQTT
const char* host = "*****";
const char* mqttUser = "*****";
const char* mqttPass = "*****";
const String mqttPrefix = "*****";
const char* mqttId = "*****";

WiFiClient net;
MQTTClient mqtt;


#define LED_PIN     D4
#define NUM_LEDS    240
#define BRIGHTNESS  255
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
bool lightsOn;
bool christmas;
int color;
bool striped;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Booting...");

    //FASTLED
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness( BRIGHTNESS );
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    //WIFI & MQTT
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);
    
    mqtt.begin(host, net);
    connect();
    mqtt.onMessage(messageReceived);
    lightsOn = true;
    christmas = true;
    color = 0;
    
    Serial.println("Setup completed...");
}


void loop()
{
    //MQTT
    if (!mqtt.connected()) {
      connect();
    }
    mqtt.loop();

    //FASTLED
    HandlePallete();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */

    //Calculate pulsating brightness
    uint8_t pulseProgression = ((millis() / 100) % 50);
    bool decrease = (bool)(((millis() / 100) / 50) % 2);
    uint8_t pulseAmplifier = 5;
    uint8_t brightness = 0;
    uint8_t pulse = pulseProgression * pulseAmplifier;
    if (decrease) {
      brightness = 255 - pulse;
    } else {
      brightness = pulse;
    }

    if (!lightsOn) {
      brightness = 0;
    }
    
    FillLEDsFromPaletteColors( startIndex, brightness);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void connect() {
  
  //while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    //WiFi.begin(ssid, password);
    //Serial.println("WiFi connection failed. Retry.");
  //}

  Serial.print("Wifi connection successful - IP-Address: ");
  Serial.println(WiFi.localIP());

  while (!mqtt.connect(mqttId, mqttUser, mqttPass)) {
    Serial.print(".");
  }

  mqtt.subscribe(mqttPrefix + "/lights/all");
  mqtt.subscribe(mqttPrefix + "/lights/christmas");
  mqtt.subscribe(mqttPrefix + "/lights/alert");
  mqtt.subscribe(mqttPrefix + "/lights/christmas/color");
  mqtt.subscribe(mqttPrefix + "/lights/christmas/striped");

  Serial.println("MQTT connected!");
}

void alert() {
    static uint8_t colorIndex = 0;
    static int blinkTimes[12] = { 100, 100, 100, 100, 100, 1000, 100, 100, 100, 100, 1000 };
    int blinked = 0;
    while (blinked < 12) {
      for( int i = 0; i < NUM_LEDS; i++) {
          if (blinked % 2) {
            leds[i] = CRGB::Black;
          } else {
            leds[i] = CRGB::Blue;
          }
        }
      colorIndex += 3;
      FastLED.show();
      delay(blinkTimes[blinked]);
      colorIndex = 0;
      blinked++;
    }
  }

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (topic == (mqttPrefix + "/lights/all")) {
    if (payload == "on") {
      lightsOn = true;
      Serial.println("lights on!");
    }
    if (payload == "off") {
      lightsOn = false;
      Serial.println("lights out!");
    }
  }
  if (topic == (mqttPrefix + "/lights/christmas")) {
    if (payload == "on") {
      lightsOn = true;
      christmas = true;
      Serial.println("lights on!");
    }
    if (payload == "off") {
      lightsOn = false;
      Serial.println("lights out!");
    }
  }
  if (topic == (mqttPrefix + "/lights/christmas/color")) {
      lightsOn = true;
      christmas = false;
      color = atoi(payload.c_str());
      Serial.println("lights on and colored!");
  }
  if (topic == (mqttPrefix + "/lights/christmas/striped")) {
    if (payload == "1") {
      striped = true;
      Serial.println("color striped!");
    }
    if (payload == "0") {
      striped = false;
      Serial.println("color continuous!");
    }
  }
  if (topic == (mqttPrefix + "/lights/alert")) {
    alert();
  }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex, uint8_t brightness)
{   
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
    if (brightness != 0) {
        addGlitter(80);
    }
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void HandlePallete()
{   
  if (christmas) {
    SetupChristmasPalette();
  } else {
    if (striped) {
      SetupStripedColorPalette();
    } else {
      SetupColorPalette();
    }
  }
  //There is a chance to controll this later
  currentBlending = LINEARBLEND;
}
void SetupChristmasPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    //COLORS!!!
    currentPalette[0] = CRGB::Red;
    currentPalette[4] = CRGB::Green;
    currentPalette[8] = CRGB::Red;
    currentPalette[12] = CRGB::Green;
    
}
void SetupColorPalette()
{
    //all 16 palette entries to color
    fill_solid( currentPalette, 16, CHSV(color,255,255));
    
}
void SetupStripedColorPalette()
{
    //all 16 palette entries to black...
    fill_solid( currentPalette, 16, CRGB::Black);
    //and every forth to color
    currentPalette[0] = CHSV(color,255,255);
    currentPalette[4] = CHSV(color,255,255);
    currentPalette[8] = CHSV(color,255,255);
    currentPalette[12] = CHSV(color,255,255);
}

