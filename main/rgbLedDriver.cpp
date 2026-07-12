/*
  Matter Light example
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "rgbLedDriver.h"
#include "matterCluster.h"
#include "matterAttribute.h"


static const char *TAG = "RgbDriver";

RgbLedDriver::RgbLedDriver(gpio_num_t pin) : _neoLed(pin)
{

}

RgbLedDriver::~RgbLedDriver()
{

}

void RgbLedDriver::setOnOff(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data)
{
    if(event == esp_matter::attribute::POST_UPDATE)
        return;

    bool on = *val;
    if(on)
        _neoLed.setOn();
    else
        _neoLed.setOff();

    _neoLed.send();
}
void RgbLedDriver::setBrightness(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data)
{
    if(event == esp_matter::attribute::POST_UPDATE)
        return;
    
    uint8_t brightness = *val;
    brightness = (brightness * 255) / 254; // Matter reserve 255 for null
    _neoLed.setPixelBrightness(brightness);
    _neoLed.send();
}


void RgbLedDriver::setHue(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data)
{
    if(event == esp_matter::attribute::POST_UPDATE)
        return;
    
    uint8_t hue = *val;
    hue = (hue * 360) / 254; // Matter reserve 255 for null
    _neoLed.setPixelHue(hue);
    _neoLed.send();

}

void RgbLedDriver::setSaturation(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data)
{
    if(event == esp_matter::attribute::POST_UPDATE)
        return;

    uint8_t saturation = *val;
    saturation = (saturation * 255) / 254; // Matter reserve 255 for null
    _neoLed.setPixelSaturation(saturation);
    _neoLed.send();
}


void RgbLedDriver::setTemperature(MatterNode::attributeEvent_t event, MatterValue* val, void* priv_data)
{
    if(event == esp_matter::attribute::POST_UPDATE)
        return;
    
    uint16_t value = *val;

    uint32_t temperature = (1000000 / (value  ? value  : 1));
    
    _neoLed.setPixelTemperature(temperature);
    _neoLed.send();
}

void RgbLedDriver::setDefaultValues(MatterEndpoint* ep)
{
    // Brightness
    MatterAttribute* attrCurrentLevel = ep->getCluster(CLUSTER_ID(LevelControl))
                                            ->getAttribute(ATTRIBUTE_ID(LevelControl, CurrentLevel));
    MatterValue brightness = attrCurrentLevel->getValue();                                       
    setBrightness(esp_matter::attribute::PRE_UPDATE, &brightness);

    // Color
    MatterCluster* colorCluster = ep->getCluster(CLUSTER_ID(ColorControl));
    MatterValue colorMode = colorCluster->getAttribute(ATTRIBUTE_ID(ColorControl, ColorMode))->getValue();
    if ((uint8_t)colorMode == (uint8_t)chip_cluster::ColorControl::ColorMode::kCurrentHueAndCurrentSaturation) {
        // Setting hue 
        MatterValue hue =  colorCluster->getAttribute(ATTRIBUTE_ID(ColorControl, CurrentHue))->getValue();
        setHue(esp_matter::attribute::PRE_UPDATE, &hue);

        // Setting saturation 
        MatterValue saturation =  colorCluster->getAttribute(ATTRIBUTE_ID(ColorControl, CurrentSaturation))->getValue();
        setSaturation(esp_matter::attribute::PRE_UPDATE, &saturation);

    } else if ((uint8_t)colorMode == (uint8_t)chip_cluster::ColorControl::ColorMode::kColorTemperature) {
        // Setting temperature
        MatterValue temperature =  colorCluster->getAttribute(ATTRIBUTE_ID(ColorControl, ColorTemperatureMireds))->getValue();
        setTemperature(esp_matter::attribute::PRE_UPDATE, &temperature );
    } else {
        ESP_LOGE(TAG, "Color mode not supported");
    }

    // Set OnOff
    MatterValue onOff = ep->getCluster(CLUSTER_ID(OnOff))
                            ->getAttribute(ATTRIBUTE_ID(OnOff, OnOff))
                            ->getValue();
  
    setOnOff(esp_matter::attribute::PRE_UPDATE, &onOff);

}