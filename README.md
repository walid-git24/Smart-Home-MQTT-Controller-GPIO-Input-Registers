# Smart-Home-MQTT-Controller-GPIO-Input-Registers

An ESP32-based smart home controller developed for **ASL National Arena — Challenge #02**. The system controls two independent output channels using local pushbuttons and supports global ON/OFF commands over MQTT.

## Project Overview

The controller provides two control methods:

- **Local button control:** each pushbutton independently toggles its corresponding LED.
- **MQTT control:** an MQTT `ON` or `OFF` command controls both output channels together.

The firmware reads the pushbutton states directly from the ESP32 GPIO input registers instead of using `digitalRead()`.

## Circuit Overview
 
**Simulation:** [Open the Wokwi Project](https://wokwi.com/projects/481021182644651009)

The circuit uses an ESP32, two pushbuttons, and two LEDs to represent the controlled output channels.

## Hardware and Pin Mapping

| Component | ESP32 GPIO | Function |
|---|---:|---|
| Button 1 (Red) | GPIO27 | Independently toggles LED 1 |
| Button 2 (Blue) | GPIO34 | Independently toggles LED 2 |
| LED 1 | GPIO26 | Output channel 1 |
| LED 2 | GPIO25 | Output channel 2 |

### Button wiring notes

- **GPIO27:** Button connected between GPIO27 and GND. The firmware uses `INPUT_PULLUP`.
- **GPIO34:** Button connected between GPIO34 and GND, with an external 10 kΩ pull-up resistor between GPIO34 and 3.3 V.
- GPIO34 is input-only and does not provide an internal pull-up or pull-down resistor. The external pull-up is therefore required.


## MQTT Configuration

| Setting | Value |
|---|---|
| Broker | `broker.hivemq.com` |
| Port | `1883` |
| Topic | `asl/msti-arena/ch02/ak-7f3a/command` |
| ON payload | `ON` |
| OFF payload | `OFF` |

Publish the exact payload `ON` or `OFF` to the configured topic:

- `ON` — turns both LEDs ON.
- `OFF` — turns both LEDs OFF.

The broker is public. Use a unique topic if deploying another instance to avoid topic collisions.

## Firmware and Implementation

Main firmware: [`sketch.ino`](sketch.ino)

The firmware uses:

- `WiFi.h` for Wi-Fi connectivity.
- `PubSubClient.h` for MQTT communication.
- `REG_READ(GPIO_IN_REG)` and `REG_READ(GPIO_IN1_REG)` for GPIO input-register reads.
- A 30 ms debounce interval to filter mechanical button bounce.
- A generated MQTT client identifier to reduce client-ID collisions.
- MQTT reconnection attempts when the client is disconnected.

### GPIO Register Reading

The firmware reads:

- GPIO27 from `GPIO_IN_REG`, using bit 27.
- GPIO34 from `GPIO_IN1_REG`, using bit 2.

This register-level approach satisfies the challenge requirement to read the button inputs through GPIO input registers rather than `digitalRead()`.

## System Architecture

```text
                 +----------------------+
                 |      MQTT Broker     |
                 |   broker.hivemq.com  |
                 +----------+-----------+
                            |
                       MQTT ON / OFF
                            |
+----------------+   +------v-------+   +------------------+
| Button 1       |-->|              |-->| LED 1 - GPIO26   |
| GPIO27         |   |              |   +------------------+
+----------------+   |     ESP32    |
                     |              |   +------------------+
+----------------+   |              |-->| LED 2 - GPIO25   |
| Button 2       |-->|              |   +------------------+
| GPIO34         |   +--------------+
+----------------+   GPIO register
                      reads + debounce
```

## Test Results

The following core functions were verified in Wokwi:

| Test | Result |
|---|---|
| GPIO register-based button detection | PASS |
| Button 1 independently toggles LED 1 | PASS |
| Button 2 independently toggles LED 2 | PASS |
| Wi-Fi connection | PASS |
| MQTT broker connection and subscription | PASS |
| MQTT `ON` controls both LEDs | PASS |
| MQTT `OFF` controls both LEDs | PASS |

Optional broker/network interruption and recovery tests were skipped. Physical-hardware tests were not performed.

## Repository Structure

```text
MSTI_Arena-ASL-02/
├── sketch.ino
├── diagram.json
├── libraries.txt
├── README.md
├── LICENSE
```

## Challenge Requirement Alignment

| Challenge requirement | Implementation |
|---|---|
| ESP32 controller | ESP32 |
| Two pushbutton inputs | GPIO27 and GPIO34 |
| Two output channels | GPIO26 and GPIO25 |
| GPIO input register reads | `REG_READ(GPIO_IN_REG)` and `REG_READ(GPIO_IN1_REG)` |
| Local button control | Each button independently toggles its assigned LED |
| MQTT control | `ON` / `OFF` commands control both LEDs |
| Wokwi simulation | Public Wokwi project and simulation evidence |

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
