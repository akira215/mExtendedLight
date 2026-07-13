/*
   Example code for matter component

  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "main.h"
#include "esp_log_level.h"
#include "matterAttribute.h"

#include <esp_err.h>
#include <esp_log.h>

#include "matterEndpoint.h"



static const char *TAG = "MainApp";
uint16_t light_endpoint_id = 0;


BlinkTask* Main::_ledBlinking = nullptr;


#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char decryption_key_start[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char decryption_key_end[] asm("_binary_esp_image_encryption_key_pem_end");

static const char *s_decryption_key = decryption_key_start;
static const uint16_t s_decryption_key_len = decryption_key_end - decryption_key_start;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA



Main App;

Main::Main()
{

     // Setting the log level for each module
    esp_log_level_set("MainApp", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("MatterNode", ESP_LOG_VERBOSE); 
    esp_log_level_set("MatterEndpoint", ESP_LOG_VERBOSE);
    esp_log_level_set("MatterCluster", ESP_LOG_VERBOSE);
    esp_log_level_set("EventLoop", ESP_LOG_DEBUG);
    esp_log_level_set("nvs", ESP_LOG_WARN); 
    esp_log_level_set("chip:DMG", ESP_LOG_WARN);
    esp_log_level_set("chip:EM", ESP_LOG_INFO);  
    esp_log_level_set("OPENTHREAD", ESP_LOG_INFO); 
    esp_log_level_set("cpu_start", ESP_LOG_WARN); 
    esp_log_level_set("data_model", ESP_LOG_WARN); 
    esp_log_level_set("mtr_nvs", ESP_LOG_WARN); 
    // Creating the Matter Node
    _mNode = MatterNode::getInstance();

}

// static
void Main::ledFlash(uint64_t speed)
{
    /*
    if(speed == 0) 
    {
        if(_ledBlinking){
        delete _ledBlinking;
        _ledBlinking = nullptr;
        }
        _led.off();
    } else if(speed == -1) {
        if(_ledBlinking){
        delete _ledBlinking;
        _ledBlinking = nullptr;
        }
        _led.on();
    } else {
        if(!_ledBlinking)
            _ledBlinking = new BlinkTask(_led, speed); // very short flash
        else
            _ledBlinking->setBlinkPeriod(speed);
    }
*/
}

//Static
void Main::shortPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    ESP_LOGV(TAG,"Toggling light - Short Press detected %ld -",id);  

    MatterAttribute* attrOnOff = static_cast<Main*>(handler_args)->getLightEndpoint()
                                    ->getCluster(CLUSTER_ID(OnOff))
                                    ->getAttribute(ATTRIBUTE_ID(OnOff, OnOff));

    bool current = attrOnOff->getValue();
    current = ! current;
    attrOnOff->updateValue(current);
}



//Static
void Main::longPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    ESP_LOGV(TAG,"Long Press detected %ld -",id);
    static_cast<Main*>(handler_args)->getNode()->factoryReset();
}


void Main::identifyHandler(MatterNode::identifyEvent_t event,  
                                    uint8_t effectId,                  
                                    uint8_t effectVariant,                     
                                    void* priv_data)
{
    ESP_LOGI(TAG,"Identifying event %u, effectId %u, effectVariant %u",(uint8_t)event, effectId, effectVariant);
}

