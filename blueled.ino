#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define NUM_PIXELS 30

// Menu related
String cmd = ""; // Reads incoming command
String mode = ""; // Mode to draw
uint8_t cnt = 0; // First char is cmd length from incoming command
bool receiving = false;

// Mode specific settings
uint16_t general_speed = 250;
uint8_t _r = 100, _g = 0, _b = 250;
int fire_settings[3] = {55, 50, 30}; // Fire -> 0: Cooling, 1: Sparking, 2: Delay

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRBW + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  strip.begin();
  for(uint8_t i=0; i<NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
    delay(50);
  }
  strip.clear();
  fadeWW();
}

// ++++++++++++++++++++++++++++++++++++++++ APP CONTROL ++++++++++++++++++++++++++++++++++++++++

void settings(String setting, uint16_t val) {
  if(setting == "BR"){ // Set strip brightness, if 0 => resets strip, so avoid it
    if(val >= 1) strip.setBrightness(val);
    else strip.setBrightness(1);
    strip.show();
  }
  else if(setting == "SP"){ // Set speed of mode
    general_speed = val+1;
  }
}

void mode_settings(String mode, int setting, int val) {
  Serial.println(val);
  if(mode == "fir") fire_settings[setting] = val;
}

void draw(String data) {
  if(data == "rbw") rainbowCycle();
  else if(data == "wwh") fadeWW();
  else if(data == "cwh") fadeCW();
  else if(data == "cwi") colorWipe(strip.Color(_r, _g, _b));
  else if(data == "ftn") fountain();
  else if(data == "plc") police();
  else if(data == "plb") police_blink();
  else if(data == "rdb") random_blink();
  else if(data == "fir") Fire(fire_settings[0], fire_settings[1], fire_settings[2]);
  else if(data == "shs") shooting_stars(4, 15, 100, 30);
}

void loop() {
  while (Serial.available()>0)
  {
    if (!receiving) cmd = "";
    receiving = true;
    cmd += (char)Serial.read();
    cnt = ((String)cmd[0]).toInt();
    Serial.println(cmd);
    if(cmd.length() >= cnt && cnt != 0){
      receiving = false;
      Serial.print("fertig");
      switch(cnt){
        case 4:
          mode = cmd.substring(1);
          strip.clear();
          strip.show();
          break;
        case 6:
          settings(cmd.substring(1, 3), cmd.substring(3, 6).toInt());
          break;
        case 8:
          mode_settings(cmd.substring(1, 4), cmd.substring(4, 5).toInt(), cmd.substring(5, 8).toInt());
          break;
      }
    }

  }
  draw(mode);
  
}

// ++++++++++++++++++++++++++++++++++++++++ LED MODES ++++++++++++++++++++++++++++++++++++++++

void shooting_stars(uint8_t ss_size, uint8_t ss_length, uint8_t ss_bright, uint8_t base_brightness) {
  uint8_t counter = 0;
  uint8_t start = random(NUM_PIXELS-1);
  strip.fill(strip.Color(base_brightness, base_brightness, base_brightness), 0, NUM_PIXELS-1);
  strip.show();

  for(int m=0; m < ss_length+ss_size; m++) {
    if(Serial.available()){ break; }
    if(m < ss_length) {
      strip.setPixelColor((start+counter)%NUM_PIXELS, base_brightness+ss_bright, base_brightness+ss_bright, base_brightness+ss_bright);
    }
    for(int k=1; k < ss_size; k++) {
      if(Serial.available()){ break; }
      if(pLen(start, (start+counter-k)%NUM_PIXELS) < pLen(start, (start+ss_length)%NUM_PIXELS)) {  
        strip.setPixelColor((start+counter-k)%NUM_PIXELS, base_brightness+(ss_bright/ss_size)*(ss_size-1-k), base_brightness+(ss_bright/ss_size)*(ss_size-1-k), base_brightness+(ss_bright/ss_size)*(ss_size-1-k));
      }
    }
    counter++;
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  delay(750);
}

void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_PIXELS/2];
  int cooldown;
  
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_PIXELS/2; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_PIXELS/2) + 2);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_PIXELS/2 - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_PIXELS/2; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  strip.show();
  delay(SpeedDelay);
}

