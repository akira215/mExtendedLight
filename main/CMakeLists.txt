set(SOURCES main.cpp
            rgbLedDriver.cpp)

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS      "."
                       PRIV_INCLUDE_DIRS "../../common/utils"
                       REQUIRES matter ws2812 periodicTask cppgpio)

target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
