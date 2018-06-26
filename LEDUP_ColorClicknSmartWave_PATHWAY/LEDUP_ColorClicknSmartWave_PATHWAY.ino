/*
 * Add including places
 */
#include <Adafruit_NeoPixel.h>
#include <RCSwitch.h>

/*
 * Add definition places
 */
#define   RADAR_PIN     D1
#define   LIGHT_PIN     D2
#define   RF_PIN        D3         // Receiver on interrupt 0 => that is pin 2
#define   BUTTON_PIN    D5
#define   LED_PIN       D7

#define   MAX_NUM_LEDS  120
uint32_t  NUM_LEDS  =   MAX_NUM_LEDS;

/*
 * Class
 */
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_RGB + NEO_KHZ800);
RCSwitch mySwitch = RCSwitch();

/*
 * Current running effect
 */
unsigned int effect = 0;
unsigned char testEffect = 0;

/*
 * RF decode storage
 */
unsigned int RF_DATA = 0;
unsigned int RF_PRODUCT_CODE = 0;
unsigned int RF_COLOR_CODE = 0;
unsigned int RF_FUNCTION_CODE = 0;
unsigned int RF_EFFECT_CODE = 0;


/*
 * Used for color-changing RF or APP
 */
unsigned char rfColor_r = 255;
unsigned char rfColor_g = 0;
unsigned char rfColor_b = 0;

/*
 * Light variable
 */
unsigned char lightCondition;

/*
 * Radar variable
 */
unsigned char radarCondition;

/*
 * Button variable
 */
unsigned char buttonCondition;

unsigned int i;   // common variable

