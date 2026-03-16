#ifndef TRF_MODULATOR_H
#define TRF_MODULATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Uncomment and change to your specific STM32 family HAL header */
// #include "stm32g4xx_hal.h"   // Example for G4 series
// #include "stm32f4xx_hal.h"   // Example for F4 series
// #include "stm32h7xx_hal.h"   // Example for H7 series

/* ========================================================================= */
/*                           TRF372017 Register Map                          */
/* ========================================================================= */

#define TRF372017_REG0_ADDR     0x02  // ADDR[4:0] = 01000 (Transmitted LSB-first as 00010)
#define TRF372017_REG1_ADDR     0x09  // ADDR[4:0] = 01001 (Transmitted LSB-first as 10010)
#define TRF372017_REG2_ADDR     0x0A  // ADDR[4:0] = 01010 (Transmitted LSB-first as 01010)
#define TRF372017_REG3_ADDR     0x0B  // ADDR[4:0] = 01011 (Transmitted LSB-first as 11010)
#define TRF372017_REG4_ADDR     0x0C  // ADDR[4:0] = 00110 (Transmitted LSB-first as 01100)
#define TRF372017_REG5_ADDR     0x0D  // ADDR[4:0] = 10110 (Transmitted LSB-first as 01101)
#define TRF372017_REG6_ADDR     0x0E  // ADDR[4:0] = 01110 (Transmitted LSB-first as 01110)
#define TRF372017_REG7_ADDR     0x0F  // ADDR[4:0] = 11110 (Transmitted LSB-first as 01111)

// Define other registers as they become available...

/* ========================================================================= */
/*                          Register 1 (PLL SPI Register)                    */
/* ========================================================================= */

/**
 * @brief Register 1 Bit Shifts and Masks
 */
#define TRF_REG1_ADDR_SHIFT     0
#define TRF_REG1_ADDR_MASK     (0x1F << TRF_REG1_ADDR_SHIFT)

#define TRF_REG1_RDIV_SHIFT     5
#define TRF_REG1_RDIV_MASK     (0x1FFF << TRF_REG1_RDIV_SHIFT)

#define TRF_REG1_RSV18_SHIFT    18
#define TRF_REG1_RSV18_MASK    (0x1 << TRF_REG1_RSV18_SHIFT)

#define TRF_REG1_REF_INV_SHIFT  19
#define TRF_REG1_REF_INV_MASK  (0x1 << TRF_REG1_REF_INV_SHIFT)

#define TRF_REG1_NEG_VCO_SHIFT  20
#define TRF_REG1_NEG_VCO_MASK  (0x1 << TRF_REG1_NEG_VCO_SHIFT)

#define TRF_REG1_ICP_SHIFT      21
#define TRF_REG1_ICP_MASK      (0x1F << TRF_REG1_ICP_SHIFT)

#define TRF_REG1_ICPDOUBLE_SHIFT 26
#define TRF_REG1_ICPDOUBLE_MASK (0x1 << TRF_REG1_ICPDOUBLE_SHIFT)

#define TRF_REG1_CAL_CLK_SEL_SHIFT 27
#define TRF_REG1_CAL_CLK_SEL_MASK (0xF << TRF_REG1_CAL_CLK_SEL_SHIFT)

#define TRF_REG1_RSV31_SHIFT    31
#define TRF_REG1_RSV31_MASK    (0x1 << TRF_REG1_RSV31_SHIFT)


/**
 * @brief Enumeration for Charge Pump Current (ICP) setting in Register 1
 */
typedef enum {
    TRF_ICP_1_94_MA = 0x00, /* B[25..21] = [00 000] */
    TRF_ICP_0_97_MA = 0x0A, /* B[25..21] = [01 010] (Default) */
    TRF_ICP_0_47_MA = 0x1F  /* B[25..21] = [11 111] */
    // Note: Other intermediate values are likely valid between 0x00 and 0x1F
} TRF_ChargePumpCurrent_t;

/**
 * @brief Configuration Structure for Register 1
 */
typedef struct {
    uint16_t rdiv;                  /*!< 13-bit Reference Divider Value (1 to 8191) */
    uint8_t  ref_inv;               /*!< Invert Reference Clock Polarity (0=Rising, 1=Falling) */
    uint8_t  neg_vco;               /*!< VCO Polarity Control (1=Negative slope) */
    TRF_ChargePumpCurrent_t icp;    /*!< Charge Pump DC Current (5 bits) */
    uint8_t  icp_double;            /*!< Set ICP to double the current (0=Disabled, 1=Enabled) */
    uint8_t  cal_clk_sel;           /*!< Multiplication/division factor for VCO calibration clock (4 bits) */
} TRF_Reg1_Config_t;

