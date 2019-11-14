# hf-pa-display
Arduino 7" UTFT URTouch display for amatuer radio HF power amplifier. 

CONTENT 
- read frequency from ICOM CI-V interface
- colored touch display shows: 
  - radio frequency
  - band, band mode  
  - output power and swr as bar graph
  - pa: voltage, current,and temperature 
  - tx rx or stby
  - error/info message
- functions:
  - automatic and manual band selector
  - bar graph with actual and max value indicator
  - min max monitoring on all values with error message and morse signal
  - switch on check list 
  - power (ptt) interrupt 
    - on all error
    - swr max
    - pwr max
    - i max
    - temperatur max
  - automatic fan control 
  - morse text generator  
  - stabelized cycle time to 30ms
- touch functions
  - reset max value in bar graph
  - reset error messages
  - select band mode and band
  - force fan on
  - info text

Video preview: https://youtu.be/KhniltJ8slQ
Project: http://www.dj8qp.de/MOSFETPA/SSPA-BLF188XR-1.html
