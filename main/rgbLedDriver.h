/*
  Matter Light example
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "ws2812.h" 

#include "matterNode.h"
#include "matterEndpoint.h"

class RgbLedDriver final
{
    Ws2812      _neoLed{static_cast<gpio_num_t>(8)};
public:
    RgbLedDriver(gpio_num_t pin);
    ~RgbLedDriver();

    void setOnOff(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data = nullptr);
    void setBrightness(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data = nullptr);
    void setHue(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data = nullptr);
    void setSaturation(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data = nullptr);
    void setTemperature(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data = nullptr);

    /// @brief set the hardware state from attributes in matter stack at startup
    void setDefaultValues(MatterEndpoint* ep);
};
