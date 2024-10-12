#include <esp_log.h>
#include <Preferences.h>
#include "dstwr_anchor.h"
#include "DW1000.h"
#include "DW1000Ranging.h"

#define DSTWR_EXPO_MOVING_AVG_WINDOW_SIZE 3
#define DSTWR_EXPO_MOVING_AVG_ALPHA (2.0 / (DSTWR_EXPO_MOVING_AVG_WINDOW_SIZE + 1))

#define DSTWR_ANCHOR_CALIBRATION_TARGET_DISTANCE 0.3 // meters
#define DSTWR_ANCHOR_CALIBRATION_TRIALS 10
#define DSTWR_ANCHOR_CALIBRATION_STARTING_DELAY 16600
#define DSTWR_ANCHOR_CALIBRATION_INIT_STEP_SIZE 100
#define DSTWR_ANCHOR_CALIBRATION_LIMIT_STEP_SIZE 3 // minimum step size allowed, this mean smallest delay change possible

typedef enum {
    DSTWR_ANCHOR_MODE_NORMAL = 0,
    DSTWR_ANCHOR_MODE_CALIBRATE,
} dstwr_anchor_mode_t;

// choose mode for anchor
#define DSTWR_ANCHOR_MODE DSTWR_ANCHOR_MODE_NORMAL // uncomment this line to disable calibration mode
// #define DSTWR_ANCHOR_MODE DSTWR_ANCHOR_MODE_CALIBRATE // uncomment this line to enable calibration mode

volatile float DRAM_ATTR anchorCalibTargetDist = DSTWR_ANCHOR_CALIBRATION_TARGET_DISTANCE;
volatile dstwr_anchor_mode_t DRAM_ATTR dstwrAnchorMode = DSTWR_ANCHOR_MODE_NORMAL;
volatile uint16_t DRAM_ATTR currentAntDelay;
volatile uint16_t DRAM_ATTR currentAntDelayCalibStepSize = DSTWR_ANCHOR_CALIBRATION_INIT_STEP_SIZE; //initial binary search step size
volatile uint16_t DRAM_ATTR currentAntDelayCalibTrial = 0;
volatile uint16_t DRAM_ATTR trialAntDelay[DSTWR_ANCHOR_CALIBRATION_TRIALS] = {0};
volatile float DRAM_ATTR lastCalibDistError = 0.0;   
volatile uint8_t DRAM_ATTR currentTrial = 0;

volatile float DRAM_ATTR movingAvgDist = 0.0;

Preferences dstwrAnchorPrefs;

