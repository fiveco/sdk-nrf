/* SPDX-License-Identifier: Apache-2.0 */

#ifndef STATUS_LEDS_H_
#define STATUS_LEDS_H_

/* Visual diagnostics on the DK's four LEDs, so board state can be read
 * without a debug probe attached:
 *
 *   LED1  heartbeat, ~1 Hz          -> MCU powered and scheduler running
 *   LED2  brief flash on UART TX    -> HCI traffic leaving towards the host
 *   LED3  brief flash on UART RX    -> HCI traffic arriving from the host
 *   LED4  brief flash per LE adv report -> radio actually receiving
 *
 * All flash helpers are ISR-safe.
 */

void status_leds_tx_activity(void);
void status_leds_rx_activity(void);
void status_leds_adv_report(void);

#endif /* STATUS_LEDS_H_ */
