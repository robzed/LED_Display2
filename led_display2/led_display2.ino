#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>

#define NUM_ROWS 24
#define NUM_COLUMNS 24
#define NUM_LEDS (NUM_ROWS*NUM_COLUMNS)
#define DATA_PIN 6

Adafruit_NeoPixel pixels(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

unsigned int max_brightness = 150;
//unsigned int make_red


// could make this more efficient if it used a list of things to run next
class Timer {
public:
  Timer(unsigned long time_in_ms) { t = millis() + time_in_ms; duration = time_in_ms; }
  Timer() { t = millis(); } // start off expired
  bool finished() { return millis() >= t; }
  void set(unsigned long time_in_ms) { t = millis() + time_in_ms; duration = time_in_ms; }
  void repeat() { t += duration; }
private:
  unsigned long t;
  unsigned long duration;
};

BLEService displayService("ec5e7473-c520-42b5-8dcd-b04f59ce996d");
BLEUnsignedCharCharacteristic display_mode_characteristic("cdf00644-b962-4ca1-8f8f-c40fe4f21c89", BLEWrite);
BLEDescriptor modeDescriptor("2901", "Display mode");
 

void set_display_mode(byte mode);

void onModeCharValueUpdate(BLEDevice central, BLECharacteristic characteristic) 
{
  Serial.println("Write mode:");
  byte value = 0;
  characteristic.readValue(value);
  Serial.println(value);

  set_display_mode(value);
}

unsigned long connTime = 0;

void blePeripheralConnectHandler(BLEDevice central) {
    Serial.println("Connected event, central: ");
    Serial.println(central.address());
    connTime = millis();
}

void blePeripheralDisconnectHandler(BLEDevice central) {
    Serial.println("Disconnected event, central: ");
    Serial.println(central.address());
}

void setup() {
  pixels.begin();
  Serial.begin(115200);
  Serial.println(F("\nLED Display Board v2\n"));
  pinMode(LED_BUILTIN, OUTPUT);
  if (!BLE.begin())
  {
    Serial.println("Starting BLE failed!\n");
    while(1) { }
  }
  else
  {
    //BLE.debug(Serial);
    BLE.setLocalName("Display board v2");
    BLE.setDeviceName("Display board v2");
    BLE.setAdvertisedService(displayService);
    display_mode_characteristic.addDescriptor(modeDescriptor);
    displayService.addCharacteristic(display_mode_characteristic);
    BLE.addService(displayService);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
    display_mode_characteristic.setEventHandler(BLEWritten, onModeCharValueUpdate);
    BLE.advertise();
  }
}

Timer display_timer;  // start off expired
int display_mode = 0;
int display_col = 0;

//##########################################################################
#define IMAGE_ROWS 20
const PROGMEM uint32_t all_off_image[] = {
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};
//##########################################################################
// xmas
//##########################################################################

// converted from Bauble.txt
const PROGMEM uint32_t bauble[] = {
0 , 6144 , 9216 , 6144 ,
15360 , 49920 , 114816 , 145472 ,
460320 , 311712 , 1048560 , 699056 ,
873808 , 1048560 , 460320 , 319968 ,
138304 , 66432 , 49920 , 15360 ,
};

// converted from xmas tree inv 0.r
const PROGMEM uint32_t xmas_tree_inv_0[] = {
0 , 5120 , 8704 , 5120 ,
8704 , 16640 , 32896 , 16640 ,
32896 , 65600 , 131104 , 32896 ,
65600 , 131104 , 262160 , 524296 ,
5120 , 5120 , 32512 , 15872 ,
};

// converted from xmas tree inv 1.r
const PROGMEM uint32_t xmas_tree_inv_1[] = {
2048 , 5120 , 8704 , 5120 ,
8704 , 17664 , 32896 , 17664 ,
32896 , 67648 , 147488 , 32896 ,
65600 , 147488 , 263248 , 655368 ,
5120 , 5120 , 32512 , 15872 ,
};

// converted from xmas tree inv 2.r
const  PROGMEM uint32_t xmas_tree_inv_2[] = {
0 , 5120 , 8704 , 5120 ,
12800 , 16640 , 41088 , 16640 ,
32896 , 65856 , 147488 , 32896 ,
67904 , 131104 , 262160 , 524296 ,
5120 , 5120 , 32512 , 15872 ,
};

// converted from xmas tree inv 4.r
const PROGMEM uint32_t xmas_tree_inv_4[] = {
2048 , 5120 , 8704 , 5120 ,
12800 , 17664 , 41088 , 17664 ,
32896 , 67904 , 147488 , 32896 ,
67904 , 147488 , 263248 , 655368 ,
5120 , 5120 , 32512 , 15872 ,
};


// converted from Holly.r
const PROGMEM uint32_t Holly[] = {
0 , 0 , 0 , 0 ,
1966140 , 1966140 , 2081276 , 2081276 ,
508400 , 522224 , 522224 , 63360 ,
14208 , 49152 , 109568 , 129536 ,
56832 , 3072 , 0 , 0 ,
};

// converted from House and trees.r
const PROGMEM uint32_t House_and_trees[] = {
2 , 2055 , 5148 , 8762 ,
16640 , 8421552 , 12653681 , 14811185 ,
12850195 , 15204363 , 16252943 , 14859943 ,
15739395 , 16427687 , 14690831 , 15739395 ,
16268807 , 16525839 , 13642271 , 13647365 ,
};

// converted from Santa in his sleigh.r
const PROGMEM uint32_t Santa_in_his_sleigh[] = {
0 , 0 , 2244608 , 1323008 ,
1865728 , 4093952 , 8335360 , 8353792 ,
4684928 , 4423488 , 4324928 , 4194392 ,
4194372 , 2097282 , 1048706 , 1048322 ,
279042 , 279044 , 2097144 , 0 ,
};

// converted from Star.r
const PROGMEM uint32_t Star[] = {
0 , 0 , 4096 , 4096 ,
14336 , 14336 , 31744 , 1048544 ,
524224 , 130816 , 65024 , 130816 ,
126720 , 247680 , 196992 , 0 ,
0 , 0 , 0 , 0 ,
};

// converted from ho ho santa.g
const PROGMEM uint32_t ho_ho_santa[] = {
0 , 5243872 , 7343128 , 5257220 ,
16386 , 2113538 , 5277682 , 5347340 ,
2355748 , 245762 , 5353466 , 7356414 ,
5258270 , 15934 , 2113534 , 5251068 ,
5251068 , 2101240 , 1008 , 0 ,
};
 

// converted from Present.r
const PROGMEM uint32_t present[] = {
0 , 65664 , 181056 , 140352 ,
130944 , 32256 , 524256 , 318240 ,
318240 , 301344 , 268320 , 524256 ,
524256 , 268320 , 268320 , 268320 ,
268320 , 268320 , 524256 , 0 ,
};


// converted from happy new year.g
const PROGMEM uint32_t happy_new_year[] = {
9018833 , 9065802 , 16506308 , 9064708 ,
9064708 , 0 , 0 , 1145352 ,
1655304 , 1405264 , 1261904 , 1144992 ,
0 , 0 , 2290488 , 1344676 ,
583608 , 558244 , 586916 , 0 ,
};


// converted from angel.r
const PROGMEM uint32_t angel[] = {
15360 , 16896 , 15360 , 0 ,
6144 , 9216 , 3686940 , 4473378 ,
4334658 , 8460417 , 8447745 , 8430849 ,
8463489 , 8536641 , 8667681 , 8929809 ,
8978193 , 5252106 , 2106372 , 26112 ,
};


// converted from gingerbread man3.r
const PROGMEM uint32_t gingerbread_man3[] = {
32256 , 33024 , 65664 , 74880 ,
65664 , 42240 , 39168 , 983280 ,
1048584 , 1054728 , 923760 , 65664 ,
71808 , 71808 , 131136 , 262176 ,
539664 , 1098504 , 1245384 , 786480 ,
};


// converted from rudolph.r
const PROGMEM uint32_t rudolph[] = {
163840 , 1032192 , 163840 , 163840 ,
2031616 , 2129920 , 13139456 , 12583392 ,
3932190 , 131089 , 131089 , 131088 ,
262176 , 655296 , 656448 , 656448 ,
1180736 , 1312896 , 1312896 , 2361472 ,
};


// converted from rudolph2.r
const PROGMEM uint32_t rudolph2[] = {
1081344 , 3211264 , 5472256 , 1966080 ,
786432 , 1277952 , 2637824 , 12595201 ,
12587007 , 3932163 , 131074 , 65538 ,
65538 , 2031618 , 2229234 , 4521554 ,
10027082 , 10813481 , 4489254 , 196636 ,
};


// converted from ho ho santa closed mouth.g
const PROGMEM uint32_t ho_ho_santa_closed_mouth[] = {
0 , 5243872 , 7343128 , 5257220 ,
16386 , 2113538 , 5277682 , 5347340 ,
2355748 , 245762 , 5353466 , 7356414 ,
5258814 , 16382 , 2113534 , 5251068 ,
5251068 , 2101240 , 1008 , 0 ,
};

// converted from snowman2.r
const PROGMEM uint32_t snowman2[] = {
0 , 0 , 15360 , 16896 ,
33024 , 74880 , 65664 , 71808 ,
2133248 , 1131142 , 4078716 , 4456486 ,
268324 , 268320 , 262176 , 268320 ,
137280 , 65664 , 49920 , 15360 ,
};

struct display_list_t {
  const PROGMEM uint32_t * const PROGMEM image;
  uint8_t wait_time;
};

const PROGMEM display_list_t xmas_list[] = {
    { xmas_tree_inv_0, 5 },
    { xmas_tree_inv_1, 5 },
    { xmas_tree_inv_2, 5 },
    { xmas_tree_inv_4, 5 },
    { xmas_tree_inv_1, 5 },
    { xmas_tree_inv_2, 5 },
    { xmas_tree_inv_4, 5 },
    { xmas_tree_inv_1, 5 },
    { xmas_tree_inv_2, 5 },
    { xmas_tree_inv_4, 5 },
    { xmas_tree_inv_0, 5 },
    
    { all_off_image, 10 },
    { bauble, 20 },
    { Holly, 20 },
    { House_and_trees, 20 },
    { Santa_in_his_sleigh, 20 },
    { Star, 20 },
    { ho_ho_santa, 5 },
    { ho_ho_santa_closed_mouth, 5 },
    { ho_ho_santa, 5 },
    { ho_ho_santa_closed_mouth, 5 },
    { ho_ho_santa, 5 },
    { ho_ho_santa_closed_mouth, 5 },
      
    // New pictures
    { rudolph, 20 },
    { present, 20 },
    { happy_new_year, 20 },
    { angel, 20 },
    { gingerbread_man3, 20 },
    { rudolph2, 20 },
    { snowman2, 20 },
    { all_off_image, 10 },
    { 0, 0 }
};

//##########################################################################
// halloween
//##########################################################################
// converted from emily_ghost.g
const PROGMEM uint32_t emily_ghost[] = {
32704 , 65504 , 65504 , 131056 ,
130672 , 237176 , 237560 , 524280 ,
519672 , 519676 , 521212 , 262140 ,
262142 , 262142 , 262143 , 262143 ,
262143 , 524287 , 474317 , 399556 ,
};

// converted from emily_pumpkin.g
const PROGMEM uint32_t emily_pumpkin[] = {
28672 , 21496 , 130052 , 393218 ,
524289 , 1049089 , 2097665 , 4196097 ,
4456449 , 4456449 , 9314369 , 8417474 ,
8388996 , 8455944 , 4324872 , 2161680 ,
1048608 , 524352 , 262528 , 261632 ,
};

// converted from emily_spider.g
const PROGMEM uint32_t emily_spider[] = {
0 , 3 , 12 , 16 ,
32 , 32320 , 130951 , 16767992 ,
130944 , 1048448 , 15850495 , 252800 ,
1959808 , 14745471 , 196608 , 786432 ,
15728640 , 0 , 0 , 0 ,
};

// converted from skull.g
const PROGMEM uint32_t skull[] = {
262112 , 524272 , 1048568 , 1048568 ,
1015672 , 933432 , 793624 , 933432 ,
1015672 , 1043448 , 519152 , 519152 ,
262112 , 131008 , 98112 , 87360 ,
87360 , 120256 , 131008 , 65408 ,
};

// converted from pumpkin_eyes_1a.g
const PROGMEM uint32_t pumpkin_eyes_1a[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
4088271 , 4088271 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};

// converted from pumpkin_eyes_1b.g
const PROGMEM uint32_t pumpkin_eyes_1b[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
3989919 , 3989919 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};

// converted from pumpkin_eyes_1c.g
const PROGMEM uint32_t pumpkin_eyes_1c[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
3793215 , 3793215 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};

// converted from pumpkin_eyes_1d.g
const PROGMEM uint32_t pumpkin_eyes_1d[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
4137447 , 4137447 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};

// converted from face1.txt
const PROGMEM uint32_t face1[] = {
16128 , 32640 , 458712 , 131040 ,
65472 , 65472 , 101472 , 217776 ,
363624 , 325576 , 28544 , 60864 ,
111456 , 183504 , 287112 , 16128 ,
0 , 0 , 0 , 0 ,
};

// converted from face2.txt
const PROGMEM uint32_t face2[] = {
16128 , 32640 , 65472 , 524280 ,
65472 , 65472 , 101472 , 86688 ,
232560 , 325576 , 290696 , 60864 ,
103008 , 180432 , 156048 , 16128 ,
0 , 0 , 0 , 0 ,
};

// converted from face3.txt
const uint32_t face3[] = {
16128 , 32640 , 65472 , 458712 ,
131040 , 65472 , 101472 , 86688 ,
99424 , 192464 , 290184 , 316104 ,
37440 , 49344 , 90528 , 409368 ,
0 , 0 , 0 , 0 ,
};

// converted from face4.txt
const uint32_t face4[] = {
16128 , 294792 , 196560 , 131040 ,
65472 , 35904 , 95136 , 86688 ,
232560 , 194512 , 290696 , 60864 ,
37440 , 246000 , 287112 , 16128 ,
0 , 0 , 0 , 0 ,
};

// converted from jsw_demon1.txt
const PROGMEM uint32_t jsw_demon1[] = {
0 , 0 , 0 , 0 ,
514 , 8706 , 43348 , 43224 ,
29028 , 8922 , 15150 , 9897 ,
9457 , 8411 , 8344 , 340 ,
0 , 0 , 0 , 0 ,
};

// converted from jsw_demon2.txt
const PROGMEM uint32_t jsw_demon2[] = {
0 , 0 , 0 , 0 ,
514 , 514 , 372 , 216 ,
20820 , 21226 , 9054 , 58793 ,
7921 , 6361 , 2201 , 2388 ,
1674 , 1024 , 0 , 0 ,
};

// converted from jsw_demon3.txt
const PROGMEM uint32_t jsw_demon3[] = {
0 , 0 , 0 , 0 ,
514 , 910 , 240 , 340 ,
730 , 950 , 4491 , 19113 ,
11227 , 38520 , 27856 , 1364 ,
650 , 256 , 128 , 64 ,
};



const PROGMEM display_list_t halloween_list[] = {
    { emily_ghost , 20 },
    { emily_pumpkin , 20 },
    { emily_spider , 20 },
    { skull , 20 },
    { all_off_image , 5 },
    
    { pumpkin_eyes_1a , 5 },
    { pumpkin_eyes_1b , 5 },
    { pumpkin_eyes_1c , 5 },
    { pumpkin_eyes_1d , 5 },
    { pumpkin_eyes_1a , 5 },
    { pumpkin_eyes_1b , 5 },
    { pumpkin_eyes_1c , 5 },
    { pumpkin_eyes_1d , 5 },
    { pumpkin_eyes_1a , 5 },
    { pumpkin_eyes_1b , 5 },
    { pumpkin_eyes_1c , 5 },
    { pumpkin_eyes_1d , 5 },
    { all_off_image , 5 },
    
    { face1 , 5 },
    { face2 , 5 },
    { face3 , 5 },
    { face4 , 5 },
    { face1 , 5 },
    { face2 , 5 },
    { face3 , 5 },
    { face4 , 5 },
    { all_off_image , 5 },
    
    { jsw_demon1 , 5 },
    { jsw_demon2 , 5 },
    { jsw_demon3 , 5 },
    { jsw_demon1 , 5 },
    { jsw_demon2 , 5 },
    { jsw_demon3 , 5 },
    { all_off_image , 5 },
    { 0, 0 }
};


void row_swipe()
{
  uint8_t r, g, b;
  
  switch(display_mode)
  {
    case 0:
      r = max_brightness; g = 0; b = 0;
      break;
    case 1:
      r = 0; g = max_brightness; b = 0;
      break;
    case 2:
    default:
      r = 0; g = 0; b = max_brightness;
      break;    
  }
//  for(int col = 0; col < NUM_COLUMNS; col++) { 

  for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
    pixels.setPixelColor(display_col+row, pixels.Color(r, g, b));
  }
  pixels.show();
  // clear this led for the next time around the loop
  for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
    pixels.setPixelColor(display_col+row, pixels.Color(0, 0, 0));
  }
  display_timer.set(200);
  
  display_col += 1;
  if(display_col >= NUM_COLUMNS) { 
    display_col = 0;
    display_mode ++; 
    if(display_mode == 3) { display_mode = 0; }
  }
}

