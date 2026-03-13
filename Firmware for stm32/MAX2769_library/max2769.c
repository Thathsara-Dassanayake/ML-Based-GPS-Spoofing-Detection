#include "max2769.h"

#define MAX2769_HAL_OK (0)

/* These APIs are provided by STM32 HAL in the target project. */
extern void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
extern int HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);

static GPIO_PinState max2769_cs_inactive_state(GPIO_PinState active_state)
{
	return (active_state == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
}

static const max2769_register_t k_default_write_order[10] =
{
	MAX2769_REG_CONF1,
	MAX2769_REG_CONF2,
	MAX2769_REG_CONF3,
	MAX2769_REG_PLLCONF,
	MAX2769_REG_DIV,
	MAX2769_REG_FDIV,
	MAX2769_REG_STRM,
	MAX2769_REG_CLK,
	MAX2769_REG_TEST1,
	MAX2769_REG_TEST2
};

const uint32_t max2769_default_reg_values[10] =
{
	MAX2769_DEFAULT_CONF1,
	MAX2769_DEFAULT_CONF2,
	MAX2769_DEFAULT_CONF3,
	MAX2769_DEFAULT_PLLCONF,
	MAX2769_DEFAULT_DIV,
	MAX2769_DEFAULT_FDIV,
	MAX2769_DEFAULT_STRM,
	MAX2769_DEFAULT_CLK,
	MAX2769_DEFAULT_TEST1,
	MAX2769_DEFAULT_TEST2
};

max2769_status_t max2769_init(max2769_t *dev,
							  SPI_HandleTypeDef *hspi,
							  GPIO_TypeDef *cs_port,
							  uint16_t cs_pin,
							  GPIO_PinState cs_active_state,
							  uint32_t spi_timeout_ms)
{
	if ((dev == NULL) || (hspi == NULL) || (cs_port == NULL))
	{
		return MAX2769_STATUS_INVALID_ARG;
	}

	dev->hspi = hspi;
	dev->cs_port = cs_port;
	dev->cs_pin = cs_pin;
	dev->cs_active_state = cs_active_state;
	dev->spi_timeout_ms = spi_timeout_ms;

	HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, max2769_cs_inactive_state(dev->cs_active_state));

	return MAX2769_STATUS_OK;
}

uint32_t max2769_build_frame(max2769_register_t reg, uint32_t data_d27_d0)
{
	return ((data_d27_d0 & 0x0FFFFFFFUL) << 4U) | ((uint32_t)reg & 0x0FU);
}

max2769_status_t max2769_write_register(max2769_t *dev, max2769_register_t reg, uint32_t data_d27_d0)
{
	uint32_t frame;
	uint8_t tx_data[4];
	int hal_result;

	if ((dev == NULL) || (dev->hspi == NULL) || (dev->cs_port == NULL))
	{
		return MAX2769_STATUS_INVALID_ARG;
	}

	if (((uint32_t)reg > (uint32_t)MAX2769_REG_TEST2) || ((data_d27_d0 & 0xF0000000UL) != 0UL))
	{
		return MAX2769_STATUS_INVALID_ARG;
	}

	frame = max2769_build_frame(reg, data_d27_d0);
	tx_data[0] = (uint8_t)((frame >> 24U) & 0xFFU);
	tx_data[1] = (uint8_t)((frame >> 16U) & 0xFFU);
	tx_data[2] = (uint8_t)((frame >> 8U) & 0xFFU);
	tx_data[3] = (uint8_t)(frame & 0xFFU);

	HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, dev->cs_active_state);
	hal_result = HAL_SPI_Transmit(dev->hspi, tx_data, (uint16_t)sizeof(tx_data), dev->spi_timeout_ms);
	HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, max2769_cs_inactive_state(dev->cs_active_state));

	if (hal_result != MAX2769_HAL_OK)
	{
		return MAX2769_STATUS_IO_ERROR;
	}

	return MAX2769_STATUS_OK;
}

max2769_status_t max2769_write_defaults(max2769_t *dev)
{
	uint32_t i;
	max2769_status_t status;

	if (dev == NULL)
	{
		return MAX2769_STATUS_INVALID_ARG;
	}

	for (i = 0U; i < 10U; ++i)
	{
		status = max2769_write_register(dev, k_default_write_order[i], max2769_default_reg_values[i]);
		if (status != MAX2769_STATUS_OK)
		{
			return status;
		}
	}

	return MAX2769_STATUS_OK;
}
