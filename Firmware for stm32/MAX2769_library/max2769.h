#ifndef MAX2769_H
#define MAX2769_H

#include <stdint.h>

/* Prefer project HAL headers when available; fallback to compatible type declarations. */
#if defined(__has_include)
#if __has_include("main.h")
#include "main.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32h7xx_hal.h")
#include "stm32h7xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32f7xx_hal.h")
#include "stm32f7xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32f4xx_hal.h")
#include "stm32f4xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32f3xx_hal.h")
#include "stm32f3xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32f1xx_hal.h")
#include "stm32f1xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32g4xx_hal.h")
#include "stm32g4xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32g0xx_hal.h")
#include "stm32g0xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32l4xx_hal.h")
#include "stm32l4xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32l0xx_hal.h")
#include "stm32l0xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32u5xx_hal.h")
#include "stm32u5xx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32wbxx_hal.h")
#include "stm32wbxx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#elif __has_include("stm32wlxx_hal.h")
#include "stm32wlxx_hal.h"
#define MAX2769_HAL_TYPES_FROM_PROJECT (1)
#endif
#endif

#ifndef MAX2769_HAL_TYPES_FROM_PROJECT
typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
typedef struct GPIO_TypeDef GPIO_TypeDef;

typedef enum
{
    GPIO_PIN_RESET = 0U,
    GPIO_PIN_SET
} GPIO_PinState;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* MAX2769 register addresses (A3:A0) */
typedef enum
{
    MAX2769_REG_CONF1   = 0x0U,
    MAX2769_REG_CONF2   = 0x1U,
    MAX2769_REG_CONF3   = 0x2U,
    MAX2769_REG_PLLCONF = 0x3U,
    MAX2769_REG_DIV     = 0x4U,
    MAX2769_REG_FDIV    = 0x5U,
    MAX2769_REG_STRM    = 0x6U,
    MAX2769_REG_CLK     = 0x7U,
    MAX2769_REG_TEST1   = 0x8U,
    MAX2769_REG_TEST2   = 0x9U
} max2769_register_t;

/* MAX2769 default register data (D27:D0) */
#define MAX2769_DEFAULT_CONF1   (0x0A2919A3UL)
#define MAX2769_DEFAULT_CONF2   (0x00550288UL)
#define MAX2769_DEFAULT_CONF3   (0x0EAFF1DCUL)
#define MAX2769_DEFAULT_PLLCONF (0x09EC0008UL)
#define MAX2769_DEFAULT_DIV     (0x00C00080UL)
#define MAX2769_DEFAULT_FDIV    (0x08000070UL)
#define MAX2769_DEFAULT_STRM    (0x08000000UL)
#define MAX2769_DEFAULT_CLK     (0x010061B2UL)
#define MAX2769_DEFAULT_TEST1   (0x01E0F401UL)
#define MAX2769_DEFAULT_TEST2   (0x014C0402UL)

/* MAX2769 CONF1 (Address 0000) field map from datasheet Table 6 */
#define MAX2769_CONF1_CHIPEN_POS   (27U)
#define MAX2769_CONF1_IDLE_POS     (26U)
#define MAX2769_CONF1_ILNA1_POS    (22U)
#define MAX2769_CONF1_ILNA2_POS    (20U)
#define MAX2769_CONF1_ILO_POS      (18U)
#define MAX2769_CONF1_IMIX_POS     (16U)
#define MAX2769_CONF1_MIXPOLE_POS  (15U)
#define MAX2769_CONF1_LNAMODE_POS  (13U)
#define MAX2769_CONF1_MIXEN_POS    (12U)
#define MAX2769_CONF1_ANTEN_POS    (11U)
#define MAX2769_CONF1_FCEN_POS     (5U)
#define MAX2769_CONF1_FBW_POS      (3U)
#define MAX2769_CONF1_F3OR5_POS    (2U)
#define MAX2769_CONF1_FCENX_POS    (1U)
#define MAX2769_CONF1_FGAIN_POS    (0U)

#define MAX2769_CONF1_CHIPEN_MASK  (0x1UL << MAX2769_CONF1_CHIPEN_POS)
#define MAX2769_CONF1_IDLE_MASK    (0x1UL << MAX2769_CONF1_IDLE_POS)
#define MAX2769_CONF1_ILNA1_MASK   (0xFUL << MAX2769_CONF1_ILNA1_POS)
#define MAX2769_CONF1_ILNA2_MASK   (0x3UL << MAX2769_CONF1_ILNA2_POS)
#define MAX2769_CONF1_ILO_MASK     (0x3UL << MAX2769_CONF1_ILO_POS)
#define MAX2769_CONF1_IMIX_MASK    (0x3UL << MAX2769_CONF1_IMIX_POS)
#define MAX2769_CONF1_MIXPOLE_MASK (0x1UL << MAX2769_CONF1_MIXPOLE_POS)
#define MAX2769_CONF1_LNAMODE_MASK (0x3UL << MAX2769_CONF1_LNAMODE_POS)
#define MAX2769_CONF1_MIXEN_MASK   (0x1UL << MAX2769_CONF1_MIXEN_POS)
#define MAX2769_CONF1_ANTEN_MASK   (0x1UL << MAX2769_CONF1_ANTEN_POS)
#define MAX2769_CONF1_FCEN_MASK    (0x3FUL << MAX2769_CONF1_FCEN_POS)
#define MAX2769_CONF1_FBW_MASK     (0x3UL << MAX2769_CONF1_FBW_POS)
#define MAX2769_CONF1_F3OR5_MASK   (0x1UL << MAX2769_CONF1_F3OR5_POS)
#define MAX2769_CONF1_FCENX_MASK   (0x1UL << MAX2769_CONF1_FCENX_POS)
#define MAX2769_CONF1_FGAIN_MASK   (0x1UL << MAX2769_CONF1_FGAIN_POS)