/**
 * @brief Default Configuration for Register 1 (Based on "Reset Value" column)
 */
static const TRF_Reg1_Config_t TRF_REG1_DEFAULT_CONFIG = {
    .rdiv = 1,                      /* B[17..5] = 0000000000001 */
    .ref_inv = 0,                   /* B[19] = 0 */
    .neg_vco = 1,                   /* B[20] = 1 */
    .icp = TRF_ICP_0_97_MA,         /* B[25..21] = 01010 */
    .icp_double = 0,                /* B[26] = 0 */
    .cal_clk_sel = 0x08             /* B[30..27] = 1000 */
};

/* ========================================================================= */
/*                          Register 2                                       */
/* ========================================================================= */

#define TRF_REG2_NINT_SHIFT       5
#define TRF_REG2_NINT_MASK       (0xFFFF << TRF_REG2_NINT_SHIFT)   // 16 bits

#define TRF_REG2_PLL_DIV_SHIFT    21
#define TRF_REG2_PLL_DIV_MASK    (0x3 << TRF_REG2_PLL_DIV_SHIFT)   // 2 bits

#define TRF_REG2_PRSC_SEL_SHIFT   23
#define TRF_REG2_PRSC_SEL_MASK   (0x1 << TRF_REG2_PRSC_SEL_SHIFT)  // 1 bit

#define TRF_REG2_RSV24_SHIFT      24
#define TRF_REG2_RSV24_MASK      (0x3 << TRF_REG2_RSV24_SHIFT)     // 2 bits (24, 25)

#define TRF_REG2_VCO_SEL_SHIFT    26
#define TRF_REG2_VCO_SEL_MASK    (0x3 << TRF_REG2_VCO_SEL_SHIFT)   // 2 bits

#define TRF_REG2_VCOSEL_MODE_SHIFT 28
#define TRF_REG2_VCOSEL_MODE_MASK (0x1 << TRF_REG2_VCOSEL_MODE_SHIFT) // 1 bit

#define TRF_REG2_CAL_ACC_SHIFT    29
#define TRF_REG2_CAL_ACC_MASK    (0x3 << TRF_REG2_CAL_ACC_SHIFT)   // 2 bits

#define TRF_REG2_EN_CAL_SHIFT     31
#define TRF_REG2_EN_CAL_MASK     (0x1 << TRF_REG2_EN_CAL_SHIFT)    // 1 bit

/**
 * @brief PLL Division Settings (Divider in front of prescaler)
 */
typedef enum {
    TRF_PLL_DIV_1 = 0x00,
    TRF_PLL_DIV_2 = 0x01,
    TRF_PLL_DIV_4 = 0x02
} TRF_PLL_Divider_t;

/**
 * @brief Prescaler Modulus Selection
 */
typedef enum {
    TRF_PRSC_4_5 = 0x00, // 4/5
    TRF_PRSC_8_9 = 0x01  // 8/9
} TRF_Prescaler_t;

/**
 * @brief Configuration Structure for Register 2
 */
typedef struct {
    uint16_t n_int;                 /*!< PLL N-divider division setting (16 bits) */
    TRF_PLL_Divider_t pll_div;      /*!< Divider in front of prescaler (2 bits) */
    TRF_Prescaler_t prsc_sel;       /*!< Prescaler modulus (0 = 4/5; 1 = 8/9) */
    uint8_t  vco_sel;               /*!< Select between 4 integrated VCOs (00=lowest, 11=highest) */
    uint8_t  vcosel_mode;           /*!< Single VCO auto-calibration mode (1 = active) */
    uint8_t  cal_acc;               /*!< Error count during cap array cal (Recommended: 00) */
    uint8_t  en_cal;                /*!< Execute VCO freq auto-cal. Set to 1 to initiate. */
} TRF_Reg2_Config_t;

/**
 * @brief Default Configuration for Register 2 (Based on "Reset Value" column)
 */
static const TRF_Reg2_Config_t TRF_REG2_DEFAULT_CONFIG = {
    .n_int = 0x0080,                /* B[20..5] = 000000010000000  (Bit 12 is 1) */
    .pll_div = TRF_PLL_DIV_2,       /* B[22..21] = 01 */
    .prsc_sel = TRF_PRSC_8_9,       /* B[23] = 1 */
    .vco_sel = 0x02,                /* B[27..26] = 10 */
    .vcosel_mode = 0,               /* B[28] = 0 */
    .cal_acc = 0,                   /* B[30..29] = 00 */
    .en_cal = 0                     /* B[31] = 0 */
};