void setup() {
  strip.begin();
  
  pinMode(RADAR_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  mySwitch.enableReceive(RF_PIN);
  
  Serial.begin(500000);   // used for debug and data receiving from ESP8266
}

void loop() {

  /*
   * Check and read the data from RF remote controls
   */
  if (mySwitch.available()){
    RF_DATA = mySwitch.getReceivedValue();
    mySwitch.resetAvailable();
    
    RF_PRODUCT_CODE = RF_DATA >> 8;
    RF_FUNCTION_CODE = RF_DATA & 0x00FF;
    RF_COLOR_CODE = RF_FUNCTION_CODE >> 4;
    RF_EFFECT_CODE = RF_FUNCTION_CODE & 0x0F;
    
    Serial.print("RF: ");
    Serial.println(RF_DATA);
    Serial.println(RF_PRODUCT_CODE);
    Serial.println(RF_FUNCTION_CODE);
    Serial.println(RF_COLOR_CODE);
    Serial.println(RF_EFFECT_CODE);

    if(RF_FUNCTION_CODE == 0){    // turn off
      effect =  0;  testEffect = 1;
    }
    else if((RF_FUNCTION_CODE >= 17)&(RF_FUNCTION_CODE <= 235)){  // effect and color coding range
      switch(RF_EFFECT_CODE){
        case 1: effect =  1;  testEffect = 1;  break;
        case 2: effect =  2;  testEffect = 1;  break;
        case 3: effect =  3;  testEffect = 1;  break;
        case 4: effect =  4;  testEffect = 1;  break;
        case 5: effect =  5;  testEffect = 1;  break;
        case 6:
        {
          effect =  6;  testEffect = 1;
          switch(RF_COLOR_CODE){
            case  1: rfColor_r = 255; rfColor_g =   0; rfColor_b =   0;  break;
            case  2: rfColor_r =   0; rfColor_g = 255; rfColor_b =   0;  break;
            case  3: rfColor_r =   0; rfColor_g =   0; rfColor_b = 255;  break;
            case  4: rfColor_r = 255; rfColor_g = 255; rfColor_b =   0;  break;
            case  5: rfColor_r =   0; rfColor_g = 255; rfColor_b = 255;  break;
            case  6: rfColor_r = 255; rfColor_g =   0; rfColor_b = 255;  break;
            case  7: rfColor_r = 255; rfColor_g = 255; rfColor_b = 255;  break;
          }
          break;
        }
        default:  effect =  0;  testEffect = 1; break;
      }
    }
  }
  Serial.print("E: ");
  Serial.print(effect);
  Serial.print('\t');

  Serial.print("T: ");
  Serial.print(testEffect);
  Serial.print('\t');

  
  /*
   * Radar checking
   */
  radarCondition = digitalRead(RADAR_PIN);
  Serial.print("R: ");
  Serial.print(radarCondition);
  Serial.print('\t');


  /*  
   *   Light checking
   */
  lightCondition = digitalRead(LIGHT_PIN);
  Serial.print("L: ");
  Serial.print(lightCondition);
  Serial.print('\t');


  /*  
   *   Button checking
   */
  buttonCondition = digitalRead(BUTTON_PIN);
  Serial.print("B: ");
  Serial.print(buttonCondition);
  Serial.print('\t');

  
  /*
   * LED showing
   */
  if(lightCondition == 1){    // at night
    if((testEffect == 0) & (radarCondition == 0)){  // NOBODY and NO_CHANGE_EFFECT
      if(effect != 0){
        for(i=0;i<NUM_LEDS;i++)  strip.setPixelColor(i, 50, 50, 0);  // YELLOW - KEEP WAITING
        strip.show();
        Serial.println("W...");
      }
      else{
        WS2812_setAllnShow(0, 0, 0);  // Turn off
        Serial.println("OFF");
      }
      delay(100);    // delay to make sure the RF can receive the signal correctly
    }
    else{ // maybe SOMEONE PASSED or the effect CHANGED, so running the effect
      testEffect = 0;  // the key to make sure that changing effect is just run once.
      Serial.print("E: ");
      
      if(effect == 0){            // Turn off
        Serial.print("e0");
        WS2812_setAllnShow(0, 0, 0);  // Turn off
        delay(100);    // delay to make sure the RF can receive the signal correctly
      }
      if(effect == 1){
        Serial.print("e1");
        WS2812_RAINBOW(100);
      }
      else if(effect == 2){
        Serial.print("e2");
        WS2812_theaterChaseRainbow(200);
      }
      else if(effect == 3){
        Serial.print("e3");
        WS2812_Fire(10,80,20);
      }
      else if(effect == 4){
        Serial.print("e4");
        WS2812_RGBFADEINOUT(100);
      }
      else if(effect == 5){
        Serial.print("e5");
        WS2812_colorWipe(random(0, 4)*64, random(0, 4)*64, random(0, 4)*64, 100);
      }
      else if(effect == 6){
        Serial.print("e6");
        WS2812_RGBFADEINOUTnCOLOR(rfColor_r, rfColor_g, rfColor_b, 100);
      }
    }
  }
  else{ // at day
    if(testEffect == 1){    // effect testing request
      testEffect = 0;  // the key to make sure that changing effect is just run once.
      
    }
    else{                   // don't have effect testing request
      Serial.print("day off");
      WS2812_setAllnShow(0, 0, 0);  // Turn off
      delay(200);                   // delay to make sure the RF can receive the signal correctly
    }
  }
  Serial.println();
}


void WS2812_setPixel(unsigned int Pixel, unsigned char red, unsigned char green, unsigned char blue) {
  strip.setPixelColor(Pixel, red, green, blue);
}

void WS2812_setAllnShow(unsigned char red, unsigned char green, unsigned char blue) {
  unsigned int i;
  for(i=0;i<NUM_LEDS;i++) strip.setPixelColor(i, red, green, blue);
  strip.show();
}

void WS2812_setAllData(unsigned char red, unsigned char green, unsigned char blue) {
  unsigned int i;
  for(i=0;i<NUM_LEDS;i++) strip.setPixelColor(i, red, green, blue);
}

void WS2812_showStrip(){
  strip.show();
}


void WS2812_RAINBOW(unsigned long Speed){
  unsigned long i;
  unsigned long oldColor;
  static unsigned long seq = 0;
  static unsigned char R = 0;
  static unsigned char G = 0;
  static unsigned char B = 0;
  
  
  if(seq == 0){
    if(R < 255) R += 51;
    else        seq = 1;
  }
  else if(seq == 1){
    if(B >  0)  B -= 51;
    else        seq = 2;
  }
  else if(seq == 2){
    if(G < 255) G += 51;
    else        seq = 3;
  }
  else if(seq == 3){
    if(R >  0)  R -= 51;
    else        seq = 4;
  }
  else if(seq == 4){
    if(B < 255) B += 51;
    else        seq = 5;
  }
  else if(seq == 5){
    if(G >  0)  G -= 51;
    else        seq = 0;
  }

  
  for(i = 1; i < NUM_LEDS; i++) strip.setPixelColor(NUM_LEDS-i, strip.getPixelColor(NUM_LEDS-1-i));
  strip.setPixelColor(0,R,G,B);

  strip.show();
  
  delay(Speed);
}

void WS2812_meteorRain(unsigned char red, unsigned char green, unsigned char blue, unsigned char meteorSize,
  unsigned char meteorTrailDecay, bool meteorRandomDecay, unsigned int speedxDelay) { 

  static unsigned int i = 0;
  static unsigned int j;

  if(i == 0)  WS2812_setAllnShow(0,0,0);
  
 
  if(i <  (NUM_LEDS+NUM_LEDS)){
    for( int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(0, 10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
    for( int j = 0; j < meteorSize; j++) {
      if( ((i-j) < NUM_LEDS) & ((i-j)>=0) ) {
        WS2812_setPixel(i-j, red, green, blue);
      } 
    }
   
    WS2812_showStrip();
    delay(speedxDelay);
    i++;
  }
  else  i = 0;
}

void fadeToBlack(unsigned int ledNo, unsigned char fadeValue) {

  unsigned long oldColor;
  unsigned char r, g, b;
  
  oldColor = strip.getPixelColor(ledNo);
  r = oldColor >> 16;
  g = oldColor >> 8;
  b = oldColor;

  r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
  g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
  b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
  
  WS2812_setPixel(ledNo, r, g, b);

}

void WS2812_Fire(unsigned int Cooling, unsigned int Sparking, unsigned int speedxDelay) {
  static unsigned char heat[MAX_NUM_LEDS];
  unsigned int cooldown;
  unsigned int i, j, k;
  // Step 1.  Cool down every cell a little
  for(i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for(k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(0, 255) < Sparking ) {
    int y = random(0, 7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for(j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  WS2812_showStrip();
  delay(speedxDelay);
}

void setPixelHeatColor (unsigned int Pixel, unsigned char temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  unsigned char t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  unsigned char  heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    WS2812_setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    WS2812_setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    WS2812_setPixel(Pixel, heatramp, 0, 0);
  }
}


void WS2812_HALLOWEENEYES(unsigned char red, unsigned char green, unsigned char blue, 
  unsigned long EyeWidth, unsigned long EyeSpace, unsigned char Fade,
    unsigned long Steps, unsigned long FadeDelay, unsigned long EndPause){
//  randomSeed(analogRead(0));
  
  int i;
  int StartPoint  = (NUM_LEDS - (2*EyeWidth) - EyeSpace )/2;
  int Start2ndEye = StartPoint + EyeWidth + EyeSpace;
  
  for(i = 0; i < EyeWidth; i++) {
    WS2812_setPixel(StartPoint + i, red, green, blue);
    WS2812_setPixel(Start2ndEye + i, red, green, blue);
  }
  
  if(Fade == true) {
    float r, g, b;
  
    for(int j = Steps; j >= 0; j--) {
      r = j*(red/Steps);
      g = j*(green/Steps);
      b = j*(blue/Steps);
      
      for(i = 0; i < EyeWidth; i++) {
        WS2812_setPixel(StartPoint + i, r, g, b);
        WS2812_setPixel(Start2ndEye + i, r, g, b);
      }
      WS2812_showStrip();
      delay(FadeDelay);
    }
  }
  
  WS2812_setAllnShow(0,0,0); // Set all black
  WS2812_showStrip();
  delay(EndPause);
}

void WS2812_theaterChaseRainbow(unsigned int speedxDelay) {
  unsigned char *c;
  unsigned int i, q;
  static unsigned int j = 0;
  j++;
  if(j >= 256)  j = 0;
  for (q=0; q < 3; q++) {
    for (int i=0; i < NUM_LEDS; i=i+3) {
      c = Wheelx( (i+j) % 255);
      WS2812_setPixel(i+q, *c, *(c+1), *(c+2));    //turn every third pixel on
    }
    WS2812_showStrip();
   
    delay(speedxDelay);
   
    for (i=0; i < NUM_LEDS; i=i+3) {
      WS2812_setPixel(i+q, 0,0,0);        //turn every third pixel off
    }
  }
}

unsigned char * Wheelx(unsigned char WheelPos) {
  static unsigned char c[3];
  
  if(WheelPos < 85) {
    c[0]=WheelPos * 3;
    c[1]=255 - WheelPos * 3;
    c[2]=0;
  }
  else if(WheelPos < 170) {
    WheelPos -= 85;
    c[0]=255 - WheelPos * 3;
    c[1]=0;
    c[2]=WheelPos * 3;
  }
  else {
    WheelPos -= 170;
    c[0]=0;
    c[1]=WheelPos * 3;
    c[2]=255 - WheelPos * 3;
  }

  return c;
}

void WS2812_colorWipe(unsigned char red, unsigned char green, unsigned char blue, unsigned int speedxDelay) {
  static unsigned int i = NUM_LEDS;
  static unsigned char R,G,B;


  if(i<NUM_LEDS)  i++;
  else{
    i = 0;
    R = red;
    G = green;
    B = blue;
  }
  
  WS2812_setPixel(i, R, G, B);
  WS2812_showStrip();
  delay(speedxDelay);

}

void WS2812_RGBFADEINOUT(unsigned long speedx){
  static unsigned char clr = 0;
  static unsigned char brn = 0;
  static unsigned char dir = 0;
  
  if(dir == 0){
    if(brn < 255) brn+=5;
    else{
      dir = 1;
      brn-=5;
    }
  }
  else{
    if(brn > 0) brn-=5;
    else{
      dir = 0;
      brn+=5;
      clr++;
      if(clr > 2) clr=0;
    }
  }
  

  switch(clr) { 
    case 0: WS2812_setAllnShow(brn,0,0); break;
    case 1: WS2812_setAllnShow(0,brn,0); break;
    case 2: WS2812_setAllnShow(0,0,brn); break;
  }
  strip.show();
  delay(speedx);
}

void WS2812_RGBFADEINOUTnCOLOR(unsigned char r, unsigned char g, unsigned char b, unsigned long speedx){
  unsigned int  temp = 0;
  static unsigned char brn = 0;
  static unsigned char dir = 0;
  
  if(dir == 0){
    if(brn < 255) brn+=5;
    else{
      dir = 1;
      brn-=5;
    }
  }
  else{
    
    if(brn > 0) brn-=5;
    else{
      dir = 0;
      brn+=5;
    }
  }

  temp = brn*r;
  r = temp >> 8;
  temp = brn*g;
  g = temp >> 8;
  temp = brn*b;
  b = temp >> 8;

  WS2812_setAllnShow(r,g,b);

  delay(speedx);
}

