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

URTouch myTouch( 6, 5, 4, 3, 2);
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
#define vgaBackgroundColor 0x10A2
#define vgaTitleUnitColor  0x632C
#define vgaValueColor      0x94B2
#define vgaBarColor        0xCE59

// Init the input and output Pins    
#define axPin_reserve0     0
#define aiPin_pwrForward   1
#define aiPin_pwrReturn    2
#define aiPin_drainVoltage 4 //3 //test
#define aiPin_aux1Voltage  4
#define aiPin_aux2Voltage  4 //5 //test
#define aiPin_pa1Amper     6
#define aiPin_pa2Amper     6 //7 //test
#define aiPin_temperatur   4 //8 //test
// digital pin 0-8 used by UTFT
#define diPin_ptt          8
#define doPin_ptt          9
#define diPin_stby        10
#define diPin_Imax        11
#define diPin_Pmax        12
#define diPin_SWRmax      13
#define doPin_Sound       14
#define doPin_air         15
#define dxPin_reserve16   16
#define dxPin_reserve17   17
#define dxPin_reserve18   18
#define diPin_cat         19 //Serail Port 1 RX
// digital pin 22-53 used by UTFT
#define doPin_freqBand1   64
#define doPin_freqBand2   65
#define doPin_freqBand3   66
#define doPin_freqBand4   67
#define doPin_freqBand5   68
#define doPin_freqBand6   69
      
// Define the analogValue variables
float pwrForwardValue;
float pwrReturnValue;
float drainVoltageValue;
float aux1VoltageValue;
float aux2VoltageValue;
float pa1AmperValue;
float pa2AmperValue;
float temperaturValue;
  
// Define the boolValue variables
bool pttValue;
bool stbyValue;
bool ImaxValue;
bool PmaxValue;
bool SWRmaxValue;
  
#define inputFactorVoltage (5.0/1023.0)
#define pwrForwardFactor (inputFactorVoltage * (2400.0/5.0))
#define pwrReturnFactor (inputFactorVoltage * (200.0/5.0))
#define drainVoltageFactor (inputFactorVoltage * (60.0/5.0)) // 5V Input = 60V PA
// #define aux1VoltageFactor (inputFactorVoltage * (30.0/5.0)) // 5V Input = 30V PA //org
#define aux1VoltageFactor (inputFactorVoltage * (28.0/5.0)) // 5V Input = 30V PA //test
#define aux2VoltageFactor (inputFactorVoltage * (15.0/5.0)) // 5V Input = 15V PA
#define pa1AmperFactor (inputFactorVoltage  * (50.0/2,5)) // 2,5V Input = 50A PA
#define pa2AmperFactor (inputFactorVoltage * (50.0/2,5)) // 2,5V Input = 50A PA
#define temperaturFactor (inputFactorVoltage * (25.0/5.0)) // erstmal nur spannungswert

#define modeManualName "MANUAL"
#define modeAutoName   "AUTO"

#define freqBand1Name "160"
#define freqBand2Name "80"
#define freqBand3Name "60-40-30"
#define freqBand4Name "20-17"
#define freqBand5Name "15-12-10"
#define freqBand6Name "6"
String BAND[6] = {freqBand1Name,freqBand2Name,freqBand3Name,freqBand4Name,freqBand5Name,freqBand6Name};
int bandIdx = 2;

#define thresholdCurrent           1.0
#define thresholdPower             5.0
#define thresholdSWR               1.3
#define thresholdTemperaturAirOn   30  

String infoString;
String warningString;
String errorString;
bool genOutputEnable = false;

// Touch
int touchX = -1;
int touchY = -1;

#define debug false
unsigned long timeAtCycleStart, timeAtCycleEnd, timeStartMorseDownTime, actualCycleTime;
int drawWidgetIndex;
  
#define cycleTime        30

#define morseDelay       40
#define morseDotDelay   (morseDelay * 1)
#define morseDashDelay  (morseDelay * 3)
#define morseSpaceDelay (morseDelay * 7)

#define morseDotChar    '.'
#define morseDashChar   '-'
#define morseSpaceChar  ' '