void slow_tickup()
{
    pixels.setPixelColor(display_col, pixels.Color(20, 20, 20));
    pixels.show();
    display_timer.set(50);

    display_col += 1;
    if((display_col % NUM_COLUMNS) == 0)
    {
        pixels.clear();      
    }
    if(display_col >= NUM_LEDS) { 
        display_col = 0;
    }

}


void pixel_walk()
{
  uint8_t r, g, b;
  
  switch(display_mode)
  {
    case 0:
      r = max_brightness; g = max_brightness; b = max_brightness;
      break;
    case 1:
      r = max_brightness; g = 0; b = 0;
      break;
    case 2:
      r = 0; g = max_brightness; b = 0;
      break;
    case 3:
    default:
      r = 0; g = 0; b = max_brightness;
      break;    
  }
  pixels.setPixelColor(display_col, pixels.Color(r, g, b));
  pixels.show();
  pixels.setPixelColor(display_col, pixels.Color(0, 0, 0));

  display_timer.set(30);
  
  display_col += 1;
  if(display_col >= NUM_LEDS) { 
    display_col = 0;
    display_mode ++; 
    if(display_mode == 4) { display_mode = 0; }
  }
}


const display_list_t* which_list = xmas_list; //halloween_list;
const display_list_t* pointer = 0;