static void newRange()
{
    float newDist = DW1000Ranging.getDistantDevice()->getRange();
    

    /* normal mode */
    if (dstwrAnchorMode == DSTWR_ANCHOR_MODE_NORMAL) {

        /* Using an expo moving average to smooth the measurements */
        // movingAvgDist = (movingAvgDist * DSTWR_MOVING_AVG_WINDOW_SIZE + newDist) / (DSTWR_MOVING_AVG_WINDOW_SIZE + 1);
        movingAvgDist = (DSTWR_EXPO_MOVING_AVG_ALPHA * newDist) + ((1.0 - DSTWR_EXPO_MOVING_AVG_ALPHA) * movingAvgDist);
        
        ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "0x%04X, Dist: %f, Moving Avg: %f", DW1000Ranging.getDistantDevice()->getShortAddress(), newDist, movingAvgDist);
        ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Error: %f", newDist - anchorCalibTargetDist);
        ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Antenna delay: %d", currentAntDelay);
    }

    /* calibration mode */
    if (dstwrAnchorMode == DSTWR_ANCHOR_MODE_CALIBRATE) {

        if (currentAntDelayCalibStepSize < DSTWR_ANCHOR_CALIBRATION_LIMIT_STEP_SIZE) {
            trialAntDelay[currentTrial] = currentAntDelay;
            currentTrial++;

            if (currentTrial >= DSTWR_ANCHOR_CALIBRATION_TRIALS) {
                ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Calibration done");

                // average the trials
                uint32_t sum = 0;
                for (int i = 0; i < DSTWR_ANCHOR_CALIBRATION_TRIALS; i++) {
                    sum += trialAntDelay[i];
                }
                currentAntDelay = sum / DSTWR_ANCHOR_CALIBRATION_TRIALS;
                ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Average Calibrated Antenna delay: %d", currentAntDelay);
                ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Calibration done");
                DW1000.setAntennaDelay(currentAntDelay); 

                // save the calibrated antenna delay
                if(dstwrAnchorPrefs.begin("dstwr_anchor", false)) {
                    dstwrAnchorPrefs.putUShort("ant_delay", currentAntDelay);
                    dstwrAnchorPrefs.end();
                    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Calibrated Antenna delay saved");
                }
                else {
                    ESP_LOGE(DSTWR_ANCHOR_LOG_TAG, "Failed to save calibrated Antenna delay");
                }

                while(1);// stop here

                dstwrAnchorMode = DSTWR_ANCHOR_MODE_NORMAL; // switch back to normal mode
                return;
            }

            // Start over the calibration process
            ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Start new calibration");
            currentAntDelay = DSTWR_ANCHOR_CALIBRATION_STARTING_DELAY;
            currentAntDelayCalibStepSize = DSTWR_ANCHOR_CALIBRATION_INIT_STEP_SIZE;
            lastCalibDistError = 0.0;
            DW1000.setAntennaDelay(currentAntDelay);

            return;
        }

        float currentCalibDistError = newDist - anchorCalibTargetDist; //error in measured distance
        if ( (currentCalibDistError * lastCalibDistError) < 0.0) {
            currentAntDelayCalibStepSize = currentAntDelayCalibStepSize / 2; // sign change, reduce step size
        }
        lastCalibDistError = currentCalibDistError;

        if (currentCalibDistError > 0.0) {
            currentAntDelay = currentAntDelay + currentAntDelayCalibStepSize; // move delay to the right
        } else {
            currentAntDelay = currentAntDelay - currentAntDelayCalibStepSize; // move delay to the left
        }

        ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "0x%04X, %f", DW1000Ranging.getDistantDevice()->getShortAddress(), newDist);
        ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Trial %d, Antenna delay: %d, Distance error: %f", currentTrial, currentAntDelay, currentCalibDistError);

        DW1000.setAntennaDelay(currentAntDelay); // set the new antenna delay
    }
}

static void newDevice(DW1000Device *device)
{
    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Device added: %X", device->getShortAddress());
}

static void inactiveDevice(DW1000Device *device)
{
    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Inactive device removed: %X", device->getShortAddress());
}

void dstwr_anchor_main() {
    esp_log_level_set(DSTWR_ANCHOR_LOG_TAG, DSTWR_ANCHOR_LOG_LEVEL);

    dstwrAnchorMode = DSTWR_ANCHOR_MODE;

    // configure the chip
    DW1000Ranging.initCommunication(DW_PIN_RST, DW_SPI_CS, DW_PIN_IRQ);
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);

    // set the beginning antenna delay
    if (dstwrAnchorMode == DSTWR_ANCHOR_MODE_CALIBRATE) {
        currentAntDelay = DSTWR_ANCHOR_CALIBRATION_STARTING_DELAY;
    } else {
        if(dstwrAnchorPrefs.begin("dstwr_anchor", false)) {
            currentAntDelay = dstwrAnchorPrefs.getUShort("ant_delay", DSTWR_ANCHOR_ADELAY_DEFAULT);
            dstwrAnchorPrefs.end();
        }
    }
    DW1000.setAntennaDelay(currentAntDelay);

    //start the module as an anchor, do not assign random short address
    DW1000Ranging.startAsAnchor((char *)DSTWR_ANCHOR_ADDR, DSTWR_ANCHOR_OPERATION_MODE, false);

    // DEBUG chip info and registers pretty printed
    char msgInfo[128];
    DW1000.getPrintableDeviceIdentifier(msgInfo);
    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Device ID: %s", msgInfo);
    DW1000.getPrintableExtendedUniqueIdentifier(msgInfo);
    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Unique ID: %s", msgInfo);
    DW1000.getPrintableNetworkIdAndShortAddress(msgInfo);
    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Network ID & Device Address: %s", msgInfo);
    DW1000.getPrintableDeviceMode(msgInfo);
    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "Device mode: %s", msgInfo);


    ESP_LOGI(DSTWR_ANCHOR_LOG_TAG, "DSTWR Anchor started");

    while(1)
    {
        DW1000Ranging.loop();
    }
}