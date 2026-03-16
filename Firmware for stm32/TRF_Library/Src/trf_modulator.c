#include "trf_modulator.h"
#include <stddef.h>

/* Assume HAL_SPI functions and GPIO functions are available via the HAL drivers. 
 * Below are weak macros/declarations to allow compilation without a specific STM32 family header.
 * In a real application, you include your specific "stm32XXxx_hal.h" and these 
 * would be resolved natively.
 */
// #define TRF_SPI_TIMEOUT 100
// extern HAL_StatusTypeDef HAL_SPI_Transmit(void *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
// extern HAL_StatusTypeDef HAL_SPI_TransmitReceive(void *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);
// extern void HAL_GPIO_WritePin(void* GPIOx, uint16_t GPIO_Pin, int PinState);

#define SPI_TIMEOUT_MS 100

/* Internal helper functions for LE (Latch Enable) control */

/**
 * @brief Pull the Latch Enable (LE) low to begin data shifting.
 */
static void TRF_LE_Low(TRF_Modulator_HandleTypeDef *hmod) {
    // HAL_GPIO_WritePin(hmod->config.le_port, hmod->config.le_pin, 0); // GPIO_PIN_RESET
}

/**
 * @brief Pulse the Latch Enable (LE) high to latch the shifted data into the internal register.
 */
static void TRF_LE_Pulse(TRF_Modulator_HandleTypeDef *hmod) {
    // HAL_GPIO_WritePin(hmod->config.le_port, hmod->config.le_pin, 1); // GPIO_PIN_SET
    
    // Optional delay here if required by datasheet for LE pulse width (e.g. 20ns)
    // Usually, HAL overhead provides enough delay, but for very fast CPUs:
    // for(volatile int i=0; i<10; i++); 
    
    // HAL_GPIO_WritePin(hmod->config.le_port, hmod->config.le_pin, 0); // GPIO_PIN_RESET
}

int8_t TRF_Modulator_Init(TRF_Modulator_HandleTypeDef *hmod) {
    if (hmod == NULL || hmod->config.spi_handle == NULL) {
        return -1; // Invalid argument
    }

    // Initialize LE pin low (idle state during non-latching)
    TRF_LE_Low(hmod);

    hmod->is_initialized = true;

    // TODO: Add TRF372017 specific initialization sequence here (e.g., resetting registers)
    
    return 0; // Success
}

int8_t TRF_Modulator_WriteRegister(TRF_Modulator_HandleTypeDef *hmod, uint8_t reg_addr, uint32_t data) {
    if (hmod == NULL || !hmod->is_initialized) {
        return -1;
    }

    // TRF372017 Packet Structure: Total 32 bits
    // Data is loaded LSB first.
    // LSB 5 bits [4:0] = Register Address
    // MSB 27 bits [31:5] = Data Payload
    
    // The data parameter already contains the 27-bit payload properly shifted
    // by the Config_Reg functions (starting at bit 5). 
    // We just need to OR the 5-bit register address at the bottom.
    uint32_t packet = data | (reg_addr & 0x1F);

    // Ensure LE is low before clocking data
    TRF_LE_Low(hmod);
    
    /* ST HAL SPI transmission. 
     * Since the STM32 is Little-Endian, passing a pointer to the 32-bit `packet`
     * string will naturally send the lowest byte first (which contains the address),
     * matching the "LSB First" requirement for the TRF372017 SPI config.
     */
    // if (HAL_SPI_Transmit(hmod->config.spi_handle, (uint8_t *)&packet, 4, SPI_TIMEOUT_MS) != 0 /* HAL_OK */) {
    //     return -2; // SPI error
    // }

    // Rising edge on LE to latch the data into the register
    TRF_LE_Pulse(hmod);

    return 0;
}

