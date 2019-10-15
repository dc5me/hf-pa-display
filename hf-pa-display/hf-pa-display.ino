#include <UTFT.h>
#include <SoftwareSerial.h>
#include <URTouch.h>
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

URTouch  myTouch( 6, 5, 4, 3, 2);
  // Declare which fonts we will be using
  // Download http://www.rinkydinkelectronics.com/r_fonts.php
  extern uint8_t SmallFont[];
  extern uint8_t Grotesk16x32[];
  extern uint8_t GroteskBold16x32[];
  extern uint8_t GroteskBold32x64[];
  extern uint8_t nadianne[];
  extern uint8_t franklingothic_normal[];

  // Define some colors 
  // Help under http://www.barth-dev.de/online/rgb565-color-picker/
  int VGA_BACKGROUND = 0x10A2; 
  int VGA_TITLE_UNIT = 0x632C;
  int VGA_VALUE = 0x94B2;
  int VGA_BAR = 0xCE59;

  // Init the input and output Pins  
  int oPin_air = 14;
  int iPin_pwrForward = 1;
  int iPin_pwrReturn = 2;
  int iPin_drainVoltage = 4;//3; //test
  int iPin_aux1Voltage = 4;
  int iPin_aux2Voltage = 4;//5; //test
  int iPin_pa1Amper = 6;
  int iPin_pa2Amper = 6;//7; //test
  int iPin_temperatur = 4;//8; //test

  int iPin_ptt = 8;
  int oPin_ptt = 9;
  int iPin_stby = 10;
  int iPin_Imax = 11;
  int iPin_Pmax = 12;
  int iPin_SWRmax = 13;
    
  int iPin_cat = 15;
  int oPin_band_1 = 16;
  int oPin_band_2 = 17;
  int oPin_band_3 = 18;
  int oPin_band_4 = 19;
  int oPin_band_5 = 20;
  int oPin_band_6 = 21;
    
  // Define the analogValue variables
  float pwrForward = 0;
  float pwrReturn = 0;
  float drainVoltage = 0;
  float aux1Voltage = 0;
  float aux2Voltage = 0;
  float pa1Amper = 0;
  float pa2Amper = 0;
  float temperatur = 0;
  // Define the boolValue variables
  bool ptt = false;
  bool stby = false;
  bool Imax =false;
  bool Pmax =false;
  bool SWRmax =false;
  
  float inputFactorVoltage = 5.0/1023.0;
  float pwrForwardFactor = inputFactorVoltage * (2400.0/5.0);
  float pwrReturnFactor = inputFactorVoltage * (200.0/5.0);
  float drainVoltageFactor = inputFactorVoltage * (60.0/5.0); // 5V Input = 60V PA
  // float aux1VoltageFactor = inputFactorVoltage * (30.0/5.0); // 5V Input = 30V PA //org
  float aux1VoltageFactor = inputFactorVoltage * (28.0/5.0); // 5V Input = 30V PA //test
  float aux2VoltageFactor = inputFactorVoltage * (15.0/5.0); // 5V Input = 15V PA
  float pa1AmperFactor = inputFactorVoltage  * (50.0/2,5); // 2,5V Input = 50A PA
  float pa2AmperFactor = inputFactorVoltage * (50.0/2,5); // 2,5V Input = 50A PA
  float temperaturFactor = inputFactorVoltage * (25.0/5.0); // erstmal nur spannungswert

  String MODE_MANUAL = "MANUAL";
  String MODE_AUTO = "AUTO";

  String BAND_1 = "160";
  String BAND_2 = "80";
  String BAND_3 = "60-40-30";
  String BAND_4 = "20-17";
  String BAND_5 = "15-12-10";
  String BAND_6 = "6";
  String BAND[6] = {BAND_1,BAND_2,BAND_3,BAND_4,BAND_5,BAND_6};
  int bandIdx = 2;

  float thresholdCurrent = 1.0;
  float thresholdPower = 5.0;
  float thresholdSWR = 1.3;
  float temperaturAirOn = 30;  

  String infoString = "";
  String warningString = "";
  String errorString = "";
  bool genOutputEnable = false;

  // Touch
  int touchX = -1;
  int touchY = -1;

  bool debug = false;
  
