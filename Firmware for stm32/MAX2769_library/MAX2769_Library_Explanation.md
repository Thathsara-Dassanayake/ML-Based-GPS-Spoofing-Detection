# MAX2769 STM32 HAL Library Detailed Explanation

This document explains the current MAX2769 library implementation in detail:
- Header API and data definitions in max2769.h
- HAL-based implementation logic in max2769.c
- Why each method is used where it is used

## 1. Design Intent

The library is written as project-base STM32 code using HAL calls directly.

Main goals:
1. Keep MAX2769 register and default-value definitions clear and centralized.
2. Provide one device context struct with SPI and CS information.
3. Provide a safe initialization path.
4. Provide a single-register write primitive.
5. Provide a one-call default-register startup write.

The code uses explicit CS control and a 32-bit frame construction aligned to MAX2769 format:
- D27:D0 = register data (28 bits)
- A3:A0 = register address (4 bits)

## 2. Header File: max2769.h

### 2.1 Include Guard

```c
#ifndef MAX2769_H
#define MAX2769_H
...
#endif
```

Why:
- Prevents duplicate symbol/type declarations when included from multiple files.

### 2.2 Standard Integer Include

```c
#include <stdint.h>
```

Why:
- The driver uses fixed-width types like uint32_t and uint16_t.

### 2.3 HAL-Compatible Type Exposure

The header declares STM32 HAL-compatible types used by the API:
- SPI_HandleTypeDef
- GPIO_TypeDef
- GPIO_PinState

Why:
- The API needs to expose HAL resources in the device struct and init function.
- This keeps your driver interface aligned to HAL-style project code.

### 2.4 C/C++ Compatibility Block

```c
#ifdef __cplusplus
extern "C" {
#endif
...
#ifdef __cplusplus
}
#endif
```

Why:
- Prevents C++ name mangling if the header is included in mixed C/C++ projects.

### 2.5 Register Address Enum

```c
typedef enum
{
    MAX2769_REG_CONF1   = 0x0U,
    ...
    MAX2769_REG_TEST2   = 0x9U
} max2769_register_t;
```

Why:
- Maps datasheet A3:A0 register addresses to readable symbols.
- Avoids magic numbers in application code.

### 2.6 Default Register Value Macros

Macros such as:
- MAX2769_DEFAULT_CONF1
- MAX2769_DEFAULT_CONF2
- ...
- MAX2769_DEFAULT_TEST2

Why:
- Encodes datasheet defaults in one place.
- Makes startup config and debug comparisons easier.

### 2.7 Status Code Enum

```c
typedef enum
{
    MAX2769_STATUS_OK = 0,
    MAX2769_STATUS_INVALID_ARG = -1,
    MAX2769_STATUS_IO_ERROR = -2
} max2769_status_t;
```

Why:
- Standardized function return behavior.
- Separates invalid usage from transport/I/O failures.

### 2.8 Device Context Struct

```c
typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_PinState cs_active_state;
    uint32_t spi_timeout_ms;
} max2769_t;
```

Why each field exists:
- hspi: SPI peripheral handle used by HAL_SPI_Transmit.
- cs_port and cs_pin: chip-select GPIO endpoint.
- cs_active_state: supports active-low or active-high CS hardware.
- spi_timeout_ms: configurable timeout for SPI transfer.

### 2.9 Extern Default-Table Declaration

```c
extern const uint32_t max2769_default_reg_values[10];
```

Why:
- Allows other modules to inspect or reuse default values without duplicating definitions.

### 2.10 Public API Prototypes

- max2769_init
- max2769_write_register
- max2769_write_defaults
- max2769_build_frame

Why this API split:
1. init: binds hardware resources once.
2. write_register: fundamental operation.
3. write_defaults: convenience startup routine.
4. build_frame: isolated frame logic for clarity and testability.

## 3. Source File: max2769.c

### 3.1 Include and HAL Return Convention

```c
#include "max2769.h"
#define MAX2769_HAL_OK (0)
```

Why:
- Source implements declarations from the header.
- Uses explicit success constant for HAL result comparison.

### 3.2 External HAL Function Declarations

