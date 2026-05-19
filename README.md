# ARK FPV — Zephyr Port

A Zephyr RTOS port for the [ARK FPV](https://arkelectron.com/product/ark-fpv/) flight controller (STM32H743II). Provides a board definition and sample application that exercises the available peripherals. Intended as a hardware-enablement platform for custom firmware.

## Prerequisites

Ubuntu 22.04 or 24.04.

```sh
sudo apt install git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget \
    python3-venv python3-dev python3-pip python3-setuptools \
    xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 \
    stlink-tools
```

Install the Zephyr SDK (v1.0.1+, required by Zephyr v4.4.0):

```sh
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v1.0.1/zephyr-sdk-1.0.1_linux-x86_64_minimal.tar.xz
tar xf zephyr-sdk-1.0.1_linux-x86_64_minimal.tar.xz -C ~/
~/zephyr-sdk-1.0.1/setup.sh -c -t arm-zephyr-eabi
```

## Workspace setup

```sh
python3 -m venv ~/zephyr-venv
source ~/zephyr-venv/bin/activate
pip install west

mkdir ~/zephyr_fpv_ws && cd ~/zephyr_fpv_ws
git clone git@github.com:ARK-Electronics/zephyr_fpv.git
west init -l zephyr_fpv
west update
pip install -r zephyr-v4.4.0/scripts/requirements.txt
```

Activate the venv (`source ~/zephyr-venv/bin/activate`) in any new terminal before running `west`.

The resulting layout:

```
~/zephyr_fpv_ws/
├── zephyr_fpv/          ← this repo (west manifest)
├── zephyr-v4.4.0/       ← pulled by west
└── modules/             ← pulled by west
```

## Build and flash

All build/flash commands run from the workspace root:

```sh
west build -b ark_fpv zephyr_fpv/app
```

Connect USB-C to the board for power, then flash with `st-flash` (STLink V2/V3 over SWD):

```sh
st-flash write build/zephyr/zephyr.bin 0x08000000
st-flash reset
```

Or use `west flash` with one of the supported runners (STM32CubeProgrammer, OpenOCD, J-Link, pyOCD):

```sh
west flash              # default: STM32CubeProgrammer
west flash -r openocd
west flash -r jlink
```

Console output is on USART3 (TX `PD8` / RX `PD9`) at 115200 baud. Connect a 3.3 V FTDI or similar USB-serial adapter.

## What the sample app does

- Prints a boot banner on USART3 and all bidirectional UARTs
- Blinks the blue status LED at 1 Hz
- Sweeps M1–M8 motor PWM across 1000–2000 us (do not connect ESCs)
- Echoes received bytes on each UART back to the sender

## Hardware

| Peripheral | Part / Interface | Status |
|---|---|---|
| UARTs (7) | USART1/2/3, UART4/5, USART6, UART7 | Working |
| Motor PWM (8) | TIM5 ch1-4, TIM8 ch1-4 | Working |
| LEDs | RGB on PE3/PE4/PE5 (active-low, open-drain) | Working |
| Sensor rail | 3.3 V LDO gated by `PI11` (gpio-hog, always on) | Working |
| CAN | FDCAN1 — TJA1051TK/3 transceiver, 120 Ohm on-board | Planned |
| SD card | SDMMC2, 4-bit, power-gated (`PC13`) | Planned |
| Barometer | BMP390 on I2C2 | Planned |
| Magnetometer | LIS2MDL on I2C4 | Planned |
| IMU | IIM-42653 on SPI1, DRDY on `PF2` | Planned |
| External SPI | SPI6 connector (CS `PI10`, DRDY `PD11`, nRST `PF10`) | Planned |

## Roadmap

1. **FDCAN1** — 1 Mbps classical CAN TX/RX (DroneCAN ecosystem)
2. **SD card** — FAT mount, file I/O for blackbox logging
3. **Barometer** — BMP390 pressure/temperature via Zephyr sensor API
4. **Magnetometer** — LIS2MDL 3-axis field via Zephyr sensor API
5. **IMU** — IIM-42653 gyro+accel at 1 kHz via SPI, DRDY-driven
6. **Integration app** — multi-threaded application tying all subsystems together

Future: USB CDC console, DroneCAN protocol stack, DSHOT, ADC battery monitoring, heater thermal control, MCUboot.
