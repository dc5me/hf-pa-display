#include <UTFT.h>
/*
Appand to your sketch
1. UTFT Libary from http://www.rinkydinkelectronics.com/library.php?id=87
2. Used fonts (line 15) from http://www.rinkydinkelectronics.com/r_fonts.php
*/

UTFT myGLCD(SSD1963_800, 38, 39, 40, 41);
/* Change in UTFT\tft_drivers\ssd1963\800\initlcd.h:
  LCD_Write_COM(0xB0);  // LCD SPECIFICATION
  LCD_Write_DATA(0x00); // old 0x24
  LCD_Write_COM(0x36);  //rotation
  LCD_Write_DATA(0x23); // old 0x22
*/

// Declare which fonts we will be using
// download http://www.rinkydinkelectronics.com/r_fonts.php
extern uint8_t SmallFont[];
extern uint8_t Grotesk16x32[];
extern uint8_t GroteskBold16x32[];
extern uint8_t GroteskBold32x64[];
extern uint8_t nadianne[];
extern uint8_t franklingothic_normal[];

// define some colors 
// help under http://www.barth-dev.de/online/rgb565-color-picker/
int VGA_BACKGROUND = 0x10A2; 
int VGA_TITLE_UNIT = 0x632C;
int VGA_VALUE = 0x94B2;
int VGA_BAR = 0xCE59;

class InfoBox
{
    /*
    This class draws a info box with value or text, title and unit.
    
    */
    // Class Variables
    String _titel;
    String _unit;
    float _value;
    String _text;

    int _xPos;
    int _yPos;
    int _height;
    int _width;
    float _minValue;
    float _maxValue;
    int _xPadding;
    int _yPadding;

    int _colorValue;
    int _colorBack;
    int _font;

  public:
    InfoBox(String titel, String unit, int xPos, int yPos, int height, int width, float minValue, float maxValue, int colorValue, int colorBack, int font)
    {
      /*store parameter*/
      _titel = titel;
      _unit = unit;
      _value = 0;
      _text = "";

      _xPos = xPos;
      _yPos = yPos;
      _height = height;
      _width = width;
      _minValue = minValue;
      _maxValue = maxValue;
      _xPadding = 4;
      _yPadding = 1;

      _colorValue = colorValue;
      _colorBack = colorBack;
      _font = font;
    }

    void init()
    {
      /*called by main setup*/
      // background
      myGLCD.setBackColor(_colorBack);
      myGLCD.setColor(_colorBack);
      myGLCD.fillRect(_xPos, _yPos, _xPos + _width, _yPos + _height);

      // titel
      myGLCD.setColor(VGA_TITLE_UNIT);
      myGLCD.setFont(SmallFont);
      int titelFontXsize = myGLCD.getFontXsize();
      int titelFontYsize = myGLCD.getFontYsize();
      int titelLength = _titel.length();
      myGLCD.print( _titel, _xPos + (_width - titelLength * titelFontXsize) - _xPadding , _yPos + _yPadding + 1) ;

      // unit
      myGLCD.setFont(franklingothic_normal);
      int unitFontXsize = myGLCD.getFontXsize();
      int unitFontYsize = myGLCD.getFontYsize();
      int unitLength = _unit.length();
      myGLCD.print(_unit, _xPos + (_width - unitLength * unitFontXsize) - _xPadding , _yPos + _yPadding + titelFontYsize + 1) ;
    }

    void setColorValue(int color)
    {
      _colorValue = color;
    }

    void setColorBack(int color)
    {
      _colorBack = color;
    }

    void setFloat(float value, int dec, int length)
    {
      _value = value;
      myGLCD.setBackColor(_colorBack);

      if (value < _minValue or value > _maxValue)
      {
        myGLCD.setColor(VGA_RED);
      }
      else
      {
        myGLCD.setColor(_colorValue);
      }
      myGLCD.setFont(_font);
      myGLCD.printNumF(_value, dec, _xPos + _xPadding, _yPos + _yPadding, '.', length);
    }

    void setInt(int value, int length)
    {
      _value = value;
      myGLCD.setBackColor(_colorBack);
      myGLCD.setColor(_colorValue);
      myGLCD.setFont(_font);
      myGLCD.printNumI(_value, _xPos + _xPadding, _yPos + _yPadding, length);
    }
    void setText(String text)
    {
      if (text != _text)
      {
        _text = text;
        init();

        myGLCD.setBackColor(_colorBack);
        myGLCD.setColor(_colorValue);
        myGLCD.setFont(_font);
        myGLCD.print( _text, _xPos + _xPadding, _yPos + _yPadding ) ;
      }
    }

    float getFloat()
    {
      return _value;
    }
};


class DisplayBar
{
    /* 
    This class draws a bar with scale, title, actual and maximum value.     
    */
    // Class Variables
    String _titel;
    String _unit;

