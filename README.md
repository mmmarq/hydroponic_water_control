# hydroponic_water_control
System to control water flow into hydroponic system

This system was designed to control water flow through a hydroponic system.

Bill of materials:
- 1 Arduino Pro Micro
- 1 LDR
- 1 10k resistor
- 1 Red led
- 1 Yellow led
- 1 Green led
- 3 220ohms resistor
- 1 5v arduino relay
- 1 Hall effect water flow sensor
- 1 Water pump

Board connections:



              +-----------+
              |o         o|
              |o     GND o|--- GND --+
              |o         o|          |
              |o     VCC o|--- +5V  +++
 Water flow --|o D2      o|         | | 10K 
     *Relay --|o D3      o|         +++
  Green Led --|o D4      o|          |
 Yellow Led --|o D5   A0 o|----------+
    Red Led --|o D6      o|          |
              |o         o|         +++
              |o         o|         | | LDR
              |o         o|         +++
              +-----------+          |
                                     5V
* Relay should turn water pump on/off

This program follow the rules bellow:

At night (comparing light level detected by LDR and value defined at MINLIGHTLEVEL):
- Keep water flow for 15 minutes (PUMPONINTERVAL)
- Stop water flow for 120 minutes(PUMPOFFNIGHTINTERVAL)

Under day light:
- Keep water flow for 15 minutes (PUMPONINTERVAL)
- Stop water flow for 60 minutes(PUMPOFFDAYINTERVAL)

Feel free to change these values

Leds:
If Green led is constant on, system is running under day light
If Green led is blinking, system consider it is at night
If Yellow led is on, water pump is on
If Red led is on, no water flow has been detected after turn water pump on, please check water flow.

You can set DEBUG true/false to turn serial debug messages on/off

Thanks

Marcelo M

