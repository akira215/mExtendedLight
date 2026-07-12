/*
   Example code for matter component

  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/


#pragma once

#include "matterNode.h"
#include "rgbLedDriver.h"

#include "cppgpio.h" // to del
#include "buttonTask.h" // to del
#include "blinkTask.h" // to del


/** Standard max values (used for remapping attributes) */
#define STANDARD_BRIGHTNESS 255
#define STANDARD_HUE 360
#define STANDARD_SATURATION 255
#define STANDARD_TEMPERATURE_FACTOR 1000000

/** Matter max values (used for remapping attributes) */
#define MATTER_BRIGHTNESS 254
#define MATTER_HUE 254
#define MATTER_SATURATION 254
#define MATTER_TEMPERATURE_FACTOR 1000000

/** Default attribute values used during initialization */
#define DEFAULT_POWER true
#define DEFAULT_BRIGHTNESS 64
#define DEFAULT_HUE 128
#define DEFAULT_SATURATION 254

class Main final
{
    GpioInput               _button {(gpio_num_t)CONFIG_PIN_BUTTON, true};
    ButtonTask*             _buttonTask = nullptr;

    static BlinkTask*       _ledBlinking;

    MatterNode*     _mNode      = nullptr;
    RgbLedDriver*   _rgbLed     = nullptr;
    MatterEndpoint* _light      = nullptr;

public:
    Main();
    void run(void);
    void setup(void);
    MatterNode* getNode() { return _mNode; }  

    static void shortPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
    static void longPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);

    void identifyHandler(MatterNode::identifyEvent_t event,  
                                    uint8_t effectId,                  
                                    uint8_t effectVariant,                     
                                    void* priv_data);

    MatterEndpoint* getLightEndpoint() { return _light; }

private:   
    /// @brief Helper to flash led
    /// @param speed flash cycle in ms. if 0, led will be set to off, 
    /// if -1 led will be switch on
    static void ledFlash(uint64_t speed);

}; // Main Class