/* ========================================================================= */
/*                          Register 3                                       */
/* ========================================================================= */

#define TRF_REG3_NFRAC_SHIFT      5
#define TRF_REG3_NFRAC_MASK      (0x1FFFFFF << TRF_REG3_NFRAC_SHIFT) // 25 bits

#define TRF_REG3_RSV30_SHIFT      30
#define TRF_REG3_RSV30_MASK      (0x3 << TRF_REG3_RSV30_SHIFT)       // 2 bits (30, 31)

/**
 * @brief Configuration Structure for Register 3
 */
typedef struct {
    uint32_t n_frac;                /*!< Fractional PLL N divider value (25 bits) */
} TRF_Reg3_Config_t;

/**
 * @brief Default Configuration for Register 3 (Based on "Reset Value" column)
 */
static const TRF_Reg3_Config_t TRF_REG3_DEFAULT_CONFIG = {
    .n_frac = 0                     /* B[29..5] = 0 */
};

/* ========================================================================= */
/*                          Register 4                                       */
/* ========================================================================= */

#define TRF_REG4_PWD_PLL_SHIFT      5
#define TRF_REG4_PWD_PLL_MASK      (0x1 << TRF_REG4_PWD_PLL_SHIFT)
#define TRF_REG4_PWD_CP_SHIFT       6
#define TRF_REG4_PWD_CP_MASK       (0x1 << TRF_REG4_PWD_CP_SHIFT)
#define TRF_REG4_PWD_VCO_SHIFT      7
#define TRF_REG4_PWD_VCO_MASK      (0x1 << TRF_REG4_PWD_VCO_SHIFT)
#define TRF_REG4_PWD_VCOMUX_SHIFT   8
#define TRF_REG4_PWD_VCOMUX_MASK   (0x1 << TRF_REG4_PWD_VCOMUX_SHIFT)
#define TRF_REG4_PWD_DIV124_SHIFT   9
#define TRF_REG4_PWD_DIV124_MASK   (0x1 << TRF_REG4_PWD_DIV124_SHIFT)
#define TRF_REG4_PWD_PRESC_SHIFT    10
#define TRF_REG4_PWD_PRESC_MASK    (0x1 << TRF_REG4_PWD_PRESC_SHIFT)
#define TRF_REG4_RSV11_SHIFT        11
#define TRF_REG4_RSV11_MASK        (0x1 << TRF_REG4_RSV11_SHIFT)
#define TRF_REG4_PWD_OUT_BUFF_SHIFT 12
#define TRF_REG4_PWD_OUT_BUFF_MASK (0x1 << TRF_REG4_PWD_OUT_BUFF_SHIFT)
#define TRF_REG4_PWD_LO_DIV_SHIFT   13
#define TRF_REG4_PWD_LO_DIV_MASK   (0x1 << TRF_REG4_PWD_LO_DIV_SHIFT)
#define TRF_REG4_PWD_TX_DIV_SHIFT   14
#define TRF_REG4_PWD_TX_DIV_MASK   (0x1 << TRF_REG4_PWD_TX_DIV_SHIFT)
#define TRF_REG4_PWD_BB_VCM_SHIFT   15
#define TRF_REG4_PWD_BB_VCM_MASK   (0x1 << TRF_REG4_PWD_BB_VCM_SHIFT)
#define TRF_REG4_PWD_DC_OFF_SHIFT   16
#define TRF_REG4_PWD_DC_OFF_MASK   (0x1 << TRF_REG4_PWD_DC_OFF_SHIFT)