/* Default field values from datasheet Table 6 */
#define MAX2769_CONF1_CHIPEN_DFLT  (1UL)
#define MAX2769_CONF1_IDLE_DFLT    (0UL)
#define MAX2769_CONF1_ILNA1_DFLT   (0x8UL)
#define MAX2769_CONF1_ILNA2_DFLT   (0x2UL)
#define MAX2769_CONF1_ILO_DFLT     (0x2UL)
#define MAX2769_CONF1_IMIX_DFLT    (0x1UL)
#define MAX2769_CONF1_MIXPOLE_DFLT (0UL)
#define MAX2769_CONF1_LNAMODE_DFLT (0x0UL)
#define MAX2769_CONF1_MIXEN_DFLT   (1UL)
#define MAX2769_CONF1_ANTEN_DFLT   (1UL)
#define MAX2769_CONF1_FCEN_DFLT    (0x0DUL)
#define MAX2769_CONF1_FBW_DFLT     (0x0UL)
#define MAX2769_CONF1_F3OR5_DFLT   (0UL)
#define MAX2769_CONF1_FCENX_DFLT   (1UL)
#define MAX2769_CONF1_FGAIN_DFLT   (1UL)

#define MAX2769_CONF1_BUILD(chipen, idle, ilna1, ilna2, ilo, imix, mixpole, lnamode, mixen, anten, fcen, fbw, f3or5, fcenx, fgain) \
    (((uint32_t)(chipen)  & 0x1UL)  << MAX2769_CONF1_CHIPEN_POS  | \
     ((uint32_t)(idle)    & 0x1UL)  << MAX2769_CONF1_IDLE_POS    | \
     ((uint32_t)(ilna1)   & 0xFUL)  << MAX2769_CONF1_ILNA1_POS   | \
     ((uint32_t)(ilna2)   & 0x3UL)  << MAX2769_CONF1_ILNA2_POS   | \
     ((uint32_t)(ilo)     & 0x3UL)  << MAX2769_CONF1_ILO_POS     | \
     ((uint32_t)(imix)    & 0x3UL)  << MAX2769_CONF1_IMIX_POS    | \
     ((uint32_t)(mixpole) & 0x1UL)  << MAX2769_CONF1_MIXPOLE_POS | \
     ((uint32_t)(lnamode) & 0x3UL)  << MAX2769_CONF1_LNAMODE_POS | \
     ((uint32_t)(mixen)   & 0x1UL)  << MAX2769_CONF1_MIXEN_POS   | \
     ((uint32_t)(anten)   & 0x1UL)  << MAX2769_CONF1_ANTEN_POS   | \
     ((uint32_t)(fcen)    & 0x3FUL) << MAX2769_CONF1_FCEN_POS    | \
     ((uint32_t)(fbw)     & 0x3UL)  << MAX2769_CONF1_FBW_POS     | \
     ((uint32_t)(f3or5)   & 0x1UL)  << MAX2769_CONF1_F3OR5_POS   | \
     ((uint32_t)(fcenx)   & 0x1UL)  << MAX2769_CONF1_FCENX_POS   | \
     ((uint32_t)(fgain)   & 0x1UL)  << MAX2769_CONF1_FGAIN_POS)

#define MAX2769_CONF1_DEFAULT_BUILT \
    MAX2769_CONF1_BUILD(MAX2769_CONF1_CHIPEN_DFLT, \
                        MAX2769_CONF1_IDLE_DFLT, \
                        MAX2769_CONF1_ILNA1_DFLT, \
                        MAX2769_CONF1_ILNA2_DFLT, \
                        MAX2769_CONF1_ILO_DFLT, \
                        MAX2769_CONF1_IMIX_DFLT, \
                        MAX2769_CONF1_MIXPOLE_DFLT, \
                        MAX2769_CONF1_LNAMODE_DFLT, \
                        MAX2769_CONF1_MIXEN_DFLT, \
                        MAX2769_CONF1_ANTEN_DFLT, \
                        MAX2769_CONF1_FCEN_DFLT, \
                        MAX2769_CONF1_FBW_DFLT, \
                        MAX2769_CONF1_F3OR5_DFLT, \
                        MAX2769_CONF1_FCENX_DFLT, \
                        MAX2769_CONF1_FGAIN_DFLT)

/* MAX2769 CONF2 (Address 0001) field map from datasheet Table 7 */
#define MAX2769_CONF2_IQEN_POS            (27U)
#define MAX2769_CONF2_GAINREF_POS         (15U)
#define MAX2769_CONF2_RESERVED_14_13_POS  (13U)
#define MAX2769_CONF2_AGCMODE_POS         (11U)
#define MAX2769_CONF2_FORMAT_POS          (9U)
#define MAX2769_CONF2_BITS_POS            (6U)
#define MAX2769_CONF2_DRVCFG_POS          (4U)
#define MAX2769_CONF2_LOEN_POS            (3U)
#define MAX2769_CONF2_RESERVED_2_POS      (2U)
#define MAX2769_CONF2_DIEID_POS           (0U)