class InfoBox
{
    // This class draws a info box with value or text, title and unit.      
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
    bool _raisedError = false;

  public:
    InfoBox(String titel, String unit, int xPos, int yPos, int height, int width, float minValue, float maxValue, int colorValue, int colorBack, int font)
    {
      // Store parameter
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
      // Called by main setup
      // Background
      myGLCD.setBackColor(_colorBack);
      myGLCD.setColor(_colorBack);
      myGLCD.fillRect(_xPos, _yPos, _xPos + _width, _yPos + _height);

      // Titel
      myGLCD.setColor(VGA_TITLE_UNIT);
      myGLCD.setFont(SmallFont);
      int titelFontXsize = myGLCD.getFontXsize();
      int titelFontYsize = myGLCD.getFontYsize();
      int titelLength = _titel.length();
      myGLCD.print( _titel, _xPos + (_width - titelLength * titelFontXsize) - _xPadding , _yPos + _yPadding + 1) ;

      // Unit
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

    bool isValueOk()
    {
      return not _raisedError;
    }    
    
    void setFloat(float value, int dec, int length)
    {
      _value = value;
      myGLCD.setBackColor(_colorBack);

      if (value < _minValue or value > _maxValue)      
      {
        myGLCD.setColor(VGA_RED);
        if (_raisedError == false and errorString == "")
        {
          _raisedError = true;
          errorString = "Error: " + _titel + " " + _value + _unit + " outside range of " + int(_minValue) + _unit + "-" + int(_maxValue)+ _unit ;          
        }
      }
      else
       {
        _raisedError = false;
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
        if (text.length()<46)
        {myGLCD.setFont(_font);}
        else
        {myGLCD.setFont(SmallFont);}
        myGLCD.print( _text, _xPos + _xPadding, _yPos + _yPadding ) ;
      }
    }    

    float getValue()
    {
      return _value;
    }

    String getText()
    {
      return _text;
    }
    
    bool isTouchInside(int x, int y )
    {
      // Check if touch is inside this widget 
      return ( (x > _xPos and x < _xPos+_width) and (y > _yPos and y < _yPos + _height));      
    }
};


class DisplayBar
{
    // This class draws a bar with scale, title, actual and maximum value.     
    // Class Variables
    String _titel;
    String _unit;

    float _value;
    float _valueMin;
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
      // Store parameter
      _titel = titel;
      _unit = unit;

      _valueMin = minValue;
      _value = _valueMin;
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

      // Filter
      _holdMaxCycles = 4;
      _deltaMaxNeg = maxValue / 100 * 4; // max decrement 4% of the max value
      _filterForValueRefresh = 0;
    }

