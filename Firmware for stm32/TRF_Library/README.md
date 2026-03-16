# TRF372017 STM32 HAL Driver Library

## Overview
This library provides a hardware abstraction layer (HAL) driver for the Texas Instruments **TRF372017** Quadrature Modulator with Integrated PLL and VCO. It is written in C and is designed to be easily integrated into STM32 projects using the standard STM32 HAL drivers.

The library maps all 8 internal registers (Registers 0 through 7) to C structures, handling the complex bit-shifting, mask operations, and SPI packing required to communicate with the IC.

## File Structure
- `Inc/trf_modulator.h`: Contains all register definitions, address mappings, shift/mask macros, data structures (`TRF_RegX_Config_t`), and API function prototypes.
- `Src/trf_modulator.c`: Contains the implementation of the SPI communication logic, latch enable (LE) control, and the register packing functions (`TRF_Modulator_Config_RegX`).

## Hardware Integration Requirements

### SPI Configuration
The TRF372017 uses a 3-wire SPI interface (`CLK`, `DATA`, `LE`) + a Readback pin (`RDBK`). To successfully communicate with the IC, your STM32 SPI peripheral **must** be configured as follows:
- **Data Size**: 8 Bits
- **First Bit**: LSB First (Crucial for correct address and data parsing)
- **Clock Polarity (CPOL)**: Low
- **Clock Phase (CPHA)**: 1 Edge (Data is read on the rising edge of CLOCK)

### Latch Enable (LE)
The `LE` pin acts as the Chip Select but is asynchronous. Data fed into the shift register gets loaded into the selected internal register on the **rising edge** of LE. 

### Enabling HAL Functions
By default, the actual STM32 HAL function calls in `Src/trf_modulator.c` are commented out to keep the library decoupled from a specific STM32 family (e.g., STM32F4, STM32G4).
**Before compiling:**
1. Open `Src/trf_modulator.c`.
2. Locate the commented `HAL_SPI_Transmit`, `HAL_SPI_TransmitReceive`, and `HAL_GPIO_WritePin` calls.
3. Uncomment them and ensure they match your project's HAL headers (you may need to `#include "stm32f4xx_hal.h"` or similar at the top).

## How It Works

### 1. The 32-bit SPI Packet
The TRF372017 expects a 32-bit packet formatted as:
- **Bits [4:0]**: Register Address
- **Bits [31:5]**: Register Data Payload

Because the STM32 architecture is Little-Endian, when the `uint32_t` packet is passed to `HAL_SPI_Transmit` over an 8-bit LSB-first SPI bus, the lowest byte (containing the address) is physically transmitted first, exactly as the TRF372017 requires.

### 2. Initialization
To initialize the modulator, you must populate the `TRF_Modulator_HandleTypeDef` and call the default initialization. `TRF_Modulator_Init_Default` writes all default register configurations in descending order (Reg 7 down to Reg 1) to ensure safe internal power sequencing.

```c
TRF_Modulator_HandleTypeDef htrf = {0};
htrf.config.spi_handle = &hspi1; // Pointer to your configured SPI handle
htrf.config.le_port = GPIOA;     // Your Latch Enable GPIO Port
htrf.config.le_pin = GPIO_PIN_4; // Your Latch Enable GPIO Pin

// Run full initialization (writes default configs Reg 7 -> Reg 1)
TRF_Modulator_Init_Default(&htrf);
```

### 3. Customizing Registers
If you need to change a parameter (e.g., the PLL N-Divider in Register 2), you can modify the default structure and write it using the library's packer functions:

```c
// Create a config struct based on the library's defaults
TRF_Reg2_Config_t reg2 = TRF_REG2_DEFAULT_CONFIG;

// Modify specific fields
reg2.n_int = 1000;          // Set N-divider
reg2.pll_div = TRF_DIV_2;   // Set PLL divider to 2

// Write to the IC
TRF_Modulator_Config_Reg2(&htrf, &reg2);
```

The `TRF_Modulator_Config_RegX` functions automatically handle masking and bit-shifting, combining your C struct into the raw 32-bit payload, and executing the SPI transaction.

### 4. Readback functionality (Register 0)
The TRF372017 readback is a two-step process controlled via Register 0:
1. Write to Register 0, enabling the readback mode and selecting the target register address.
2. Clock the SPI to read 32 bits from the MISO/RDBK line.

The library handles this sequence inside `TRF_Modulator_ReadRegister`.
To parse the raw 32-bit readback data into human-readable fields (Chip ID, VCO Frequency Count, Cal details), use `TRF_Modulator_Parse_Reg0_Read`:

```c
uint32_t raw_data = 0;
TRF_Reg0_Read_Data_t parsed_data;

// Read from the IC (e.g., trying to read status from Reg 0)
if (TRF_Modulator_ReadRegister(&htrf, TRF372017_REG0_ADDR, &raw_data) == 0) {
    // Parse the raw 32-bit integer into the struct
    TRF_Modulator_Parse_Reg0_Read(raw_data, &parsed_data);
    
    // Now you can access parsed_data.chip_id, parsed_data.count_freq, etc.
}
```