#define MAX2769_CONF2_IQEN_MASK           (0x1UL << MAX2769_CONF2_IQEN_POS)
#define MAX2769_CONF2_GAINREF_MASK        (0xFFFUL << MAX2769_CONF2_GAINREF_POS)
#define MAX2769_CONF2_RESERVED_14_13_MASK (0x3UL << MAX2769_CONF2_RESERVED_14_13_POS)
#define MAX2769_CONF2_AGCMODE_MASK        (0x3UL << MAX2769_CONF2_AGCMODE_POS)
#define MAX2769_CONF2_FORMAT_MASK         (0x3UL << MAX2769_CONF2_FORMAT_POS)
#define MAX2769_CONF2_BITS_MASK           (0x7UL << MAX2769_CONF2_BITS_POS)
#define MAX2769_CONF2_DRVCFG_MASK         (0x3UL << MAX2769_CONF2_DRVCFG_POS)
#define MAX2769_CONF2_LOEN_MASK           (0x1UL << MAX2769_CONF2_LOEN_POS)
#define MAX2769_CONF2_RESERVED_2_MASK     (0x1UL << MAX2769_CONF2_RESERVED_2_POS)
#define MAX2769_CONF2_DIEID_MASK          (0x3UL << MAX2769_CONF2_DIEID_POS)

/* Default field values from datasheet Table 7 */
#define MAX2769_CONF2_IQEN_DFLT           (0UL)
#define MAX2769_CONF2_GAINREF_DFLT        (170UL)
#define MAX2769_CONF2_RESERVED_14_13_DFLT (0UL)
#define MAX2769_CONF2_AGCMODE_DFLT        (0UL)
#define MAX2769_CONF2_FORMAT_DFLT         (0x1UL)
#define MAX2769_CONF2_BITS_DFLT           (0x2UL)
#define MAX2769_CONF2_DRVCFG_DFLT         (0UL)
#define MAX2769_CONF2_LOEN_DFLT           (1UL)
#define MAX2769_CONF2_RESERVED_2_DFLT     (0UL)
#define MAX2769_CONF2_DIEID_DFLT          (0UL)

#define MAX2769_CONF2_BUILD(iqen, gainref, reserved_14_13, agcmode, format, bits, drvcfg, loen, reserved_2, dieid) \
    (((uint32_t)(iqen)           & 0x1UL)   << MAX2769_CONF2_IQEN_POS           | \
     ((uint32_t)(gainref)        & 0xFFFUL) << MAX2769_CONF2_GAINREF_POS        | \
     ((uint32_t)(reserved_14_13) & 0x3UL)   << MAX2769_CONF2_RESERVED_14_13_POS | \
     ((uint32_t)(agcmode)        & 0x3UL)   << MAX2769_CONF2_AGCMODE_POS        | \
     ((uint32_t)(format)         & 0x3UL)   << MAX2769_CONF2_FORMAT_POS         | \
     ((uint32_t)(bits)           & 0x7UL)   << MAX2769_CONF2_BITS_POS           | \
     ((uint32_t)(drvcfg)         & 0x3UL)   << MAX2769_CONF2_DRVCFG_POS         | \
     ((uint32_t)(loen)           & 0x1UL)   << MAX2769_CONF2_LOEN_POS           | \
     ((uint32_t)(reserved_2)     & 0x1UL)   << MAX2769_CONF2_RESERVED_2_POS     | \
     ((uint32_t)(dieid)          & 0x3UL)   << MAX2769_CONF2_DIEID_POS)

#define MAX2769_CONF2_DEFAULT_BUILT \
    MAX2769_CONF2_BUILD(MAX2769_CONF2_IQEN_DFLT, \
                        MAX2769_CONF2_GAINREF_DFLT, \
                        MAX2769_CONF2_RESERVED_14_13_DFLT, \
                        MAX2769_CONF2_AGCMODE_DFLT, \
                        MAX2769_CONF2_FORMAT_DFLT, \
                        MAX2769_CONF2_BITS_DFLT, \
                        MAX2769_CONF2_DRVCFG_DFLT, \
                        MAX2769_CONF2_LOEN_DFLT, \
                        MAX2769_CONF2_RESERVED_2_DFLT, \
                        MAX2769_CONF2_DIEID_DFLT)

/* MAX2769 CONF3 (Address 0010) field map from datasheet Table 8 */
#define MAX2769_CONF3_GAININ_POS          (22U)
#define MAX2769_CONF3_FSLOWEN_POS         (21U)
#define MAX2769_CONF3_HILOADEN_POS        (20U)
#define MAX2769_CONF3_ADCEN_POS           (19U)
#define MAX2769_CONF3_DRVEN_POS           (18U)
#define MAX2769_CONF3_FOFSTEN_POS         (17U)
#define MAX2769_CONF3_FILTEN_POS          (16U)
#define MAX2769_CONF3_FHIPEN_POS          (15U)
#define MAX2769_CONF3_RESERVED_14_POS     (14U)
#define MAX2769_CONF3_PGAIEN_POS          (13U)
#define MAX2769_CONF3_PGAQEN_POS          (12U)
#define MAX2769_CONF3_STRMEN_POS          (11U)
#define MAX2769_CONF3_STRMSTART_POS       (10U)
#define MAX2769_CONF3_STRMSTOP_POS        (9U)
#define MAX2769_CONF3_STRMCOUNT_POS       (6U)
#define MAX2769_CONF3_STRMBITS_POS        (4U)
#define MAX2769_CONF3_STAMPEN_POS         (3U)
#define MAX2769_CONF3_TIMESYNCEN_POS      (2U)
#define MAX2769_CONF3_DATSYNCEN_POS       (1U)
#define MAX2769_CONF3_STRMRST_POS         (0U)

