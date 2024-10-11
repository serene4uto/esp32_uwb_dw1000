#ifndef __DSTWR_ANCHOR_H__
#define __DSTWR_ANCHOR_H__

#define DW_SPI_SCK 18
#define DW_SPI_MISO 19
#define DW_SPI_MOSI 23
#define DW_SPI_CS 4

#define DW_PIN_RST 27 // reset pin
#define DW_PIN_IRQ 34 // irq pin
#define DW_PIN_SS 4   // spi select pin

#define DSTWR_ANCHOR_LOG_TAG "DSTWR_ANCHOR"
#define DSTWR_ANCHOR_LOG_LEVEL ESP_LOG_INFO

#define DSTWR_ANCHOR_OPERATION_MODE DW1000.MODE_LONGDATA_RANGE_ACCURACY

#define DSTWR_ANCHOR_ADELAY_DEFAULT 16384
// leftmost two bytes below will become the "short address"
// #define DSTWR_ANCHOR_ADDR "84:01:22:EA:82:60:3B:9C"
// #define DSTWR_ANCHOR_ADELAY 16502

// #define DSTWR_ANCHOR_ADDR "84:02:22:EA:82:60:3B:9C"
// #define DSTWR_ANCHOR_ADELAY 16500

// #define DSTWR_ANCHOR_ADDR "84:03:22:EA:82:60:3B:9C"
// #define DSTWR_ANCHOR_ADELAY 16495

#define DSTWR_ANCHOR_ADDR "84:04:22:EA:82:60:3B:9C"
// #define DSTWR_ANCHOR_ADELAY 16502 


extern void dstwr_anchor_main();

#endif // __DSTWR_ANCHOR_H__