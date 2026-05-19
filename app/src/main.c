/*
 * Copyright (c) 2026 ARK Electronics LLC
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sweep PWM duty across M1..M8 at 50 Hz servo PWM.
 *
 * SAFETY: don't connect ESCs while this is running — the duty cycle
 * walks across the full 1000..2000 µs throttle range continuously.
 *
 * USART1/2/4/5/6/7 still get banner
 * + RX echo and the blue LED still toggles. Adds a per-motor duty sweep
 * with the current pulse width echoed over the console.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

struct uart_port {
	const struct device *dev;
	const char *name;
	bool tx;
};

static const struct uart_port uarts[] = {
	{ DEVICE_DT_GET(DT_NODELABEL(usart1)), "usart1", true  },
	{ DEVICE_DT_GET(DT_NODELABEL(usart2)), "usart2", false },
	{ DEVICE_DT_GET(DT_NODELABEL(uart4)),  "uart4",  false },
	{ DEVICE_DT_GET(DT_NODELABEL(uart5)),  "uart5",  true  },
	{ DEVICE_DT_GET(DT_NODELABEL(usart6)), "usart6", true  },
	{ DEVICE_DT_GET(DT_NODELABEL(uart7)),  "uart7",  true  },
};

#define MOTOR(n) PWM_DT_SPEC_GET(DT_ALIAS(motor##n))
static const struct pwm_dt_spec motors[] = {
	MOTOR(1), MOTOR(2), MOTOR(3), MOTOR(4),
	MOTOR(5), MOTOR(6), MOTOR(7), MOTOR(8),
};

/* Standard servo range: 1 ms = min, 1.5 ms = neutral, 2 ms = max. */
#define PULSE_MIN_NS  PWM_USEC(1000)
#define PULSE_MAX_NS  PWM_USEC(2000)
#define PULSE_STEP_NS PWM_USEC(100)

static void uart_send(const struct device *dev, const char *s)
{
	for (; *s; s++) {
		uart_poll_out(dev, (uint8_t)*s);
	}
}

static void uart_drain(const struct device *dev)
{
	uint8_t c;

	while (uart_poll_in(dev, &c) == 0) {
		uart_poll_out(dev, c);
	}
}

int main(void)
{
	char banner[24];
	uint32_t pulse_ns = PULSE_MIN_NS;
	int direction = 1;

	printk("ARK FPV (Zephyr) - alive\n");

	(void)gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	for (size_t i = 0; i < ARRAY_SIZE(uarts); i++) {
		if (!device_is_ready(uarts[i].dev)) {
			printk("uart %s not ready\n", uarts[i].name);
		}
	}
	for (size_t i = 0; i < ARRAY_SIZE(motors); i++) {
		if (!pwm_is_ready_dt(&motors[i])) {
			printk("motor%zu PWM not ready\n", i + 1);
		}
	}

	while (1) {
		for (size_t i = 0; i < ARRAY_SIZE(uarts); i++) {
			uart_drain(uarts[i].dev);
			if (uarts[i].tx) {
				snprintf(banner, sizeof(banner),
					 "%s\r\n", uarts[i].name);
				uart_send(uarts[i].dev, banner);
			}
		}

		for (size_t i = 0; i < ARRAY_SIZE(motors); i++) {
			int ret = pwm_set_dt(&motors[i],
					     motors[i].period, pulse_ns);
			if (ret < 0) {
				printk("motor%zu pwm_set failed: %d\n",
				       i + 1, ret);
			}
		}
		printk("motor pulse = %u us\n", pulse_ns / 1000U);

		pulse_ns += (uint32_t)direction * PULSE_STEP_NS;
		if (pulse_ns >= PULSE_MAX_NS) {
			pulse_ns = PULSE_MAX_NS;
			direction = -1;
		} else if (pulse_ns <= PULSE_MIN_NS) {
			pulse_ns = PULSE_MIN_NS;
			direction = 1;
		}

		gpio_pin_toggle_dt(&led);
		k_sleep(K_MSEC(1000));
	}

	return 0;
}