#define MAX2769_CONF3_GAININ_MASK         (0x3FUL << MAX2769_CONF3_GAININ_POS)
#define MAX2769_CONF3_FSLOWEN_MASK        (0x1UL << MAX2769_CONF3_FSLOWEN_POS)
#define MAX2769_CONF3_HILOADEN_MASK       (0x1UL << MAX2769_CONF3_HILOADEN_POS)
#define MAX2769_CONF3_ADCEN_MASK          (0x1UL << MAX2769_CONF3_ADCEN_POS)
#define MAX2769_CONF3_DRVEN_MASK          (0x1UL << MAX2769_CONF3_DRVEN_POS)
#define MAX2769_CONF3_FOFSTEN_MASK        (0x1UL << MAX2769_CONF3_FOFSTEN_POS)
#define MAX2769_CONF3_FILTEN_MASK         (0x1UL << MAX2769_CONF3_FILTEN_POS)
#define MAX2769_CONF3_FHIPEN_MASK         (0x1UL << MAX2769_CONF3_FHIPEN_POS)
#define MAX2769_CONF3_RESERVED_14_MASK    (0x1UL << MAX2769_CONF3_RESERVED_14_POS)
#define MAX2769_CONF3_PGAIEN_MASK         (0x1UL << MAX2769_CONF3_PGAIEN_POS)
#define MAX2769_CONF3_PGAQEN_MASK         (0x1UL << MAX2769_CONF3_PGAQEN_POS)
#define MAX2769_CONF3_STRMEN_MASK         (0x1UL << MAX2769_CONF3_STRMEN_POS)
#define MAX2769_CONF3_STRMSTART_MASK      (0x1UL << MAX2769_CONF3_STRMSTART_POS)
#define MAX2769_CONF3_STRMSTOP_MASK       (0x1UL << MAX2769_CONF3_STRMSTOP_POS)
#define MAX2769_CONF3_STRMCOUNT_MASK      (0x7UL << MAX2769_CONF3_STRMCOUNT_POS)
#define MAX2769_CONF3_STRMBITS_MASK       (0x3UL << MAX2769_CONF3_STRMBITS_POS)
#define MAX2769_CONF3_STAMPEN_MASK        (0x1UL << MAX2769_CONF3_STAMPEN_POS)
#define MAX2769_CONF3_TIMESYNCEN_MASK     (0x1UL << MAX2769_CONF3_TIMESYNCEN_POS)
#define MAX2769_CONF3_DATSYNCEN_MASK      (0x1UL << MAX2769_CONF3_DATSYNCEN_POS)
#define MAX2769_CONF3_STRMRST_MASK        (0x1UL << MAX2769_CONF3_STRMRST_POS)

/* Default field values from datasheet Table 8 */
#define MAX2769_CONF3_GAININ_DFLT         (0x3AUL)
#define MAX2769_CONF3_FSLOWEN_DFLT        (1UL)
#define MAX2769_CONF3_HILOADEN_DFLT       (0UL)
#define MAX2769_CONF3_ADCEN_DFLT          (1UL)
#define MAX2769_CONF3_DRVEN_DFLT          (1UL)
#define MAX2769_CONF3_FOFSTEN_DFLT        (1UL)
#define MAX2769_CONF3_FILTEN_DFLT         (1UL)
#define MAX2769_CONF3_FHIPEN_DFLT         (1UL)
#define MAX2769_CONF3_RESERVED_14_DFLT    (1UL)
#define MAX2769_CONF3_PGAIEN_DFLT         (1UL)
#define MAX2769_CONF3_PGAQEN_DFLT         (0UL)
#define MAX2769_CONF3_STRMEN_DFLT         (0UL)
#define MAX2769_CONF3_STRMSTART_DFLT      (0UL)
#define MAX2769_CONF3_STRMSTOP_DFLT       (0UL)
#define MAX2769_CONF3_STRMCOUNT_DFLT      (0x7UL)
#define MAX2769_CONF3_STRMBITS_DFLT       (0x1UL)
#define MAX2769_CONF3_STAMPEN_DFLT        (1UL)
#define MAX2769_CONF3_TIMESYNCEN_DFLT     (1UL)
#define MAX2769_CONF3_DATSYNCEN_DFLT      (0UL)
#define MAX2769_CONF3_STRMRST_DFLT        (0UL)

