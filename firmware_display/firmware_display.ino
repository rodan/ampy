
#include <inttypes.h>
#include <ctype.h>
#include <SPI.h>
#include <SD.h>
#include "EPD.h"
#include "S5813A.h"
#include "EReader.h"

// the eink related libraries are available from
// https://github.com/wyolum/EPD

int cmd;
char buffer[50];

void setup()
{
  Serial.begin(9600);
  ereader.setup(EPD_2_7);
}

void loop()
{
  
  if(Serial.available() > 0)
  { 
    memset(buffer, 0, sizeof(buffer));   
    cmd = Serial.read();
    Serial.readBytesUntil(0x0a, buffer, 50);
    ereader.spi_attach();
    if (cmd == '1')
      ereader.put_ascii(25, 20, buffer, true);
    else if (cmd == '2')
      ereader.put_ascii(25, 36, buffer, true);
    else if (cmd == '3')
      ereader.put_ascii(25, 52, buffer, true);
    else if (cmd == '4')
      ereader.put_ascii(25, 68, buffer, true);
    else if (cmd == '5')
      ereader.put_ascii(25, 84, buffer, true);
    else if (cmd == '6')
      ereader.put_ascii(25, 100, buffer, true);
    else if (cmd == '7')
      ereader.put_ascii(25, 116, buffer, true);
    else if (cmd == '8')
      ereader.put_ascii(25, 132, buffer, true);
    else if (cmd == '9')
      ereader.put_ascii(25, 148, buffer, true);
    else if (cmd == 'A')
    {
      ereader.show();
      ereader.spi_detach();  //To avoid screen washout.
    }
  }
  
}