    void init()
    {
      // Called by main setup
      // Background
      myGLCD.setBackColor(_colorBack);
      myGLCD.setColor(_colorBack);
      myGLCD.fillRect(_xPos , _yPos, _xPos + _width, _yPos + _height);

      // Title
      myGLCD.setFont(franklingothic_normal);
      myGLCD.setColor(VGA_TITLE_UNIT);
      myGLCD.print( _titel, _xPos + _xPadding , _yPos + _yPadding);

      // Info boxes
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
      // Draw the scale with value and warning levels
      myGLCD.setColor(VGA_VALUE);

      // Horizontal base line
      myGLCD.fillRect(xPos, yPos + height - 2, xPos + width, yPos + height);

      // Draw warning level
      int warningLevel1 = (_warnValue1 - _minValue) / _rangeValue * _widthBar;
      int warningLevel2 = (_warnValue2 - _minValue) / _rangeValue * _widthBar;
      myGLCD.setColor(VGA_YELLOW);
      myGLCD.fillRect(xPos + warningLevel1, yPos + height - 1, xPos + warningLevel2, yPos + height);
      myGLCD.setColor(VGA_RED);
      myGLCD.fillRect(xPos + warningLevel2, yPos + height - 1, xPos + width, yPos + height);

      // Draw helplines and values
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
            // Small values as float with 1 dec
            myGLCD.printNumF(helpValue, 1, xPosHelpline + 3, yPos);
          }
          else
          {
            // Larg values as int
            myGLCD.printNumI( helpValue, xPosHelpline + 3 , yPos ) ;
          }
        }
      }
    }
    
    void setValue(float value)
    {
      // Set value and draw bar and info box
      // Refresh the info box only all 4 updates
      _filterForValueRefresh++;
      if (_filterForValueRefresh >= 4)
      {
        _filterForValueRefresh = 0;
      }

      // Filter value. slow down the decrements
      if (value < _valueOld);
      {
        _delta = _valueOld - value;
        if (_delta > _deltaMaxNeg)
        {
          value = _valueOld - _deltaMaxNeg;
        }
      }

      // Set the actual value info box
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

      // Update the bar
      _value = value;
      setValueMax(_value);
      _level = (value - _minValue) / _rangeValue * _widthBar;

      if (_level > _widthBar)
      {
        _level = _widthBar;
      }
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
      // Set the maximum value
      if (value > _valueMax)
      {
        _valueMax = value;

        // Set the maximum value info box
        if (value < 100)
        {
          ptrMaxBox->setFloat( value,   1,  4);
        }
        else
        {
          ptrMaxBox->setInt( value,  4);
        }
      }
    }
    
    void resetValueMax()
    {
      _valueMax = _valueMin;
      ptrMaxBox->setInt( _valueMin,  4);
      //setValueMax( float(1) );
    }

    float getValue( )
    {
      // Return the actual value
      return _value;
    }
    
    bool isTouchInside(int x, int y )
    {
      // Check if touch is inside this widget 
      return ( (x > _xPos and x < _xPos+_width) and (y > _yPos and y < _yPos + _height));      
    }
};