#define TRF_REG4_EN_EXTVCO_SHIFT    17
#define TRF_REG4_EN_EXTVCO_MASK    (0x1 << TRF_REG4_EN_EXTVCO_SHIFT)
#define TRF_REG4_EN_ISOURCE_SHIFT   18
#define TRF_REG4_EN_ISOURCE_MASK   (0x1 << TRF_REG4_EN_ISOURCE_SHIFT)
#define TRF_REG4_LD_ANA_PREC_SHIFT  19
#define TRF_REG4_LD_ANA_PREC_MASK  (0x3 << TRF_REG4_LD_ANA_PREC_SHIFT)
#define TRF_REG4_CP_TRISTATE_SHIFT  21
#define TRF_REG4_CP_TRISTATE_MASK  (0x3 << TRF_REG4_CP_TRISTATE_SHIFT)
#define TRF_REG4_SPEEDUP_SHIFT      23
#define TRF_REG4_SPEEDUP_MASK      (0x1 << TRF_REG4_SPEEDUP_SHIFT)
#define TRF_REG4_LD_DIG_PREC_SHIFT  24
#define TRF_REG4_LD_DIG_PREC_MASK  (0x1 << TRF_REG4_LD_DIG_PREC_SHIFT)
#define TRF_REG4_EN_DITH_SHIFT      25
#define TRF_REG4_EN_DITH_MASK      (0x1 << TRF_REG4_EN_DITH_SHIFT)
#define TRF_REG4_MOD_ORD_SHIFT      26
#define TRF_REG4_MOD_ORD_MASK      (0x3 << TRF_REG4_MOD_ORD_SHIFT)
#define TRF_REG4_DITH_SEL_SHIFT     28
#define TRF_REG4_DITH_SEL_MASK     (0x1 << TRF_REG4_DITH_SEL_SHIFT)
#define TRF_REG4_DEL_SD_CLK_SHIFT   29
#define TRF_REG4_DEL_SD_CLK_MASK   (0x3 << TRF_REG4_DEL_SD_CLK_SHIFT)
#define TRF_REG4_EN_FRAC_SHIFT      31
#define TRF_REG4_EN_FRAC_MASK      (0x1 << TRF_REG4_EN_FRAC_SHIFT)

/**
 * @brief Configuration Structure for Register 4
 */
typedef struct {
    uint8_t pwd_pll;
    uint8_t pwd_cp;
    uint8_t pwd_vco;
    uint8_t pwd_vcomux;
    uint8_t pwd_div124;
    uint8_t pwd_presc;
    uint8_t pwd_out_buff;
    uint8_t pwd_lo_div;
    uint8_t pwd_tx_div;
    uint8_t pwd_bb_vcm;
    uint8_t pwd_dc_off;
    uint8_t en_extvco;
    uint8_t en_isource;
    uint8_t ld_ana_prec;    // 2 bits
    uint8_t cp_tristate;    // 2 bits
    uint8_t speedup;
    uint8_t ld_dig_prec;
    uint8_t en_dith;
    uint8_t mod_ord;        // 2 bits
    uint8_t dith_sel;
    uint8_t del_sd_clk;     // 2 bits
    uint8_t en_frac;
} TRF_Reg4_Config_t;

/**
 * @brief Default Configuration for Register 4 (Based on "Reset Value" column)
 */
static const TRF_Reg4_Config_t TRF_REG4_DEFAULT_CONFIG = {
    .pwd_pll = 0, .pwd_cp = 0, .pwd_vco = 0, .pwd_vcomux = 0, .pwd_div124 = 0, .pwd_presc = 0,
    .pwd_out_buff = 1, .pwd_lo_div = 1, .pwd_tx_div = 1, .pwd_bb_vcm = 1, .pwd_dc_off = 1,
    .en_extvco = 0, .en_isource = 0, .ld_ana_prec = 0, .cp_tristate = 0, .speedup = 0,
    .ld_dig_prec = 0, .en_dith = 1, .mod_ord = 0x02, .dith_sel = 0, .del_sd_clk = 0x02, .en_frac = 0
};

/* ========================================================================= */
/*                          Register 5                                       */
/* ========================================================================= */

