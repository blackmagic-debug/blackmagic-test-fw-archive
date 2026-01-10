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
use defmt::info;
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
	use embassy_stm32::rcc::
	{
		AHBPrescaler, APBPrescaler, Pll, PllMul, PllPDiv, PllPreDiv, PllQDiv, PllSource, Sysclk
	};

	let mut config = Config::default();
	// Use the HSI as our PLL clock source
	config.rcc.hsi = true;
	config.rcc.pll_src = PllSource::HSI;
	// And the PLL as our SysClk source
	config.rcc.sys = Sysclk::PLL1_P;
	// Set up the PLL to spin our input clock from the HSI up to 96MHz
	config.rcc.pll = Some(
		Pll
		{
			prediv: PllPreDiv::DIV16,
			mul: PllMul::MUL336,
			divp: Some(PllPDiv::DIV4),
			divq: Some(PllQDiv::DIV7),
			divr: None
		}
	);
	// Set up the prescalers so we don't overdrive anything
	config.rcc.ahb_pre = AHBPrescaler::DIV1;
	config.rcc.apb1_pre = APBPrescaler::DIV2;
	config.rcc.apb2_pre = APBPrescaler::DIV1;

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

	info!("Initialising USART2 and starting TX exercises");
	let mut uart = uartInit(resources.uart);

	loop
	{
		for char in UART_DATA.as_bytes()
		{
			uart.blocking_write(&[*char])
				.expect("Blocking UART write somehow failed - how the heck?!");
		}
		info!("Block complete, looping");
	}
}
