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
#endif

#endif
