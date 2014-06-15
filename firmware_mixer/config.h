#ifndef _CONFIG_H_
#define _CONFIG_H_

//#define USE_UART
//#define USE_WATCHDOG
//#define CONFIG_DEBUG


////////////////////////////////////////
//
// stop changing anything beyond this point
//

#ifndef USE_UART
    #define USE_I2C
    #define I2C_SLAVE
#endif

#define OLIMEX_DEVBOARD

#endif
