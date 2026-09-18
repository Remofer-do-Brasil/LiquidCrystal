#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include <lib/Print.h>
#include <span>

// commands
#define LCD_CLEAR_DISPLAY   0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE_SET  0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT    0x10
#define LCD_FUNCTION_SET    0x20
#define LCD_SET_CGRAM_ADDR  0x40
#define LCD_SET_DDRAM_ADDR  0x80

// flags for display entry mode
#define LCD_ENTRY_RIGHT           0x00
#define LCD_ENTRY_LEFT            0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSOR_ON  0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON   0x01
#define LCD_BLINK_OFF  0x00

// flags for display/cursor shift
#define LCD_DISPLAY_MOVE 0x08
#define LCD_CURSOR_MOVE  0x00
#define LCD_MOVE_RIGHT   0x04
#define LCD_MOVE_LEFT    0x00

// flags for function set
#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2_LINE    0x08
#define LCD_1_LINE    0x00
#define LCD_5x10_DOTS 0x04
#define LCD_5x8_DOTS  0x00

class LiquidCrystal : public rm_sg01::Print {
public:
  LiquidCrystal(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6,
                uint8_t d7);
  LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  LiquidCrystal(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  ~LiquidCrystal() override = default;

  void init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5,
            uint8_t d6, uint8_t d7);

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8_DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoScroll();
  void noAutoScroll();

  void setRowOffsets(uint8_t row0, uint8_t row1, uint8_t row2, uint8_t row3);
  void createChar(uint8_t, std::span<uint8_t, 8>);
  void setCursor(uint8_t col, uint8_t row);
  size_t write(uint8_t value) override;
  void command(uint8_t);

  using rm_sg01::Print::write;

private:
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void write8bits(uint8_t);
  void pulseEnable() const;

  uint8_t _rs_pin;     // LOW: command.  HIGH: character.
  uint8_t _rw_pin;     // LOW: write to LCD.  HIGH: read from LCD.
  uint8_t _enable_pin; // activated by a HIGH pulse.
  std::array<uint8_t, 8> _data_pins;

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _numlines;
  std::array<uint8_t, 4> _row_offsets;
};

#endif