String morseNewMsg;
String morseOutputMsg;              // example: "CQ "
int nextMorseCodeIndex;             // ----------^
char nextMorseCode;                 // "C"
String nextMorseTonSequence;        // "-.-."
int nextMorseTonSequenceIndex;      // -^
char nextSound;                     // "-"
int morseDownTime;                  // signal run time: dash dot or space
int morseStep;                      // step chain
int morseOutputMsgLength;

float pwrForwardReturnRatio;
float swrValue; 
  
char* morseLetter[] = {
  ".-",     // A
  "-...",   // B
  "-.-.",   // C
  "-..",    // D
  ".",      // E
  "..-.",   // F
  "--.",    // G
  "....",   // H
  "..",     // I
  ".---",   // J
  "-.-",    // K
  ".-..",   // L
  "--",     // M
  "-.",     // N
  "---",    // O
  ".--.",   // P
  "--.-",   // Q
  ".-.",    // R
  "...",    // S
  "-",      // T
  "..-",    // U
  "...-",   // V
  ".--",    // W
  "-..-",   // X
  "-.--",   // Y
  "--.."    // Z
};

char* morseNumbers[] = {
  "-----",   // 0
  ".----",   // 1
  "..---",   // 2
  "...--",   // 3
  "....-",   // 4
  ".....",   // 5
  "-....",   // 6
  "--...",   // 7
  "---..",   // 8
  "----."    // 9
};


class InfoBox
{
    // This class draws a info box with value or text, title and unit.      
    // Class Variables
    String _title, _unit, _text;
    
    float _value, _minValue, _maxValue;
    
    int _xPos, _yPos, _height, _width;
    int _xPadding, _yPadding;
    int _colorValue,_colorBack, _font;

    bool _raisedError = false;
    bool _drawLater =  false;

