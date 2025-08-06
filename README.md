# 3D Print Bed Thermostat

A touchscreen-based thermostat system using an Arduino Due, designed to control a heating pad (e.g. for 3D printers) with a custom thermistor calibration and solid-state relay switching.

## Features

- 2.4" TFT touchscreen (ILI9341) with resistive touch
- -5°C / -1°C / +1°C / +5°C / Start / Stop buttons for control
- Real-time temperature display
- Custom Steinhart-Hart thermistor calibration
- Bang-bang heating control logic
- SSR-based heater switching

## Hardware

- Arduino Due with TFT Shield
- 100kΩ NTC Thermistor + voltage divider
- Solid State Relay (SSR) + heatsink
- Meanwell RD-65A PSU
- 120 V, 750 W heating pad with integrated thermistor

## Thermistor Calibration

- 12-point Steinhart-Hart equation (see `test/`)
- Accurate temperature range: 20°C – 150°C

## Circuit Notes

- Thermistor measured via voltage divider
- SSR controlled via Arduino digital output
- Capacitor filtering used to reduce ADC noise

## Getting Started

1. Upload the Arduino sketch from `src/`
2. Wire thermistor, SSR, and power supply
3. Power the Arduino via 12V input
4. Use the touchscreen to adjust and monitor temperature

## Future Improvements

- Add fans for active cooling and airflow
- Design and print a custom enclosure
- Incorporate a power rocker switch for safety
- Upgrade to PID control for smoother temperature regulation

## License

MIT License

---

For documentation and schematics, see `docs/` or contact the author.
