/* SPDX-License-Identifier: Apache-2.0 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>

#include "status_leds.h"

#define HEARTBEAT_PERIOD_MS 500  /* toggles every 500 ms -> ~1 Hz blink */
#define FLASH_DURATION_MS   20   /* long enough to be visible, short enough
                                  * not to merge under sustained traffic
                                  */

static const struct gpio_dt_spec led_heartbeat =
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_uart_tx =
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led_uart_rx =
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led_adv =
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

static bool leds_ready;

static void heartbeat_fn(struct k_timer *timer);
static void tx_off_fn(struct k_timer *timer);
static void rx_off_fn(struct k_timer *timer);
static void adv_off_fn(struct k_timer *timer);

K_TIMER_DEFINE(heartbeat_timer, heartbeat_fn, NULL);
K_TIMER_DEFINE(tx_off_timer, tx_off_fn, NULL);
K_TIMER_DEFINE(rx_off_timer, rx_off_fn, NULL);
K_TIMER_DEFINE(adv_off_timer, adv_off_fn, NULL);

static void heartbeat_fn(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	gpio_pin_toggle_dt(&led_heartbeat);
}

static void tx_off_fn(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	gpio_pin_set_dt(&led_uart_tx, 0);
}

static void rx_off_fn(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	gpio_pin_set_dt(&led_uart_rx, 0);
}

static void adv_off_fn(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	gpio_pin_set_dt(&led_adv, 0);
}

static void flash(const struct gpio_dt_spec *led, struct k_timer *off_timer)
{
	if (!leds_ready) {
		return;
	}

	gpio_pin_set_dt(led, 1);
	/* Restarts the timer if a new event arrives while still lit, so the
	 * LED stays on through a burst rather than flickering per packet.
	 */
	k_timer_start(off_timer, K_MSEC(FLASH_DURATION_MS), K_NO_WAIT);
}

void status_leds_tx_activity(void)
{
	flash(&led_uart_tx, &tx_off_timer);
}

void status_leds_rx_activity(void)
{
	flash(&led_uart_rx, &rx_off_timer);
}

void status_leds_adv_report(void)
{
	flash(&led_adv, &adv_off_timer);
}

static int status_leds_init(void)
{
	const struct gpio_dt_spec *leds[] = {
		&led_heartbeat, &led_uart_tx, &led_uart_rx, &led_adv,
	};

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(leds[i])) {
			return -ENODEV;
		}

		if (gpio_pin_configure_dt(leds[i], GPIO_OUTPUT_INACTIVE) < 0) {
			return -EIO;
		}
	}

	leds_ready = true;
	k_timer_start(&heartbeat_timer, K_MSEC(HEARTBEAT_PERIOD_MS),
		      K_MSEC(HEARTBEAT_PERIOD_MS));

	return 0;
}

SYS_INIT(status_leds_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
