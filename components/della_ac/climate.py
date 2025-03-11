"""Della AC climate component for ESPHome."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, uart
from esphome.const import (
    CONF_ID,
    CONF_UART_ID,
)

# Define configuration variables
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["climate"]

della_ac_ns = cg.esphome_ns.namespace("della_ac")
DellaAC = della_ac_ns.class_("DellaAC", climate.Climate, cg.Component)

# Schema for ESPHome YAML configuration
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(DellaAC),
        cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    }
).extend(cv.COMPONENT_SCHEMA)

# Generate C++ code from the YAML configuration
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    cg.add(var.set_uart(uart_component)) 