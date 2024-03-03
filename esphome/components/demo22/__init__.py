import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    binary_sensor,
    number,
    sensor,
    switch,
    text_sensor,
)
from esphome.const import (
    CONF_ACCURACY_DECIMALS,
    CONF_BINARY_SENSORS,
    CONF_DEVICE_CLASS,
    CONF_FORCE_UPDATE,
    CONF_ICON,
    CONF_ID,
    CONF_INVERTED,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_NAME,
    CONF_OUTPUT_ID,
    CONF_SENSORS,
    CONF_STATE_CLASS,
    CONF_STEP,
    CONF_SWITCHES,
    CONF_TEXT_SENSORS,
    CONF_TYPE,
    CONF_UNIT_OF_MEASUREMENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_MOISTURE,
    DEVICE_CLASS_MOTION,
    DEVICE_CLASS_TEMPERATURE,
    ICON_BLUETOOTH,
    ICON_BLUR,
    ICON_THERMOMETER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_WATT_HOURS,
)

AUTO_LOAD = [
    "sensor",
]

demo_ns = cg.esphome_ns.namespace("demo")
DemoSensor = demo_ns.class_("DemoSensor", sensor.Sensor, cg.PollingComponent)





CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DemoSensor),
        cv.Optional(
            CONF_SENSORS,
            default=[
                {
                    CONF_NAME: "Demo Temperature Sensor",
                    CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
                    CONF_ICON: ICON_THERMOMETER,
                    CONF_ACCURACY_DECIMALS: 1,
                    CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
                    CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
                },
            ],
        ): [
            sensor.sensor_schema(DemoSensor, accuracy_decimals=0).extend(
                cv.polling_component_schema("60s")
            )
        ],
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    for conf in config[CONF_SENSORS]:
        var = await sensor.new_sensor(conf)
        await cg.register_component(var, conf)

