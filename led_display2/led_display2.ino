#include <Adafruit_NeoPixel.h>
#include <ArduinoBLE.h>

#define NUM_ROWS 24
#define NUM_COLUMNS 24
#define NUM_LEDS (NUM_ROWS*NUM_COLUMNS)
#define DATA_PIN 6

Adafruit_NeoPixel pixels(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

unsigned char max_brightness = 150;
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
BLEUnsignedCharCharacteristic display_mode_characteristic("cdf00644-b962-4ca1-8f8f-c40fe4f21c89", BLEWrite | BLERead);
BLEDescriptor modeDescriptor("2901", "Display mode");

BLEUnsignedCharCharacteristic brightness_characteristic("3f35a274-3456-49a8-aefc-2bf6c6389838", BLEWrite | BLERead);
BLEDescriptor brightnessDescriptor("2901", "Brightness");
 

void set_display_mode(byte mode);

void onModeCharValueUpdate(BLEDevice central, BLECharacteristic characteristic) 
{
  Serial.println("Write mode:");
  byte value = 0;
  characteristic.readValue(value);
  Serial.println(value);

  set_display_mode(value);
}

void onBrightnessCharValueUpdate(BLEDevice central, BLECharacteristic characteristic) 
{
  Serial.println("Brightness:");
  byte value = 0;
  characteristic.readValue(value);
  Serial.println(value);

  // value constraint (randowly set ... avoid large full display currents).
  if(value > 160) { value = 160; }
  
  max_brightness = value;
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
  Serial.println(F("\n\rLED Display Board v2\n\r"));
  pinMode(LED_BUILTIN, OUTPUT);
  if (!BLE.begin())
  {
    Serial.println("Starting BLE failed!\n\r");
    while(1) { }
  }
  else
  {
    //BLE.debug(Serial);
    BLE.setLocalName("Display board v2");
    BLE.setDeviceName("Display board v2");
    BLE.setAdvertisedService(displayService);
    display_mode_characteristic.addDescriptor(modeDescriptor);
    display_mode_characteristic.writeValue(0); 
    displayService.addCharacteristic(display_mode_characteristic);
    display_mode_characteristic.setEventHandler(BLEWritten, onModeCharValueUpdate);
    
    brightness_characteristic.addDescriptor(brightnessDescriptor);
    brightness_characteristic.writeValue(max_brightness); 
    displayService.addCharacteristic(brightness_characteristic);    
    brightness_characteristic.setEventHandler(BLEWritten, onBrightnessCharValueUpdate);
    
    BLE.addService(displayService);
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
    BLE.advertise();
  }
}

//##########################################################################

Timer display_timer;  // start off expired
int display_mode = 0;
int display_col = 0;

//##########################################################################

class ImageBase {
public:
  virtual void display() = 0;
  virtual bool last() { return false; }
};

//##########################################################################

const int BINARY20LINE_ELEMENTS = 20;

class Binary20Line : public ImageBase {
public:
  void display();
  Binary20Line(const uint32_t *pArray) : ref_image(pArray) { }
  const uint32_t *ref_image;
};

void Binary20Line::display()
{
  // convert image
  pixels.clear();
  const uint32_t *image = ref_image;

  // There are only 20 line on the images
  for(int row = 0; row < (BINARY20LINE_ELEMENTS * NUM_COLUMNS); row += NUM_COLUMNS)
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
}

//##########################################################################
//#define IMAGE_ROWS 20
//const PROGMEM uint32_t all_off_image[] = {
//0 , 0 , 0 , 0 ,
//0 , 0 , 0 , 0 ,
//0 , 0 , 0 , 0 ,
//0 , 0 , 0 , 0 ,
//0 , 0 , 0 , 0 ,
//};
class OffImageType : public ImageBase {
public:
  void display() { pixels.clear(); pixels.show(); }
};
OffImageType all_off_image;

//##########################################################################
class LastType : public ImageBase {
public:
  void display() { }
  virtual bool last() { return true; }
};
LastType end_of_list;

//##########################################################################
class TextImages : public ImageBase {
public:
  void display();
  TextImages(const char **pArray, int number_of_lines) : ref_image(pArray), num_lines(number_of_lines) { }
  const char **ref_image;
  int num_lines;
};

void TextImages::display()
{
  // convert image
  pixels.clear();
  const char **image = ref_image;

  // There are only 20 line on the images
  for(int row = 0, row_count = 0; row_count < num_lines; row_count++, row += NUM_COLUMNS)
  {
    const char* line = *image++;
    for(int col=0; col < NUM_COLUMNS; col++)
    {
      char c = *line++;
      if(c == 0) break;
      switch(c)
      {
        case ' ':
          break;
        case 'W':
          pixels.setPixelColor(col+row, pixels.Color(max_brightness, max_brightness, max_brightness));
          break;
        case 'R':
          pixels.setPixelColor(col+row, pixels.Color(max_brightness, 0, 0));
          break;
        case 'G':
          pixels.setPixelColor(col+row, pixels.Color(0, max_brightness, 0));
          break;
        case 'g':
          pixels.setPixelColor(col+row, pixels.Color(0, max_brightness/2, 0));
          break;
        case 'B':
          pixels.setPixelColor(col+row, pixels.Color(0, 0, max_brightness));
          break;
        case 'C':
          pixels.setPixelColor(col+row, pixels.Color(0, max_brightness, max_brightness));
          break;
        case 'Y':
          pixels.setPixelColor(col+row, pixels.Color(max_brightness, max_brightness, 0));
          break;
        default:
          pixels.setPixelColor(col+row, pixels.Color(max_brightness/2, 0, 0));
          break;
      }
    }
  }
  pixels.show();
}

//##########################################################################
// xmas
//##########################################################################

const char PROGMEM *bauble_string_array[] PROGMEM = {
"                        ",
"           WW           ",
"          W  W          ",
"           WW           ",
"          WWWW          ",
"        WW    WW        ",
"       WWW      W       ",
"      W   WWW    W      ",
"     WWW     WW   W     ",
"     W  WW     WW W     ",
"    WWWWWWWWWWWWWWWW    ",
"    W R R R R R R RW    ",
"    WR R R R R R R W    ",
"    WWWWWWWWWWWWWWWW    ",
"     WWW     WW   W     ",
"     W  WWW    WWWW     ",
"      W    WWW   W      ",
"       W      WWW       ",
"        WW    WW        ",
"          WWWW          ",
};
TextImages bauble(bauble_string_array, sizeof(bauble_string_array)/sizeof(char*));

const char PROGMEM *xmas_tree_inv_0_string_array[] PROGMEM = {
"                        ",
"           W W          ",
"          W   W         ",
"           W W          ",
"          W   W         ",
"         W     W        ",
"        W       W       ",
"         W     W        ",
"        W       W       ",
"       W         W      ",
"      W           W     ",
"        W       W       ",
"       W         W      ",
"      W           W     ",
"     W             W    ",
"    W               W   ",
"           W W          ",
"           W W          ",
"         WWWWWWW        ",
"          WWWWW         ",
};
TextImages xmas_tree_inv_0(xmas_tree_inv_0_string_array, sizeof(xmas_tree_inv_0_string_array)/sizeof(char*));

const char PROGMEM *xmas_tree_inv_1_string_array[] PROGMEM = {
"            W           ",
"           W W          ",
"          W   W         ",
"           W W          ",
"          W   W         ",
"         W   W W        ",
"        W       W       ",
"         W   W W        ",
"        W       W       ",
"       W    W    W      ",
"      W  W        W     ",
"        W       W       ",
"       W         W      ",
"      W  W        W     ",
"     W       W   W W    ",
"    W W             W   ",
"           W W          ",
"           W W          ",
"         WWWWWWW        ",
"          WWWWW         ",
};
TextImages xmas_tree_inv_1(xmas_tree_inv_1_string_array, sizeof(xmas_tree_inv_1_string_array)/sizeof(char*));

const char PROGMEM *xmas_tree_inv_2_string_array[] PROGMEM = {
"                        ",
"           W W          ",
"          W   W         ",
"           W W          ",
"          WW  W         ",
"         W     W        ",
"        W W     W       ",
"         W     W        ",
"        W       W       ",
"       W       W W      ",
"      W  W        W     ",
"        W       W       ",
"       W    W  W W      ",
"      W           W     ",
"     W             W    ",
"    W               W   ",
"           W W          ",
"           W W          ",
"         WWWWWWW        ",
"          WWWWW         ",
};
TextImages xmas_tree_inv_2(xmas_tree_inv_2_string_array, sizeof(xmas_tree_inv_2_string_array)/sizeof(char*));

const char PROGMEM *xmas_tree_inv_4_string_array[] PROGMEM = {
"            W           ",
"           W W          ",
"          W   W         ",
"           W W          ",
"          WW  W         ",
"         W   W W        ",
"        W W     W       ",
"         W   W W        ",
"        W       W       ",
"       W    W  W W      ",
"      W  W        W     ",
"        W       W       ",
"       W    W  W W      ",
"      W  W        W     ",
"     W       W   W W    ",
"    W W             W   ",
"           W W          ",
"           W W          ",
"         WWWWWWW        ",
"          WWWWW         ",
};
TextImages xmas_tree_inv_4(xmas_tree_inv_4_string_array, sizeof(xmas_tree_inv_4_string_array)/sizeof(char*));

const char PROGMEM *Holly_string_array[] PROGMEM = {
"                        ",
"                        ",
"                        ",
"                        ",
"   WWWW           WWWW  ",
"   WWWW           WWWW  ",
"   WWWWWWW     WWWWWWW  ",
"   WWWWWWW     WWWWWWW  ",
"     WWWWW     WWWWW    ",
"     WWWWWWW WWWWWWW    ",
"     WWWWWWW WWWWWWW    ",
"        WWWW WWWW       ",
"          WW WWWW       ",
"        WW              ",
"       WW W WW          ",
"       WWWWWW W         ",
"        WW WWWW         ",
"            WW          ",
"                        ",
"                        ",
};
TextImages Holly(Holly_string_array, sizeof(Holly_string_array)/sizeof(char*));

const char PROGMEM *House_and_trees_string_array[] PROGMEM = {
"                      W ",
"            W        WWW",
"           W W     WWW  ",
"          W   W   WWW W ",
"         W     W        ",
"W       W       W WW    ",
"WW     W   W W   WWW   W",
"WWW   W           WW   W",
"WW   W     W W     W  WW",
"WWW W               W WW",
"WWWWW               WWWW",
"WWW   W W WWWWW W W  WWW",
"WWWW      W W W       WW",
"WWWWW W W W W W W W  WWW",
"WWW       W W W     WWWW",
"WWWW      W W W       WW",
"WWWWW     WWWWW      WWW",
"WWWWWW    W W W     WWWW",
"WW W      W W W    WWWWW",
"WW W      WWWWW      W W",
};
TextImages House_and_trees(House_and_trees_string_array, sizeof(House_and_trees_string_array)/sizeof(char*));

const char PROGMEM *Santa_in_his_sleigh_string_array[] PROGMEM = {
"                        ",
"                        ",
"  W   W  W              ",
"   W W    WW            ",
"   WWW   WWWW           ",
"  WWWWW  WWWW           ",
" WWWWWWW  WW            ",
" WWWWWWW WWWW           ",
" W   WWW WWWWW  W       ",
" W    WW WWWWWWW W      ",
" W     WWWWWWWW  W      ",
" W               W WW   ",
" W               W   W  ",
"  W             W     W ",
"   W            W     W ",
"    WWWWWWWWWWWW      W ",
"     W   W    W       W ",
"     W   W    W      W  ",
"   WWWWWWWWWWWWWWWWWW   ",
"                        ",
};
TextImages Santa_in_his_sleigh(Santa_in_his_sleigh_string_array, sizeof(Santa_in_his_sleigh_string_array)/sizeof(char*));

const char PROGMEM *Star_string_array[] PROGMEM = {
"                        ",
"                        ",
"           W            ",
"           W            ",
"          WWW           ",
"          WWW           ",
"         WWWWW          ",
"    WWWWWWWWWWWWWWW     ",
"     WWWWWWWWWWWWW      ",
"       WWWWWWWWW        ",
"        WWWWWWW         ",
"       WWWWWWWWW        ",
"       WWWW WWWW        ",
"      WWWW   WWWW       ",
"      WW       WW       ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages Star(Star_string_array, sizeof(Star_string_array)/sizeof(char*));

const char PROGMEM *ho_ho_santa_string_array[] PROGMEM = {
"                        ",
" W W          WWWWW     ",
" WWW        WW     WW   ",
" W W      WWW        W  ",
"         W            W ",
"  W      W            W ",
" W W    W    WWWWWWW  W ",
" W W   WW  WW       WW  ",
"  W   WWWWWW  W   W  W  ",
"      WWWW            W ",
" W W   WW W WWWWWWWWW W ",
" WWW      WWWWWWWWWWWWW ",
" W W      WWWW     WWWW ",
"          WWWWW   WWWWW ",
"  W       WWWWWWWWWWWWW ",
" W W       WWWWWWWWWWW  ",
" W W       WWWWWWWWWWW  ",
"  W         WWWWWWWWW   ",
"              WWWWWW    ",
"                        ",
};
TextImages ho_ho_santa(ho_ho_santa_string_array, sizeof(ho_ho_santa_string_array)/sizeof(char*));

const char PROGMEM *present_string_array[] PROGMEM = {
"                        ",
"       W        W       ",
"      W WW    WW W      ",
"      W   W  W   W      ",
"       WWWWWWWWWW       ",
"         WWWWWW         ",
"     WWWWWWWWWWWWWW     ",
"     W  WW WW WW  W     ",
"     W  WW WW WW  W     ",
"     W  W  WW  W  W     ",
"     W     WW     W     ",
"     WWWWWWWWWWWWWW     ",
"     WWWWWWWWWWWWWW     ",
"     W     WW     W     ",
"     W     WW     W     ",
"     W     WW     W     ",
"     W     WW     W     ",
"     W     WW     W     ",
"     WWWWWWWWWWWWWW     ",
"                        ",
};
TextImages present(present_string_array, sizeof(present_string_array)/sizeof(char*));

const char PROGMEM *happy_new_year_string_array[] PROGMEM = {
"W   W  WW  WWW WWW W   W",
"W   W W  W W W W W  W W ",
"WWWWW WWWW WWW WWW   W  ",
"W   W W  W W   W     W  ",
"W   W W  W W   W     W  ",
"                        ",
"                        ",
"   W   W WWWW W     W   ",
"   WW  W W    W     W   ",
"   W W W WWW   W W W    ",
"   W  WW W     W W W    ",
"   W   W WWWW   W W     ",
"                        ",
"                        ",
"  W   W WWWW  WW  WWW   ",
"   W W  W    W  W W  W  ",
"    W   WWW  WWWW WWW   ",
"    W   W    W  W W  W  ",
"    W   WWWW W  W W  W  ",
"                        ",
};
TextImages happy_new_year(happy_new_year_string_array, sizeof(happy_new_year_string_array)/sizeof(char*));

const char PROGMEM *angel_string_array[] PROGMEM = {
"          WWWW          ",
"         W    W         ",
"          WWWW          ",
"                        ",
"           WW           ",
"          W  W          ",
"  WWW    W    W    WWW  ",
" W   W   W    W   W   W ",
" W    W   W  W   W    W ",
"W      W   WW   W      W",
"W       WWW  WWW       W",
"W       W W  W W       W",
"W      W  W  W  W      W",
"W     W  W    W  W     W",
"W    W   W    W   W    W",
"W   W    W    W    W   W",
"W   W   WWWWWWWW   W   W",
" W W      W  W      W W ",
"  W       W  W       W  ",
"         WW  WW         ",
};
TextImages angel(angel_string_array, sizeof(angel_string_array)/sizeof(char*));

const char PROGMEM *gingerbread_man3_string_array[] PROGMEM = {
"         WWWWWW         ",
"        W      W        ",
"       W        W       ",
"       W  W  W  W       ",
"       W        W       ",
"        W W  W W        ",
"        W  WW  W        ",
"    WWWW        WWWW    ",
"   W                W   ",
"   W       WW       W   ",
"    WWW    WW    WWW    ",
"       W        W       ",
"       W   WW   W       ",
"       W   WW   W       ",
"      W          W      ",
"     W            W     ",
"    W     WWWW     W    ",
"   W    WW    WW    W   ",
"   W  WW        WW  W   ",
"    WW            WW    ",
};
TextImages gingerbread_man3(gingerbread_man3_string_array, sizeof(gingerbread_man3_string_array)/sizeof(char*));

const char PROGMEM *rudolph_string_array[] PROGMEM = {
"      W W               ",
"    WWWWWW              ",
"      W W               ",
"      W W               ",
"   WWWWW                ",
"  W     W               ",
"WW  W    WWWWWW         ",
"WW             WWWW     ",
"  WWWW             WWWW ",
"      W            W   W",
"      W            W   W",
"      W            W    ",
"     W            W     ",
"    W  WWWWWWWWWWW      ",
"    W W      W   W      ",
"    W W      W   W      ",
"   W  W      W   W      ",
"   W W      W   W       ",
"   W W      W   W       ",
"  W  W      W   W       ",
};
TextImages rudolph(rudolph_string_array, sizeof(rudolph_string_array)/sizeof(char*));

const char PROGMEM *rudolph2_string_array[] PROGMEM = {
"   W    W               ",
"  WW   W                ",
" W W  WWW               ",
"   WWWW                 ",
"    WW                  ",
"   W  WWW               ",
"  W W    W              ",
"WW        WW           W",
"WW          WWWWWWWWWWWW",
"  WWWW                WW",
"      W               W ",
"       W              W ",
"       W              W ",
"   WWWWW              W ",
"  W   W       WWWWWW  W ",
" W   W  WWWWWWW  W W  W ",
"W  WW  W         W  W W ",
"W W  W W          W W  W",
" W   W  W         W  WW ",
"      WW           WWW  ",
};
TextImages rudolph2(rudolph2_string_array, sizeof(rudolph2_string_array)/sizeof(char*));

const char PROGMEM *ho_ho_santa_closed_mouth_string_array[] PROGMEM = {
"                        ",
" W W          WWWWW     ",
" WWW        WW     WW   ",
" W W      WWW        W  ",
"         W            W ",
"  W      W            W ",
" W W    W    WWWWWWW  W ",
" W W   WW  WW       WW  ",
"  W   WWWWWW  W   W  W  ",
"      WWWW            W ",
" W W   WW W WWWWWWWWW W ",
" WWW      WWWWWWWWWWWWW ",
" W W      WWWWW   WWWWW ",
"          WWWWWWWWWWWWW ",
"  W       WWWWWWWWWWWWW ",
" W W       WWWWWWWWWWW  ",
" W W       WWWWWWWWWWW  ",
"  W         WWWWWWWWW   ",
"              WWWWWW    ",
"                        ",
};
TextImages ho_ho_santa_closed_mouth(ho_ho_santa_closed_mouth_string_array, sizeof(ho_ho_santa_closed_mouth_string_array)/sizeof(char*));

const char PROGMEM *snowman2_string_array[] PROGMEM = {
"                        ",
"                        ",
"          WWWW          ",
"         W    W         ",
"        W      W        ",
"       W  W  W  W       ",
"       W        W       ",
"       W   WW   W       ",
"  W     W   WW W        ",
"   W   W W    W W    WW ",
"  WWWWW   WWWW   WWWWW  ",
" W   W            W  WW ",
"     W     WW     W  W  ",
"     W     WW     W     ",
"     W            W     ",
"     W     WW     W     ",
"      W    WW    W      ",
"       W        W       ",
"        WW    WW        ",
"          WWWW          ",
};
TextImages snowman2(snowman2_string_array, sizeof(snowman2_string_array)/sizeof(char*));

// converted from Bauble.txt
/*
const PROGMEM uint32_t bauble_array[BINARY20LINE_ELEMENTS] = {
0 , 6144 , 9216 , 6144 ,
15360 , 49920 , 114816 , 145472 ,
460320 , 311712 , 1048560 , 699056 ,
873808 , 1048560 , 460320 , 319968 ,
138304 , 66432 , 49920 , 15360 ,
};
Binary20Line bauble(bauble_array);

// converted from xmas tree inv 0.r
const PROGMEM uint32_t xmas_tree_inv_0_array[] = {
0 , 5120 , 8704 , 5120 ,
8704 , 16640 , 32896 , 16640 ,
32896 , 65600 , 131104 , 32896 ,
65600 , 131104 , 262160 , 524296 ,
5120 , 5120 , 32512 , 15872 ,
};
Binary20Line xmas_tree_inv_0(xmas_tree_inv_0_array);

// converted from xmas tree inv 1.r
const PROGMEM uint32_t xmas_tree_inv_1_array[] = {
2048 , 5120 , 8704 , 5120 ,
8704 , 17664 , 32896 , 17664 ,
32896 , 67648 , 147488 , 32896 ,
65600 , 147488 , 263248 , 655368 ,
5120 , 5120 , 32512 , 15872 ,
};
Binary20Line xmas_tree_inv_1(xmas_tree_inv_1_array);

// converted from xmas tree inv 2.r
const  PROGMEM uint32_t xmas_tree_inv_2_array[] = {
0 , 5120 , 8704 , 5120 ,
12800 , 16640 , 41088 , 16640 ,
32896 , 65856 , 147488 , 32896 ,
67904 , 131104 , 262160 , 524296 ,
5120 , 5120 , 32512 , 15872 ,
};
Binary20Line xmas_tree_inv_2(xmas_tree_inv_2_array);

// converted from xmas tree inv 4.r
const PROGMEM uint32_t xmas_tree_inv_4_array[] = {
2048 , 5120 , 8704 , 5120 ,
12800 , 17664 , 41088 , 17664 ,
32896 , 67904 , 147488 , 32896 ,
67904 , 147488 , 263248 , 655368 ,
5120 , 5120 , 32512 , 15872 ,
};
Binary20Line xmas_tree_inv_4(xmas_tree_inv_4_array);

// converted from Holly.r
const PROGMEM uint32_t Holly_array[] = {
0 , 0 , 0 , 0 ,
1966140 , 1966140 , 2081276 , 2081276 ,
508400 , 522224 , 522224 , 63360 ,
14208 , 49152 , 109568 , 129536 ,
56832 , 3072 , 0 , 0 ,
};
Binary20Line Holly(Holly_array);

// converted from House and trees.r
const PROGMEM uint32_t House_and_trees_array[] = {
2 , 2055 , 5148 , 8762 ,
16640 , 8421552 , 12653681 , 14811185 ,
12850195 , 15204363 , 16252943 , 14859943 ,
15739395 , 16427687 , 14690831 , 15739395 ,
16268807 , 16525839 , 13642271 , 13647365 ,
};
Binary20Line House_and_trees(House_and_trees_array);

// converted from Santa in his sleigh.r
const PROGMEM uint32_t Santa_in_his_sleigh_array[] = {
0 , 0 , 2244608 , 1323008 ,
1865728 , 4093952 , 8335360 , 8353792 ,
4684928 , 4423488 , 4324928 , 4194392 ,
4194372 , 2097282 , 1048706 , 1048322 ,
279042 , 279044 , 2097144 , 0 ,
};
Binary20Line Santa_in_his_sleigh(Santa_in_his_sleigh_array);

// converted from Star.r
const PROGMEM uint32_t Star_array[] = {
0 , 0 , 4096 , 4096 ,
14336 , 14336 , 31744 , 1048544 ,
524224 , 130816 , 65024 , 130816 ,
126720 , 247680 , 196992 , 0 ,
0 , 0 , 0 , 0 ,
};
Binary20Line Star(Star_array);

// converted from ho ho santa.g
const PROGMEM uint32_t ho_ho_santa_array[] = {
0 , 5243872 , 7343128 , 5257220 ,
16386 , 2113538 , 5277682 , 5347340 ,
2355748 , 245762 , 5353466 , 7356414 ,
5258270 , 15934 , 2113534 , 5251068 ,
5251068 , 2101240 , 1008 , 0 ,
};
Binary20Line ho_ho_santa(ho_ho_santa_array);

// converted from Present.r
const PROGMEM uint32_t present_array[] = {
0 , 65664 , 181056 , 140352 ,
130944 , 32256 , 524256 , 318240 ,
318240 , 301344 , 268320 , 524256 ,
524256 , 268320 , 268320 , 268320 ,
268320 , 268320 , 524256 , 0 ,
};
Binary20Line present(present_array);

// converted from happy new year.g
const PROGMEM uint32_t happy_new_year_array[] = {
9018833 , 9065802 , 16506308 , 9064708 ,
9064708 , 0 , 0 , 1145352 ,
1655304 , 1405264 , 1261904 , 1144992 ,
0 , 0 , 2290488 , 1344676 ,
583608 , 558244 , 586916 , 0 ,
};
Binary20Line happy_new_year(happy_new_year_array);

// converted from angel.r
const PROGMEM uint32_t angel_array[] = {
15360 , 16896 , 15360 , 0 ,
6144 , 9216 , 3686940 , 4473378 ,
4334658 , 8460417 , 8447745 , 8430849 ,
8463489 , 8536641 , 8667681 , 8929809 ,
8978193 , 5252106 , 2106372 , 26112 ,
};
Binary20Line angel(angel_array);

// converted from gingerbread man3.r
const PROGMEM uint32_t gingerbread_man3_array[] = {
32256 , 33024 , 65664 , 74880 ,
65664 , 42240 , 39168 , 983280 ,
1048584 , 1054728 , 923760 , 65664 ,
71808 , 71808 , 131136 , 262176 ,
539664 , 1098504 , 1245384 , 786480 ,
};
Binary20Line gingerbread_man3(gingerbread_man3_array);

// converted from rudolph.r
const PROGMEM uint32_t rudolph_array[] = {
163840 , 1032192 , 163840 , 163840 ,
2031616 , 2129920 , 13139456 , 12583392 ,
3932190 , 131089 , 131089 , 131088 ,
262176 , 655296 , 656448 , 656448 ,
1180736 , 1312896 , 1312896 , 2361472 ,
};
Binary20Line rudolph(rudolph_array);

// converted from rudolph2.r
const PROGMEM uint32_t rudolph2_array[] = {
1081344 , 3211264 , 5472256 , 1966080 ,
786432 , 1277952 , 2637824 , 12595201 ,
12587007 , 3932163 , 131074 , 65538 ,
65538 , 2031618 , 2229234 , 4521554 ,
10027082 , 10813481 , 4489254 , 196636 ,
};
Binary20Line rudolph2(rudolph2_array);

// converted from ho ho santa closed mouth.g
const PROGMEM uint32_t ho_ho_santa_closed_mouth_array[] = {
0 , 5243872 , 7343128 , 5257220 ,
16386 , 2113538 , 5277682 , 5347340 ,
2355748 , 245762 , 5353466 , 7356414 ,
5258814 , 16382 , 2113534 , 5251068 ,
5251068 , 2101240 , 1008 , 0 ,
};
Binary20Line ho_ho_santa_closed_mouth(ho_ho_santa_closed_mouth_array);

// converted from snowman2.r
const PROGMEM uint32_t snowman2_array[] = {
0 , 0 , 15360 , 16896 ,
33024 , 74880 , 65664 , 71808 ,
2133248 , 1131142 , 4078716 , 4456486 ,
268324 , 268320 , 262176 , 268320 ,
137280 , 65664 , 49920 , 15360 ,
};
Binary20Line snowman2(snowman2_array);
*/

struct display_list_t {
  ImageBase& image;
  //const PROGMEM uint32_t * const PROGMEM image;
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
    { end_of_list, 0 }
};

//##########################################################################
// halloween
//##########################################################################

const char PROGMEM *emily_ghost_string_array[] PROGMEM = {
"         WWWWWWWWW      ",
"        WWWWWWWWWWW     ",
"        WWWWWWWWWWW     ",
"       WWWWWWWWWWWWW    ",
"       WWWWWWWW  WWW    ",
"      WWW  WWWW  WWWW   ",
"      WWW  WWWWWWWWWW   ",
"     WWWWWWWWWWWWWWWW   ",
"     WWWWWW WW WWWWWW   ",
"     WWWWWW WW WWWWWWW  ",
"     WWWWWWW  WWWWWWWW  ",
"      WWWWWWWWWWWWWWWW  ",
"      WWWWWWWWWWWWWWWWW ",
"      WWWWWWWWWWWWWWWWW ",
"      WWWWWWWWWWWWWWWWWW",
"      WWWWWWWWWWWWWWWWWW",
"      WWWWWWWWWWWWWWWWWW",
"     WWWWWWWWWWWWWWWWWWW",
"     WWW  WWWW  WW  WW W",
"     WW    WW   WW   W  ",
};
TextImages emily_ghost(emily_ghost_string_array, sizeof(emily_ghost_string_array)/sizeof(char*));

const char PROGMEM *emily_pumpkin_string_array[] PROGMEM = {
"         WWW            ",
"         W W  WWWWWWW   ",
"       WWWWWWW       W  ",
"     WW               W ",
"    W                  W",
"   W          W        W",
"  W           W        W",
" W           WWW       W",
" W   W                 W",
" W   W                 W",
"W   WWW   W      W     W",
"W        WWW    WW    W ",
"W              WW    W  ",
"W      W     WWW    W   ",
" W     WWWWWWWW     W   ",
"  W     WWWWWW     W    ",
"   W              W     ",
"    W            W      ",
"     W         WW       ",
"      WWWWWWWWW         ",
};
TextImages emily_pumpkin(emily_pumpkin_string_array, sizeof(emily_pumpkin_string_array)/sizeof(char*));

const char PROGMEM *emily_spider_string_array[] PROGMEM = {
"                        ",
"                      WW",
"                    WW  ",
"                   W    ",
"                  W     ",
"         WWWWWW  W      ",
"       WWWWWWWWWW    WWW",
"WWWWWWWWWW WW WWWWWWW   ",
"       WWWWWWWWWW       ",
"    WWWWWWWWWWWWW       ",
"WWWW   WWW WW WWWWWWWWWW",
"      WWWW WW WWW       ",
"   WWW WWWW  WWWW       ",
"WWW     WWWWWWWW WWWWWWW",
"      WW                ",
"    WW                  ",
"WWWW                    ",
"                        ",
"                        ",
"                        ",
};
TextImages emily_spider(emily_spider_string_array, sizeof(emily_spider_string_array)/sizeof(char*));

const char PROGMEM *skull_string_array[] PROGMEM = {
"      WWWWWWWWWWWWW     ",
"     WWWWWWWWWWWWWWW    ",
"    WWWWWWWWWWWWWWWWW   ",
"    WWWWWWWWWWWWWWWWW   ",
"    WWWW WWWWWWW WWWW   ",
"    WWW   WWWWW   WWW   ",
"    WW     WWW     WW   ",
"    WWW   WWWWW   WWW   ",
"    WWWW WWWWWWW WWWW   ",
"    WWWWWWW W WWWWWWW   ",
"     WWWWWW W WWWWWW    ",
"     WWWWWW W WWWWWW    ",
"      WWWWWWWWWWWWW     ",
"       WWWWWWWWWWW      ",
"       W WWWWWWW W      ",
"       W W W W W W      ",
"       W W W W W W      ",
"       WWW W W WWW      ",
"       WWWWWWWWWWW      ",
"        WWWWWWWWW       ",
};
TextImages skull(skull_string_array, sizeof(skull_string_array)/sizeof(char*));

const char PROGMEM *pumpkin_eyes_1a_string_array[] PROGMEM = {
"  WW                  WW",
"  WWW                WWW",
"  WWWWW            WWWWW",
"  WWWWWWWW      WWWWWWWW",
"  WWWWW  WW    WWW  WWWW",
"  WWWWW  WW    WWW  WWWW",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages pumpkin_eyes_1a(pumpkin_eyes_1a_string_array, sizeof(pumpkin_eyes_1a_string_array)/sizeof(char*));

const char PROGMEM *pumpkin_eyes_1b_string_array[] PROGMEM = {
"  WW                  WW",
"  WWW                WWW",
"  WWWWW            WWWWW",
"  WWWWWWWW      WWWWWWWW",
"  WWWW  WWW    WW  WWWWW",
"  WWWW  WWW    WW  WWWWW",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages pumpkin_eyes_1b(pumpkin_eyes_1b_string_array, sizeof(pumpkin_eyes_1b_string_array)/sizeof(char*));

const char PROGMEM *pumpkin_eyes_1c_string_array[] PROGMEM = {
"  WW                  WW",
"  WWW                WWW",
"  WWWWW            WWWWW",
"  WWWWWWWW      WWWWWWWW",
"  WWW  WWWW    W  WWWWWW",
"  WWW  WWWW    W  WWWWWW",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages pumpkin_eyes_1c(pumpkin_eyes_1c_string_array, sizeof(pumpkin_eyes_1c_string_array)/sizeof(char*));

const char PROGMEM *pumpkin_eyes_1d_string_array[] PROGMEM = {
"  WW                  WW",
"  WWW                WWW",
"  WWWWW            WWWWW",
"  WWWWWWWW      WWWWWWWW",
"  WWWWWW  W    WWWW  WWW",
"  WWWWWW  W    WWWW  WWW",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages pumpkin_eyes_1d(pumpkin_eyes_1d_string_array, sizeof(pumpkin_eyes_1d_string_array)/sizeof(char*));

const char PROGMEM *face1_string_array[] PROGMEM = {
"          WWWWWW        ",
"         WWWWWWWW       ",
"     WW WWWWWWWWWW WW   ",
"       WWWWWWWWWWWW     ",
"        WWWWWWWWWW      ",
"        WWWWWWWWWW      ",
"       WW   WW   WW     ",
"      WW W W  W W WW    ",
"     W WW   WW   WW W   ",
"     W  WWWW WWWWW  W   ",
"         WW WWWWW       ",
"        WWW WW WWW      ",
"       WW WW  WW WW     ",
"      W WW  WW  WW W    ",
"     W   WW    WW   W   ",
"          WWWWWW        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages face1(face1_string_array, sizeof(face1_string_array)/sizeof(char*));

const char PROGMEM *face2_string_array[] PROGMEM = {
"          WWWWWW        ",
"         WWWWWWWW       ",
"        WWWWWWWWWW      ",
"     WWWWWWWWWWWWWWWW   ",
"        WWWWWWWWWW      ",
"        WWWWWWWWWW      ",
"       WW   WW   WW     ",
"       W W W  W W W     ",
"      WWW   WW   WWW    ",
"     W  WWWW WWWWW  W   ",
"     W   WW WWWWW   W   ",
"        WWW WW WWW      ",
"       WW  W  W  WW     ",
"      W WW      WW W    ",
"      W  WW    WW  W    ",
"          WWWWWW        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages face2(face2_string_array, sizeof(face2_string_array)/sizeof(char*));

const char PROGMEM *face3_string_array[] PROGMEM = {
"          WWWWWW        ",
"         WWWWWWWW       ",
"        WWWWWWWWWW      ",
"     WW WWWWWWWWWW WW   ",
"       WWWWWWWWWWWW     ",
"        WWWWWWWWWW      ",
"       WW   WW   WW     ",
"       W W W  W W W     ",
"       WW    W   WW     ",
"      W WWW WWWWWW W    ",
"     W   WW WW WW   W   ",
"     W  WW W  W WW  W   ",
"        W  W  W  W      ",
"        WW      WW      ",
"       W WW    WW W     ",
"     WW   WWWWWW   WW   ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages face3(face3_string_array, sizeof(face3_string_array)/sizeof(char*));

const char PROGMEM *face4_string_array[] PROGMEM = {
"          WWWWWW        ",
"     W   WWWWWWWW   W   ",
"      W WWWWWWWWWW W    ",
"       WWWWWWWWWWWW     ",
"        WWWWWWWWWW      ",
"        W   WW   W      ",
"       W WWW  WWW W     ",
"       W W W  W W W     ",
"      WWW   WW   WWW    ",
"      W WWWW WWWWW W    ",
"     W   WW WWWWW   W   ",
"        WWW WW WWW      ",
"        W  W  W  W      ",
"      WWWW      WWWW    ",
"     W   WW    WW   W   ",
"          WWWWWW        ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages face4(face4_string_array, sizeof(face4_string_array)/sizeof(char*));

const char PROGMEM *jsw_demon1_string_array[] PROGMEM = {
"                        ",
"                        ",
"                        ",
"                        ",
"              W       W ",
"          W   W       W ",
"        W W W  W W W W  ",
"        W W W   WW WW   ",
"         WWW   W WW  W  ",
"          W   W WW WW W ",
"          WWW WW  W WWW ",
"          W  WW W W W  W",
"          W  W  WWWW   W",
"          W     WW WW WW",
"          W     W  WW   ",
"               W W W W  ",
"                        ",
"                        ",
"                        ",
"                        ",
};
TextImages jsw_demon1(jsw_demon1_string_array, sizeof(jsw_demon1_string_array)/sizeof(char*));

const char PROGMEM *jsw_demon2_string_array[] PROGMEM = {
"                        ",
"                        ",
"                        ",
"                        ",
"              W       W ",
"              W       W ",
"               W WWW W  ",
"                WW WW   ",
"         W W   W W W W  ",
"         W W  W WWW W W ",
"          W   WW W WWWW ",
"        WWW  W WW W W  W",
"           WWWW WWWW   W",
"           WW   WW WW  W",
"            W   W  WW  W",
"            W  W W W W  ",
"             WW W   W W ",
"             W          ",
"                        ",
"                        ",
};
TextImages jsw_demon2(jsw_demon2_string_array, sizeof(jsw_demon2_string_array)/sizeof(char*));

const char PROGMEM *jsw_demon3_string_array[] PROGMEM = {
"                        ",
"                        ",
"                        ",
"                        ",
"              W       W ",
"              WWW   WWW ",
"                WWWW    ",
"               W W W W  ",
"              W WW WW W ",
"              WWW WW WW ",
"           W   WW   W WW",
"         W  W W W W W  W",
"          W W WWWW WW WW",
"        W  W WW  WWWW   ",
"         WW WW  WW W    ",
"             W W W W W  ",
"              W W   W W ",
"               W        ",
"                W       ",
"                 W      ",
};
TextImages jsw_demon3(jsw_demon3_string_array, sizeof(jsw_demon3_string_array)/sizeof(char*));

/*
// converted from emily_ghost.g
const PROGMEM uint32_t emily_ghost_array[] = {
32704 , 65504 , 65504 , 131056 ,
130672 , 237176 , 237560 , 524280 ,
519672 , 519676 , 521212 , 262140 ,
262142 , 262142 , 262143 , 262143 ,
262143 , 524287 , 474317 , 399556 ,
};
Binary20Line emily_ghost(emily_ghost_array);

// converted from emily_pumpkin.g
const PROGMEM uint32_t emily_pumpkin_array[] = {
28672 , 21496 , 130052 , 393218 ,
524289 , 1049089 , 2097665 , 4196097 ,
4456449 , 4456449 , 9314369 , 8417474 ,
8388996 , 8455944 , 4324872 , 2161680 ,
1048608 , 524352 , 262528 , 261632 ,
};
Binary20Line emily_pumpkin(emily_pumpkin_array);

// converted from emily_spider.g
const PROGMEM uint32_t emily_spider_array[] = {
0 , 3 , 12 , 16 ,
32 , 32320 , 130951 , 16767992 ,
130944 , 1048448 , 15850495 , 252800 ,
1959808 , 14745471 , 196608 , 786432 ,
15728640 , 0 , 0 , 0 ,
};
Binary20Line emily_spider(emily_spider_array);

// converted from skull.g
const PROGMEM uint32_t skull_array[] = {
262112 , 524272 , 1048568 , 1048568 ,
1015672 , 933432 , 793624 , 933432 ,
1015672 , 1043448 , 519152 , 519152 ,
262112 , 131008 , 98112 , 87360 ,
87360 , 120256 , 131008 , 65408 ,
};
Binary20Line skull(skull_array);

// converted from pumpkin_eyes_1a.g
const PROGMEM uint32_t pumpkin_eyes_1a_array[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
4088271 , 4088271 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};
Binary20Line pumpkin_eyes_1a(pumpkin_eyes_1a_array);

// converted from pumpkin_eyes_1b.g
const PROGMEM uint32_t pumpkin_eyes_1b_array[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
3989919 , 3989919 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};
Binary20Line pumpkin_eyes_1b(pumpkin_eyes_1b_array);

// converted from pumpkin_eyes_1c.g
const PROGMEM uint32_t pumpkin_eyes_1c_array[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
3793215 , 3793215 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};
Binary20Line pumpkin_eyes_1c(pumpkin_eyes_1c_array);

// converted from pumpkin_eyes_1d.g
const PROGMEM uint32_t pumpkin_eyes_1d_array[] = {
3145731 , 3670023 , 4063263 , 4178175 ,
4137447 , 4137447 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 ,
};
Binary20Line pumpkin_eyes_1d(pumpkin_eyes_1d_array);

// converted from face1.txt
const PROGMEM uint32_t face1_array[] = {
16128 , 32640 , 458712 , 131040 ,
65472 , 65472 , 101472 , 217776 ,
363624 , 325576 , 28544 , 60864 ,
111456 , 183504 , 287112 , 16128 ,
0 , 0 , 0 , 0 ,
};
Binary20Line face1(face1_array);

// converted from face2.txt
const PROGMEM uint32_t face2_array[] = {
16128 , 32640 , 65472 , 524280 ,
65472 , 65472 , 101472 , 86688 ,
232560 , 325576 , 290696 , 60864 ,
103008 , 180432 , 156048 , 16128 ,
0 , 0 , 0 , 0 ,
};
Binary20Line face2(face2_array);

// converted from face3.txt
const uint32_t face3_array[] = {
16128 , 32640 , 65472 , 458712 ,
131040 , 65472 , 101472 , 86688 ,
99424 , 192464 , 290184 , 316104 ,
37440 , 49344 , 90528 , 409368 ,
0 , 0 , 0 , 0 ,
};
Binary20Line face3(face3_array);

// converted from face4.txt
const uint32_t face4_array[] = {
16128 , 294792 , 196560 , 131040 ,
65472 , 35904 , 95136 , 86688 ,
232560 , 194512 , 290696 , 60864 ,
37440 , 246000 , 287112 , 16128 ,
0 , 0 , 0 , 0 ,
};
Binary20Line face4(face4_array);

// converted from jsw_demon1.txt
const PROGMEM uint32_t jsw_demon1_array[] = {
0 , 0 , 0 , 0 ,
514 , 8706 , 43348 , 43224 ,
29028 , 8922 , 15150 , 9897 ,
9457 , 8411 , 8344 , 340 ,
0 , 0 , 0 , 0 ,
};
Binary20Line jsw_demon1(jsw_demon1_array);

// converted from jsw_demon2.txt
const PROGMEM uint32_t jsw_demon2_array[] = {
0 , 0 , 0 , 0 ,
514 , 514 , 372 , 216 ,
20820 , 21226 , 9054 , 58793 ,
7921 , 6361 , 2201 , 2388 ,
1674 , 1024 , 0 , 0 ,
};
Binary20Line jsw_demon2(jsw_demon2_array);

// converted from jsw_demon3.txt
const PROGMEM uint32_t jsw_demon3_array[] = {
0 , 0 , 0 , 0 ,
514 , 910 , 240 , 340 ,
730 , 950 , 4491 , 19113 ,
11227 , 38520 , 27856 , 1364 ,
650 , 256 , 128 , 64 ,
};
Binary20Line jsw_demon3(jsw_demon3_array);
*/

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
    { end_of_list, 0 }
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
  pointer->image.display();
  display_timer.set(pointer->wait_time*100);

  // next image?
  pointer++;
  if(pointer->image.last())
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
      /*if (millis() > connTime + 100000) {
          Serial.println("time to disconnect");
          BLE.disconnect();
      } */
  }
}