int8_t TRF_Modulator_ReadRegister(TRF_Modulator_HandleTypeDef *hmod, uint8_t reg_addr, uint32_t *data) {
    if (hmod == NULL || data == NULL || !hmod->is_initialized) {
        return -1;
    }

    // Step 1: Write to Register 0 to put TRF372017 in read-back mode for the target register
    TRF_Reg0_Write_Config_t rb_cfg = {
        .count_mode_mux_sel = 0, // Default to measuring Max frequency if VCO counter is enabled
        .rb_reg = (reg_addr & 0x7), // 3 LSBs of the address (e.g., 000 to 111)
        .rb_enable = 1           // 1 = Put the device in Readback Mode
    };
    
    if (TRF_Modulator_Config_Reg0_Write(hmod, &rb_cfg) != 0) {
        return -2;
    }

    // Step 2: Perform the read cycle
    // The TRF372017 expects 32 clock cycles where data is clocked OUT on MISO on the rising edge of CLK.
    uint32_t tx_packet = 0; // Empty payload to clock the SPI
    uint32_t rx_packet = 0;

    TRF_LE_Low(hmod);

    // TODO: Uncomment and link your specific STM32 HAL function here
    // if (HAL_SPI_TransmitReceive(hmod->config.spi_handle, (uint8_t *)&tx_packet, (uint8_t *)&rx_packet, 4, SPI_TIMEOUT_MS) != 0) {
    //     return -2;
    // }

    TRF_LE_Pulse(hmod);

    // Extract the raw 32-bit data. The specific format depends on whether you're 
    // reading Reg0 data or a standard register, but according to the datasheet, 
    // all readbacks tend to return the "Register 0 field format" (Chip ID, Count, etc).
    *data = rx_packet;

    return 0;
}

int8_t TRF_Modulator_Init_Default(TRF_Modulator_HandleTypeDef *hmod) {
    if (hmod == NULL) return -1;
    
    // First run the base initialization (sets LE low)
    if (TRF_Modulator_Init(hmod) != 0) return -2;

    // Generally, RF modulators should be initialized in descending register order 
    // (from highest address to lowest) to ensure proper power up sequencing.
    if (TRF_Modulator_Config_Reg7(hmod, &TRF_REG7_DEFAULT_CONFIG) != 0) return -3;
    if (TRF_Modulator_Config_Reg6(hmod, &TRF_REG6_DEFAULT_CONFIG) != 0) return -3;
    if (TRF_Modulator_Config_Reg5(hmod, &TRF_REG5_DEFAULT_CONFIG) != 0) return -3;
    if (TRF_Modulator_Config_Reg4(hmod, &TRF_REG4_DEFAULT_CONFIG) != 0) return -3;
    if (TRF_Modulator_Config_Reg3(hmod, &TRF_REG3_DEFAULT_CONFIG) != 0) return -3;
    if (TRF_Modulator_Config_Reg2(hmod, &TRF_REG2_DEFAULT_CONFIG) != 0) return -3;
    if (TRF_Modulator_Config_Reg1(hmod, &TRF_REG1_DEFAULT_CONFIG) != 0) return -3;

    return 0;
}

/* ========================================================================= */
/*                          Register Configurations                          */
/* ========================================================================= */

int8_t TRF_Modulator_Config_Reg1(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg1_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    // Pack the Reference Divider (13 bits)
    reg_data |= ((config->rdiv << TRF_REG1_RDIV_SHIFT) & TRF_REG1_RDIV_MASK);

    // Pack the Reference Clock Polarity Inverse bit (1 bit)
    reg_data |= ((config->ref_inv << TRF_REG1_REF_INV_SHIFT) & TRF_REG1_REF_INV_MASK);

    // Pack the VCO Polarity Control bit (1 bit)
    reg_data |= ((config->neg_vco << TRF_REG1_NEG_VCO_SHIFT) & TRF_REG1_NEG_VCO_MASK);

    // Pack the Charge Pump Current (5 bits)
    reg_data |= ((config->icp << TRF_REG1_ICP_SHIFT) & TRF_REG1_ICP_MASK);

    // Pack the ICP Double bit (1 bit)
    reg_data |= ((config->icp_double << TRF_REG1_ICPDOUBLE_SHIFT) & TRF_REG1_ICPDOUBLE_MASK);

    // Pack the VCO Calibration Clock Divider/Multiplier (4 bits)
    reg_data |= ((config->cal_clk_sel << TRF_REG1_CAL_CLK_SEL_SHIFT) & TRF_REG1_CAL_CLK_SEL_MASK);
    
    // Bits 18 and 31 are RSV (Reserved), keeping them 0 per default setting.

    // Finally, write this 27-bit payload to Register 1
    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG1_ADDR, reg_data);
}