#define MAX2769_CONF3_BUILD(gainin, fslowen, hiloaden, adcen, drven, fofsten, filten, fhipen, reserved_14, pgaien, pgaqen, strmen, strmstart, strmstop, strmcount, strmbits, stampen, timesyncen, datsyncen, strmrst) \
    (((uint32_t)(gainin)     & 0x3FUL) << MAX2769_CONF3_GAININ_POS      | \
     ((uint32_t)(fslowen)    & 0x1UL)  << MAX2769_CONF3_FSLOWEN_POS     | \
     ((uint32_t)(hiloaden)   & 0x1UL)  << MAX2769_CONF3_HILOADEN_POS    | \
     ((uint32_t)(adcen)      & 0x1UL)  << MAX2769_CONF3_ADCEN_POS       | \
     ((uint32_t)(drven)      & 0x1UL)  << MAX2769_CONF3_DRVEN_POS       | \
     ((uint32_t)(fofsten)    & 0x1UL)  << MAX2769_CONF3_FOFSTEN_POS     | \
     ((uint32_t)(filten)     & 0x1UL)  << MAX2769_CONF3_FILTEN_POS      | \
     ((uint32_t)(fhipen)     & 0x1UL)  << MAX2769_CONF3_FHIPEN_POS      | \
     ((uint32_t)(reserved_14)& 0x1UL)  << MAX2769_CONF3_RESERVED_14_POS | \
     ((uint32_t)(pgaien)     & 0x1UL)  << MAX2769_CONF3_PGAIEN_POS      | \
     ((uint32_t)(pgaqen)     & 0x1UL)  << MAX2769_CONF3_PGAQEN_POS      | \
     ((uint32_t)(strmen)     & 0x1UL)  << MAX2769_CONF3_STRMEN_POS      | \
     ((uint32_t)(strmstart)  & 0x1UL)  << MAX2769_CONF3_STRMSTART_POS   | \
     ((uint32_t)(strmstop)   & 0x1UL)  << MAX2769_CONF3_STRMSTOP_POS    | \
     ((uint32_t)(strmcount)  & 0x7UL)  << MAX2769_CONF3_STRMCOUNT_POS   | \
     ((uint32_t)(strmbits)   & 0x3UL)  << MAX2769_CONF3_STRMBITS_POS    | \
     ((uint32_t)(stampen)    & 0x1UL)  << MAX2769_CONF3_STAMPEN_POS     | \
     ((uint32_t)(timesyncen) & 0x1UL)  << MAX2769_CONF3_TIMESYNCEN_POS  | \
     ((uint32_t)(datsyncen)  & 0x1UL)  << MAX2769_CONF3_DATSYNCEN_POS   | \
     ((uint32_t)(strmrst)    & 0x1UL)  << MAX2769_CONF3_STRMRST_POS)

#define MAX2769_CONF3_DEFAULT_BUILT \
    MAX2769_CONF3_BUILD(MAX2769_CONF3_GAININ_DFLT, \
                        MAX2769_CONF3_FSLOWEN_DFLT, \
                        MAX2769_CONF3_HILOADEN_DFLT, \
                        MAX2769_CONF3_ADCEN_DFLT, \
                        MAX2769_CONF3_DRVEN_DFLT, \
                        MAX2769_CONF3_FOFSTEN_DFLT, \
                        MAX2769_CONF3_FILTEN_DFLT, \
                        MAX2769_CONF3_FHIPEN_DFLT, \
                        MAX2769_CONF3_RESERVED_14_DFLT, \
                        MAX2769_CONF3_PGAIEN_DFLT, \
                        MAX2769_CONF3_PGAQEN_DFLT, \
                        MAX2769_CONF3_STRMEN_DFLT, \
                        MAX2769_CONF3_STRMSTART_DFLT, \
                        MAX2769_CONF3_STRMSTOP_DFLT, \
                        MAX2769_CONF3_STRMCOUNT_DFLT, \
                        MAX2769_CONF3_STRMBITS_DFLT, \
                        MAX2769_CONF3_STAMPEN_DFLT, \
                        MAX2769_CONF3_TIMESYNCEN_DFLT, \
                        MAX2769_CONF3_DATSYNCEN_DFLT, \
                        MAX2769_CONF3_STRMRST_DFLT)

/* MAX2769 PLLCONF (Address 0011) field map from datasheet Table 9 */
#define MAX2769_PLLCONF_VCOEN_POS         (27U)
#define MAX2769_PLLCONF_IVCO_POS          (26U)
#define MAX2769_PLLCONF_RESERVED_25_POS   (25U)
#define MAX2769_PLLCONF_REFOUTEN_POS      (24U)
#define MAX2769_PLLCONF_RESERVED_23_POS   (23U)
#define MAX2769_PLLCONF_REFDIV_POS        (21U)
#define MAX2769_PLLCONF_IXTAL_POS         (19U)
#define MAX2769_PLLCONF_XTALCAP_POS       (14U)
#define MAX2769_PLLCONF_LDMUX_POS         (10U)
#define MAX2769_PLLCONF_ICP_POS           (9U)
#define MAX2769_PLLCONF_PFDEN_POS         (8U)
#define MAX2769_PLLCONF_RESERVED_7_POS    (7U)
#define MAX2769_PLLCONF_CPTEST_POS        (4U)
#define MAX2769_PLLCONF_INT_PLL_POS       (3U)
#define MAX2769_PLLCONF_PWRSAV_POS        (2U)
#define MAX2769_PLLCONF_RESERVED_1_POS    (1U)
#define MAX2769_PLLCONF_RESERVED_0_POS    (0U)