```c
extern void HAL_GPIO_WritePin(...);
extern int HAL_SPI_Transmit(...);
```

Why:
- The driver calls HAL directly for GPIO and SPI operations.

### 3.3 CS Inactive-State Helper

```c
static GPIO_PinState max2769_cs_inactive_state(GPIO_PinState active_state)
```

What it does:
- Returns the opposite state of configured active CS.

Why:
- Makes code robust for both active-low and active-high board wiring.
- Avoids duplicating inversion logic in multiple functions.

### 3.4 Default Write Order Table

```c
static const max2769_register_t k_default_write_order[10] = {...};
```

Why:
- Defines explicit startup write sequence.
- Keeps sequence data-driven and easy to update.

### 3.5 Default Value Table Definition

```c
const uint32_t max2769_default_reg_values[10] = {...};
```

Why:
- One canonical location for all D27:D0 default values.
- Paired with write-order table for startup loop.

### 3.6 max2769_init

Function behavior:
1. Validate pointers (dev, hspi, cs_port).
2. Store SPI/CS configuration in device context.
3. Drive CS to inactive immediately.
4. Return status.

Why this method here:
- Initialization centralizes hardware binding and idle CS setup.
- Setting CS inactive early prevents accidental device selection.

### 3.7 max2769_build_frame

```c
return ((data_d27_d0 & 0x0FFFFFFFUL) << 4U) | ((uint32_t)reg & 0x0FU);
```

What it does:
- Masks data to 28 bits.
- Shifts data to bits [31:4].
- Places address nibble in bits [3:0].

Why:
- Exactly matches MAX2769 serial frame format.
- Explicit masking prevents unintended upper-bit leakage.

### 3.8 max2769_write_register

Function behavior step-by-step:
1. Validate context and arguments.
2. Reject invalid register numbers and non-28-bit data.
3. Build 32-bit frame.
4. Convert frame to 4 transmit bytes (MSB first).
5. Assert CS.
6. Call HAL_SPI_Transmit for 4 bytes.
7. Deassert CS.
8. Return IO_ERROR if transmit fails, else OK.

Why this method in this location:
- This is the core primitive used by all higher-level operations.
- CS handling wraps only one SPI transaction, making boundaries clear and deterministic.

### 3.9 max2769_write_defaults

Function behavior:
1. Validate device pointer.
2. Iterate all 10 registers.
3. Write each register/value pair using max2769_write_register.
4. Fail fast on first error.

Why:
- Provides deterministic startup state with one API call.
- Reuses the validated single-write path for consistency.

## 4. Why This Architecture Is Good for Your Project

1. HAL-native usage:
- Matches STM32 Cube/HAL workflow directly.

2. Clear separation of concerns:
- Header defines interface/constants.
- Source defines behavior and HAL transactions.

3. Deterministic startup:
- Explicit write_defaults avoids dependence on uncertain runtime conditions.

4. Easy maintenance:
- Register defaults and write order are table-driven.

## 5. Runtime Usage Pattern

Typical call sequence from application startup:
1. Initialize SPI and GPIO through STM32 HAL/Cube init code.
2. Create a max2769_t instance.
3. Call max2769_init with SPI handle, CS GPIO, CS polarity, timeout.
4. Call max2769_write_defaults.
5. Optionally call max2769_write_register for custom tuning values.

## 6. Important Integration Notes

1. SPI mode:
- Configure mode, bit order, and speed per MAX2769 timing and serial protocol requirements.

2. CS polarity:
- Set cs_active_state to match your hardware wiring.

3. Timing:
- If your board is very fast and routing is long/noisy, verify CS-to-clock and clock pulse timing margins.

4. Error handling:
- Current behavior reports transmit failure; system-level retry/reset policy should be handled by application layer.

## 7. Summary

The library now provides:
1. MAX2769 register-address map.
2. Datasheet default values.
3. HAL-based initialization and SPI write path.
4. One-call default register programming routine.

This is a clean base for project-level STM32 firmware and can be extended with custom register profiles as your RF chain configuration evolves.

## 8. Added CONF1 Bitfield Data (Table 6)

The library now also includes Configuration-1 bitfield definitions from Table 6 in max2769.h.

