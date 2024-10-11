#ifndef __UROS_H__
#define __UROS_H__

#include <Arduino.h>
#include "dstwr_tag.h"

// WiFi info
#define WIFI_SSID "serene_2G"
#define WIFI_PSK "10541054"

#define UROS_LOG_TAG "UROS"
#define UROS_LOG_LEVEL ESP_LOG_INFO

// uRos info
#define UROS_AGENT_IP {192,168,1,12}
#define UROS_AGENT_PORT 8888

#define UROS_RANGE_PUB_NODE_NAME "uros_uwb_tag_range_pub"
#define UROS_RANGE_PUB_NAMESPACE ""
#define UROS_RANGE_PUB_TOPIC_NAME "uros_uwb_tag_range"



typedef struct {
    uint16_t short_address;
    float range;
} uros_uwb_anchor_range_t;

extern QueueHandle_t uros_range_queue;

extern void uros_range_pub_task_init();
extern void uros_range_pub_task(void *pvParameters);

#endif // __UROS_H__