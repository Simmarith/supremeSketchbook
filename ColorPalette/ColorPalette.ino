#include <FastLED.h>

#define LED_PIN     D4
#define NUM_LEDS    240
#define BRIGHTNESS  255
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


void setup() {
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}


void loop()
{
    ChangePalettePeriodically();
    
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
    
    FillLEDsFromPaletteColors( startIndex, brightness);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex, uint8_t brightness)
{   
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
    addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        SetupChristmasPalette();
        currentBlending = LINEARBLEND;
    }
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
