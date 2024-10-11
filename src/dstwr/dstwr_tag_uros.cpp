// #include <Arduino.h>
// #include <esp_log.h>

// #include "dstwr_tag.h"
// #include "DW1000.h"
// #include "DW1000Ranging.h"

// // uint8_t dstwrAnchorRxCnt = 0;
// // float dstwrCurrentRange = 0.0;

// static void new_range_cb() {
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "%02X, %02f", DW1000Ranging.getDistantDevice()->getShortAddress(), DW1000Ranging.getDistantDevice()->getRange());
// }

// static void new_anchor_cb(DW1000Device *device) {
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "Device added: %X", device->getShortAddress());
// }

// static void inactive_anchor_cb(DW1000Device *device) {
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "Inactive device removed: %X", device->getShortAddress());
// }


// void dstwr_tag_main() {

//     esp_log_level_set(DSTWR_TAG_LOG_TAG, DSTWR_TAG_LOG_LEVEL);

//     DW1000Ranging.initCommunication(DW_PIN_RST, DW_SPI_CS, DW_PIN_IRQ);
//     DW1000Ranging.attachNewRange(new_range_cb);
//     DW1000Ranging.attachNewDevice(new_anchor_cb);
//     DW1000Ranging.attachInactiveDevice(inactive_anchor_cb);
//     DW1000Ranging.startAsTag((char *)DSTWR_TAG_ADDRESS_STR, DSTWR_TAG_OPERATION_MODE, false);

//     // DEBUG chip info and registers pretty printed
//     char msgInfo[128];
//     DW1000.getPrintableDeviceIdentifier(msgInfo);
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "Device ID: %s", msgInfo);
//     DW1000.getPrintableExtendedUniqueIdentifier(msgInfo);
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "Unique ID: %s", msgInfo);
//     DW1000.getPrintableNetworkIdAndShortAddress(msgInfo);
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "Network ID & Device Address: %s", msgInfo);
//     DW1000.getPrintableDeviceMode(msgInfo);
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "Device mode: %s", msgInfo);
//     uint16_t current_Adelay = DW1000.getAntennaDelay();
//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "Antenna delay: %d", current_Adelay);


//     ESP_LOGI(DSTWR_TAG_LOG_TAG, "DSTWR Tag started");

//     while(1)
//     {
//         DW1000Ranging.loop();
//     }
// }