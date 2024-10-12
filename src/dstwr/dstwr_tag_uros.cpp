#include <Arduino.h>
#include <esp_log.h>

#include "dstwr_tag.h"
#include "DW1000.h"
#include "DW1000Ranging.h"

#include "uros.h"

uros_uwb_anchor_range_t last_anchor_distance[DSTWR_TAG_MAX_ANCHORS];

static void dstwr_tag_uros_tag_task(void *pvParameters);

static void new_range_cb() {
    DW1000Device *anchorDevice = NULL;
    uint8_t numAnchorsToSend = DSTWR_TAG_MAX_ANCHORS;


    // ESP_LOGI(DSTWR_TAG_LOG_TAG, "0x%02X, %02f", anchorDevice->getShortAddress(), anchorDevice->getRange());

    if (DW1000Ranging.getNetworkDevicesNumber() < DSTWR_TAG_MIN_ANCHORS) {
        ESP_LOGI(DSTWR_TAG_LOG_TAG, "Not enough anchors");
        return;
    }

    if (DW1000Ranging.getNetworkDevicesNumber() < DSTWR_TAG_MAX_ANCHORS) {
        numAnchorsToSend = DW1000Ranging.getNetworkDevicesNumber();
    }

    for (int i = 0; i < numAnchorsToSend; i++) {
        anchorDevice = &DW1000Ranging.getNetworkDevices()[i];
        // print time, short address and range
        // ESP_LOGI(DSTWR_TAG_LOG_TAG, "%d, 0x%02X, %02f", anchorDevice->getLastActivity(), anchorDevice->getShortAddress(), anchorDevice->getRange());

        //TODO: sort to get most recent anchors
        last_anchor_distance[i].short_address = anchorDevice->getShortAddress();
        last_anchor_distance[i].range = anchorDevice->getRange();

        if (anchorDevice->getRange() < 0) {
            ESP_LOGI(DSTWR_TAG_LOG_TAG, "Range failed");
            return;
        }

        last_anchor_distance[i].range = anchorDevice->getRange();
    }

    // ESP_LOGI(DSTWR_TAG_LOG_TAG, "-----------------");

    xQueueSend(uros_range_queue, last_anchor_distance, portMAX_DELAY);

}

static void new_anchor_cb(DW1000Device *device) {
    ESP_LOGI(DSTWR_TAG_LOG_TAG, "Device added: %X", device->getShortAddress());
}

static void inactive_anchor_cb(DW1000Device *device) {
    ESP_LOGI(DSTWR_TAG_LOG_TAG, "Inactive device removed: %X", device->getShortAddress());
}

void dstwr_tag_uros_tag_task_init() {
    esp_log_level_set(DSTWR_TAG_LOG_TAG, DSTWR_TAG_LOG_LEVEL);

    DW1000Ranging.initCommunication(DW_PIN_RST, DW_SPI_CS, DW_PIN_IRQ);
    DW1000Ranging.attachNewRange(new_range_cb);
    DW1000Ranging.attachNewDevice(new_anchor_cb);
    DW1000Ranging.attachInactiveDevice(inactive_anchor_cb);
    DW1000Ranging.startAsTag((char *)DSTWR_TAG_ADDRESS_STR, DSTWR_TAG_OPERATION_MODE, false);

    // DEBUG chip info and registers pretty printed
    char msgInfo[128];
    DW1000.getPrintableDeviceIdentifier(msgInfo);
    ESP_LOGI(DSTWR_TAG_LOG_TAG, "Device ID: %s", msgInfo);
    DW1000.getPrintableExtendedUniqueIdentifier(msgInfo);
    ESP_LOGI(DSTWR_TAG_LOG_TAG, "Unique ID: %s", msgInfo);
    DW1000.getPrintableNetworkIdAndShortAddress(msgInfo);
    ESP_LOGI(DSTWR_TAG_LOG_TAG, "Network ID & Device Address: %s", msgInfo);
    DW1000.getPrintableDeviceMode(msgInfo);
    ESP_LOGI(DSTWR_TAG_LOG_TAG, "Device mode: %s", msgInfo);
    uint16_t current_Adelay = DW1000.getAntennaDelay();
    ESP_LOGI(DSTWR_TAG_LOG_TAG, "Antenna delay: %d", current_Adelay);

    ESP_LOGI(DSTWR_TAG_LOG_TAG, "DSTWR Tag started");
}

static void dstwr_tag_uros_tag_task(void *pvParameters) {
    while(1)
    {
        DW1000Ranging.loop();
    }
}

void dstwr_tag_uros_main() {

    uros_range_pub_task_init(); // first

    dstwr_tag_uros_tag_task_init();

    xTaskCreate(uros_range_pub_task, "uros_range_pub_task", 4096, NULL, 5, NULL);
    xTaskCreate(dstwr_tag_uros_tag_task, "dstwr_tag_uros_tag_task", 4096, NULL, 5, NULL);

    while(1)
    {
        delay(100);
    }
}