#define TRF_REG5_VCOBIAS_RTRIM_SHIFT  5
#define TRF_REG5_VCOBIAS_RTRIM_MASK  (0x7 << TRF_REG5_VCOBIAS_RTRIM_SHIFT) // 3 bits
#define TRF_REG5_PLLBIAS_RTRIM_SHIFT  8
#define TRF_REG5_PLLBIAS_RTRIM_MASK  (0x3 << TRF_REG5_PLLBIAS_RTRIM_SHIFT) // 2 bits
#define TRF_REG5_VCO_BIAS_SHIFT       10
#define TRF_REG5_VCO_BIAS_MASK       (0xF << TRF_REG5_VCO_BIAS_SHIFT)      // 4 bits
#define TRF_REG5_VCOBUF_BIAS_SHIFT    14
#define TRF_REG5_VCOBUF_BIAS_MASK    (0x3 << TRF_REG5_VCOBUF_BIAS_SHIFT)   // 2 bits
#define TRF_REG5_VCOMUX_BIAS_SHIFT    16
#define TRF_REG5_VCOMUX_BIAS_MASK    (0x3 << TRF_REG5_VCOMUX_BIAS_SHIFT)   // 2 bits
#define TRF_REG5_BUFOUT_BIAS_SHIFT    18
#define TRF_REG5_BUFOUT_BIAS_MASK    (0x3 << TRF_REG5_BUFOUT_BIAS_SHIFT)   // 2 bits
#define TRF_REG5_RSV20_SHIFT          20
#define TRF_REG5_RSV20_MASK          (0x3 << TRF_REG5_RSV20_SHIFT)         // 2 bits (20=0, 21=1)
#define TRF_REG5_VCO_CAL_IB_SHIFT     22
#define TRF_REG5_VCO_CAL_IB_MASK     (0x1 << TRF_REG5_VCO_CAL_IB_SHIFT)    // 1 bit
#define TRF_REG5_VCO_CAL_REF_SHIFT    23
#define TRF_REG5_VCO_CAL_REF_MASK    (0x7 << TRF_REG5_VCO_CAL_REF_SHIFT)   // 3 bits
#define TRF_REG5_VCO_AMPL_CTRL_SHIFT  26
#define TRF_REG5_VCO_AMPL_CTRL_MASK  (0x3 << TRF_REG5_VCO_AMPL_CTRL_SHIFT) // 2 bits
#define TRF_REG5_VCO_VB_CTRL_SHIFT    28
#define TRF_REG5_VCO_VB_CTRL_MASK    (0x3 << TRF_REG5_VCO_VB_CTRL_SHIFT)   // 2 bits
#define TRF_REG5_RSV30_SHIFT          30
#define TRF_REG5_RSV30_MASK          (0x1 << TRF_REG5_RSV30_SHIFT)         // 1 bit (30=0)
#define TRF_REG5_EN_LD_ISOURCE_SHIFT  31
#define TRF_REG5_EN_LD_ISOURCE_MASK  (0x1 << TRF_REG5_EN_LD_ISOURCE_SHIFT) // 1 bit

/**
 * @brief Configuration Structure for Register 5
 */
typedef struct {
    uint8_t vcobias_rtrim;  // 3 bits
    uint8_t pllbias_rtrim;  // 2 bits
    uint8_t vco_bias;       // 4 bits
    uint8_t vcobuf_bias;    // 2 bits
    uint8_t vcomux_bias;    // 2 bits
    uint8_t bufout_bias;    // 2 bits
    uint8_t vco_cal_ib;
    uint8_t vco_cal_ref;    // 3 bits
    uint8_t vco_ampl_ctrl;  // 2 bits
    uint8_t vco_vb_ctrl;    // 2 bits
    uint8_t en_ld_isource;
} TRF_Reg5_Config_t;

/**
 * @brief Default Configuration for Register 5 (Based on "Recommended programming" where applicable, or reset)
 */
static const TRF_Reg5_Config_t TRF_REG5_DEFAULT_CONFIG = {
    .vcobias_rtrim = 0x04,  // b100
    .pllbias_rtrim = 0x02,  // b10
    // VCO Bias recommended varies with VCC_VCO2. Default 3.3V is 0101 (0x5).
    .vco_bias = 0x05,      
    .vcobuf_bias = 0x02,    // b10
    .vcomux_bias = 0x03,    // b11
    .bufout_bias = 0x02,    // No clear recommended, reset is 10 (0x2)
    .vco_cal_ib = 0,        // 0
    .vco_cal_ref = 0x02,    // b010
    .vco_ampl_ctrl = 0x03,  // b11
    .vco_vb_ctrl = 0x00,    // b00
    .en_ld_isource = 0      // b0
};

/* ========================================================================= */
/*                          Register 6                                       */
/* ========================================================================= */

#define TRF_REG6_IOFF_SHIFT         5
#define TRF_REG6_IOFF_MASK         (0xFF << TRF_REG6_IOFF_SHIFT)       // 8 bits
#define TRF_REG6_QOFF_SHIFT         13
#define TRF_REG6_QOFF_MASK         (0xFF << TRF_REG6_QOFF_SHIFT)       // 8 bits
#define TRF_REG6_VREF_SEL_SHIFT     21
#define TRF_REG6_VREF_SEL_MASK     (0x7 << TRF_REG6_VREF_SEL_SHIFT)    // 3 bits
#define TRF_REG6_TX_DIV_SEL_SHIFT   24
#define TRF_REG6_TX_DIV_SEL_MASK   (0x3 << TRF_REG6_TX_DIV_SEL_SHIFT)  // 2 bits
#define TRF_REG6_LO_DIV_SEL_SHIFT   26
#define TRF_REG6_LO_DIV_SEL_MASK   (0x3 << TRF_REG6_LO_DIV_SEL_SHIFT)  // 2 bits
#define TRF_REG6_TX_DIV_BIAS_SHIFT  28
#define TRF_REG6_TX_DIV_BIAS_MASK  (0x3 << TRF_REG6_TX_DIV_BIAS_SHIFT) // 2 bits
#define TRF_REG6_LO_DIV_BIAS_SHIFT  30
#define TRF_REG6_LO_DIV_BIAS_MASK  (0x3 << TRF_REG6_LO_DIV_BIAS_SHIFT) // 2 bits

