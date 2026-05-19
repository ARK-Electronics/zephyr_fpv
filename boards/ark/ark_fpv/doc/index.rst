.. zephyr:board:: ark_fpv

Overview
********

The `ARK FPV`_ is a flight controller built around the STM32H743IIK6
(Cortex-M7, 480 MHz, 2 MB flash, 1 MB SRAM). It carries an Invensense
IIM-42653 IMU on SPI1, a Bosch BMP390 barometer on I2C2, and an ST IIS2MDC
magnetometer on I2C4. A microSD slot, FDCAN1 transceiver, eight motor PWM
outputs, and seven UARTs round out the I/O.

This Zephyr board port targets customer-driven custom firmware running
directly on ARK FPV hardware. The toy application (``app/``) exercises the
bring-up of each subsystem (work in progress).

Hardware
========

- **MCU**: STMicroelectronics STM32H743IIK6 (UFBGA176)
- **Clock**: 16 MHz HSE → PLL1 → 480 MHz SYSCLK (VOS0)
- **Sensors**: IIM-42653 IMU, BMP390 barometer, IIS2MDC magnetometer
- **Storage**: microSD (SDMMC2 4-bit)
- **Bus**: FDCAN1 transceiver (TJA1051)
- **Motors**: 8× PWM on TIM5/TIM8 (M1–M8), plus an optional M9 on TIM4
- **UARTs**: 7 (USART1/2/3/6 + UART4/5/7), USART3 is the debug console
- **USB**: USB OTG-FS on PA11/PA12 (VBUS detect on PA9)

Supported features
============================

- Boot from internal flash at ``0x08000000`` (replaces the PX4 bootloader —
  see *Bootloader story* below).
- USART3 console on ``PD8`` (TX) / ``PD9`` (RX) at 115200 8N1.
- RGB status LEDs on ``PE3`` (red), ``PE4`` (green), ``PE5`` (blue) — all
  active-low, open-drain.
- 3.3 V sensor rail (``VDD_3V3_SENSORS1_EN`` on ``PI11``) is asserted at
  kernel start via a ``gpio-hog`` so later work can talk to sensors
  without any per-app initialisation.


Bootloader story
================

The factory PX4 bootloader at ``0x08000000`` – ``0x0801FFFF`` is replaced
by the Zephyr image, which loads bare-metal at ``0x08000000``. This is
the simplest topology for development.A future phase can introduce MCUboot
or preserve the PX4 boot protocol for OTA via the existing flashing tools.

Re-flashing the PX4 bootloader later requires only a normal SWD flash of
the PX4-bootloader binary at ``0x08000000``.

Programming and debugging
=========================

The SWD connector (J10) exposes ``FMU_SWDIO`` (``PA13``)
and ``FMU_SWCLK`` (``PA14``) plus the USART3 console pair. Any of the
runners ``openocd``, ``jlink``, ``pyocd``, or ``stm32cubeprogrammer`` will
work — pick whichever your debug probe supports.

Build and flash:

.. code-block:: console

   west build -p auto -b ark_fpv app
   west flash

After flash, the board prints a one-line banner on USART3 once per second
and toggles the blue status LED at 1 Hz.

References
==========

- ARK FPV product page: https://arkelectron.com/product/ark-fpv-flight-controller/
- STM32H743 reference manual (RM0433)
- ST datasheet (DS12110)

.. _ARK FPV: https://arkelectron.com/product/ark-fpv-flight-controller/
