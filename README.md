# MPU6050 microSD Logger

An STM32F401RE project that acquires MPU6050 measurements over I2C using DMA, buffers them into 512-byte blocks, and stores them on a microSD card over SPI. A UART shell controls logging, displays sensor values, reads recorded blocks, and clears the current session's logs.

**Author:** Omar Ahmed  
**Target:** NUCLEO-F401RE / STM32F401RE  
**Development environment:** STM32CubeIDE and STM32CubeMX, using STM32 HAL

## Features

- MPU6050 identification and initialization with status codes for configuration failures.
- Timer-triggered I2C DMA acquisition of acceleration, temperature, and angular velocity.
- Separate sensor accumulation and SD write buffers.
- Raw 512-byte SD block writes and reads over SPI.
- Separate write-completion polling and read-token polling in the main loop to not block sensor sampling.
- UART command reception using circular DMA and receive-to-idle events.
- Decoded sensor output in g, degrees per second, and degrees Celsius.
- Commands for status, pause/resume, live display, reading, and clearing logs.

## Hardware and connections

Required hardware: the STM32 board, an MPU6050 module, a microSD card with an SPI-compatible interface module, and a USB connection for programming and serial communication.

The following assignments come from the project's peripheral and GPIO configuration:

| Interface | Signal | STM32 pin | Connect to |
| --- | --- | --- | --- |
| I2C1 | SCL | PB6 | MPU6050 SCL |
| I2C1 | SDA | PB7 | MPU6050 SDA |
| SPI1 | SCK | PB3 | SD SCK/CLK |
| SPI1 | MISO | PA6 | SD MISO/DO |
| SPI1 | MOSI | PA7 | SD MOSI/DI |
| GPIO | SD_CS | PA4 | SD CS |
| USART2 | TX | PA2 | Serial adapter RX, if using an external adapter |
| USART2 | RX | PA3 | Serial adapter TX, if using an external adapter |
| GPIO | Activity output | PA5 | Board LED/activity indication |

Connect a common ground. Use module supply voltages and signal levels compatible with the actual hardware; the SD module's supply requirement depends on its regulator and interface circuitry. The I2C pins are configured without internal pull-ups, so suitable external or module-provided pull-ups are needed. The MPU6050 address is configured as `0x68`; wire its address selection accordingly.

**SPI clock is on PB3, not PA5.** PA5 is used as a GPIO activity output in this project.

## Build and run

1. Open STM32CubeIDE and import the repository as an existing project.
2. Review `MPU6050-microSD-Logger.ioc` and the wiring table before connecting the modules.
3. Build the project and flash the STM32 through ST-LINK.
4. Open the board's serial port with these settings:
   - 115200 baud
   - 8 data bits, no parity, 1 stop bit
   - Send commands with a carriage return or newline
5. Enter `help` to list commands.
6. Wait for a completed block write before requesting `read block 100000`.

The project includes the FatFs middleware pack and calls `MX_FATFS_Init()`. The logger's storage path in `sd.c` uses direct SPI block commands rather than FatFs file operations.

Peripheral initialization has been separated into `peripheral_config.c`. Review generated changes when using CubeMX so initialization functions and HAL handles are not duplicated.

## Configuration and timing

| Setting | Current source configuration |
| --- | --- |
| System clock | 84 MHz, based on the configured 8 MHz HSE input |
| I2C1 | 100 kHz, 7-bit addressing |
| MPU6050 address | `0x68` |
| Accelerometer range | ±2 g |
| Gyroscope range | ±250 degrees/s |
| Sensor configuration | DLPF_CFG = 3, SMPLRT_DIV = 199 |
| Sensor register update rate | 5 Hz with the above configuration |
| TIM2 sample requests | 2 Hz / every 500 ms |
| SPI1 | Mode 0, 8-bit, MSB first, software chip select |
| SPI initialization clock | 84 MHz / 256 ≈ 328 kHz |
| SPI clock after SD initialization | 84 MHz / 8 = 10.5 MHz |
| First log block | `SD_START_BLOCK = 100000` |

TIM2 uses prescaler 8399 and period 4999. With its 84 MHz timer clock, the interval is 500 ms. The sensor's internal update rate and the application's read-request rate are separate settings.

At two successfully logged frames per second, a 36-frame block fills in approximately 18 seconds. Blocking work, pauses, and transfer failures can increase that interval.

## Data flow

```text
TIM2 sample request
        |
        v
MPU6050 -- I2C DMA --> i2c_rx_buffer (14 bytes)
                              |
                              v
                    mpu_log_buffer (512 bytes)
                    36 frames + 8 padding bytes
                              |
                              v
                    sd_write_buffer (512 bytes)
                              |
                              v
                     SPI CMD24 block write
                              |
                              v
                    Completion polling + CMD13
                              |
                              v
                    Advance sd_next_block

Shell read request --> SPI CMD17 --> sd_read_buffer
                                          |
                                          v
                              Decode and print over UART
```

I2C receive completion sets a flag for main to consume the frame. Once the accumulation buffer is full, main copies it to the available SD write buffer and resets the accumulation index.

SD payload transfers use blocking SPI calls. The main loop polls separately while waiting for card programming or a read data token.