void Main::setup(void)
{
    _rgbLed = new RgbLedDriver(static_cast<gpio_num_t>(CONFIG_PIN_LED));

    _button.enablePullup();
    _buttonTask = new ButtonTask (_button);
    _buttonTask->setShortPressHandler(&shortPressHandler, (void*)this);
    _buttonTask->setLongPressHandler(&longPressHandler,(void*)this);

    ENDPOINT_CONFIG(extended_color_light) light_config;
    light_config.on_off.on_off = DEFAULT_POWER;
    light_config.on_off_lighting.start_up_on_off = nullptr;
    light_config.level_control.current_level = DEFAULT_BRIGHTNESS;
    light_config.level_control.on_level = DEFAULT_BRIGHTNESS;
    light_config.level_control_lighting.start_up_current_level = DEFAULT_BRIGHTNESS;
    light_config.color_control.color_mode = (uint8_t)chip_cluster::ColorControl::ColorMode::kColorTemperature;
    light_config.color_control.enhanced_color_mode = (uint8_t)chip_cluster::ColorControl::ColorMode::kColorTemperature;
    light_config.color_control_color_temperature.start_up_color_temperature_mireds = nullptr;


    // Pass the SDK create function. The wrapper deduces the config type automatically.
    _light = App.getNode()->createEndpoint(&light_config);
    ESP_LOGE(TAG, "Akira Light created with endpoint_id %d", _light->getEndpointId());

    _light->registerIdentifyHandler(&Main::identifyHandler, this);


    MatterAttribute* attrOnOff = _light->getCluster(CLUSTER_ID(OnOff))
                                        ->getAttribute(ATTRIBUTE_ID(OnOff, OnOff));
    
    MatterCluster* clusterLevelControl = _light->getCluster(CLUSTER_ID(LevelControl));
    MatterAttribute* attrCurrentLevel = clusterLevelControl->getAttribute(ATTRIBUTE_ID(LevelControl, CurrentLevel));
    ESP_LOGE(TAG, "Got the Cluster %d - Attribute %d", clusterLevelControl->getClusterId(), attrCurrentLevel->getAttributeId());

    attrCurrentLevel->setDeferredPersistence();

    

    MatterCluster* clusterColorControl = _light->getCluster(CLUSTER_ID(ColorControl));
    MatterAttribute* attrCurrentX = clusterColorControl->getAttribute(ATTRIBUTE_ID(ColorControl, CurrentX));
    MatterAttribute* attrCurrentY = clusterColorControl->getAttribute(ATTRIBUTE_ID(ColorControl, CurrentY));
    MatterAttribute* attrColorTemp = clusterColorControl->getAttribute(ATTRIBUTE_ID(ColorControl, ColorTemperatureMireds));
    
    //cluster_t *cluster = cluster::get(endpoint, ColorControl::Id);
    FEATURE_CONFIG(color_control, hue_saturation) hueSaturation_cfg;
    hueSaturation_cfg.current_hue = DEFAULT_HUE;
    hueSaturation_cfg.current_saturation = DEFAULT_SATURATION;

    clusterColorControl->addFeature(&hueSaturation_cfg);  

    MatterAttribute* attrHue = clusterColorControl->getAttribute(ATTRIBUTE_ID(ColorControl, CurrentHue));
    MatterAttribute* attrSaturation = clusterColorControl->getAttribute(ATTRIBUTE_ID(ColorControl, CurrentSaturation));


    // Assuming 'endpoint' is your newly created endpoint pointer
    // 1. Create a Fixed Label Cluster (for permanent vendor tags)

    /*
    esp_matter::cluster::fixed_label::config_t fl_config;
    //esp_matter::cluster_t *fl_cluster = esp_matter::cluster::fixed_label::create(endpoint, &fl_config, CLUSTER_FLAG_SERVER);
    _light->createCluster(&fl_config, esp_matter::CLUSTER_FLAG_SERVER);
    */

    // 2. Create a User Label Cluster (for user-reconfigurable tags)
    esp_matter::cluster::user_label::adl_config_t ul_config;
    _light->createCluster(&ul_config, esp_matter::CLUSTER_FLAG_SERVER);
    //esp_matter::cluster_t *ul_cluster = esp_matter::cluster::user_label::create(endpoint, &ul_config, CLUSTER_FLAG_SERVER);

     // Crucial: Matter limits both label and value strings to exactly 16 characters max
    const char* default_label = "room";
    const char* default_value = "Unassigned";

    // 3. Construct a standard Matter/CHIP LabelStruct entry
    // This utilizes the underlying project-chip architecture embedded in ESP-Matter
    chip_cluster::UserLabel::Structs::LabelStruct::Type custom_label;
    custom_label.label = chip::CharSpan(default_label, strlen(default_label));
    custom_label.value = chip::CharSpan(default_value, strlen(default_value));

    // 4. Wrap it into an iterable array container (UserLabel dictates a List)
    // For a simple single default label, we allocate an array of size 1
    chip_cluster::UserLabel::Structs::LabelStruct::Type label_list_array[1];
    label_list_array[0] = custom_label;

    // 5. Wrap your native structure array into an ESP-Matter generic attribute value wrapper
    esp_matter_attr_val_t initial_val;
    initial_val.type = ESP_MATTER_VAL_TYPE_ARRAY;
    initial_val.val.a.b = (uint8_t *)label_list_array;
    initial_val.val.a.s = sizeof(label_list_array);


    esp_matter::attribute::update(
        _light->getEndpointId(), 
        CLUSTER_ID(UserLabel),                           // Cluster ID (0x0041)
        ATTRIBUTE_ID(UserLabel, LabelList),   // Attribute ID (0x0000)
        &initial_val
    );
    




    //MatterAttribute* attrSaturation = clusterColorControl->addAttribute(ATTRIBUTE_ID(ColorControl, CurrentSaturation),
    //                                                                esp_matter::ATTRIBUTE_FLAG_NONVOLATILE,
    //                                                                (uint8_t)(0));

    //MatterAttribute* attrHue = clusterColorControl->addAttribute(ATTRIBUTE_ID(ColorControl, CurrentHue),
    //                                                               esp_matter::ATTRIBUTE_FLAG_NONVOLATILE,
    //                                                                (uint8_t)(0));

    //MatterAttribute* attrSaturation = clusterColorControl->addAttribute(ATTRIBUTE_ID(ColorControl, CurrentSaturation),
    //                                                                esp_matter::ATTRIBUTE_FLAG_NONVOLATILE,
    //                                                                (uint8_t)(0));
    
    attrCurrentX->setDeferredPersistence();
    attrCurrentY->setDeferredPersistence();
    attrColorTemp->setDeferredPersistence();

    attrHue->setDeferredPersistence();
    attrSaturation->setDeferredPersistence();

    ESP_LOGI(TAG, "Current Level instance address: %p", attrCurrentLevel );
    ESP_LOGI(TAG, "Current Level Pointer variable stack location: %p", (void*)&attrCurrentLevel );

    ESP_LOGI(TAG, "Saturation instance address: %p", attrSaturation  );
    ESP_LOGI(TAG, "Saturation  Pointer variable stack location: %p", (void*)&attrSaturation );

    ESP_LOGI(TAG, "HUE instance address: %p", attrHue );
    ESP_LOGI(TAG, "HUE Pointer variable stack location: %p", (void*)&attrHue );
    // Connecting signals
    attrOnOff->registerAttrUpdateHandler(&RgbLedDriver::setOnOff, _rgbLed);
    attrCurrentLevel->registerAttrUpdateHandler(&RgbLedDriver::setBrightness, _rgbLed);

    attrHue->registerAttrUpdateHandler(&RgbLedDriver::setHue, _rgbLed);
    attrSaturation->registerAttrUpdateHandler(&RgbLedDriver::setSaturation, _rgbLed);
    attrColorTemp->registerAttrUpdateHandler(&RgbLedDriver::setTemperature, _rgbLed);

}

void Main::run(void)
{
    App.getNode()->start();

    // Set default value
    _rgbLed->setDefaultValues(_light);
}



extern "C" void app_main()
{
    App.setup();
    App.run();

}