int8_t TRF_Modulator_Config_Reg2(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg2_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    // Pack N-Divider (16 bits)
    reg_data |= ((config->n_int << TRF_REG2_NINT_SHIFT) & TRF_REG2_NINT_MASK);

    // Pack PLL Divider Setting (2 bits)
    reg_data |= ((config->pll_div << TRF_REG2_PLL_DIV_SHIFT) & TRF_REG2_PLL_DIV_MASK);

    // Pack Prescaler Select (1 bit)
    reg_data |= ((config->prsc_sel << TRF_REG2_PRSC_SEL_SHIFT) & TRF_REG2_PRSC_SEL_MASK);

    // RSV bits 24 and 25 are 0
    
    // Pack VCO Select (2 bits)
    reg_data |= ((config->vco_sel << TRF_REG2_VCO_SEL_SHIFT) & TRF_REG2_VCO_SEL_MASK);
    
    // Pack VCOSEL Mode (1 bit)
    reg_data |= ((config->vcosel_mode << TRF_REG2_VCOSEL_MODE_SHIFT) & TRF_REG2_VCOSEL_MODE_MASK);

    // Pack Cal Accuracy (2 bits)
    reg_data |= ((config->cal_acc << TRF_REG2_CAL_ACC_SHIFT) & TRF_REG2_CAL_ACC_MASK);

    // Pack Enable Cal (1 bit)
    reg_data |= ((config->en_cal << TRF_REG2_EN_CAL_SHIFT) & TRF_REG2_EN_CAL_MASK);

    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG2_ADDR, reg_data);
}

int8_t TRF_Modulator_Config_Reg3(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg3_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    // Pack Fractional N-Divider (25 bits)
    reg_data |= ((config->n_frac << TRF_REG3_NFRAC_SHIFT) & TRF_REG3_NFRAC_MASK);

    // RSV bits 30 and 31 are 0
    
    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG3_ADDR, reg_data);
}

int8_t TRF_Modulator_Config_Reg4(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg4_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    reg_data |= ((config->pwd_pll << TRF_REG4_PWD_PLL_SHIFT) & TRF_REG4_PWD_PLL_MASK);
    reg_data |= ((config->pwd_cp << TRF_REG4_PWD_CP_SHIFT) & TRF_REG4_PWD_CP_MASK);
    reg_data |= ((config->pwd_vco << TRF_REG4_PWD_VCO_SHIFT) & TRF_REG4_PWD_VCO_MASK);
    reg_data |= ((config->pwd_vcomux << TRF_REG4_PWD_VCOMUX_SHIFT) & TRF_REG4_PWD_VCOMUX_MASK);
    reg_data |= ((config->pwd_div124 << TRF_REG4_PWD_DIV124_SHIFT) & TRF_REG4_PWD_DIV124_MASK);
    reg_data |= ((config->pwd_presc << TRF_REG4_PWD_PRESC_SHIFT) & TRF_REG4_PWD_PRESC_MASK);
    // RSV bit 11 is 0
    reg_data |= ((config->pwd_out_buff << TRF_REG4_PWD_OUT_BUFF_SHIFT) & TRF_REG4_PWD_OUT_BUFF_MASK);
    reg_data |= ((config->pwd_lo_div << TRF_REG4_PWD_LO_DIV_SHIFT) & TRF_REG4_PWD_LO_DIV_MASK);
    reg_data |= ((config->pwd_tx_div << TRF_REG4_PWD_TX_DIV_SHIFT) & TRF_REG4_PWD_TX_DIV_MASK);
    reg_data |= ((config->pwd_bb_vcm << TRF_REG4_PWD_BB_VCM_SHIFT) & TRF_REG4_PWD_BB_VCM_MASK);
    reg_data |= ((config->pwd_dc_off << TRF_REG4_PWD_DC_OFF_SHIFT) & TRF_REG4_PWD_DC_OFF_MASK);
    
    reg_data |= ((config->en_extvco << TRF_REG4_EN_EXTVCO_SHIFT) & TRF_REG4_EN_EXTVCO_MASK);
    reg_data |= ((config->en_isource << TRF_REG4_EN_ISOURCE_SHIFT) & TRF_REG4_EN_ISOURCE_MASK);
    reg_data |= ((config->ld_ana_prec << TRF_REG4_LD_ANA_PREC_SHIFT) & TRF_REG4_LD_ANA_PREC_MASK);
    // Bit 20 implies LD_ANA_PREC is 2 bits, covered by the mask
    reg_data |= ((config->cp_tristate << TRF_REG4_CP_TRISTATE_SHIFT) & TRF_REG4_CP_TRISTATE_MASK);
    reg_data |= ((config->speedup << TRF_REG4_SPEEDUP_SHIFT) & TRF_REG4_SPEEDUP_MASK);
    reg_data |= ((config->ld_dig_prec << TRF_REG4_LD_DIG_PREC_SHIFT) & TRF_REG4_LD_DIG_PREC_MASK);
    reg_data |= ((config->en_dith << TRF_REG4_EN_DITH_SHIFT) & TRF_REG4_EN_DITH_MASK);
    reg_data |= ((config->mod_ord << TRF_REG4_MOD_ORD_SHIFT) & TRF_REG4_MOD_ORD_MASK);
    reg_data |= ((config->dith_sel << TRF_REG4_DITH_SEL_SHIFT) & TRF_REG4_DITH_SEL_MASK);
    reg_data |= ((config->del_sd_clk << TRF_REG4_DEL_SD_CLK_SHIFT) & TRF_REG4_DEL_SD_CLK_MASK);
    reg_data |= ((config->en_frac << TRF_REG4_EN_FRAC_SHIFT) & TRF_REG4_EN_FRAC_MASK);

    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG4_ADDR, reg_data);
}