// SETUP the grafic objects
//                        titel         unit    xPos  yPos  height  width, _minValue,  _maxValue,  colorValue   colorBack         font
InfoBox modeBox(          "MODE",       "",     395,  60,   32,     200,   0,          0,          VGA_VALUE,   VGA_BACKGROUND,   Grotesk16x32);
InfoBox frequencyBox(     "Frequency",  "MHZ",  395,  20,   32,     200,   1.7,        55.0,       VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox bandBox(          "LPF",        "m",    20,   20,   72,     350,   0,          0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold32x64);

InfoBox drainVoltageBox(  "DRAIN",      "V",    20,   340,  32,     125,   50,         53,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox aux1VoltageBox(   "AUX R",      "V",    170,  340,  32,     125,   22,         25,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox aux2VoltageBox(   "AUX B",      "V",    320,  340,  32,     125,   11,         14,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox airBox(           "AIR",        "",     470,  340,  32,     125,   0,          0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);

InfoBox pa1AmperBox(      "PA 1",       "A",    20,   380,  32,     125,   0,          24.9,       VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox pa2AmperBox(      "PA 2",       "A",    170,  380,  32,     125,   0,          24.9,       VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox temperaturBox(    "",           "`C",   320,  380,  32,     125,   10,         60,         VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);
InfoBox emptyBox(         "",           "",     470,  380,  32,     125,   0,          0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);

InfoBox msgBox(           "",           "",     20,   420,   32,    760,   0,          0,          VGA_VALUE,   VGA_BACKGROUND,   Grotesk16x32);
InfoBox txRxBox(          "",           "",     655,  340,   72,    125,   0,          0,          VGA_VALUE,   VGA_BACKGROUND,   GroteskBold16x32);

//                        titel,        unit,  xPos,  yPos,  height,width, minValue,   maxValue,  warnValue1, warnValue2, colorBar,  colorBack,      noOffHelplines
DisplayBar pwrBar(        "PWR",        "W",    20,   126,   80,    760,   0,          2500,      750,        1750,       VGA_BAR,   VGA_BACKGROUND, 10 );
DisplayBar swrBar(        "SWR",        "",     20,   226,   80,    760,   1,          5,         3,          4,          VGA_BAR,   VGA_BACKGROUND, 16 );

void setup()
{
  // Run the setup and init everything 
  if (debug == true)
  {
    Serial.begin(9600);
  }
  myGLCD.InitLCD();
  myGLCD.clrScr();

  myTouch.InitTouch(LANDSCAPE);  
  myTouch.setPrecision(PREC_MEDIUM);
  
  // Set call sign and version
  myGLCD.setFont(nadianne);
  myGLCD.setColor(VGA_TITLE_UNIT);
  myGLCD.print("DJ8QP ", RIGHT, 20);
  myGLCD.print("DC5ME ", RIGHT, 40);
  myGLCD.setFont(SmallFont);
  myGLCD.print("V1.4  ", RIGHT, 60);
  
  // Init the grafic objects
  frequencyBox.init();
  frequencyBox.setFloat(  7.015, 3,  6);
  modeBox.init();
  modeBox.setText(MODE_MANUAL);
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
  bandBox.setText(BAND[bandIdx]);  
  airBox.init();
  airBox.setText("OFF");  
  emptyBox.init();
}

void loop()
{
  // Run the main loop
  // Read touch X and Y values  
  if (myTouch.dataAvailable())
    {
    myTouch.read();
    touchX = myTouch.getX();
    touchY = myTouch.getY();  

    if (debug == true)
      {
      myGLCD.drawPixel (touchX, touchY);
      myGLCD.fillRect(700, 80, 780, 100);
      myGLCD.setColor(VGA_TITLE_UNIT);
      myGLCD.setFont(SmallFont);  
      myGLCD.printNumI(touchX, 710, 85);  
      myGLCD.printNumI(touchY, 750, 85);      
      }
    }

  //-----------------------------------------------------------------------------
  // Read all inputs
  pwrForward = analogRead(iPin_pwrForward) * pwrForwardFactor; 
  pwrReturn = analogRead(iPin_pwrReturn)* pwrReturnFactor;
  drainVoltage = analogRead(iPin_drainVoltage)* drainVoltageFactor;
  aux1Voltage = analogRead(iPin_aux1Voltage)* aux1VoltageFactor;
  aux2Voltage = analogRead(iPin_aux2Voltage)*aux2VoltageFactor;
  pa1Amper = analogRead(iPin_pa1Amper) * pa1AmperFactor;
  pa2Amper = analogRead(iPin_pa2Amper) * pa2AmperFactor;
  temperatur = analogRead(iPin_temperatur) * temperaturFactor;

  ptt = not digitalRead(iPin_ptt);
  stby = digitalRead(iPin_stby);
  Imax = digitalRead(iPin_Imax);
  Pmax = digitalRead(iPin_Pmax);
  SWRmax = digitalRead(iPin_SWRmax);

  if (debug == true)
    {  
    Serial.println("---------------------");
    Serial.println(ptt);
    Serial.println(stby);

    Serial.print("pwrForward");
    Serial.println(pwrForward);
    Serial.print("pwrReturn");
    Serial.println(pwrReturn);
    Serial.print("drainVoltage");
    Serial.println(drainVoltage);
    Serial.print("aux1Voltage");
    Serial.println(aux1Voltage);
    Serial.print("aux2Voltage");
    Serial.println(aux2Voltage);
    Serial.print("pa1Amper");
    Serial.println(pa1Amper);
    Serial.print("pa2Amper");
    Serial.println(pa2Amper);
    Serial.print("temperatur");
    Serial.println(temperatur);   
    }
    
  //-----------------------------------------------------------------------------  
  // Set display values. The widgets monitors the values and output an errorString 
    
  pwrBar.setValue(pwrForward);
  
  // Calculate swr
  float pRatio;
  float swr; 
  if (pwrReturn > 0)
  {
    pRatio = pwrForward/pwrReturn;        
    swr = fabs( (1.0+sqrt(pRatio)) / (1.0-sqrt(pRatio)));
  }
  else
  {
    swr = 1;
  }
  swrBar.setValue(swr);

  drainVoltageBox.setFloat( drainVoltage,   1,  4 );
  aux1VoltageBox.setFloat( aux1Voltage,   1,  4 );
  aux2VoltageBox.setFloat( aux2Voltage,   1,  4 );
  pa1AmperBox.setFloat( pa1Amper,    1,  4 );
  pa2AmperBox.setFloat( pa1Amper,   1,  4 );
  temperaturBox.setFloat( temperatur,   1,  4 ); 

  // Monitor additional inputs and set errorString
  if (genOutputEnable == true)
  {
    if (Imax == true)
    {
      errorString  = "Error: Imax detected";
    }
    if (Pmax == true)
    {
      errorString  = "Error: Pmax detected";
    }
    if (SWRmax == true)
    {
      errorString  = "Error: SWR max detected";
    }
  }  
      
  //-----------------------------------------------------------------------------
  // Touch events
  if (touchX != -1 and touchY!=-1)
  {
  if (modeBox.isTouchInside(touchX, touchY))  
  {
    if (modeBox.getText() == MODE_MANUAL)   
    {
      modeBox.setText(MODE_AUTO);
    }
    else
    {
      modeBox.setText(MODE_MANUAL);
    }  
  }
  else if (modeBox.getText() == MODE_MANUAL and bandBox.isTouchInside(touchX, touchY))
  {
    if (bandIdx >= 5)
    {
      bandIdx=0;
    }
    else
    {
      bandIdx++;
    }
    bandBox.setText(BAND[bandIdx]);    
  }
  else if (pwrBar.isTouchInside(touchX, touchY))
  {
    pwrBar.resetValueMax();  
  }
  else if (swrBar.isTouchInside(touchX, touchY))
  {
    swrBar.resetValueMax();  
  }
  else if (msgBox.isTouchInside(touchX, touchY))
  {
    if (msgBox.getText() == "")
    {
      infoString = "More Information on: www.dj8qp.de" ;   
    }
    else
    {
      msgBox.setText("");  
      errorString = "";
      infoString = "";
    }
  }
  else if (airBox.isTouchInside(touchX, touchY))
  {
    airBox.setText("ON");    
  }
  
  // Reset touch values
  touchX, touchY = -1;
  }

  //-----------------------------------------------------------------------------
  // Start init test       
  if (genOutputEnable == false and errorString == "" and warningString == "")
  {    
    if (not drainVoltageBox.isValueOk())
    {
      errorString  = "Startup error: Drain voltage not Ok";
    }
    else if (not aux1VoltageBox.isValueOk())
    {
      errorString  = "Startup error: Auxilarry 1 voltage not Ok";
    }
    else if (not aux2VoltageBox.isValueOk())
    {
      errorString  = "Startup error: Auxilarry 2 voltage not Ok";
    }
    else if (not temperaturBox.isValueOk())
    {
      errorString  = "Startup error: Temperature not Ok";
    }
    else if (pa1AmperBox.getValue() > thresholdCurrent) 
    {      
      errorString  = "Startup error: PA 1 Current not 0A";
    }
    else if (pa2AmperBox.getValue() > thresholdCurrent) 
    {
      errorString  = "Startup error: PA 2 Current not 0A";
    }
    else if (pwrBar.getValue() > thresholdPower)
    {
      errorString  = "Startup error: PWR detected";
    }
    else if (swrBar.getValue() > thresholdSWR) 
    {
      errorString  = "Startup error: SWR detected";
    }  
    else if (not stby) 
    {
      errorString  = "Startup error: PA not in STBY";
    }
    else if (ptt == true)
    {
      errorString  = "Startup error: PTT detected";
    }
    else if (Imax == true)
    {
      errorString  = "Startup error: Imax detected";
    }
    else if (Pmax == true)
    {
      errorString  = "Startup error: Pmax detected";
    }
    else if (SWRmax == true)
    {
      errorString  = "Startup error: SWR max detected";
    }
    else
    {
      genOutputEnable = true;
      infoString = "Startup completed.";
    }  
  }

  // Reset genOutputEnable on any errorString
  if (errorString != "")
  {
    genOutputEnable = false;
  }

  //-----------------------------------------------------------------------------
  // Signal evaluation
  if (stby)
  {    
    txRxBox.setColorValue(VGA_BACKGROUND);
    txRxBox.setColorBack(VGA_YELLOW);
    txRxBox.setText("STBY");
  }
  else
  {
    if (ptt)
    {      
      txRxBox.setColorValue(VGA_BACKGROUND);
      txRxBox.setColorBack(VGA_RED);
      txRxBox.setText("TX");  
    }
    else
    {
      txRxBox.setColorValue(VGA_BACKGROUND);
      txRxBox.setColorBack(VGA_GREEN);
      txRxBox.setText("RX");
    } 
  }
  
  if (temperatur >= temperaturAirOn or errorString != "")
  {
    airBox.setText("ON");    
  }
  else if (temperatur <= temperaturAirOn-5)
  {
    airBox.setText("OFF");    
  }

  if (modeBox.getText() == MODE_AUTO)
  {
    float frequency = frequencyBox.getValue();  
    if (1.7 <= frequency and frequency  <= 2.1)
    {
      bandIdx = 0;
    }
    else if (3.4 <= frequency and frequency <= 3.9)
    {
      bandIdx = 1;
    }
    else if (5.2 <= frequency  and frequency <= 10.25)
    {
      bandIdx = 2;
    }
    else if (13.9 <= frequency  and frequency <= 18.25)
    {
      bandIdx = 3;
    }
    else if (20.9 <= frequency  and frequency <= 29.8)
    {
      bandIdx = 4;
    }
    else if (49.9 <= frequency  and frequency <= 51.1)
    {
      bandIdx = 5;
    }
    else
    {
      errorString = "Frequency does not fit to any band";
    }
    bandBox.setText(BAND[bandIdx]);
  }

  //-----------------------------------------------------------------------------
  // Write to outputs
  if (airBox.getText() == "OFF")
  {
    digitalWrite(oPin_air, false);
  }
  else
  {
    digitalWrite(oPin_air, true);
  }

  if (genOutputEnable and ptt)
  {
    digitalWrite(oPin_ptt, true);
  }
  else
  {
    digitalWrite(oPin_ptt, false);
  }

  if (ptt == false and stby and (pa1AmperBox.getValue() < thresholdCurrent) and (pa2AmperBox.getValue() < thresholdCurrent) )
  {
    digitalWrite(oPin_band_1, false);
    digitalWrite(oPin_band_2, false);
    digitalWrite(oPin_band_3, false);
    digitalWrite(oPin_band_4, false);
    digitalWrite(oPin_band_5, false);
    digitalWrite(oPin_band_6, false);

    if( bandIdx == 0 )
    {digitalWrite(oPin_band_1, true);}
    else if( bandIdx == 1 )
    {digitalWrite(oPin_band_2, true);}
    else if( bandIdx == 2 )
    {digitalWrite(oPin_band_3, true);}
    else if( bandIdx == 3 )
    {digitalWrite(oPin_band_4, true);}
    else if( bandIdx == 4 )
    {digitalWrite(oPin_band_5, true);}
    else if( bandIdx == 5 )
    {digitalWrite(oPin_band_6, true);}
  }
 
  //-----------------------------------------------------------------------------
  // Display error messages
  if (errorString != "")
  {
    msgBox.setColorValue(VGA_RED);
    msgBox.setText(errorString);
  }
  else if (warningString  != "")
  {
    msgBox.setColorValue(VGA_YELLOW);  
    msgBox.setText(warningString);
  }
  else if (infoString != "")
  {
    msgBox.setColorValue(VGA_VALUE);  
    msgBox.setText(infoString);
  }
  
  delay(25);    
}