#define MAX2769_PLLCONF_VCOEN_MASK        (0x1UL << MAX2769_PLLCONF_VCOEN_POS)
#define MAX2769_PLLCONF_IVCO_MASK         (0x1UL << MAX2769_PLLCONF_IVCO_POS)
#define MAX2769_PLLCONF_RESERVED_25_MASK  (0x1UL << MAX2769_PLLCONF_RESERVED_25_POS)
#define MAX2769_PLLCONF_REFOUTEN_MASK     (0x1UL << MAX2769_PLLCONF_REFOUTEN_POS)
#define MAX2769_PLLCONF_RESERVED_23_MASK  (0x1UL << MAX2769_PLLCONF_RESERVED_23_POS)
#define MAX2769_PLLCONF_REFDIV_MASK       (0x3UL << MAX2769_PLLCONF_REFDIV_POS)
#define MAX2769_PLLCONF_IXTAL_MASK        (0x3UL << MAX2769_PLLCONF_IXTAL_POS)
#define MAX2769_PLLCONF_XTALCAP_MASK      (0x1FUL << MAX2769_PLLCONF_XTALCAP_POS)
#define MAX2769_PLLCONF_LDMUX_MASK        (0xFUL << MAX2769_PLLCONF_LDMUX_POS)
#define MAX2769_PLLCONF_ICP_MASK          (0x1UL << MAX2769_PLLCONF_ICP_POS)
#define MAX2769_PLLCONF_PFDEN_MASK        (0x1UL << MAX2769_PLLCONF_PFDEN_POS)
#define MAX2769_PLLCONF_RESERVED_7_MASK   (0x1UL << MAX2769_PLLCONF_RESERVED_7_POS)
#define MAX2769_PLLCONF_CPTEST_MASK       (0x7UL << MAX2769_PLLCONF_CPTEST_POS)
#define MAX2769_PLLCONF_INT_PLL_MASK      (0x1UL << MAX2769_PLLCONF_INT_PLL_POS)
#define MAX2769_PLLCONF_PWRSAV_MASK       (0x1UL << MAX2769_PLLCONF_PWRSAV_POS)
#define MAX2769_PLLCONF_RESERVED_1_MASK   (0x1UL << MAX2769_PLLCONF_RESERVED_1_POS)
#define MAX2769_PLLCONF_RESERVED_0_MASK   (0x1UL << MAX2769_PLLCONF_RESERVED_0_POS)

/* Default field values from datasheet Table 9 */
#define MAX2769_PLLCONF_VCOEN_DFLT        (1UL)
#define MAX2769_PLLCONF_IVCO_DFLT         (0UL)
#define MAX2769_PLLCONF_RESERVED_25_DFLT  (0UL)
#define MAX2769_PLLCONF_REFOUTEN_DFLT     (1UL)
#define MAX2769_PLLCONF_RESERVED_23_DFLT  (1UL)
#define MAX2769_PLLCONF_REFDIV_DFLT       (0x3UL)
#define MAX2769_PLLCONF_IXTAL_DFLT        (0x1UL)
#define MAX2769_PLLCONF_XTALCAP_DFLT      (0x10UL)
#define MAX2769_PLLCONF_LDMUX_DFLT        (0x0UL)
#define MAX2769_PLLCONF_ICP_DFLT          (0UL)
#define MAX2769_PLLCONF_PFDEN_DFLT        (0UL)
#define MAX2769_PLLCONF_RESERVED_7_DFLT   (0UL)
#define MAX2769_PLLCONF_CPTEST_DFLT       (0x0UL)
#define MAX2769_PLLCONF_INT_PLL_DFLT      (1UL)
#define MAX2769_PLLCONF_PWRSAV_DFLT       (0UL)
#define MAX2769_PLLCONF_RESERVED_1_DFLT   (0UL)
#define MAX2769_PLLCONF_RESERVED_0_DFLT   (0UL)

#define MAX2769_PLLCONF_BUILD(vcoen, ivco, reserved_25, refouten, reserved_23, refdiv, ixtal, xtalcap, ldmux, icp, pfden, reserved_7, cptest, int_pll, pwrsav, reserved_1, reserved_0) \
    (((uint32_t)(vcoen)      & 0x1UL)  << MAX2769_PLLCONF_VCOEN_POS       | \
     ((uint32_t)(ivco)       & 0x1UL)  << MAX2769_PLLCONF_IVCO_POS        | \
     ((uint32_t)(reserved_25)& 0x1UL)  << MAX2769_PLLCONF_RESERVED_25_POS | \
     ((uint32_t)(refouten)   & 0x1UL)  << MAX2769_PLLCONF_REFOUTEN_POS    | \
     ((uint32_t)(reserved_23)& 0x1UL)  << MAX2769_PLLCONF_RESERVED_23_POS | \
     ((uint32_t)(refdiv)     & 0x3UL)  << MAX2769_PLLCONF_REFDIV_POS      | \
     ((uint32_t)(ixtal)      & 0x3UL)  << MAX2769_PLLCONF_IXTAL_POS       | \
     ((uint32_t)(xtalcap)    & 0x1FUL) << MAX2769_PLLCONF_XTALCAP_POS     | \
     ((uint32_t)(ldmux)      & 0xFUL)  << MAX2769_PLLCONF_LDMUX_POS       | \
     ((uint32_t)(icp)        & 0x1UL)  << MAX2769_PLLCONF_ICP_POS         | \
     ((uint32_t)(pfden)      & 0x1UL)  << MAX2769_PLLCONF_PFDEN_POS       | \
     ((uint32_t)(reserved_7) & 0x1UL)  << MAX2769_PLLCONF_RESERVED_7_POS  | \
     ((uint32_t)(cptest)     & 0x7UL)  << MAX2769_PLLCONF_CPTEST_POS      | \
     ((uint32_t)(int_pll)    & 0x1UL)  << MAX2769_PLLCONF_INT_PLL_POS     | \
     ((uint32_t)(pwrsav)     & 0x1UL)  << MAX2769_PLLCONF_PWRSAV_POS      | \
     ((uint32_t)(reserved_1) & 0x1UL)  << MAX2769_PLLCONF_RESERVED_1_POS  | \
     ((uint32_t)(reserved_0) & 0x1UL)  << MAX2769_PLLCONF_RESERVED_0_POS)

