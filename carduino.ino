#include <LiquidCrystal.h>
#include <FastLED.h>
#include <menu.h>
#include <string.h>

// LED
#define LED_PIN     2
#define NUM_LEDS    300 // Change this to the number of LEDs you have
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
CRGB primary;
CRGB secondary;

// LCD
const int rs=3,
          en=4,
          d4=6,
          d5=7,
          d6=8,
          d7=9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Button
const int btn1=10,
          btn2=11,
          btn3=12;
int btn1_state, btn2_state, btn3_state;
unsigned long current_time, btn1_last_pressed, btn2_last_pressed, btn3_last_pressed;
#define BUTTON_THRESHOLD 200

// Menuing
#define MENU_WINDOW 8
#define MENU_SIZE 3
#define COLOR_SIZE 9
#define COLOR_STATE_SIZE 3
#define PATTERN_SIZE 7
Menu *menu;
const char *status[MENU_SIZE] = {
  "Colors",
  "Patterns",
  "On/Off"
};
const char *color_strings[COLOR_SIZE] = {
  "Red",
  "Pink",
  "Orange",
  "Yellow",
  "Green",
  "Blue",
  "Purple",
  "White"
};
const char *color_states[COLOR_STATE_SIZE] = {
  "Primary",
  "Second"
};
const char *pattern_strings[PATTERN_SIZE] = {
  "Solid",
  "Flash",
  "Strobe",
  "Pulse",
  "Alt.",
  "Cart"
};
char *back = "Back";
char *window;
char state;

void showButton(char btn);
char get_menu_size(char icon);
void populate_menu(Menu *menu, char icon);
void populate_menu_main(Menu *menu);
void populate_menu_color_state(Menu *menu);
void populate_menu_color(Menu *menu);
void populate_menu_pattern(Menu *menu);
void change_menu(char icon, char *desc);
void menu_draw(Menu *menu);
void color_set(char icon, char *color);


/*
      ----- MENUS -----
*/

char get_menu_size(char icon){
  switch(icon){
    case 'C':
      return COLOR_SIZE;
    case 'c':
      return COLOR_STATE_SIZE;
    case 'p':
      return PATTERN_SIZE;
    case 'm':
    case 'b':
      return MENU_SIZE;
    default:
      return 0;
  }
}

void populate_menu(Menu *menu, char icon){
  switch(icon){
    case 'c':
      populate_menu_color_state(menu);
      break;
    case 'C':
      populate_menu_color(menu);
      break;
    case 'p':
      populate_menu_pattern(menu);
      break;
    case 'm':
    case 'b':
      populate_menu_main(menu);
    default:
      return;
  }
}

void change_menu(char icon, char *desc){
  char size;
  Serial.print("Changing menu to ");
  Serial.println(desc);
  size = get_menu_size(icon);
  menu_free(menu, 1);
  menu = menu_create(8,0,MENU_WINDOW, size);
  populate_menu(menu, icon);
  menu_draw(menu);
}

void populate_menu_main(Menu *menu){
  menu_register(menu, 0, 'c', status[0], change_menu);
  menu_register(menu, 1, 'p', status[1], change_menu);
  menu_register(menu, 2, 'o', status[2], 0x0);
}

void populate_menu_color_state(Menu *menu){
  menu_register(menu, 0, 'b', back, change_menu);
  menu_register(menu, 1, 'p', color_states[0], color_set_state);
  menu_register(menu, 2, 's', color_states[1], color_set_state);
}

void populate_menu_color(Menu *menu){
  menu_register(menu, 0, 'b', back, change_menu);
  menu_register(menu, 1, 'r', color_strings[0], color_set);
  menu_register(menu, 2, 'p', color_strings[1], color_set);
  menu_register(menu, 3, 'o', color_strings[2], color_set);
  menu_register(menu, 4, 'y', color_strings[3], color_set);
  menu_register(menu, 5, 'g', color_strings[4], color_set);
  menu_register(menu, 6, 'b', color_strings[5], color_set);
  menu_register(menu, 7, 'p', color_strings[6], color_set);
  menu_register(menu, 8, 'w', color_strings[7], color_set);
}

void populate_menu_pattern(Menu *menu){
  menu_register(menu, 0, 'b', back, change_menu);
  menu_register(menu, 1, 's', pattern_strings[0], 0x0);
  menu_register(menu, 2, 'f', pattern_strings[1], 0x0);
  menu_register(menu, 3, 's', pattern_strings[2], 0x0);
  menu_register(menu, 4, 'p', pattern_strings[3], 0x0);
  menu_register(menu, 5, 'a', pattern_strings[4], 0x0);
  menu_register(menu, 6, 'c', pattern_strings[5], 0x0);
}

void menu_draw(Menu *menu){
  window = menu_window(menu);
  lcd.clear();
  lcd.setCursor(menu->x, menu->y);
  lcd.print(window);
  lcd.setCursor(menu->x, menu->y+1);
  lcd.print(menu_message(menu));
  free(window);
}

/*
      ----- COLORS -----
*/

void color_set_state(char icon, char *desc){
  state = icon;
  change_menu('C', desc);
}

void color_set(char icon, char *color_string){
  CRGB color;
  if(!strcmp(color_string, "Red"))
    color = CRGB::Red;
  else if(!strcmp(color_string, "Blue"))
    color = CRGB::Blue;
  else if(!strcmp(color_string, "Green"))
    color = CRGB::Green;
  else if(!strcmp(color_string, "Yellow"))
    color = CRGB::Yellow;
  else if(!strcmp(color_string, "Pink"))
    color = CRGB::Pink;
  else if(!strcmp(color_string, "Orange"))
    color = CRGB::Orange;
  else if(!strcmp(color_string, "Purple"))
    color = CRGB::Purple;
  else if(!strcmp(color_string, "White"))
    color = CRGB::White;
  else
    color = CRGB::Black;

  Serial.print("Set ");
  if(state=='p'){
    primary = color;
    Serial.print("primary to ");
  }
  else if(state=='s'){
    secondary = color;
    Serial.print("secondary to ");
  }
  Serial.println(color_string);
  state=0;
  change_menu('m', 'Main');
}

/*
      ----- CONTROL -----
*/

void setup() {
  delay(3000);
  state=0;

  Serial.begin(9600);
  Serial.print("STARTING...\n");

  // LED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  // LCD
  lcd.begin(16, 2);
  lcd.setCursor(1,1);
  lcd.print("Starting...");

  // Button
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);

  // Menu
  change_menu('m', "Main");
}

void loop() {
  char modified;
  
  modified = 0;
  btn1_state = digitalRead(btn1);
  btn2_state = digitalRead(btn2);
  btn3_state = digitalRead(btn3);
  current_time = millis();

  if(btn1_state == HIGH && current_time - btn1_last_pressed > BUTTON_THRESHOLD){
    btn1_last_pressed = current_time;
    menu_select(menu, menu->cursor-1);
    modified = 1;
  }
  if(btn2_state == HIGH && current_time - btn2_last_pressed > BUTTON_THRESHOLD){
    btn2_last_pressed = current_time;
    menu_select(menu, menu->cursor+1);
    modified = 1;
  }
  if(btn3_state == HIGH && current_time - btn3_last_pressed > BUTTON_THRESHOLD){
    btn3_last_pressed = current_time;
    menu_activate(menu);
  }
  if(modified){
    menu_draw(menu);
  }
}