int8_t TRF_Modulator_Config_Reg5(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg5_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    reg_data |= ((config->vcobias_rtrim << TRF_REG5_VCOBIAS_RTRIM_SHIFT) & TRF_REG5_VCOBIAS_RTRIM_MASK);
    reg_data |= ((config->pllbias_rtrim << TRF_REG5_PLLBIAS_RTRIM_SHIFT) & TRF_REG5_PLLBIAS_RTRIM_MASK);
    reg_data |= ((config->vco_bias << TRF_REG5_VCO_BIAS_SHIFT) & TRF_REG5_VCO_BIAS_MASK);
    reg_data |= ((config->vcobuf_bias << TRF_REG5_VCOBUF_BIAS_SHIFT) & TRF_REG5_VCOBUF_BIAS_MASK);
    reg_data |= ((config->vcomux_bias << TRF_REG5_VCOMUX_BIAS_SHIFT) & TRF_REG5_VCOMUX_BIAS_MASK);
    reg_data |= ((config->bufout_bias << TRF_REG5_BUFOUT_BIAS_SHIFT) & TRF_REG5_BUFOUT_BIAS_MASK);
    
    // RSV Bits 20 and 21: Bit 20 is 0, Bit 21 is 1. We hardcode this per the reset value column.
    reg_data |= (2 << TRF_REG5_RSV20_SHIFT); // 0b10 = 2

    reg_data |= ((config->vco_cal_ib << TRF_REG5_VCO_CAL_IB_SHIFT) & TRF_REG5_VCO_CAL_IB_MASK);
    reg_data |= ((config->vco_cal_ref << TRF_REG5_VCO_CAL_REF_SHIFT) & TRF_REG5_VCO_CAL_REF_MASK);
    reg_data |= ((config->vco_ampl_ctrl << TRF_REG5_VCO_AMPL_CTRL_SHIFT) & TRF_REG5_VCO_AMPL_CTRL_MASK);
    reg_data |= ((config->vco_vb_ctrl << TRF_REG5_VCO_VB_CTRL_SHIFT) & TRF_REG5_VCO_VB_CTRL_MASK);
    
    // RSV Bit 30 is 0
    
    reg_data |= ((config->en_ld_isource << TRF_REG5_EN_LD_ISOURCE_SHIFT) & TRF_REG5_EN_LD_ISOURCE_MASK);

    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG5_ADDR, reg_data);
}

int8_t TRF_Modulator_Config_Reg6(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg6_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    reg_data |= ((config->ioff << TRF_REG6_IOFF_SHIFT) & TRF_REG6_IOFF_MASK);
    reg_data |= ((config->qoff << TRF_REG6_QOFF_SHIFT) & TRF_REG6_QOFF_MASK);
    reg_data |= ((config->vref_sel << TRF_REG6_VREF_SEL_SHIFT) & TRF_REG6_VREF_SEL_MASK);
    reg_data |= ((config->tx_div_sel << TRF_REG6_TX_DIV_SEL_SHIFT) & TRF_REG6_TX_DIV_SEL_MASK);
    reg_data |= ((config->lo_div_sel << TRF_REG6_LO_DIV_SEL_SHIFT) & TRF_REG6_LO_DIV_SEL_MASK);
    reg_data |= ((config->tx_div_bias << TRF_REG6_TX_DIV_BIAS_SHIFT) & TRF_REG6_TX_DIV_BIAS_MASK);
    reg_data |= ((config->lo_div_bias << TRF_REG6_LO_DIV_BIAS_SHIFT) & TRF_REG6_LO_DIV_BIAS_MASK);

    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG6_ADDR, reg_data);
}

