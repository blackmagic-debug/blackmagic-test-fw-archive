// SPDX-License-Identifier: BSD-3-Clause

#![allow(non_snake_case)]
#![warn(clippy::pedantic)]
#![no_std]
#![no_main]

use assign_resources::assign_resources;
use embassy_stm32::
{
    Config, Peri, Peripherals, mode::Blocking, peripherals, usart::{self, Uart}
};
use embassy_executor::Spawner;
// Magically inject the parts of the defmt machinary that are needed for doing defmt over RTT 🙃
use defmt_rtt as _;
// Magically inject #[panic_handler] so we get panic handling.. don't ask, it's absolutely magic how this can do that.
use panic_probe as _;

const UART_DATA: &'static str = "abcdefghijklmopqrstuvwxyz-0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ=";

assign_resources!
{
	uart: UartResources
	{
		peri: USART2,
		tx: PA2,
		rx: PA3,
	}
}

fn systemInit() -> Peripherals
{
	let config = Config::default();
	embassy_stm32::init(config)
}

fn uartInit(uart: UartResources) -> Uart<'static, Blocking>
{
	// Set up a configuration for this UART to run how we want it to
	let mut config = usart::Config::default();
	config.baudrate = 115200;
	// Configure for 8N1 operation
	config.data_bits = usart::DataBits::DataBits8;
	config.stop_bits = usart::StopBits::STOP1;
	config.parity = usart::Parity::ParityNone;
	// Make sure the TX pin is driven push-pull
	config.tx_config = usart::OutputConfig::PushPull;

	Uart::new_blocking(uart.peri, uart.rx, uart.tx, config)
		.expect("Failed to configure USART2")
}

#[embassy_executor::main]
async fn main(_spawner: Spawner)
{
	let peripherals = systemInit();
	let resources = split_resources!(peripherals);

	let mut uart = uartInit(resources.uart);

	loop
	{
		for char in UART_DATA.as_bytes()
		{
			uart.blocking_write(&[*char])
				.expect("Blocking UART write somehow failed - how the heck?!");
		}
	}
}
