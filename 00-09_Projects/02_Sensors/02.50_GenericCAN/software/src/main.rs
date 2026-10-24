#![no_std]
#![no_main]

use embassy_executor::{Spawner, task};
use embassy_stm32::adc::SampleTime;
use embassy_stm32::can::Frame;
use embassy_stm32::pac;
use embassy_stm32::peripherals::CAN;
use embassy_stm32::{bind_interrupts, can};
use embassy_sync::blocking_mutex::raw::CriticalSectionRawMutex;
use embassy_sync::channel::{Channel, Sender};
use embassy_time::Timer;
use {defmt_rtt as _, panic_probe as _};

use lhrdb::messages;

const QUEUE_SIZE: usize = 1024;

bind_interrupts!(struct Irqs {
    CAN_RX0  => can::Rx0InterruptHandler<CAN>;
    CAN_RX1 => can::Rx1InterruptHandler<CAN>;
    CAN_TX => can::TxInterruptHandler<CAN>;
    CAN_SCE => can::SceInterruptHandler<CAN>;
});

#[embassy_executor::main]
async fn main(spawner: Spawner) -> ! {
    let p = embassy_stm32::init(Default::default());

    let mut can = can::Can::new(p.CAN, p.PA11, p.PA12, Irqs);

    can.set_bitrate(1_000_000);

    let msg = messages::Id360::new(100., 100., 100., 100).unwrap();
    let frame = Frame::new_standard(0x363, msg.raw()).unwrap();
    can.write(&frame).await;
    can.flush_all().await;

    // Initialize ADC
    pac::RCC.apb2enr().modify(|w| {
        w.set_adcen(true);
    });

    pac::ADC1.cr2().modify(|w| {
        w.set_cont(false);
        w.set_adon(true);
        w.set_dma(false);
        w.set_align(false);
    });

    pac::ADC1.smpr2().modify(|w| {
        w.set_smp0(SampleTime::CYCLES7_5);
        w.set_smp1(SampleTime::CYCLES7_5);
        w.set_smp2(SampleTime::CYCLES7_5);
        w.set_smp3(SampleTime::CYCLES7_5);
    });

    pac::ADC1.sqr1().modify(|w| {
        w.set_l(0);
    });

    // Initialize Write Channel
    static WRITE_CHANNEL: Channel<CriticalSectionRawMutex, Frame, QUEUE_SIZE> = Channel::new();

    spawner
        .spawn(adc_conversion(WRITE_CHANNEL.sender()))
        .unwrap();

    loop {
        let frame = WRITE_CHANNEL.receive().await;
        can.write(&frame).await;
        can.flush_all().await;
    }
}

#[task]
async fn adc_conversion(
    queue_sender: Sender<'static, CriticalSectionRawMutex, Frame, QUEUE_SIZE>,
) -> ! {
    loop {
        // Start with channel 0
        pac::ADC1.sqr3().modify(|w| {
            w.set_sq1(0);
        });
        // Write to adon starts conversion
        pac::ADC1.cr2().modify(|w| {
            w.set_adon(true);
        });
        // Wait until conversion is complete
        while !pac::ADC1.sr().read().eoc() {}

        // Gather data
        let conversion_data = pac::ADC1.dr().read().data();

        let payload: [u8; 2] = [conversion_data as u8, (conversion_data << 8) as u8];
        let frame = Frame::new_standard(0x400, &payload).unwrap();
        queue_sender.send(frame).await;

        Timer::after_millis(1000).await;
    }
}