int8_t TRF_Modulator_Config_Reg7(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg7_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    // RSV Bits 5, 6 are 0

    reg_data |= ((config->vco_trim << TRF_REG7_VCO_TRIM_SHIFT) & TRF_REG7_VCO_TRIM_MASK);
    
    // RSV Bit 13 is 0

    reg_data |= ((config->vco_test_mode << TRF_REG7_VCO_TEST_MODE_SHIFT) & TRF_REG7_VCO_TEST_MODE_MASK);
    reg_data |= ((config->cal_bypass << TRF_REG7_CAL_BYPASS_SHIFT) & TRF_REG7_CAL_BYPASS_MASK);
    reg_data |= ((config->mux_ctrl << TRF_REG7_MUX_CTRL_SHIFT) & TRF_REG7_MUX_CTRL_MASK);
    reg_data |= ((config->isource_sink << TRF_REG7_ISOURCE_SINK_SHIFT) & TRF_REG7_ISOURCE_SINK_MASK);
    reg_data |= ((config->isource_trim << TRF_REG7_ISOURCE_TRIM_SHIFT) & TRF_REG7_ISOURCE_TRIM_MASK);
    reg_data |= ((config->pd_tc << TRF_REG7_PD_TC_SHIFT) & TRF_REG7_PD_TC_MASK);
    reg_data |= ((config->ib_vcm_sel << TRF_REG7_IB_VCM_SEL_SHIFT) & TRF_REG7_IB_VCM_SEL_MASK);

    // RSV Bits 26, 27, 28: Bit 26=0, 27=0, 28=1. So value is b100 = 4.
    reg_data |= (4 << TRF_REG7_RSV26_SHIFT);

    reg_data |= ((config->dcoffset_i << TRF_REG7_DCOFFSET_I_SHIFT) & TRF_REG7_DCOFFSET_I_MASK);
    reg_data |= ((config->vco_bias_sel << TRF_REG7_VCO_BIAS_SEL_SHIFT) & TRF_REG7_VCO_BIAS_SEL_MASK);

    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG7_ADDR, reg_data);
}

int8_t TRF_Modulator_Config_Reg0_Write(TRF_Modulator_HandleTypeDef *hmod, const TRF_Reg0_Write_Config_t *config) {
    if (hmod == NULL || config == NULL || !hmod->is_initialized) {
        return -1;
    }

    uint32_t reg_data = 0;

    reg_data |= ((config->count_mode_mux_sel << TRF_REG0_W_COUNT_MODE_MUX_SEL_SHIFT) & TRF_REG0_W_COUNT_MODE_MUX_SEL_MASK);
    reg_data |= ((config->rb_reg << TRF_REG0_W_RB_REG_SHIFT) & TRF_REG0_W_RB_REG_MASK);
    reg_data |= ((config->rb_enable << TRF_REG0_W_RB_ENABLE_SHIFT) & TRF_REG0_W_RB_ENABLE_MASK);

    return TRF_Modulator_WriteRegister(hmod, TRF372017_REG0_ADDR, reg_data);
}

void TRF_Modulator_Parse_Reg0_Read(uint32_t raw_data, TRF_Reg0_Read_Data_t *parsed_data) {
    if (parsed_data == NULL) return;

    parsed_data->chip_id = (raw_data & TRF_REG0_R_CHIP_ID_MASK) >> TRF_REG0_R_CHIP_ID_SHIFT;
    parsed_data->r_sat_err = (raw_data & TRF_REG0_R_R_SAT_ERR_MASK) >> TRF_REG0_R_R_SAT_ERR_SHIFT;
    
    // Autocal results vs Frequency Count depend on VCO_TEST_MODE in Reg 7, 
    // but we can extract both bitfields regardless of validity
    parsed_data->vco_trim_autocal = (raw_data & TRF_REG0_R_VCO_TRIM_AUTOCAL_MASK) >> TRF_REG0_R_VCO_TRIM_AUTOCAL_SHIFT;
    parsed_data->vco_sel_autocal = (raw_data & TRF_REG0_R_VCO_SEL_AUTOCAL_MASK) >> TRF_REG0_R_VCO_SEL_AUTOCAL_SHIFT;
    parsed_data->count_freq = (raw_data & TRF_REG0_R_COUNT_FREQ_MASK) >> TRF_REG0_R_COUNT_FREQ_SHIFT;
}