#define MAX2769_PLLCONF_DEFAULT_BUILT \
    MAX2769_PLLCONF_BUILD(MAX2769_PLLCONF_VCOEN_DFLT, \
                          MAX2769_PLLCONF_IVCO_DFLT, \
                          MAX2769_PLLCONF_RESERVED_25_DFLT, \
                          MAX2769_PLLCONF_REFOUTEN_DFLT, \
                          MAX2769_PLLCONF_RESERVED_23_DFLT, \
                          MAX2769_PLLCONF_REFDIV_DFLT, \
                          MAX2769_PLLCONF_IXTAL_DFLT, \
                          MAX2769_PLLCONF_XTALCAP_DFLT, \
                          MAX2769_PLLCONF_LDMUX_DFLT, \
                          MAX2769_PLLCONF_ICP_DFLT, \
                          MAX2769_PLLCONF_PFDEN_DFLT, \
                          MAX2769_PLLCONF_RESERVED_7_DFLT, \
                          MAX2769_PLLCONF_CPTEST_DFLT, \
                          MAX2769_PLLCONF_INT_PLL_DFLT, \
                          MAX2769_PLLCONF_PWRSAV_DFLT, \
                          MAX2769_PLLCONF_RESERVED_1_DFLT, \
                          MAX2769_PLLCONF_RESERVED_0_DFLT)

/* MAX2769 DIV (Address 0100) field map from datasheet Table 10 */
#define MAX2769_DIV_NDIV_POS            (13U)
#define MAX2769_DIV_RDIV_POS            (3U)
#define MAX2769_DIV_RESERVED_2_0_POS    (0U)

#define MAX2769_DIV_NDIV_MASK           (0x7FFFUL << MAX2769_DIV_NDIV_POS)
#define MAX2769_DIV_RDIV_MASK           (0x3FFUL << MAX2769_DIV_RDIV_POS)
#define MAX2769_DIV_RESERVED_2_0_MASK   (0x7UL << MAX2769_DIV_RESERVED_2_0_POS)

#define MAX2769_DIV_NDIV_DFLT           (153UL)
#define MAX2769_DIV_RDIV_DFLT           (16UL)
#define MAX2769_DIV_RESERVED_2_0_DFLT   (0UL)

#define MAX2769_DIV_BUILD(ndiv, rdiv, reserved_2_0) \
    (((uint32_t)(ndiv)         & 0x7FFFUL) << MAX2769_DIV_NDIV_POS         | \
     ((uint32_t)(rdiv)         & 0x3FFUL)  << MAX2769_DIV_RDIV_POS         | \
     ((uint32_t)(reserved_2_0) & 0x7UL)    << MAX2769_DIV_RESERVED_2_0_POS)

#define MAX2769_DIV_DEFAULT_BUILT \
    MAX2769_DIV_BUILD(MAX2769_DIV_NDIV_DFLT, \
                      MAX2769_DIV_RDIV_DFLT, \
                      MAX2769_DIV_RESERVED_2_0_DFLT)

/* MAX2769 FDIV (Address 0101) field map from datasheet Table 11 */
#define MAX2769_FDIV_FDIV_POS           (8U)
#define MAX2769_FDIV_RESERVED_7_0_POS   (0U)

#define MAX2769_FDIV_FDIV_MASK          (0xFFFFFUL << MAX2769_FDIV_FDIV_POS)
#define MAX2769_FDIV_RESERVED_7_0_MASK  (0xFFUL << MAX2769_FDIV_RESERVED_7_0_POS)

#define MAX2769_FDIV_FDIV_DFLT          (0x80000UL)
#define MAX2769_FDIV_RESERVED_7_0_DFLT  (0x70UL)

#define MAX2769_FDIV_BUILD(fdiv, reserved_7_0) \
    (((uint32_t)(fdiv)         & 0xFFFFFUL) << MAX2769_FDIV_FDIV_POS         | \
     ((uint32_t)(reserved_7_0) & 0xFFUL)    << MAX2769_FDIV_RESERVED_7_0_POS)

#define MAX2769_FDIV_DEFAULT_BUILT \
    MAX2769_FDIV_BUILD(MAX2769_FDIV_FDIV_DFLT, \
                       MAX2769_FDIV_RESERVED_7_0_DFLT)

/* MAX2769 STRM (Address 0110) field map from datasheet Table 12 */
#define MAX2769_STRM_FRAMECOUNT_POS     (0U)
#define MAX2769_STRM_FRAMECOUNT_MASK    (0x0FFFFFFFUL << MAX2769_STRM_FRAMECOUNT_POS)
#define MAX2769_STRM_FRAMECOUNT_DFLT    (0x800000UL)

#define MAX2769_STRM_BUILD(framecount) \
    (((uint32_t)(framecount) & 0x0FFFFFFFUL) << MAX2769_STRM_FRAMECOUNT_POS)

#define MAX2769_STRM_DEFAULT_BUILT \
    MAX2769_STRM_BUILD(MAX2769_STRM_FRAMECOUNT_DFLT)