/**
 * @brief Configuration Structure for Register 6
 */
typedef struct {
    uint8_t ioff;           /*!< Adjust Iref current for I DC offset (8 bits) */
    uint8_t qoff;           /*!< Adjust Iref current for Q DC offset (8 bits) */
    uint8_t vref_sel;       /*!< Adjust Vref in baseband common mode (3 bits) */
    uint8_t tx_div_sel;     /*!< Adjust Tx path divider (2 bits) */
    uint8_t lo_div_sel;     /*!< Adjust LO path divider (2 bits) */
    uint8_t tx_div_bias;    /*!< TX divider bias reference current (2 bits) */
    uint8_t lo_div_bias;    /*!< LO divider bias reference current (2 bits) */
} TRF_Reg6_Config_t;

/**
 * @brief Default Configuration for Register 6 (Based on "Reset Value" column)
 */
static const TRF_Reg6_Config_t TRF_REG6_DEFAULT_CONFIG = {
    .ioff = 0x80,           // 10000000 (Mid scale)
    .qoff = 0x80,           // 10000000 (Mid scale)
    .vref_sel = 0x04,       // b100 (Recommended programming)
    .tx_div_sel = 0x00,     // 00 = Div1
    .lo_div_sel = 0x00,     // 00 = Div1
    .tx_div_bias = 0x02,    // 10 = 50 uA
    .lo_div_bias = 0x02     // 10 = 50 uA
};

/* ========================================================================= */
/*                          Register 7                                       */
/* ========================================================================= */

#define TRF_REG7_RSV5_SHIFT         5
#define TRF_REG7_RSV5_MASK         (0x3 << TRF_REG7_RSV5_SHIFT)        // 2 bits (5, 6) both 0
#define TRF_REG7_VCO_TRIM_SHIFT     7
#define TRF_REG7_VCO_TRIM_MASK     (0x3F << TRF_REG7_VCO_TRIM_SHIFT)   // 6 bits
#define TRF_REG7_RSV13_SHIFT        13
#define TRF_REG7_RSV13_MASK        (0x1 << TRF_REG7_RSV13_SHIFT)       // 1 bit
#define TRF_REG7_VCO_TEST_MODE_SHIFT 14
#define TRF_REG7_VCO_TEST_MODE_MASK (0x1 << TRF_REG7_VCO_TEST_MODE_SHIFT)// 1 bit
#define TRF_REG7_CAL_BYPASS_SHIFT   15
#define TRF_REG7_CAL_BYPASS_MASK   (0x1 << TRF_REG7_CAL_BYPASS_SHIFT)  // 1 bit
#define TRF_REG7_MUX_CTRL_SHIFT     16
#define TRF_REG7_MUX_CTRL_MASK     (0x7 << TRF_REG7_MUX_CTRL_SHIFT)    // 3 bits
#define TRF_REG7_ISOURCE_SINK_SHIFT 19
#define TRF_REG7_ISOURCE_SINK_MASK (0x1 << TRF_REG7_ISOURCE_SINK_SHIFT)// 1 bit
#define TRF_REG7_ISOURCE_TRIM_SHIFT 20
#define TRF_REG7_ISOURCE_TRIM_MASK (0x7 << TRF_REG7_ISOURCE_TRIM_SHIFT)// 3 bits
#define TRF_REG7_PD_TC_SHIFT        23
#define TRF_REG7_PD_TC_MASK        (0x3 << TRF_REG7_PD_TC_SHIFT)       // 2 bits
#define TRF_REG7_IB_VCM_SEL_SHIFT   25
#define TRF_REG7_IB_VCM_SEL_MASK   (0x1 << TRF_REG7_IB_VCM_SEL_SHIFT)  // 1 bit
#define TRF_REG7_RSV26_SHIFT        26
#define TRF_REG7_RSV26_MASK        (0x7 << TRF_REG7_RSV26_SHIFT)       // 3 bits (26=0, 27=0, 28=1)
#define TRF_REG7_DCOFFSET_I_SHIFT   29
#define TRF_REG7_DCOFFSET_I_MASK   (0x3 << TRF_REG7_DCOFFSET_I_SHIFT)  // 2 bits
#define TRF_REG7_VCO_BIAS_SEL_SHIFT 31
#define TRF_REG7_VCO_BIAS_SEL_MASK (0x1 << TRF_REG7_VCO_BIAS_SEL_SHIFT)// 1 bit

