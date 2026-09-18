#include <LiquidCrystal.h>
#include <pico/stdlib.h>

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that it's in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5,
                             uint8_t d6, uint8_t d7) {
  init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6,
                             uint8_t d7) {
  init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
  init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
  init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

void LiquidCrystal::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4,
                         uint8_t d5, uint8_t d6, uint8_t d7) {
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;

  _data_pins.at(0) = d0;
  _data_pins.at(1) = d1;
  _data_pins.at(2) = d2;
  _data_pins.at(3) = d3;
  _data_pins.at(4) = d4;
  _data_pins.at(5) = d5;
  _data_pins.at(6) = d6;
  _data_pins.at(7) = d7;

  if (fourbitmode) {
    _displayfunction = LCD_4BIT_MODE | LCD_1_LINE | LCD_5x8_DOTS;
  } else {
    _displayfunction = LCD_8BIT_MODE | LCD_1_LINE | LCD_5x8_DOTS;
  }

  begin(16, 1);
}

void LiquidCrystal::begin(uint8_t cols, uint8_t rows, uint8_t charsize) {
  if (rows > 1) {
    _displayfunction |= LCD_2_LINE;
  }
  _numlines = rows;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  // for some 1 line displays you can select a 10 pixel high font
  if ((charsize != LCD_5x8_DOTS) && (rows == 1)) {
    _displayfunction |= LCD_5x10_DOTS;
  }

  gpio_init(_rs_pin);
  gpio_set_dir(_rs_pin, GPIO_OUT);
  // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
  if (_rw_pin != 255) {
    gpio_init(_rw_pin);
    gpio_set_dir(_rw_pin, GPIO_OUT);
  }
  gpio_init(_enable_pin);
  gpio_set_dir(_enable_pin, GPIO_OUT);

  // Do these once, instead of every time a character is drawn for speed reasons.
  for (int i = 0; i < ((_displayfunction & LCD_8BIT_MODE) ? 8 : 4); ++i) {
    gpio_init(_data_pins.at(i));
    gpio_set_dir(_data_pins.at(i), GPIO_OUT);
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40 ms after power rises above 2.7 V
  // before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
  sleep_us(50000);
  // Now we pull both RS and R/W low to begin commands
  gpio_put(_rs_pin, false);
  gpio_put(_enable_pin, false);
  if (_rw_pin != 255) {
    gpio_put(_rw_pin, false);
  }

  // put the LCD into 4 bit or 8 bit mode
  if (!(_displayfunction & LCD_8BIT_MODE)) {
    // this is according to the Hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    sleep_us(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    sleep_us(4500); // wait min 4.1ms

    // third go!
    write4bits(0x03);
    sleep_us(150);

    // finally, set to 4-bit interface
    write4bits(0x02);
  } else {
    // this is according to the Hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTION_SET | _displayfunction);
    sleep_us(4500); // wait more than 4.1ms

    // second try
    command(LCD_FUNCTION_SET | _displayfunction);
    sleep_us(150);

    // third go
    command(LCD_FUNCTION_SET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTION_SET | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT;
  // set the entry mode
  command(LCD_ENTRY_MODE_SET | _displaymode);
}

void LiquidCrystal::setRowOffsets(uint8_t row0, uint8_t row1, uint8_t row2, uint8_t row3) {
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void LiquidCrystal::clear() {
  command(LCD_CLEAR_DISPLAY); // clear display, set cursor position to zero
  sleep_us(2000);             // this command takes a long time!
}

void LiquidCrystal::home() {
  command(LCD_RETURN_HOME); // set cursor position to zero
  sleep_us(2000);           // this command takes a long time!
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row) {
  if (const size_t max_lines = std::size(_row_offsets); row >= max_lines) {
    row = static_cast<uint8_t>(max_lines) - 1; // we count rows starting w/0
  }
  if (row >= _numlines) {
    row = _numlines - 1; // we count rows starting w/0
  }

  command(LCD_SET_DDRAM_ADDR | (col + _row_offsets.at(row)));
}

// Turn the display on/off (quickly)
void LiquidCrystal::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAY_ON;
  command(LCD_DISPLAY_CONTROL | _displaycontrol);
}
void LiquidCrystal::display() {
  _displaycontrol |= LCD_DISPLAY_ON;
  command(LCD_DISPLAY_CONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::noCursor() {
  _displaycontrol &= ~LCD_CURSOR_ON;
  command(LCD_DISPLAY_CONTROL | _displaycontrol);
}
void LiquidCrystal::cursor() {
  _displaycontrol |= LCD_CURSOR_ON;
  command(LCD_DISPLAY_CONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::noBlink() {
  _displaycontrol &= ~LCD_BLINK_ON;
  command(LCD_DISPLAY_CONTROL | _displaycontrol);
}
void LiquidCrystal::blink() {
  _displaycontrol |= LCD_BLINK_ON;
  command(LCD_DISPLAY_CONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft() { command(LCD_CURSOR_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_LEFT); }
void LiquidCrystal::scrollDisplayRight() { command(LCD_CURSOR_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_RIGHT); }

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight() {
  _displaymode |= LCD_ENTRY_LEFT;
  command(LCD_ENTRY_MODE_SET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft() {
  _displaymode &= ~LCD_ENTRY_LEFT;
  command(LCD_ENTRY_MODE_SET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal::autoScroll() {
  _displaymode |= LCD_ENTRY_SHIFT_INCREMENT;
  command(LCD_ENTRY_MODE_SET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal::noAutoScroll() {
  _displaymode &= ~LCD_ENTRY_SHIFT_INCREMENT;
  command(LCD_ENTRY_MODE_SET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar(uint8_t location, std::span<uint8_t, 8> charmap) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SET_CGRAM_ADDR | static_cast<uint8_t>(location << 3));
  for (unsigned char i : charmap) {
    write(i);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal::command(uint8_t value) { send(value, false); }

inline size_t LiquidCrystal::write(uint8_t value) {
  send(value, true);
  return 1; // assume success
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystal::send(uint8_t value, uint8_t mode) {
  gpio_put(_rs_pin, mode);

  // if there is a RW pin indicated, set it low to Write
  if (_rw_pin != 255) {
    gpio_put(_rw_pin, false);
  }

  if (_displayfunction & LCD_8BIT_MODE) {
    write8bits(value);
  } else {
    write4bits(value >> 4);
    write4bits(value);
  }
}

void LiquidCrystal::pulseEnable() const {
  gpio_put(_enable_pin, false);
  sleep_us(1);
  gpio_put(_enable_pin, true);
  sleep_us(1); // enable pulse must be >450 ns
  gpio_put(_enable_pin, false);
  sleep_us(100); // commands need > 37 us to settle
}

void LiquidCrystal::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    gpio_put(_data_pins.at(i), (value >> i) & 0x01);
  }

  pulseEnable();
}

void LiquidCrystal::write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    gpio_put(_data_pins.at(i), (value >> i) & 0x01);
  }

  pulseEnable();
}