    float _value;
    float _valueOld;
    float _valueMax;

    float _minValue;
    float _maxValue;
    float _rangeValue;
    float _warnValue1;
    float _warnValue2;

    int _xPos;
    int _yPos;
    int _height;
    int _width;
    int _xPadding;
    int _yPadding;

    int _colorBar;
    int _colorBack;
    int _font;

    int _xPosBar;
    int _yPosBar;
    int _heightBar;
    int _widthBar;
    int _noOffHelplines;

    float _level;
    float _levelOld;
    float _delta;

    // filter
    int _holdMaxCycles;
    int _filterForValueRefresh;
    float _deltaMaxNeg;

    InfoBox *ptrActBox;
    InfoBox *ptrMaxBox;

  public:
    DisplayBar(String titel, String unit, int xPos, int yPos, int height, int width, float minValue, float maxValue, float warnValue1, float warnValue2, int colorBar, int colorBack, int noOffHelplines )

    {
      // store parameter
      _titel = titel;
      _unit = unit;

      _value = minValue;
      _valueOld = _value;
      _valueMax = 0;

      _minValue = minValue;
      _maxValue = maxValue;
      _rangeValue = _maxValue - _minValue;
      _warnValue1 = warnValue1;
      _warnValue2 = warnValue2;

      _xPos = xPos;
      _yPos = yPos;
      _height = height;
      _width = width;
      _xPadding = 4; // x padding inside the box
      _yPadding = 1; // y padding inside the box

      _colorBar = colorBar;
      _colorBack = colorBack;
      _font = 1;

      _noOffHelplines = noOffHelplines;

      // filter
      _holdMaxCycles = 4;
      _deltaMaxNeg = maxValue / 100 * 4; // max decrement 4% of the max value
      _filterForValueRefresh = 0;
    }