What was added:
1. Bit positions for each CONF1 field.
2. Bit masks for each CONF1 field.
3. Default field values from the datasheet table.
4. A build macro to compose a CONF1 value from individual fields.
5. A prebuilt default macro from all Table-6 default fields.

Key fields covered:
- CHIPEN [27]
- IDLE [26]
- ILNA1 [25:22]
- ILNA2 [21:20]
- ILO [19:18]
- IMIX [17:16]
- MIXPOLE [15]
- LNAMODE [14:13]
- MIXEN [12]
- ANTEN [11]
- FCEN [10:5]
- FBW [4:3]
- F3OR5 [2]
- FCENX [1]
- FGAIN [0]

Usage example:

```c
uint32_t conf1_custom = MAX2769_CONF1_BUILD(
    1U, /* CHIPEN */
    0U, /* IDLE */
    0x8U, /* ILNA1 */
    0x2U, /* ILNA2 */
    0x2U, /* ILO */
    0x1U, /* IMIX */
    0U, /* MIXPOLE */
    0x0U, /* LNAMODE */
    1U, /* MIXEN */
    1U, /* ANTEN */
    0x0DU, /* FCEN */
    0x0U, /* FBW */
    0U, /* F3OR5 */
    1U, /* FCENX */
    1U  /* FGAIN */
);

max2769_write_register(&dev, MAX2769_REG_CONF1, conf1_custom);
```

Default-ready macro:

```c
max2769_write_register(&dev, MAX2769_REG_CONF1, MAX2769_CONF1_DEFAULT_BUILT);
```

Note:
- MAX2769_DEFAULT_CONF1 is still kept as the full register default constant.
- MAX2769_CONF1_DEFAULT_BUILT gives the same value assembled from field defaults, which is useful when you want to change only one field while keeping others explicit.

## 9. Added CONF2 Bitfield Data (Table 7)

The library now also includes Configuration-2 bitfield definitions from Table 7 in max2769.h.

What was added:
1. Bit positions for each CONF2 field.
2. Bit masks for each CONF2 field.
3. Default field values from the datasheet table.
4. A build macro to compose a CONF2 value from individual fields.
5. A prebuilt default macro from all Table-7 default fields.

Key fields covered:
- IQEN [27]
- GAINREF [26:15]
- RESERVED [14:13]
- AGCMODE [12:11]
- FORMAT [10:9]
- BITS [8:6]
- DRVCFG [5:4]
- LOEN [3]
- RESERVED [2]
- DIEID [1:0]

Usage example:

```c
uint32_t conf2_custom = MAX2769_CONF2_BUILD(
    0U,    /* IQEN */
    170U,  /* GAINREF */
    0U,    /* RESERVED [14:13], keep 0 */
    0U,    /* AGCMODE */
    0x1U,  /* FORMAT */
    0x2U,  /* BITS */
    0U,    /* DRVCFG */
    1U,    /* LOEN */
    0U,    /* RESERVED [2], keep 0 */
    0U     /* DIEID */
);

max2769_write_register(&dev, MAX2769_REG_CONF2, conf2_custom);
```

Default-ready macro:

```c
max2769_write_register(&dev, MAX2769_REG_CONF2, MAX2769_CONF2_DEFAULT_BUILT);
```

Note:
- MAX2769_DEFAULT_CONF2 is still kept as the full register default constant.
- MAX2769_CONF2_DEFAULT_BUILT evaluates to the same default value: 0x00550288.
- Keep RESERVED bits at 0 unless a newer datasheet revision states otherwise.

## 10. Added CONF3 Bitfield Data (Table 8)

The library now includes Configuration-3 bitfield definitions from Table 8 in max2769.h.

What was added:
1. Bit positions for each CONF3 field.
2. Bit masks for each CONF3 field.
3. Default field values from the datasheet table.
4. A build macro to compose a CONF3 value from individual fields.
5. A prebuilt default macro from all Table-8 default fields.