/**
 * @brief Configuration Structure for Register 7
 */
typedef struct {
    uint8_t vco_trim;       /*!< VCO capacitor array control bits (6 bits) */
    uint8_t vco_test_mode;  /*!< Counter mode: measure max/min freq of VCO */
    uint8_t cal_bypass;     /*!< Bypass VCO auto-calibration (1 = active) */
    uint8_t mux_ctrl;       /*!< Select signal for test output at pin 5 (3 bits) */
    uint8_t isource_sink;   /*!< Charge pump offset current polarity */
    uint8_t isource_trim;   /*!< Adjust isource bias current in frac-n mode (3 bits) */
    uint8_t pd_tc;          /*!< Time constant control for PWD_OUT_BUFF (2 bits) */
    uint8_t ib_vcm_sel;     /*!< Select const/ptat current for Common mode bias */
    uint8_t dcoffset_i;     /*!< Adjust BB input DC offset Iref (2 bits) */
    uint8_t vco_bias_sel;   /*!< Select VCO_BIAS trim settings source */
} TRF_Reg7_Config_t;

/**
 * @brief Default Configuration for Register 7 (Based on "Reset Value" column)
 */
static const TRF_Reg7_Config_t TRF_REG7_DEFAULT_CONFIG = {
    .vco_trim = 0x20,       // 100000 (Bit 12 is 1)
    .vco_test_mode = 0,     // 0
    .cal_bypass = 0,        // 0
    .mux_ctrl = 0x01,       // 001 (Bit 16 is 1)
    .isource_sink = 0,      // 0
    .isource_trim = 0x04,   // 100 (Bit 22 is 1)
    .pd_tc = 0x00,          // 00
    .ib_vcm_sel = 0,        // 0
    .dcoffset_i = 0x02,     // 10 (Bit 30 is 1)
    .vco_bias_sel = 0x01    // Recommended programming = 1
};

/* ========================================================================= */
/*                          Register 0 (Readback)                            */
/* ========================================================================= */

// Register 0 - Write (Setup Readback)
#define TRF_REG0_W_COUNT_MODE_MUX_SEL_SHIFT 27
#define TRF_REG0_W_COUNT_MODE_MUX_SEL_MASK (0x1 << TRF_REG0_W_COUNT_MODE_MUX_SEL_SHIFT)
#define TRF_REG0_W_RB_REG_SHIFT             28
#define TRF_REG0_W_RB_REG_MASK             (0x7 << TRF_REG0_W_RB_REG_SHIFT)
#define TRF_REG0_W_RB_ENABLE_SHIFT          31
#define TRF_REG0_W_RB_ENABLE_MASK          (0x1 << TRF_REG0_W_RB_ENABLE_SHIFT)

/**
 * @brief Configuration Structure for Register 0 (Write / Setup Readback)
 */
typedef struct {
    uint8_t count_mode_mux_sel; /*!< VCO freq count mode (0=Max, 1=Min) */
    uint8_t rb_reg;             /*!< Address of the register to read back (000 to 111) */
    uint8_t rb_enable;          /*!< Put device in Readback Mode (1=Enable) */
} TRF_Reg0_Write_Config_t;

// Register 0 - Read (Readback Data)
#define TRF_REG0_R_CHIP_ID_SHIFT            5
#define TRF_REG0_R_CHIP_ID_MASK            (0x3 << TRF_REG0_R_CHIP_ID_SHIFT)      // 2 bits
#define TRF_REG0_R_R_SAT_ERR_SHIFT          12
#define TRF_REG0_R_R_SAT_ERR_MASK          (0x1 << TRF_REG0_R_R_SAT_ERR_SHIFT)    // 1 bit
#define TRF_REG0_R_VCO_TRIM_AUTOCAL_SHIFT   15
#define TRF_REG0_R_VCO_TRIM_AUTOCAL_MASK   (0x3F << TRF_REG0_R_VCO_TRIM_AUTOCAL_SHIFT) // 6 bits
#define TRF_REG0_R_VCO_SEL_AUTOCAL_SHIFT    22
#define TRF_REG0_R_VCO_SEL_AUTOCAL_MASK    (0x3 << TRF_REG0_R_VCO_SEL_AUTOCAL_SHIFT) // 2 bits
#define TRF_REG0_R_COUNT_FREQ_SHIFT         13
#define TRF_REG0_R_COUNT_FREQ_MASK         (0x3FFFF << TRF_REG0_R_COUNT_FREQ_SHIFT) // 18 bits

