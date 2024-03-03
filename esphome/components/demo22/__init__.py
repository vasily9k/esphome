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
    "binary_sensor",
    "number",
    "sensor",
    "switch",
    "text_sensor",
]

demo_ns = cg.esphome_ns.namespace("demo")
DemoBinarySensor = demo_ns.class_(
    "DemoBinarySensor", binary_sensor.BinarySensor, cg.PollingComponent
)
DemoNumber = demo_ns.class_("DemoNumber", number.Number, cg.Component)
DemoNumberType = demo_ns.enum("DemoNumberType", is_class=True)
DemoSensor = demo_ns.class_("DemoSensor", sensor.Sensor, cg.PollingComponent)
DemoSwitch = demo_ns.class_("DemoSwitch", switch.Switch, cg.Component)
DemoTextSensor = demo_ns.class_(
    "DemoTextSensor", text_sensor.TextSensor, cg.PollingComponent
)


NUMBER_TYPES = {
    1: DemoNumberType.TYPE_1,
    2: DemoNumberType.TYPE_2,
    3: DemoNumberType.TYPE_3,
}


CONF_NUMBERS = "numbers"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(
            CONF_BINARY_SENSORS,
            default=[
                {
                    CONF_NAME: "Demo Basement Floor Wet",
                    CONF_DEVICE_CLASS: DEVICE_CLASS_MOISTURE,
                },
                {
                    CONF_NAME: "Demo Movement Backyard",
                    CONF_DEVICE_CLASS: DEVICE_CLASS_MOTION,
                },
            ],
        ): [
            binary_sensor.binary_sensor_schema(DemoBinarySensor).extend(
                cv.polling_component_schema("60s")
            )
        ],
        cv.Optional(
            CONF_NUMBERS,
            default=[
                {
                    CONF_NAME: "Demo Number 0-100",
                    CONF_TYPE: 1,
                    CONF_MIN_VALUE: 0,
                    CONF_MAX_VALUE: 100,
                    CONF_STEP: 1,
                },
                {
                    CONF_NAME: "Demo Number -50-50",
                    CONF_TYPE: 2,
                    CONF_MIN_VALUE: -50,
                    CONF_MAX_VALUE: 50,
                    CONF_STEP: 5,
                },
                {
                    CONF_NAME: "Demo Number 40-60",
                    CONF_TYPE: 3,
                    CONF_MIN_VALUE: 40,
                    CONF_MAX_VALUE: 60,
                    CONF_STEP: 0.2,
                },
            ],
        ): [
            number.number_schema(DemoNumber)
            .extend(cv.COMPONENT_SCHEMA)
            .extend(
                {
                    cv.Required(CONF_TYPE): cv.enum(NUMBER_TYPES, int=True),
                    cv.Required(CONF_MIN_VALUE): cv.float_,
                    cv.Required(CONF_MAX_VALUE): cv.float_,
                    cv.Required(CONF_STEP): cv.float_,
                }
            )
        ],
        cv.Optional(
            CONF_SENSORS,
            default=[
                {
                    CONF_NAME: "Demo Plain Sensor",
                },
                {
                    CONF_NAME: "Demo Temperature Sensor",
                    CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
                    CONF_ICON: ICON_THERMOMETER,
                    CONF_ACCURACY_DECIMALS: 1,
                    CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
                    CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
                },
                {
                    CONF_NAME: "Demo Temperature Sensor",
                    CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
                    CONF_ICON: ICON_THERMOMETER,
                    CONF_ACCURACY_DECIMALS: 1,
                    CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
                    CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
                },
                {
                    CONF_NAME: "Demo Force Update Sensor",
                    CONF_UNIT_OF_MEASUREMENT: UNIT_PERCENT,
                    CONF_ACCURACY_DECIMALS: 0,
                    CONF_DEVICE_CLASS: DEVICE_CLASS_HUMIDITY,
                    CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
                    CONF_FORCE_UPDATE: True,
                },
                {
                    CONF_NAME: "Demo Energy Sensor",
                    CONF_UNIT_OF_MEASUREMENT: UNIT_WATT_HOURS,
                    CONF_ACCURACY_DECIMALS: 0,
                    CONF_DEVICE_CLASS: DEVICE_CLASS_ENERGY,
                    CONF_STATE_CLASS: STATE_CLASS_TOTAL_INCREASING,
                },
            ],
        ): [
            sensor.sensor_schema(DemoSensor, accuracy_decimals=0).extend(
                cv.polling_component_schema("60s")
            )
        ],
        cv.Optional(
            CONF_SWITCHES,
            default=[
                {
                    CONF_NAME: "Demo Switch 1",
                },
                {
                    CONF_NAME: "Demo Switch 2",
                    CONF_INVERTED: True,
                    CONF_ICON: ICON_BLUETOOTH,
                },
            ],
        ): [switch.switch_schema(DemoSwitch).extend(cv.COMPONENT_SCHEMA)],
        cv.Optional(
            CONF_TEXT_SENSORS,
            default=[
                {
                    CONF_NAME: "Demo Text Sensor 1",
                },
                {
                    CONF_NAME: "Demo Text Sensor 2",
                    CONF_ICON: ICON_BLUR,
                },
            ],
        ): [
            text_sensor.text_sensor_schema(DemoTextSensor).extend(
                cv.polling_component_schema("60s")
            )
        ],
    }
)


async def to_code(config):
    for conf in config[CONF_BINARY_SENSORS]:
        var = await binary_sensor.new_binary_sensor(conf)
        await cg.register_component(var, conf)

    for conf in config[CONF_NUMBERS]:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await number.register_number(
            var,
            conf,
            min_value=conf[CONF_MIN_VALUE],
            max_value=conf[CONF_MAX_VALUE],
            step=conf[CONF_STEP],
        )
        cg.add(var.set_type(conf[CONF_TYPE]))

    for conf in config[CONF_SENSORS]:
        var = await sensor.new_sensor(conf)
        await cg.register_component(var, conf)

    for conf in config[CONF_SWITCHES]:
        var = await switch.new_switch(conf)
        await cg.register_component(var, conf)

    for conf in config[CONF_TEXT_SENSORS]:
        var = await text_sensor.new_text_sensor(conf)
        await cg.register_component(var, conf)
