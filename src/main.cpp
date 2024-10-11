#include <Arduino.h>


/* Choices */   
#define TEST_DSTWR 1

#define DSTWR_SNIFFER 0
#define DSTWR_ANCHOR 1
#define DSTWR_TAG 2
#define DSTWR_TAG_UROS 3


/* Configuration */
#define TEST_MODE TEST_DSTWR

#define UWB_ROLE DSTWR_ANCHOR
// #define UWB_ROLE DSTWR_TAG
// #define UWB_ROLE DSTWR_SNIFFER


#if (TEST_MODE == TEST_DSTWR)
    #if (UWB_ROLE == DSTWR_ANCHOR)
        #include "dstwr_anchor.h"
    #elif (UWB_ROLE == DSTWR_TAG)
        #include "dstwr_tag.h"
    #elif (UWB_ROLE == DSTWR_SNIFFER)
        #include "dstwr_sniffer.h"
    #endif
#endif


void setup() {

#if (TEST_MODE == TEST_DSTWR)
    #if (UWB_ROLE == DSTWR_ANCHOR)
        dstwr_anchor_main();
    #elif (UWB_ROLE == DSTWR_TAG)
        dstwr_tag_main();
    #elif (UWB_ROLE == DSTWR_SNIFFER)
        dstwr_sniffer_main();
    #endif
#endif

}

void loop() {

}