/**
 * @brief Parsed Data Structure for Register 0 Readback
 */
typedef struct {
    uint8_t chip_id;            /*!< Chip ID (B[6..5]) */
    uint8_t r_sat_err;          /*!< Error flag for calibration speed */
    uint8_t vco_trim_autocal;   /*!< Autocal results for VCO_TRIM (available when VCO_TEST_MODE = 0) */
    uint8_t vco_sel_autocal;    /*!< Autocal results for VCO_SEL (available when VCO_TEST_MODE = 0) */
    uint32_t count_freq;        /*!< VCO frequency counter (available when COUNT_MODE_MUX_SEL valid and VCO_TEST_MODE = 1) */
} TRF_Reg0_Read_Data_t;


/* ========================================================================= */
/*                          Hardware Abstraction Types                       */
/* ========================================================================= */

/**
 * @brief SPI Communication Note:
 * TRF372017 requires SPI configured as:
 * - Data Size: 8 Bits (we send 4 bytes = 32 bits)
 * - First Bit: LSB First
 * - Clock Polarity (CPOL): Low (Mode 0)
 * - Clock Phase (CPHA): 1 Edge (Mode 0)
 */

/**
 * @brief Communication interface configuration mapping.
 */
typedef struct {
    void *spi_handle;          // Pointer to SPI_HandleTypeDef 
    void *le_port;             // Pointer to GPIO_TypeDef for Latch Enable (LE) port
    uint16_t le_pin;           // GPIO_Pin for Latch Enable (LE)
    // Optional: Add other control pins here (e.g., Enable, Reset, Lock Detect)
} TRF_Modulator_Config_t;

/**
 * @brief TRF Modulator Handle Structure Definition
 */
typedef struct {
    TRF_Modulator_Config_t config;
    bool is_initialized;
    // Add IC specific state variables here if needed
} TRF_Modulator_HandleTypeDef;

/**
 * @brief  Initialize the TRF modulator.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Init(TRF_Modulator_HandleTypeDef *hmod);

/**
 * @brief  Write data to a specific register (TRF372017 specific 32-bit transaction).
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  reg_addr: 5-bit register address.
 * @param  data: 27-bit payload data.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_WriteRegister(TRF_Modulator_HandleTypeDef *hmod, uint8_t reg_addr, uint32_t data);

/**
 * @brief  Read data from a specific register.
 *         Note: Typically TRF372017 requires a write sequence to set the read address, 
 *         followed by reading back on the RDBK pin. Ensure proper hardware mapping.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  reg_addr: 5-bit register address to read from.
 * @param  data: Pointer to variable where the 27-bit read data will be stored.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_ReadRegister(TRF_Modulator_HandleTypeDef *hmod, uint8_t reg_addr, uint32_t *data);

/**
 * @brief  Configure Register 1 (PLL SPI Register).
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 1.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg1(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg1_Config_t *config);

/**
 * @brief  Configure Register 2.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 2.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg2(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg2_Config_t *config);

/**
 * @brief  Configure Register 3.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 3.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg3(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg3_Config_t *config);

/**
 * @brief  Configure Register 4.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 4.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg4(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg4_Config_t *config);

/**
 * @brief  Configure Register 5.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 5.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg5(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg5_Config_t *config);

/**
 * @brief  Configure Register 6.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 6.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg6(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg6_Config_t *config);

/**
 * @brief  Configure Register 7.
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 7.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg7(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg7_Config_t *config);

/**
 * @brief  Configure Register 0 (Write / Setup Readback mode).
 * @param  hmod: Pointer to a TRF_Modulator_HandleTypeDef structure.
 * @param  config: Configuration structure for Register 0 write.
 * @retval 0 on success, <0 on failure.
 */
int8_t TRF_Modulator_Config_Reg0_Write(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg0_Write_Config_t *config);

/**
 * @brief  Parse Readback Data from Register 0.
 * @param  raw_data: 32-bit data read from the TRF MISO pin.
 * @param  parsed_data: Pointer to the struct where the parsed data will be stored.
 */
void TRF_Modulator_Parse_Reg0_Read(uint32_t raw_data, TRF_Reg0_Read_Data_t *parsed_data);

#ifdef __cplusplus
}
#endif

#endif /* TRF_MODULATOR_H */