## Stored block format

Each log block contains 36 consecutive 14-byte frames followed by eight `0xFF` padding bytes.

| Block byte range | Contents |
| --- | --- |
| 0–13 | Frame 0 |
| 14–27 | Frame 1 |
| … | … |
| 490–503 | Frame 35 |
| 504–511 | Padding: eight bytes of `0xFF` |

Each frame contains seven signed 16-bit values, stored high byte first:

| Frame offsets | Measurement | Conversion |
| --- | --- | --- |
| 0–1 | Acceleration X | raw / 16384.0 → g |
| 2–3 | Acceleration Y | raw / 16384.0 → g |
| 4–5 | Acceleration Z | raw / 16384.0 → g |
| 6–7 | Temperature | raw / 340.0 + 36.53 → °C |
| 8–9 | Angular velocity X | raw / 131.0 → degrees/s |
| 10–11 | Angular velocity Y | raw / 131.0 → degrees/s |
| 12–13 | Angular velocity Z | raw / 131.0 → degrees/s |

The decoder uses fixed offsets and frame lengths. It does not search for `0xFF` separators, because `0xFF` can also occur in valid measurements. The block format contains no timestamps or persistent session metadata.

## Shell commands

| Command | Action |
| --- | --- |
| `help` | Print command usage. |
| `status` | Print SD initialization/card information, addressing mode, block counters, and logging state. |
| `read block <number>` | Reads and decode a block recorded in the current session. |
| `pause sensor logging` | Pauses new SD writes; an active write is allowed to finish. |
| `resume sensor logging` | Resumes logging. |
| `display live sensor` | Enables repeated display of the latest sensor buffer. |
| `pause live sensor` | Disables live display. |
| `clear sd` | Zeros the current session's recorded blocks and reset logging buffers and counters. |

Example session:

```text
help
status
pause sensor logging
read block 100000
resume sensor logging
```

The accepted read range is `SD_START_BLOCK <= block < sd_next_block`. A block must have completed writing before it can be requested through the shell.

`clear sd` overwrites the recorded block range with zeros; it does not format the card. It returns `SD_BUSY` if a checked transfer is active, so retry after that transfer finishes. Clearing runs synchronously and can take longer as the recorded range grows. If it fails partway through, some blocks may already have been overwritten.

## Source layout

| Files | Responsibility |
| --- | --- |
| `Core/Src/main.c`, `Core/Inc/main.h` | Startup, main-loop coordination, timer callback, shared pin definitions. |
| `Core/Src/peripheral_config.c`, `Core/Inc/peripheral_config.h` | Clock/peripheral initialization and shared HAL handles. |
| `Core/Src/mpu6050.c`, `Core/Inc/mpu6050.h` | Sensor registers, initialization, I2C DMA sampling, callbacks, and live conversion. |
| `Core/Src/sd.c`, `Core/Inc/sd.h` | SPI SD initialization, block transfers, polling, status strings, and log clearing. |
| `Core/Src/shell.c`, `Core/Inc/shell.h` | UART command reception, parsing, formatted output, status, and block decoding. |
| `Core/Src/stm32f4xx_hal_msp.c` | Peripheral clocks, alternate-function pins, DMA links, and interrupt setup. |
| `Core/Src/stm32f4xx_it.c` | Interrupt handlers forwarding events to HAL. |
| `FATFS/`, `Middlewares/` | Included middleware and generated integration files. |
| `MPU6050-microSD-Logger.ioc` | CubeMX project configuration. |

## Storage behavior and limitations

- **Raw storage:** block 100000 is an application-selected address, not a region reserved by a filesystem. Rw writes can overwrite filesystem data.
- **Session-only tracking:** initialization resets `sd_next_block` to `SD_START_BLOCK`. After a restart, logging begins there again. Previous session boundaries are not recovered, and `clear sd` only knows the current session's recorded range.
- **Timing:** terminal output and clearing block main-loop work. Sample requests use a flag, not a queue, so requests can merge while main is busy. Live display may repeat the latest frame rather than print one line per new sample.
- **Pause behavior:** pausing logging pauses data logging but does not pause sensor sampling, so sensor live values can still be displayed.

## Verification

Useful checks after changes:

1. Confirm MPU6050 initialization succeeds and status reports the SD initialized.
2. Wait for a completed block write and confirm the next-block address advances.
3. Read that block and inspect decoded frames.
4. Pause and resume logging and confirm an active write is allowed to complete.
5. Clear a small current-session log, then record a new block.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| Missing floating-point values | Float formatting support in the active linker configuration. |
| MPU6050 initialization fails | Power, SCL/SDA wiring, pull-ups, and configured I2C address. |
| CMD0/SD initialization fails | Module power, CS, PB3 clock wiring, MISO/MOSI, and the initialization SPI speed. |
| Invalid block address | Request a successfully written block from the current session's range. |


## References

- **RM0368** — STM32F401 reference manual
- **UM1724** — NUCLEO-64 board manual
- **UM1725** — STM32F4 HAL and LL driver reference
- **SD Specifications part 1 Physical Layer Simplified Specifications**
- **MPU-6000 and MPU-6050 Register Map and Descriptions**