    void init()
    {
      /* called by main setup */
      // background
      myGLCD.setBackColor(_colorBack);
      myGLCD.setColor(_colorBack);
      myGLCD.fillRect(_xPos , _yPos, _xPos + _width, _yPos + _height);

      // title
      myGLCD.setFont(franklingothic_normal);
      myGLCD.setColor(VGA_TITLE_UNIT);
      myGLCD.print( _titel, _xPos + _xPadding , _yPos + _yPadding);

      // info boxes
      int xPosInfoBox = _xPos + _width - _xPadding - 125;
      int yPosInfoBox = _yPos + _height - _yPadding - 64 ;
      _xPosBar = _xPos + _xPadding;
      _yPosBar = _yPos + (_height / 2);
      _heightBar = (_yPos + _height) - _yPosBar - 4;
      _widthBar = xPosInfoBox - _xPos - 2 * _xPadding;
      //                               titel    unit     xPos           yPos            height  width, minValue, maxValue,    colorValue   colorBack         font
      ptrActBox = new InfoBox (        "",      _unit,   xPosInfoBox,   yPosInfoBox,      32,     125, 0,        _maxValue,   VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
      ptrMaxBox = new InfoBox (        "PEP",   _unit,   xPosInfoBox,   yPosInfoBox + 32, 32,     125, 0,        _maxValue,   VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
      ptrActBox->init();
      ptrMaxBox->init();

      //        xPos,     yPos,           height, width
      drawScale(_xPosBar, _yPosBar - 18,  15,     _widthBar);
      myGLCD.drawRect(_xPosBar, _yPosBar, _xPosBar + _widthBar, _yPosBar + _heightBar);
      _xPosBar = _xPosBar + 1;
      _yPosBar = _yPosBar + 1;
      _widthBar = _widthBar - 2;
      _heightBar = _heightBar - 2;

      myGLCD.setColor( VGA_BLACK);
      myGLCD.drawRect(_xPosBar, _yPosBar, _xPosBar + _widthBar, _yPosBar + _heightBar);
      _xPosBar = _xPosBar + 1;
      _yPosBar = _yPosBar + 1;
      _widthBar = _widthBar - 2;
      _heightBar = _heightBar - 2;
    }

    void drawScale(int xPos, int yPos, int height, int width)
    {
      /* draw the scale with value and warning levels*/
      myGLCD.setColor(VGA_VALUE);

      // horizontal base line
      myGLCD.fillRect(xPos, yPos + height - 2, xPos + width, yPos + height);

      // draw warning level
      int warningLevel1 = (_warnValue1 - _minValue) / _rangeValue * _widthBar;
      int warningLevel2 = (_warnValue2 - _minValue) / _rangeValue * _widthBar;
      myGLCD.setColor(VGA_YELLOW);
      myGLCD.fillRect(xPos + warningLevel1, yPos + height - 1, xPos + warningLevel2, yPos + height);
      myGLCD.setColor(VGA_RED);
      myGLCD.fillRect(xPos + warningLevel2, yPos + height - 1, xPos + width, yPos + height);

      // draw helplines and values
      myGLCD.setColor(VGA_VALUE);
      myGLCD.setFont(SmallFont);

      float xPosHelpline;
      float helpValue;
      float helpValueIncrement = _rangeValue / _noOffHelplines;

      for (float helpline = 0; helpline <= _noOffHelplines; helpline++)
      {
        helpValue = _minValue + (helpline * helpValueIncrement);
        xPosHelpline = xPos + (helpline / _noOffHelplines * _widthBar);
        myGLCD.drawLine(xPosHelpline, yPos, xPosHelpline, yPos + height - 2);
        if (helpline != _noOffHelplines)
        {
          if (helpValue <= 10 & helpValue > 0)
          {
            // small values as float with 1 dec
            myGLCD.printNumF(helpValue, 1, xPosHelpline + 3, yPos);
          }
          else
          {
            // larg values as int
            myGLCD.printNumI( helpValue, xPosHelpline + 3 , yPos ) ;
          }
        }
      }
    }

    void setValue(float value)
    {
      /* set value and draw bar and info box*/
      // refresh the info box only all 4 updates
      _filterForValueRefresh++;
      if (_filterForValueRefresh >= 4)
      {
        _filterForValueRefresh = 0;
      }

      // filter value. slow down the decrements
      if (value < _valueOld);
      {
        _delta = _valueOld - value;
        if (_delta > _deltaMaxNeg)
        {
          value = _valueOld - _deltaMaxNeg;
        }
      }

      // set the actual value info box
      if (_filterForValueRefresh == 0)
      {
        if (value < 100)
        {
          ptrActBox->setFloat( value,   1,  4);
        }
        else
        {
          ptrActBox->setInt(         value,  4);
        }
      }

      // update the bar
      _value = value;
      setValueMax(_value);
      _level = (value - _minValue) / _rangeValue * _widthBar;
      if (_level > _levelOld)
      {
        myGLCD.setColor(_colorBar);
        myGLCD.fillRect(_xPosBar + _levelOld, _yPosBar, _xPosBar + _level, _yPosBar + _heightBar);
      }
      else
      {
        myGLCD.setColor(_colorBack);
        myGLCD.fillRect(_xPosBar + _level , _yPosBar, _xPosBar + _levelOld , _yPosBar + _heightBar);
      }

      _levelOld = _level;
      _valueOld = value;
    }

    void setValueMax(float value)
    {
      /* set the maximum value */
      if (value > _valueMax)
      {
        _valueMax = value;

        // set the maximum value info box
        if (value < 100)
        {
          ptrMaxBox->setFloat( value,   1,  4);
        }
        else
        {
          ptrMaxBox->setInt(         value,  4);
        }
      }
    }

    void getValue( )
    {
      /* return the actual value*/
      return _value;
    }
};

/* SETUP the grafic objects*/
//                        titel         unit    xPos  yPos  height  width, _minValue, _maxValue,  colorValue   colorBack         font
InfoBox frequencyBox(     "Frequency",  "MHZ",  20,   20,   80,     275,   1.7,      55.0,      VGA_VALUE,   VGA_BACKGROUND,   GroteskBold32x64);
InfoBox bandBox(          "LPF",        "m",    470,  20,   32,     175,   0,         0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox modeBox(          "MODE",       "",     470,  68,   32,     175,   0,         0,          VGA_VALUE,   VGA_BACKGROUND,   Grotesk16x32);

InfoBox drainVoltageBox(  "DRAIN",      "V",    20,   340,  32,     125,  50,         53,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox pa1AmperBox(      "PA 1",       "A",    20,   380,  32,     125,  0,          25,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox pa2AmperBox(      "PA 2",       "A",    170,  380,  32,     125,  0,          25,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox aux1VoltageBox(   "AUX R",      "V",    170,  340,  32,     125,  22,         25,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox aux2VoltageBox(   "AUX B",      "V",    320,  340,  32,     125,  11,         14,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox temperaturBox(    "",           "`C",   320,  380,  32,     125,  10,         60,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);

//
InfoBox airBox(           "AIR",       "",     470,  340,  32,     125,  0,           0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox emptyBox(         "",          "",     470,  380,  32,     125,  0,           0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);

InfoBox msgBox(           "",          "",     20,   420,   32,    760,  0,           0,          VGA_VALUE,   VGA_BACKGROUND,   Grotesk16x32);
InfoBox txRxBox(          "",          "",     655,  340,   72,    125,  0,           0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);

//                        titel,       unit,  xPos,  yPos,  height,width, minValue, maxValue, warnValue1, warnValue2, colorBar,  colorBack,      noOffHelplines
DisplayBar pwrBar(        "PWR",       "W",    20,   120,   80,    760,   0,        2500,      750,       1750,       VGA_BAR,   VGA_BACKGROUND, 10 );
DisplayBar swrBar(        "SWR",       "",     20,   220,   80,    760,   1,        6,         3,         4,          VGA_BAR,   VGA_BACKGROUND, 10 );
//DisplayBar swrBar(      "SWR",       "",     20,   220,   80,    760,   11,       23,        14,        16,         VGA_BAR,   VGA_BACKGROUND, 6 );

void setup()
{
  /*run the setup and init everything*/
  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();

  // set call sign and version
  myGLCD.setFont(nadianne);
  myGLCD.setColor(VGA_TITLE_UNIT);
  myGLCD.print("DJ8QP ", RIGHT, 20);
  myGLCD.print("V1.3 ", RIGHT, 40);

  // init the grafic objects
  frequencyBox.init();
  modeBox.init();
  drainVoltageBox.init();
  pa1AmperBox.init();
  pa2AmperBox.init();
  aux1VoltageBox.init();
  aux2VoltageBox.init();
  temperaturBox.init();
  msgBox.init();
  txRxBox.init();

  pwrBar.init();
  swrBar.init();
  bandBox.init();
  airBox.init();
  emptyBox.init();
}

void loop()
{
  /*run the main loop*/

  // Simulation
  txRxBox.setColorValue(VGA_BACKGROUND);
  txRxBox.setColorBack(VGA_YELLOW);
  txRxBox.setText("STBY");
  delay (1000);
  msgBox.setColorValue(VGA_YELLOW);
  msgBox.setText("Systemtest wird gestartet...");
  delay (1000);
  temperaturBox.setFloat(         22.4,   1,  4);
  msgBox.setText("Systemtest: Temperatur OK");
  delay (1000);
  aux1VoltageBox.setFloat(         24.2,   1,  4);
  msgBox.setText("Systemtest: Spannung AUX R OK");
  delay (1000);
  aux2VoltageBox.setFloat(         12.3,   1,  4);
  msgBox.setText("Systemtest: Spannung AUX B OK");
  delay (1000);
  drainVoltageBox.setFloat(         51.1,   1,  4);
  msgBox.setText("Systemtest: Spannung DRAIN OK");
  delay (1000);
  pa1AmperBox.setFloat(         0.0,    1,  4);
  msgBox.setText("Systemtest: Einschaltstrom PA1 OK");
  delay (1000);
  pa2AmperBox.setFloat(         0.1,   1,  4);
  msgBox.setText("Systemtest: Einschaltstrom PA2 OK");
  delay (1000);
  msgBox.setText("Systemtest: PTT OK");
  delay (1000);
  frequencyBox.setFloat(  14.275, 3,  5);
  bandBox.setText("15-12-10");
  msgBox.setText("Systemtest: CAT OK");
  delay (1000);
  modeBox.setText("MANUEL");
  airBox.setText("ON");

  msgBox.setColorValue(VGA_VALUE);
  msgBox.setText("Systemtest erfolgreich beendet. 73, DC5ME");


  txRxBox.setColorValue(VGA_BACKGROUND);
  txRxBox.setColorBack(VGA_GREEN);
  txRxBox.setText("RX");

  delay (2000);
  msgBox.setText("");
  modeBox.setText("AUTO");

  txRxBox.setColorValue(VGA_BACKGROUND);
  txRxBox.setColorBack(VGA_RED);
  txRxBox.setText("TX");

  float simValPwrBar;
  float simValSwrBar;
  for (int event = 1; event < 15; event++)
  {
    drainVoltageBox.setFloat( 48 + 1.0 * random(100, 500) / 100, 1,  4);
    temperaturBox.setFloat(   temperaturBox.getFloat() + 2.7, 1,  4);
    aux1VoltageBox.setFloat(  23 + 1.0 * random(100, 500) / 100 ,   1,  4);
    aux2VoltageBox.setFloat(  9 + 1.0 * random(100, 500) / 100 ,   1,  4);

    simValPwrBar = random(2400);
    simValSwrBar = random(100, 585);
    for (int event = 1; event < 25; event++)
    {
      pwrBar.setValue(random(simValPwrBar));
      swrBar.setValue(1.0 * random(100, simValSwrBar ) / 100);

      pa1AmperBox.setFloat(    19 + 1.0 * random(100, 1000) / 100, 1,  4);
      pa2AmperBox.setFloat(    19 + 1.0 * random(100, 1000) / 100, 1,  4);

      delay(25);
    }
  }

  msgBox.setColorValue(VGA_RED);
  msgBox.setText("ACHTUNG: Temperatur zu hoch! #RESET");

  pwrBar.setValue(0);
  swrBar.setValue(1);
  txRxBox.setColorValue(VGA_BACKGROUND);
  txRxBox.setColorBack(VGA_YELLOW);
  txRxBox.setText("STBY");

  // restart simulation
  delay (1000);
}