void slideshow()
{
  if(not pointer)
  {
    pointer = which_list;
  }
  // convert image
  pixels.clear();
  const uint32_t *image = pointer->image;

  // There are only 20 line on the images
  for(int row = 0; row < (IMAGE_ROWS*NUM_COLUMNS); row += NUM_COLUMNS)
  //for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS)
  {
    uint32_t line = *image++;
    for(int col=0, col_mask=0x800000; col < NUM_COLUMNS; col_mask >>= 1, col++)
    {
      if(line & col_mask) {
          pixels.setPixelColor(col+row, pixels.Color(70, 70, 100));
      }
    }
  }
  pixels.show();
  
  display_timer.set(pointer->wait_time*100);

  // next image?
  pointer++;
  if(not pointer->image)
  {
    pointer = 0;
  }
}

// could use Virtual functions instead of function pointer
void (*display_pointer) ()  = slideshow;

void show_display()
{
  if(not display_timer.finished()) return;
  display_pointer();
}

void set_display_mode(byte mode)
{
  switch(mode)
  {
    case 1:
    case '1':
    case 'X':
    case 'x':
      display_pointer = slideshow;
      which_list = xmas_list;
      pointer = 0;
      break;
    case 2:
    case '2':
    case 'H':
    case 'h':
      display_pointer = slideshow;
      which_list = halloween_list;
      pointer = 0;
      break;
    case 3:
    case '3':
    case 'R':
    case 'r':
      display_pointer = row_swipe;
      break;
    case 4:
    case '4':
    case 'P':
    case 'p':
      display_pointer = pixel_walk;
      break;
    case 5:
    case '5':
    case 'T':
    case 't':
      display_pointer = slow_tickup;
      break;
    default:
      display_pointer = slideshow;
      which_list = halloween_list;
      pointer = 0;
      break;
  }
}


int LED_mode = 0;
Timer LED_timer;
void do_LED()
{
  if(not LED_timer.finished()) return;

  switch(LED_mode)
  {
    case 0:
    default:
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      LED_timer.set(100);                       // wait for a second
      LED_mode = 0;
      break;
    case 1:
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      LED_timer.set(100);                       // wait for a second
      break;
    case 2:
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      LED_timer.set(100);                       // wait for a second
      break;
    case 3:
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      LED_timer.set(1000);                       // wait for a second
      break;
  }
  LED_mode ++;
}

void loop() {
  show_display();
  do_LED();

  BLE.poll();

  if (BLE.connected()) {
      if (millis() > connTime + 100000) {
          Serial.println("time to disconnect");
          BLE.disconnect();
      }
  }
}