Key fields covered:
- GAININ [27:22]
- FSLOWEN [21]
- HILOADEN [20]
- ADCEN [19]
- DRVEN [18]
- FOFSTEN [17]
- FILTEN [16]
- FHIPEN [15]
- RESERVED [14]
- PGAIEN [13]
- PGAQEN [12]
- STRMEN [11]
- STRMSTART [10]
- STRMSTOP [9]
- STRMCOUNT [8:6]
- STRMBITS [5:4]
- STAMPEN [3]
- TIMESYNCEN [2]
- DATSYNCEN [1]
- STRMRST [0]

Default-ready macro:

```c
max2769_write_register(&dev, MAX2769_REG_CONF3, MAX2769_CONF3_DEFAULT_BUILT);
```

## 11. Added PLLCONF Bitfield Data (Table 9)

The library now includes PLL configuration bitfield definitions from Table 9 in max2769.h.

What was added:
1. Bit positions for each PLLCONF field.
2. Bit masks for each PLLCONF field.
3. Default field values from the datasheet table.
4. A build macro to compose a PLLCONF value from individual fields.
5. A prebuilt default macro from all Table-9 default fields.

Key fields covered:
- VCOEN [27]
- IVCO [26]
- RESERVED [25]
- REFOUTEN [24]
- RESERVED [23]
- REFDIV [22:21]
- IXTAL [20:19]
- XTALCAP [18:14]
- LDMUX [13:10]
- ICP [9]
- PFDEN [8]
- RESERVED [7]
- CPTEST [6:4]
- INT_PLL [3]
- PWRSAV [2]
- RESERVED [1:0]

Default-ready macro:

```c
max2769_write_register(&dev, MAX2769_REG_PLLCONF, MAX2769_PLLCONF_DEFAULT_BUILT);
```

Note:
- Reserved field defaults are explicitly included in both CONF3 and PLLCONF macros so you can preserve datasheet-recommended reserved states while changing only target fields.

## 12. Added DIV/FDIV/STRM/CLK/TEST Bitfield Data (Tables 10-15)

The library now includes field-level definitions for all remaining registers in max2769.h.

### 12.1 DIV (Address 0100, Table 10)

Added:
1. NDIV [27:13]
2. RDIV [12:3]
3. RESERVED [2:0]
4. MAX2769_DIV_BUILD(...)
5. MAX2769_DIV_DEFAULT_BUILT

Default check:
- MAX2769_DIV_DEFAULT_BUILT evaluates to 0x00C00080.

### 12.2 FDIV (Address 0101, Table 11)

Added:
1. FDIV [27:8]
2. RESERVED [7:0]
3. MAX2769_FDIV_BUILD(...)
4. MAX2769_FDIV_DEFAULT_BUILT

Default check:
- MAX2769_FDIV_DEFAULT_BUILT evaluates to 0x08000070.

Note:
- Reserved bits [7:0] in FDIV default to 0x70 per the table/default register value.

### 12.3 STRM (Address 0110, Table 12)

Added:
1. FRAMECOUNT [27:0]
2. MAX2769_STRM_BUILD(...)
3. MAX2769_STRM_DEFAULT_BUILT

Default check:
- MAX2769_STRM_DEFAULT_BUILT evaluates to 0x08000000.

### 12.4 CLK (Address 0111, Table 13)

Added:
1. L_CNT [27:16]
2. M_CNT [15:4]
3. FCLKIN [3]
4. ADCCLK [2]
5. SERCLK [1]
6. MODE [0]
7. MAX2769_CLK_BUILD(...)
8. MAX2769_CLK_DEFAULT_BUILT

Default check:
- MAX2769_CLK_DEFAULT_BUILT evaluates to 0x010061B2.

### 12.5 TEST1/TEST2 (Addresses 1000/1001, Tables 14-15)

Added:
1. TEST1 reserved field [27:0]
2. TEST2 reserved field [27:0]
3. MAX2769_TEST1_BUILD(...), MAX2769_TEST2_BUILD(...)
4. MAX2769_TEST1_DEFAULT_BUILT, MAX2769_TEST2_DEFAULT_BUILT

Default checks:
- MAX2769_TEST1_DEFAULT_BUILT evaluates to 0x01E0F401.
- MAX2769_TEST2_DEFAULT_BUILT evaluates to 0x014C0402.