/* MAX2769 CLK (Address 0111) field map from datasheet Table 13 */
#define MAX2769_CLK_L_CNT_POS           (16U)
#define MAX2769_CLK_M_CNT_POS           (4U)
#define MAX2769_CLK_FCLKIN_POS          (3U)
#define MAX2769_CLK_ADCCLK_POS          (2U)
#define MAX2769_CLK_SERCLK_POS          (1U)
#define MAX2769_CLK_MODE_POS            (0U)

#define MAX2769_CLK_L_CNT_MASK          (0xFFFUL << MAX2769_CLK_L_CNT_POS)
#define MAX2769_CLK_M_CNT_MASK          (0xFFFUL << MAX2769_CLK_M_CNT_POS)
#define MAX2769_CLK_FCLKIN_MASK         (0x1UL << MAX2769_CLK_FCLKIN_POS)
#define MAX2769_CLK_ADCCLK_MASK         (0x1UL << MAX2769_CLK_ADCCLK_POS)
#define MAX2769_CLK_SERCLK_MASK         (0x1UL << MAX2769_CLK_SERCLK_POS)
#define MAX2769_CLK_MODE_MASK           (0x1UL << MAX2769_CLK_MODE_POS)

#define MAX2769_CLK_L_CNT_DFLT          (256UL)
#define MAX2769_CLK_M_CNT_DFLT          (1563UL)
#define MAX2769_CLK_FCLKIN_DFLT         (0UL)
#define MAX2769_CLK_ADCCLK_DFLT         (0UL)
#define MAX2769_CLK_SERCLK_DFLT         (1UL)
#define MAX2769_CLK_MODE_DFLT           (0UL)

#define MAX2769_CLK_BUILD(l_cnt, m_cnt, fclkin, adcclk, serclk, mode) \
    (((uint32_t)(l_cnt)  & 0xFFFUL) << MAX2769_CLK_L_CNT_POS  | \
     ((uint32_t)(m_cnt)  & 0xFFFUL) << MAX2769_CLK_M_CNT_POS  | \
     ((uint32_t)(fclkin) & 0x1UL)   << MAX2769_CLK_FCLKIN_POS | \
     ((uint32_t)(adcclk) & 0x1UL)   << MAX2769_CLK_ADCCLK_POS | \
     ((uint32_t)(serclk) & 0x1UL)   << MAX2769_CLK_SERCLK_POS | \
     ((uint32_t)(mode)   & 0x1UL)   << MAX2769_CLK_MODE_POS)

#define MAX2769_CLK_DEFAULT_BUILT \
    MAX2769_CLK_BUILD(MAX2769_CLK_L_CNT_DFLT, \
                      MAX2769_CLK_M_CNT_DFLT, \
                      MAX2769_CLK_FCLKIN_DFLT, \
                      MAX2769_CLK_ADCCLK_DFLT, \
                      MAX2769_CLK_SERCLK_DFLT, \
                      MAX2769_CLK_MODE_DFLT)

/* MAX2769 TEST1 (Address 1000) and TEST2 (Address 1001) from Tables 14-15 */
#define MAX2769_TEST1_RESERVED_27_0_POS   (0U)
#define MAX2769_TEST1_RESERVED_27_0_MASK  (0x0FFFFFFFUL << MAX2769_TEST1_RESERVED_27_0_POS)
#define MAX2769_TEST1_RESERVED_27_0_DFLT  (0x1E0F401UL)

#define MAX2769_TEST2_RESERVED_27_0_POS   (0U)
#define MAX2769_TEST2_RESERVED_27_0_MASK  (0x0FFFFFFFUL << MAX2769_TEST2_RESERVED_27_0_POS)
#define MAX2769_TEST2_RESERVED_27_0_DFLT  (0x14C0402UL)

#define MAX2769_TEST1_BUILD(reserved_27_0) \
    (((uint32_t)(reserved_27_0) & 0x0FFFFFFFUL) << MAX2769_TEST1_RESERVED_27_0_POS)

#define MAX2769_TEST2_BUILD(reserved_27_0) \
    (((uint32_t)(reserved_27_0) & 0x0FFFFFFFUL) << MAX2769_TEST2_RESERVED_27_0_POS)

#define MAX2769_TEST1_DEFAULT_BUILT \
    MAX2769_TEST1_BUILD(MAX2769_TEST1_RESERVED_27_0_DFLT)

#define MAX2769_TEST2_DEFAULT_BUILT \
    MAX2769_TEST2_BUILD(MAX2769_TEST2_RESERVED_27_0_DFLT)

typedef enum
{
    MAX2769_STATUS_OK = 0,
    MAX2769_STATUS_INVALID_ARG = -1,
    MAX2769_STATUS_IO_ERROR = -2
} max2769_status_t;

typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_PinState cs_active_state;
    uint32_t spi_timeout_ms;
} max2769_t;

extern const uint32_t max2769_default_reg_values[10];

max2769_status_t max2769_init(max2769_t *dev,
                              SPI_HandleTypeDef *hspi,
                              GPIO_TypeDef *cs_port,
                              uint16_t cs_pin,
                              GPIO_PinState cs_active_state,
                              uint32_t spi_timeout_ms);
max2769_status_t max2769_write_register(max2769_t *dev, max2769_register_t reg, uint32_t data_d27_d0);
max2769_status_t max2769_write_defaults(max2769_t *dev);
uint32_t max2769_build_frame(max2769_register_t reg, uint32_t data_d27_d0);

#ifdef __cplusplus
}
#endif

#endif /* MAX2769_H */
