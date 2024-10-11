#ifndef __DSTWR_TAG_H__
#define __DSTWR_TAG_H__

#define DSTWR_TAG_LOG_TAG "DSTWR_TAG"
#define DSTWR_TAG_LOG_LEVEL ESP_LOG_INFO

#define DSTWR_TAG_ADDRESS_STR "7D:01:22:EA:82:60:3B:9C"
#define DSTWR_TAG_OPERATION_MODE DW1000.MODE_LONGDATA_RANGE_ACCURACY

// connection pins
#define DW_SPI_SCK 18
#define DW_SPI_MISO 19
#define DW_SPI_MOSI 23
#define DW_SPI_CS 4

#define DW_PIN_RST 27
#define DW_PIN_IRQ 34

extern void dstwr_tag_main();

#endif // __DSTWR_TAG_H__