void fountain() {
  uint32_t c = randomColor();
  for(uint8_t i=0; i < 30; i++) {
    if(Serial.available()){ break; }
    strip.setPixelColor(i, c);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  c = randomColor();
  for(uint8_t i=30; i > 0; i--) {
    if(Serial.available()){ break; }
    strip.setPixelColor(i-1, c);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
}

void police() {
  for(uint8_t i=0; i < 30; i++) {
    if(Serial.available()){ break; }
    strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  for(uint8_t i=30; i > 0; i--) {
    if(Serial.available()){ break; }
    strip.setPixelColor(i, strip.Color(0, 0, 255));
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
}

void police_blink() {
  
  for(uint8_t i=0; i < 17; i++) {
    if(Serial.available()){ break; }
    strip.fill(strip.Color(i*15, 0, 0), 0, 29);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  for(uint8_t i=0; i < 17; i++) {
    if(Serial.available()){ break; }
    strip.fill(strip.Color(255-15*i, 0, 0), 0, 29);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  for(uint8_t i=0; i < 17; i++) {
    if(Serial.available()){ break; }
    strip.fill(strip.Color(0, 0, i*15), 0, 29);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  for(uint8_t i=0; i < 17; i++) {
    if(Serial.available()){ break; }
    strip.fill(strip.Color(0, 0, 255-i*15), 0, 29);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
}

void random_blink() {
  uint32_t c = randomColor();
  strip.fill(c, 0, 29);
  strip.setBrightness(0);
  strip.show();
  for(uint8_t i=0; i < 17; i++) {
    if(Serial.available()){ strip.setBrightness(100); strip.show(); break; }
    strip.setBrightness(i*15);
    strip.fill(c, 0, 29);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  for(uint8_t i=0; i < 17; i++) {
    if(Serial.available()){ strip.setBrightness(100); strip.show(); break; }
    strip.begin();
    strip.setBrightness(255-i*15);
    strip.fill(c, 0, 29);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  strip.setBrightness(100);
  strip.show();
}

void fadeWW() { // Warm white
  for(uint8_t i=0; i<25; i++) {
    if(Serial.available()){ break; }
    strip.fill(strip.Color(0, 0, 0, i*10), 0, 30);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  mode = ""; // Clear `mode` to just run it once
}

void fadeCW() { // Cold white 
  for(uint8_t i=0; i<25; i++) {
    if(Serial.available()){ break; }
    strip.fill(strip.Color(i*10, i*10, i*10), 0, 30);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
  mode = ""; // Clear `mode` to just run it once
}

void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    if(Serial.available() && cnt == 4){ break; }
    strip.setPixelColor(i, c);
    strip.show();
    delay(((uint16_t)(2500 / general_speed)));
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i, j;
  for(j=0; j<256/**5*/; j++) { // 5 cycles of all colors on wheel
    if(Serial.available()){ break; }
    for(i=0; i< strip.numPixels(); i++) {
      if(Serial.available()){ break; }
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(((uint16_t)(2500 / general_speed))-4);
  }
}

// ++++++++++++++++++++++++++++++++++++++++ HELPER FUNCTIONS ++++++++++++++++++++++++++++++++++++++++

int pLen(int start, int pos) {
  if(pos>start) return pos-start;
  else return ((NUM_PIXELS-start)%NUM_PIXELS + pos%NUM_PIXELS)%NUM_PIXELS;
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    strip.setPixelColor(Pixel, 255, 255, heatramp);
    strip.setPixelColor(NUM_PIXELS-1-Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    strip.setPixelColor(Pixel, 255, heatramp, 0);
    strip.setPixelColor(NUM_PIXELS-1-Pixel, 255, heatramp, 0);
  } else {                               // coolest
    strip.setPixelColor(Pixel, heatramp, 0, 0);
    strip.setPixelColor(NUM_PIXELS-1-Pixel, heatramp, 0, 0);
  }
}

uint32_t randomColor() {
  switch(random(0, 99)%3){
  case 0:
    return strip.Color(random(30, 100), random(20, 140), 255);
  case 1:
    return strip.Color(random(30, 100), 255, random(20, 100));
  case 2:
    return strip.Color(255, random(30, 100), random(20, 100));
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
