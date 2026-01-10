// SPDX-License-Identifier: BSD-3-Clause

#![allow(non_snake_case)]
#![warn(clippy::pedantic)]
#![no_std]
#![no_main]

use assign_resources::assign_resources;
use embassy_stm32::
{
    Config, Peri, Peripherals, peripherals,
};
use embassy_executor::Spawner;
// Magically inject the parts of the defmt machinary that are needed for doing defmt over RTT 🙃
use defmt_rtt as _;
// Magically inject #[panic_handler] so we get panic handling.. don't ask, it's absolutely magic how this can do that.
use panic_probe as _;

assign_resources!
{
	uart: UartResources
	{
		rx: PA2,
		tx: PA3,
	}
}

fn systemInit() -> Peripherals
{
	let config = Config::default();
	embassy_stm32::init(config)
}

#[embassy_executor::main]
async fn main(_spawner: Spawner)
{
	let peripherals = systemInit();
	let _resources = split_resources!(peripherals);
}
