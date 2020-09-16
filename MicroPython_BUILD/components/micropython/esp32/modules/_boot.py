
import display
from machine import I2C
import axp202
import time

i2c = I2C(scl=22, sda=21)
pmu = axp202.PMU(i2c)
pmu.setLDO2Voltage(3300)
pmu.enablePower(axp202.AXP202_LDO2)
i2c.deinit()

tft = display.TFT()
tft.init()
tft.deinit()
