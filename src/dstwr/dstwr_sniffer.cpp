#include <esp_log.h>
#include "dstwr_sniffer.h"
#include "DW1000.h"
#include "DW1000Ranging.h"

#define DSTWR_SNIFFER_LOG_TAG "DSTWR_SNIFFER"
#define DSTWR_SNIFFER_LOG_LEVEL ESP_LOG_INFO

#define DSTWR_SNIFFER_OPERATION_MODE DW1000.MODE_LONGDATA_RANGE_ACCURACY

// connection pins
#define DW_SPI_SCK 18
#define DW_SPI_MISO 19
#define DW_SPI_MOSI 23
#define DW_SPI_CS 4

#define DW_PIN_RST 27
#define DW_PIN_IRQ 34

volatile boolean DRAM_ATTR txDone = false;
volatile boolean DRAM_ATTR rxDone = false;
volatile boolean DRAM_ATTR rxError = false;

void startRx() {
	DW1000.newReceive();
	DW1000.setDefaults();
	// so we don't need to restart the receiver manually
	DW1000.receivePermanently(true);
	DW1000.startReceive();
}


void handleTxDone() {
  txDone = true;
}

void handleRxDone() {
  rxDone = true;
}

void handleRxError() {
  rxError = true;
}


int16_t detectMsgType(uint8_t t_data[]) {
  if(t_data[0] == FC_1_BLINK) {
  	return BLINK;
  }
  else if(t_data[0] == FC_1 && t_data[1] == FC_2) {
  	//we have a long MAC frame message (ranging init)
  	return t_data[LONG_MAC_LEN];
  }
  else if(t_data[0] == FC_1 && t_data[1] == FC_2_SHORT) {
  	//we have a short mac frame message (poll, range, range report, etc..)
  	return t_data[SHORT_MAC_LEN];
  }
}

void dstwr_sniffer_main() {

  esp_log_level_set(DSTWR_SNIFFER_LOG_TAG, DSTWR_SNIFFER_LOG_LEVEL);

  // initialize the driver
  DW1000.begin(DW_PIN_IRQ, DW_PIN_RST);
  DW1000.select(DW_SPI_CS);

	// general configuration
	DW1000.newConfiguration();
	DW1000.setDefaults();
	DW1000.enableMode(DSTWR_SNIFFER_OPERATION_MODE);
	DW1000.commitConfiguration();

  // attach callback for (successfully) sent and received messages
  DW1000.attachSentHandler(handleTxDone);
	DW1000.attachReceivedHandler(handleRxDone);
  DW1000.attachReceiveFailedHandler(handleRxError);

  // DEBUG chip info and registers pretty printed
  char msgInfo[128];
  DW1000.getPrintableDeviceIdentifier(msgInfo);
  ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Device ID: %s", msgInfo);
  DW1000.getPrintableExtendedUniqueIdentifier(msgInfo);
  ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Unique ID: %s", msgInfo);
  DW1000.getPrintableNetworkIdAndShortAddress(msgInfo);
  ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Network ID & Device Address: %s", msgInfo);
  DW1000.getPrintableDeviceMode(msgInfo);
  ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Device mode: %s", msgInfo);

  startRx();

  while(1) { 

    if (rxError) {
      ESP_LOGE(DSTWR_SNIFFER_LOG_TAG, "RX error");
      rxError = false;
      return;
    }

    if (rxDone) {
      rxDone = false;
      // read received data
      uint16_t rxDataLen = DW1000.getDataLength();
      uint8_t* rxData = (uint8_t*)malloc(rxDataLen);
      DW1000.getData(rxData, rxDataLen);

      int16_t msgType = detectMsgType(rxData);

      if (msgType == BLINK) {
        ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Received BLINK message");
      } else if (msgType == POLL) {
        ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Received POLL message");
      } else if (msgType == POLL_ACK) {
        ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Received POLL_ACK message");
      } else if (msgType == RANGE) {
        ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Received RANGE message");
      } else if (msgType == RANGE_REPORT) {
        ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Received RANGE_REPORT message");
      } else if (msgType == RANGE_FAILED) {
        ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Received RANGE_FAILED message");
      } else {
        ESP_LOGI(DSTWR_SNIFFER_LOG_TAG, "Received unknown message type");
      }
    }
  }
}