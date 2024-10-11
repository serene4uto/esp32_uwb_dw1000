#include <esp_log.h>
#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uwb_interfaces/msg/uwb_range.h>

#include "dstwr_tag.h"
#include "uros.h"

// Macro for error checking
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

uros_uwb_anchor_range_t received_anchor_distances[DSTWR_TAG_MAX_ANCHORS];

rcl_allocator_t rcl_rangepub_allocator;
rclc_support_t rcl_rangepub_support;
rcl_node_t rcl_rangepub_node;
rclc_executor_t rcl_rangepub_executor;

rcl_publisher_t rcl_rangepub_publisher;
uwb_interfaces__msg__UwbRange uwb_range_msg;

QueueHandle_t uros_range_queue = NULL;

IPAddress uRos_Agent_IP(UROS_AGENT_IP);
uint16_t uRos_Agent_Port = UROS_AGENT_PORT;

// Error handling loop
static void error_loop() {
    ESP_LOGE(DSTWR_TAG_LOG_TAG, "Error in uROS: %s", rcl_get_error_string().str);
    while (1) {
        delay(100);
    }
}

void uros_range_pub_task_init() {
    esp_log_level_set(UROS_LOG_TAG, UROS_LOG_LEVEL);

    set_microros_wifi_transports((char *)WIFI_SSID, (char *)WIFI_PSK, uRos_Agent_IP, uRos_Agent_Port);
    delay(2000);

    rcl_rangepub_allocator = rcl_get_default_allocator();
    
    // Initialize the support structure
    RCCHECK(rclc_support_init(&rcl_rangepub_support, 0, NULL, &rcl_rangepub_allocator));

    String uRos_topic_str;
    String uRos_node_name;
    if (strcmp(DSTWR_TAG_ADDRESS_STR, DSTWR_TAG_ADDRESS_STR_TAG1) == 0) {
        uRos_topic_str = String("tag1") + "___" + UROS_RANGE_PUB_TOPIC_NAME;
        uRos_node_name = String("tag1") + "___" + UROS_RANGE_PUB_NODE_NAME;
    } else if (strcmp(DSTWR_TAG_ADDRESS_STR, DSTWR_TAG_ADDRESS_STR_TAG2) == 0) {
        uRos_topic_str = String("tag2") + "___" + UROS_RANGE_PUB_TOPIC_NAME;
        uRos_node_name = String("tag2") + "___" + UROS_RANGE_PUB_NODE_NAME;
    }
    const char* uRos_topic_name_c = uRos_topic_str.c_str();
    const char* uRos_node_name_c = uRos_node_name.c_str();
    // Create a node
    RCCHECK(rclc_node_init_default(&rcl_rangepub_node, uRos_node_name_c, UROS_RANGE_PUB_NAMESPACE, &rcl_rangepub_support));


    RCCHECK(rclc_publisher_init_best_effort(
        &rcl_rangepub_publisher,
        &rcl_rangepub_node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(uwb_interfaces, msg, UwbRange),
        uRos_topic_name_c));

    // Create the executor
    RCCHECK(rclc_executor_init(&rcl_rangepub_executor, &rcl_rangepub_support.context, 1, &rcl_rangepub_allocator));

    uros_range_queue = xQueueCreate(10, sizeof(uros_uwb_anchor_range_t) * DSTWR_TAG_MAX_ANCHORS);

    ESP_LOGI(UROS_LOG_TAG, "uROS range publisher initialized");
}

void uros_range_pub_task(void *pvParameters) {

    while(1) {
        // Receive distances from the queue
        if(xQueueReceive(uros_range_queue, &received_anchor_distances, portMAX_DELAY) == pdTRUE) {


            uwb_range_msg.anchor_ids.data = (uint32_t *)malloc(DSTWR_TAG_MAX_ANCHORS * sizeof(uint32_t));
            uwb_range_msg.range_values.data = (float *)malloc(DSTWR_TAG_MAX_ANCHORS * sizeof(float));

            if (uwb_range_msg.anchor_ids.data == NULL || uwb_range_msg.range_values.data == NULL) {
                ESP_LOGE(UROS_LOG_TAG, "Memory allocation failed");
                while (1);
            }

            uwb_range_msg.anchor_ids.size = DSTWR_TAG_MAX_ANCHORS;
            uwb_range_msg.range_values.size = DSTWR_TAG_MAX_ANCHORS;
            uwb_range_msg.anchor_ids.capacity = DSTWR_TAG_MAX_ANCHORS;
            uwb_range_msg.range_values.capacity = DSTWR_TAG_MAX_ANCHORS;

            // Fill the message with received data
            for (int i = 0; i < DSTWR_TAG_MAX_ANCHORS; i++) {
            //   uwb_range_msg.anchor_ids.data[i] = i + 1;
                uwb_range_msg.anchor_ids.data[i] = received_anchor_distances[i].short_address;
                uwb_range_msg.range_values.data[i] = received_anchor_distances[i].range;
            }

            // Publish the message
            RCSOFTCHECK(rcl_publish(&rcl_rangepub_publisher, &uwb_range_msg, NULL));

            // Free the memory
            free(uwb_range_msg.anchor_ids.data);
            free(uwb_range_msg.range_values.data);
        }
    }
}