  public:
    InfoBox(String title, String unit, int xPos, int yPos, int height, int width, float minValue, float maxValue, int colorValue, int colorBack, int font)
    {
      // Store parameter
      _title = title;
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

      // Title
      myGLCD.setColor(vgaTitleUnitColor);
      myGLCD.setFont(SmallFont);
      int titleFontXsize = myGLCD.getFontXsize();
      int titleFontYsize = myGLCD.getFontYsize();
      int titleLength = _title.length();
      myGLCD.print( _title, _xPos + (_width - titleLength * titleFontXsize) - _xPadding , _yPos + _yPadding + 1) ;

      // Unit
      myGLCD.setFont(franklingothic_normal);
      int unitFontXsize = myGLCD.getFontXsize();
      int unitFontYsize = myGLCD.getFontYsize();
      int unitLength = _unit.length();
      myGLCD.print(_unit, _xPos + (_width - unitLength * unitFontXsize) - _xPadding , _yPos + _yPadding + titleFontYsize + 1) ;
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
    
    void setFloat(float value, int dec, int length, bool show)
    {
      if ((value != _value) or _drawLater)
      {
      _value = value;
      myGLCD.setBackColor(_colorBack);

      if (value < _minValue or value > _maxValue)      
      {
        myGLCD.setColor(VGA_RED);
        if (_raisedError == false and errorString == "")
        {
          _raisedError = true;
          errorString = "Error: " + _title + " " + _value + _unit + " outside range of " + int(_minValue) + _unit + "-" + int(_maxValue)+ _unit ;          
          morseNewMsg = "err " + String(_title);
        }
      }
      else
       {
        _raisedError = false;
        myGLCD.setColor(_colorValue);
      }

      if (show)
      {
        myGLCD.setFont(_font);
        myGLCD.printNumF(_value, dec, _xPos + _xPadding, _yPos + _yPadding, '.', length);      
        _drawLater=false;
      }
      else
      {
        _drawLater=true;
      }
      }
    }

    void setInt(int value, int length, bool show)
    {
      if ((value != _value) or _drawLater)
      {
      _value = value;
      
      myGLCD.setBackColor(_colorBack);

      if (value < _minValue or value > _maxValue)      
      {
        myGLCD.setColor(VGA_RED);
        if (_raisedError == false and errorString == "")
        {
          _raisedError = true;
          errorString = "Error: " + _title + " " + _value + _unit + " outside range of " + int(_minValue) + _unit + "-" + int(_maxValue)+ _unit ;          
          morseNewMsg = "err " + String(_title);
        }
      }
      else
       {
        _raisedError = false;
        myGLCD.setColor(_colorValue);
      }
      
      if (show)
      {
        myGLCD.setFont(_font);
        myGLCD.printNumI(_value, _xPos + _xPadding, _yPos + _yPadding, length);
        _drawLater=false;
      }
      else
      {
        _drawLater=true;
      }
      }
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
    String _title, _unit;

    float _value, _valueMin, _valueOld, _valueMax;
    float _minValue, _maxValue, _rangeValue;
    float _warnValue1, _warnValue2;
    float _level, _levelOld, _delta;
    
    int _xPos, _yPos;
    int _xPosBar, _yPosBar, _heightBar, _widthBar;
    int _height, _width;
    int _xPadding, _yPadding;
    int _colorBar, _colorBack, _font;    
    int _noOffHelplines;

    // filter
    int _holdMaxCycles;
    int _filterForValueRefresh;
    float _deltaMaxNeg;
    bool _showMax;

    InfoBox *ptrActBox;
    InfoBox *ptrMaxBox;

  public:
    DisplayBar(String title, String unit, int xPos, int yPos, int height, int width, float minValue, float maxValue, float warnValue1, float warnValue2, int colorBar, int colorBack, int noOffHelplines )

    {
      // Store parameter
      _title = title;
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
      _showMax = false;
      
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
      myGLCD.setColor(vgaTitleUnitColor);
      myGLCD.print( _title, _xPos + _xPadding , _yPos + _yPadding);

      // Info boxes
      int xPosInfoBox = _xPos + _width - _xPadding - 125;
      int yPosInfoBox = _yPos + _height - _yPadding - 64 ;
      _xPosBar = _xPos + _xPadding;
      _yPosBar = _yPos + (_height / 2);
      _heightBar = (_yPos + _height) - _yPosBar - 4;
      _widthBar = xPosInfoBox - _xPos - 2 * _xPadding;
      
      //                               title    unit     xPos           yPos              height  width, minValue, maxValue,  colorValue       colorBack             font
      ptrActBox = new InfoBox (        "",      _unit,   xPosInfoBox,   yPosInfoBox,      32,     125,   0,        _maxValue, vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
      ptrMaxBox = new InfoBox (        "PEP",   _unit,   xPosInfoBox,   yPosInfoBox + 32, 32,     125,   0,        _maxValue, vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
      
      ptrActBox->init();
      ptrMaxBox->init();

      //        xPos,     yPos,           height,  width
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
      myGLCD.setColor(vgaValueColor);

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
      myGLCD.setColor(vgaValueColor);
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
    
    void setValue(float value, bool show)
    {
      // Set value and draw bar and info box
      // Refresh the info box only all 4 updates
            
      if (value < _valueOld);
      {
        _delta = _valueOld - value;
        if (_delta > _deltaMaxNeg)
        {
          value = _valueOld - _deltaMaxNeg;
        }
      }

      // Set the actual value info box     
        if (value < 100)
        {
          ptrActBox->setFloat( value,   1,  4, show);
        }
        else
        {
          ptrActBox->setInt(   value,       4, show);
        }
      
      // Update the bar
      _value = value;
      if (_showMax)
      {
        setValueMax(_value);
      }
      
      if (show)
      {
        _showMax = true;
      }
      else
      {
        _showMax = false;
      }
      
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
          ptrMaxBox->setFloat( value,   1,  4, true);
        }
        else
        {
          ptrMaxBox->setInt( value,  4, true);
        }
      }
    }
    
    void resetValueMax()
    {
      _valueMax = -1;      
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

void morse()
  {         
  switch (morseStep)
  {
    
    case 0:
      // Check for new message and get the next char
      morseOutputMsgLength = morseOutputMsg.length();
      if (morseOutputMsgLength == 0)
      {
        morseOutputMsg = morseNewMsg;
      }
      else
      {
        nextMorseCode = morseOutputMsg[nextMorseCodeIndex];

        if (nextMorseCode >= 'a' && nextMorseCode <= 'z')
        {
          nextMorseTonSequence = morseLetter[nextMorseCode - 'a'];
        }

        else if (nextMorseCode >= 'A' && nextMorseCode <= 'Z')
        {
          nextMorseTonSequence = morseLetter[nextMorseCode - 'A'];
        }

        else if (nextMorseCode >= '0' && nextMorseCode <= '9')
        {
          nextMorseTonSequence = morseNumbers[nextMorseCode - '0'];
        }

        else if (nextMorseCode == ' ')
        {
          nextMorseTonSequence = ' ';
        }
        nextMorseTonSequenceIndex = 0;
        morseStep++;
      }
      break;
    case 1:
      // Get the next sound: dash, dot or space
      nextSound = nextMorseTonSequence[nextMorseTonSequenceIndex];

      if (nextSound == morseDotChar)
      {
        digitalWrite(doPin_Sound, HIGH);       
        morseDownTime = morseDotDelay;
      }
      else if (nextSound == morseDashChar)
      {
        digitalWrite(doPin_Sound, HIGH);        
        morseDownTime = morseDashDelay;
      }
      else if (nextSound == morseSpaceChar)
      {
        digitalWrite(doPin_Sound, LOW);        
        morseDownTime = morseSpaceDelay;
      }
      morseStep++;
      timeStartMorseDownTime = millis();
      break;
    case 2:
      // Check the downtime, until the signal will change      
      if ((millis() - timeStartMorseDownTime) > morseDownTime)
      {
        if (nextSound == ' ')
        {
          morseDownTime = 0;
        }
        else
        {          
          morseDownTime = morseDelay;
        }
        digitalWrite(doPin_Sound, LOW);
        timeStartMorseDownTime = millis();
        morseStep++;
      }
      break;
    case 3:
      // Check the downtime, until the silence ends      
      if ((millis() - timeStartMorseDownTime) > morseDownTime)
      {
        if (nextMorseTonSequenceIndex <= nextMorseTonSequence.length())
        {
          nextMorseTonSequenceIndex++;
          morseStep = (morseStep - 2);
        }
        else
        {
          morseStep++;
        }
      }
      break;
    case 4:
      // increment the index to the next char of the message, or start at the beginning
      if (nextMorseCodeIndex <= morseOutputMsgLength)
      {
        nextMorseCodeIndex++;
      }
      else
      {
        nextMorseCodeIndex = 0;
        morseOutputMsg = "";
        morseNewMsg = "";
      }
      morseStep = 0;
      break;
   }  
  }    
  
// SETUP the grafic objects
//                        title         unit    xPos  yPos  height  width, _minValue,  _maxValue,  colorValue       colorBack             font
InfoBox modeBox(          "MODE",       "",     395,  60,   32,     200,   0,          0,          vgaValueColor,   vgaBackgroundColor,   Grotesk16x32);
InfoBox frequencyBox(     "Frequency",  "MHZ",  395,  20,   32,     200,   1.7,        55.0,       vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
InfoBox bandBox(          "LPF",        "m",    20,   20,   72,     350,   0,          0,          vgaValueColor,   vgaBackgroundColor,   GroteskBold32x64);

InfoBox drainVoltageBox(  "DRAIN",      "V",    20,   340,  32,     125,   50,         53,         vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
InfoBox aux1VoltageBox(   "AUX R",      "V",    170,  340,  32,     125,   22,         25,         vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
InfoBox aux2VoltageBox(   "AUX B",      "V",    320,  340,  32,     125,   11,         14,         vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
InfoBox airBox(           "AIR",        "",     470,  340,  32,     125,   0,          0,          vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);

InfoBox pa1AmperBox(      "PA 1",       "A",    20,   380,  32,     125,   0,          24.9,       vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
InfoBox pa2AmperBox(      "PA 2",       "A",    170,  380,  32,     125,   0,          24.9,       vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
InfoBox temperaturBox(    "",           "`C",   320,  380,  32,     125,   10,         60,         vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);
InfoBox emptyBox(         "",           "",     470,  380,  32,     125,   0,          0,          vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);

InfoBox msgBox(           "",           "",     20,   420,   32,    760,   0,          0,          vgaValueColor,   vgaBackgroundColor,   Grotesk16x32);
InfoBox txRxBox(          "",           "",     655,  340,   72,    125,   0,          0,          vgaValueColor,   vgaBackgroundColor,   GroteskBold16x32);

//                        title,        unit,  xPos,  yPos,  height,width, minValue,   maxValue,  warnValue1, warnValue2, colorBar,      colorBack,          noOffHelplines
DisplayBar pwrBar(        "PWR",        "W",    20,   126,   80,    760,   0,          2500,      750,        1750,       vgaBarColor,   vgaBackgroundColor, 10 );
DisplayBar swrBar(        "SWR",        "",     20,   226,   80,    760,   1,          5,         3,          4,          vgaBarColor,   vgaBackgroundColor, 16 );

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

  pinMode(doPin_freqBand1, OUTPUT);
  pinMode(doPin_freqBand2, OUTPUT);
  pinMode(doPin_freqBand3, OUTPUT);
  pinMode(doPin_freqBand4, OUTPUT);
  pinMode(doPin_freqBand5, OUTPUT);
  pinMode(doPin_freqBand6, OUTPUT);
  pinMode(doPin_Sound, OUTPUT);
    
  // Set call sign and version
  myGLCD.setFont(nadianne);
  myGLCD.setColor(vgaTitleUnitColor);
  myGLCD.print("DJ8QP ", RIGHT, 20);
  myGLCD.print("DC5ME ", RIGHT, 40);
  myGLCD.setFont(SmallFont);
  myGLCD.print("V1.5  ", RIGHT, 60);
  
  // Init the grafic objects
  frequencyBox.init();
  frequencyBox.setFloat(  7.015, 3,  6, true); // test
  modeBox.init();
  modeBox.setText(modeManualName);
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
  timeAtCycleStart = millis();
  morse();
  // Read touch X and Y values  
  if (myTouch.dataAvailable())
    {    
    myTouch.read();
    touchX = myTouch.getX();
    touchY = myTouch.getY();  

    if (debug == true)
      {        
      myGLCD.drawPixel(touchX, touchY);
      myGLCD.fillRect(700, 80, 780, 100);
      myGLCD.setColor(vgaTitleUnitColor);
      myGLCD.setFont(SmallFont);  
      myGLCD.printNumI(touchX, 710, 85);  
      myGLCD.printNumI(touchY, 750, 85);            
      }
    }

  //-----------------------------------------------------------------------------
  // Read all inputs
  pwrForwardValue = analogRead(aiPin_pwrForward) * pwrForwardFactor; 
  pwrReturnValue = analogRead(aiPin_pwrReturn)* pwrReturnFactor;
  drainVoltageValue = analogRead(aiPin_drainVoltage)* drainVoltageFactor;
  aux1VoltageValue = analogRead(aiPin_aux1Voltage)* aux1VoltageFactor;
  aux2VoltageValue = analogRead(aiPin_aux2Voltage)*aux2VoltageFactor;
  pa1AmperValue = analogRead(aiPin_pa1Amper) * pa1AmperFactor;
  pa2AmperValue = analogRead(aiPin_pa2Amper) * pa2AmperFactor;
  temperaturValue = analogRead(aiPin_temperatur) * temperaturFactor;

  pttValue = not digitalRead(diPin_ptt);
  stbyValue = digitalRead(diPin_stby);
  ImaxValue = digitalRead(diPin_Imax);
  PmaxValue = digitalRead(diPin_Pmax);
  SWRmaxValue = digitalRead(diPin_SWRmax);

  if (debug == true)
    {      
    Serial.println("---------------------");

    Serial.print("ptt");
    Serial.println(pttValue);
    Serial.print("stby");
    Serial.println(stbyValue);

    Serial.print("pwrForward");
    Serial.println(pwrForwardValue);
    Serial.print("pwrReturn");
    Serial.println(pwrReturnValue);
    Serial.print("drainVoltage");
    Serial.println(drainVoltageValue);
    Serial.print("aux1Voltage");
    Serial.println(aux1VoltageValue);
    Serial.print("aux2Voltage");
    Serial.println(aux2VoltageValue);
    Serial.print("pa1Amper");
    Serial.println(pa1AmperValue);
    Serial.print("pa2Amper");
    Serial.println(pa2AmperValue);
    Serial.print("temperatur");
    Serial.println(temperaturValue);       
    }
    
  //-----------------------------------------------------------------------------  
  // Set display values. The widgets monitors the values and output an errorString 
    
  pwrBar.setValue(pwrForwardValue,                     drawWidgetIndex==1);  
  // Calculate swr  
  if (pwrReturnValue > 0)
  {
    pwrForwardReturnRatio = pwrForwardValue/pwrReturnValue;        
    swrValue = fabs( (1.0+sqrt(pwrForwardReturnRatio)) / (1.0-sqrt(pwrForwardReturnRatio)));
  }
  else
  {
    swrValue = 1;
  }  
  swrBar.setValue(swrValue,                            drawWidgetIndex==3);
    
  drainVoltageBox.setFloat( drainVoltageValue,  1,  4, drawWidgetIndex==5 );
  aux1VoltageBox.setFloat(  aux1VoltageValue,   1,  4, drawWidgetIndex==6 );
  aux2VoltageBox.setFloat(  aux2VoltageValue,   1,  4, drawWidgetIndex==7 );
  pa1AmperBox.setFloat(     pa1AmperValue,      1,  4, drawWidgetIndex==8 );
  pa2AmperBox.setFloat(     pa1AmperValue,      1,  4, drawWidgetIndex==9 );
  temperaturBox.setFloat(   temperaturValue,    1,  4, drawWidgetIndex==10 ); 

  // Draw index defines the infoBox that can draw new values on tht utft. 
  // If all infoBoxes would draw together, the cycletime is to long and not constant for the morse output.
  if (drawWidgetIndex==10)
  {
    drawWidgetIndex = 0;
  }
  else
  {
    drawWidgetIndex++;
  }
  
  // Monitor additional inputs and set errorString
  if (genOutputEnable == true)
  {
    if (ImaxValue == true)
    {
      errorString  = "Error: Protector Imax detected";
      morseNewMsg = "err cur ";
    }
    if (PmaxValue == true)
    {
      errorString  = "Error: Protector Pmax detected";
      morseNewMsg = "err pwr ";
    }
    if (SWRmaxValue == true)
    {
      errorString  = "Error: Protector SWR max detected";
      morseNewMsg = "err swr ";
    }
  }  
      
  //-----------------------------------------------------------------------------
  // Touch events
  if (touchX != -1 and touchY!=-1)
  {
  if (modeBox.isTouchInside(touchX, touchY))  
  {
    if (modeBox.getText() == modeManualName)   
    {
      modeBox.setText(modeAutoName);
    }
    else
    {
      modeBox.setText(modeManualName);
    }  
  }
  else if (modeBox.getText() == modeManualName and bandBox.isTouchInside(touchX, touchY))
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
      morseNewMsg = "err volt ";
    }
    else if (not aux1VoltageBox.isValueOk())
    {
      errorString  = "Startup error: Auxilarry 1 voltage not Ok";
      morseNewMsg = "err volt ";
    }
    else if (not aux2VoltageBox.isValueOk())
    {
      errorString  = "Startup error: Auxilarry 2 voltage not Ok";
      morseNewMsg = "err volt ";
    }
    else if (not temperaturBox.isValueOk())
    {
      errorString  = "Startup error: Temperature not Ok";
      morseNewMsg = "err tmp ";
    }
    else if (pa1AmperBox.getValue() > thresholdCurrent) 
    {      
      errorString  = "Startup error: PA 1 Current not 0A";
      morseNewMsg = "err amp ";
    }
    else if (pa2AmperBox.getValue() > thresholdCurrent) 
    {
      errorString  = "Startup error: PA 2 Current not 0A";
      morseNewMsg = "err amp ";
    }
    else if (pwrBar.getValue() > thresholdPower)
    {
      errorString  = "Startup error: PWR detected";
      morseNewMsg = "err pwr ";
    }
    else if (swrBar.getValue() > thresholdSWR) 
    {
      errorString  = "Startup error: SWR detected";
      morseNewMsg = "err swr ";
    }  
    else if (not stbyValue) 
    {
      errorString  = "Startup error: PA not in STBY";
    }
    else if (pttValue == true)
    {
      errorString  = "Startup error: PTT detected";
      morseNewMsg = "err ptt ";
    }
    else if (ImaxValue == true)
    {
      errorString  = "Startup error: Protector Imax detected";      
      morseNewMsg = "err cur ";
    }
    else if (PmaxValue == true)
    {
      errorString  = "Startup error: Protector Pmax detected";
      morseNewMsg = "err pwr ";
    }
    else if (SWRmaxValue == true)
    {
      errorString  = "Startup error: Protector SWR max detected";
      morseNewMsg = "err swr ";
    }
    else
    {
      genOutputEnable = true;
      infoString = "Startup completed.";
      morseNewMsg = "73 ";
    }  
  }

  // Reset genOutputEnable on any errorString
  if (errorString != "")
  {
    genOutputEnable = false;
  }

  //-----------------------------------------------------------------------------
  // Signal evaluation
  if (stbyValue)
  {    
    txRxBox.setColorValue(vgaBackgroundColor);
    txRxBox.setColorBack(VGA_YELLOW);
    txRxBox.setText("STBY");
  }
  else
  {
    if (pttValue)
    {      
      txRxBox.setColorValue(vgaBackgroundColor);
      txRxBox.setColorBack(VGA_RED);
      txRxBox.setText("TX");  
    }
    else
    {
      txRxBox.setColorValue(vgaBackgroundColor);
      txRxBox.setColorBack(VGA_GREEN);
      txRxBox.setText("RX");
    } 
  }
  
  if (temperaturValue >= thresholdTemperaturAirOn or errorString != "")
  {
    airBox.setText("ON");    
  }
  else if (temperaturValue <= thresholdTemperaturAirOn-5)
  {
    airBox.setText("OFF");    
  }

  if (modeBox.getText() == modeAutoName)
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
    digitalWrite(doPin_air, false);
  }
  else
  {
    digitalWrite(doPin_air, true);
  }

  if (genOutputEnable and pttValue)
  {
    digitalWrite(doPin_ptt, true);
  }
  else
  {
    digitalWrite(doPin_ptt, false);
  }

  if (pttValue == false and stbyValue and (pa1AmperBox.getValue() < thresholdCurrent) and (pa2AmperBox.getValue() < thresholdCurrent) )
  {
    digitalWrite(doPin_freqBand1, false);
    digitalWrite(doPin_freqBand2, false);
    digitalWrite(doPin_freqBand3, false);
    digitalWrite(doPin_freqBand4, false);
    digitalWrite(doPin_freqBand5, false);
    digitalWrite(doPin_freqBand6, false);

    if( bandIdx == 0 )
    {digitalWrite(doPin_freqBand1, true);}
    else if( bandIdx == 1 )
    {digitalWrite(doPin_freqBand2, true);}
    else if( bandIdx == 2 )
    {digitalWrite(doPin_freqBand3, true);}
    else if( bandIdx == 3 )
    {digitalWrite(doPin_freqBand4, true);}
    else if( bandIdx == 4 )
    {digitalWrite(doPin_freqBand5, true);}
    else if( bandIdx == 5 )
    {digitalWrite(doPin_freqBand6, true);}
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
    msgBox.setColorValue(vgaValueColor);  
    msgBox.setText(infoString);
  }

  //-----------------------------------------------------------------------------
  // Keep the cycle time constant
  timeAtCycleEnd = millis();    
  actualCycleTime = timeAtCycleEnd - timeAtCycleStart;      
        
  if (actualCycleTime < cycleTime)
    {        
      delay(cycleTime - actualCycleTime);
    }
    
}
