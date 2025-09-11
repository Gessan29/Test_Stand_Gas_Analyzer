/*
 * Файл с функциями непосредственной проверки контрольных точек и ключевых элементов
 */
 #include "hardware.h"
#include <stdint.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;

uint16_t vol_raw;
uint32_t vol_average, tok;
                                               // функции подсчета переменных

void set_pins( uint8_t a3, uint8_t a2, uint8_t a1, uint8_t a0 ){

	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, a3 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, a2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, a1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, a0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void test_voltage(uint8_t* buf, uint32_t channel){
	vol_average = 0;
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	    {
	    	buf[0] = STATUS_EXEC_ERROR;
	    	        return;
	    }

	HAL_ADC_Start(&hadc1);
	for (int i = 0; i < SAMPLES; i++) {
		 while (!LL_ADC_IsActiveFlag_EOCS(ADC1)) {}
		 	 	 LL_ADC_ClearFlag_EOCS(ADC1);
		         vol_raw = HAL_ADC_GetValue(&hadc1);
		         vol_average += vol_raw;
		}
	HAL_ADC_Stop(&hadc1);

	vol_average = vol_average * REFERENCE_VOLTAGE / (ADC_BIT_RATE * SAMPLES);

	buf[1] = (uint8_t)(vol_average & 0xFF);
	buf[2] = (uint8_t)((vol_average >> 8) & 0xFF);
    buf[3] = (uint8_t)((vol_average >> 16) & 0xFF);
	buf[4] = (uint8_t)((vol_average >> 24) & 0xFF);
	buf[0] = STATUS_OK;
    return;
}

void apply_relay(GPIO_TypeDef *PORT, uint32_t PIN){
	            return(SET_BIT(PORT->BSRR, PIN)); }


void uart_tx_rx(UART_HandleTypeDef* uart, uint8_t* buf, uint8_t* tx, uint8_t* rx, size_t size){
	if (HAL_UART_Transmit(uart, tx, size, TIMEOUT_RX) != HAL_OK) {
		        buf[0] = STATUS_TIMED_OUT;
		        return;
		    }
		 if (uart->RxState == HAL_UART_STATE_READY) {
			 if (HAL_UART_Receive(uart, rx, size, TIMEOUT_RX) != HAL_OK) {
			             buf[0] = STATUS_TIMED_OUT;
			             return;
			         }
			     } else {
			         HAL_UART_AbortReceive(uart);
			         if (HAL_UART_Receive(uart, rx, size, TIMEOUT_RX) != HAL_OK) {
			             buf[0] = STATUS_TIMED_OUT;
			             return;
			         }
			     }
}

int compare_arrays(uint8_t arr1[], uint8_t arr2[], size_t size){
	for(int i = 0; i < size; i++){
		if (arr1[i] != arr2[i]){
			return 1;
		}
	}
	return 0;
}


                                                 // функции управления



void apply_voltage_relay_1(uint8_t* buf) // PC14
{
	switch (buf[1]) {
		case CLOSE_RELAY:
			apply_relay(RELAY_PORT_C, RELAY_1_PIN_0);
			if (READ_BIT(RELAY_PORT_C->IDR, RELAY_1_PIN_1) != 0){
								buf[0] = STATUS_EXEC_ERROR;
							} else {
							    buf[0] = STATUS_OK;
							}
			return;
		case OPEN_RELAY:
			apply_relay(RELAY_PORT_C, RELAY_1_PIN_1);
			if (READ_BIT(RELAY_PORT_C->IDR, RELAY_1_PIN_1) != 0){
								buf[0] = STATUS_OK;
							} else {
							    buf[0] = STATUS_EXEC_ERROR;
							}
			return;
		default:
			buf[0] = STATUS_INVALID_CMD;
			return;
	}
}

void test_voltage_4_point(uint8_t* buf)
{
	apply_relay(GPIOB, MUX_EN);
	switch (buf[1])
	{
	case CHECKPOINT_5V_PW_Peltier:
		set_pins(0, 0, 1, 1); // set_pins(1, 0, 0, 0);
		break;

	case CHECKPOINT_5V3:
		set_pins(0, 0, 0, 0); // set_pins(0, 1, 1, 0);
		break;

	case CHECKPOINT_3V3:
		set_pins(0, 1, 1, 0); // set_pins(0, 0, 1, 1);
		break;

	case CHECKPOINT_4V_PW_Laser:
		set_pins(0, 0, 1, 0); // set_pins(0, 0, 0, 0);
		break;
	default:
		buf[0] = STATUS_INVALID_CMD;
		return;
	}
	test_voltage(buf, ADC_MUX);
	apply_relay(GPIOB, MUX_DIS);
}

void test_voltage_current(uint8_t* buf)
{
	switch (buf[1])
	{
	case SYPPLY_VOLTAGE:
		uint32_t channel = ADC_SYPPLY_VOLTAGE;
		test_voltage(buf, channel);
		return;

	case SUPPLY_CURRENT:
		uint32_t channel_1 = ADC_SUPPLY_CURRENT;
		test_voltage(buf, channel_1);
		return;

	default:
		buf[0] = STATUS_INVALID_CMD;
		return;
	}
}

void apply_voltage_relay_2(uint8_t* buf) // PB9
{
	switch (buf[1]) {
			case CLOSE_RELAY:
				apply_relay(RELAY_PORT_B, RELAY_2_PIN_0);
				if (READ_BIT(RELAY_PORT_B->IDR, RELAY_2_PIN_1) != 0){
						buf[0] = STATUS_EXEC_ERROR;
					} else {
					    buf[0] = STATUS_OK;
					}
				return;
			case OPEN_RELAY:
				apply_relay(RELAY_PORT_B, RELAY_2_PIN_1);
				if (READ_BIT(RELAY_PORT_B->IDR, RELAY_2_PIN_1) != 0){
						buf[0] = STATUS_OK;
					} else {
					    buf[0] = STATUS_EXEC_ERROR;
					}
				return;
			default:
				buf[0] = STATUS_INVALID_CMD;
				return;
		}
}

void test_voltage_11_point(uint8_t* buf)
{
	apply_relay(GPIOB, MUX_EN);
	switch (buf[1])
	{
	case CHECKPOINT_1_2V:
		set_pins(1, 0, 1, 1);
		break;

	case CHECKPOINT_1_8V:
		set_pins(1, 1, 0, 0);
		break;

	case CHECKPOINT_2_5V:
		set_pins(1, 1, 1, 0);
		break;

	case CHECKPOINT_GPS_5V:
		set_pins(0, 0, 0, 1);
		break;

	case CHECKPOINT_5V_REFP:
		set_pins(0, 1, 0, 0);
		break;

	case CHECKPOINT_5VAA_sensor:
		set_pins(0, 1, 0, 1);
		break;

	case CHECKPOINT_5VAA_NEG:
		set_pins(1, 0, 0, 1);
		break;

	case CHECKPOINT_1_8VA:
		set_pins(1, 0, 1, 0);
		break;

	case CHECKPOINT_5VAA_Amq_A:
		set_pins(1, 1, 0, 1);
		break;

	case CHECKPOINT_2_048_NEG:
		set_pins(1, 0, 0, 0);
		break;

	case CHECKPOINT_5V_Amq_R:
		set_pins(1, 1, 1, 1);
		break;

	default:
		buf[0] = STATUS_INVALID_CMD;
		return;
	}
	test_voltage(buf, ADC_MUX);
	apply_relay(GPIOB, MUX_DIS);
}

void test_corrent_laser(uint8_t* buf)
{
	uint16_t adcSamples[SAMPLES_LASER];
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_LASER;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
		    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		    {
		    	buf[0] = STATUS_EXEC_ERROR;
		    	return;
		    }
	HAL_ADC_Start(&hadc1);
		    for (int i = 0; i < SAMPLES_LASER; i++) {
			     while (!LL_ADC_IsActiveFlag_EOCS(ADC1)) {}
			 	 	    LL_ADC_ClearFlag_EOCS(ADC1);
			 	 	    adcSamples[i] = HAL_ADC_GetValue(&hadc1);
			}
		    HAL_ADC_Stop(&hadc1);
		    for (int i = 0; i < SAMPLES_LASER; i++){
			     vol_average = adcSamples[i] * REFERENCE_VOLTAGE / ADC_BIT_RATE;
			     buf[i * 2 + 1] = (uint8_t)(vol_average & 0xFF);
			     buf[i * 2 + 2] = (uint8_t)(vol_average >> 8 & 0xFF);
		    }
	buf[0] = STATUS_OK;
}

void test_voltage_peltie(uint8_t* buf)
{
	int32_t vol_average_1 = 0, vol_average_2 = 0;
	int16_t vol_raw, tok = 0, res_shunt = RES_SHUNT_PELTIE;
	ADC_ChannelConfTypeDef sConfig = {0};

	sConfig.Channel = ADC_PELTIE_1;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		buf[0] = STATUS_EXEC_ERROR;
		return;
	}

	HAL_ADC_Start(&hadc1);
	for (int i = 0; i < SAMPLES; i++) {
		 while (!LL_ADC_IsActiveFlag_EOCS(ADC1)) {}
				LL_ADC_ClearFlag_EOCS(ADC1);
				vol_raw = HAL_ADC_GetValue(&hadc1);
				vol_average_1 += vol_raw;
	}

	HAL_ADC_Stop(&hadc1);
	vol_average_1 = vol_average_1 * REFERENCE_VOLTAGE / (ADC_BIT_RATE * SAMPLES);

	sConfig.Channel = ADC_PELTIE_2;

	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		buf[0] = STATUS_EXEC_ERROR;
		 return;
	}

	HAL_ADC_Start(&hadc1);
	for (int i = 0; i < SAMPLES; i++) {
		 while (!LL_ADC_IsActiveFlag_EOCS(ADC1)) {}
				LL_ADC_ClearFlag_EOCS(ADC1);
				vol_raw = HAL_ADC_GetValue(&hadc1);
				vol_average_2 += vol_raw;
	}

	HAL_ADC_Stop(&hadc1);
	vol_average_2 = vol_average_2 * REFERENCE_VOLTAGE / (ADC_BIT_RATE * SAMPLES);

	vol_average_1 = vol_average_1 - vol_average_2;

	if (vol_average_1 > 0) {
		buf[5] = 0; // напряжение положительное
	} else {
		buf[5] = 1; // напряжение отрицательное
		vol_average_1 = -vol_average_1;
	}

	buf[0] = STATUS_OK;

	int16_t vol_16 = (int16_t)vol_average_1;

	buf[1] = vol_16 & 0xFF;
	buf[2] = (vol_16 >> 8) & 0xFF;

	tok = (int16_t)((vol_average_1 * 1000) / res_shunt); // мА

	buf[3] = tok & 0xFF;
	buf[4] = (tok >> 8) & 0xFF;
	return;
}

void apply_voltage_relay_5(uint8_t* buf) // PC13
{
	switch (buf[1]) {
			case CLOSE_RELAY:
				apply_relay(RELAY_PORT_C, RELAY_5_PIN_0);
				if (READ_BIT(RELAY_PORT_C->IDR, RELAY_5_PIN_1) != 0){
						buf[0] = STATUS_EXEC_ERROR;
					} else {
					    buf[0] = STATUS_OK;
					}
				return;
			case OPEN_RELAY:
				apply_relay(RELAY_PORT_C, RELAY_5_PIN_1);
				if (READ_BIT(RELAY_PORT_C->IDR, RELAY_5_PIN_1) != 0){
						buf[0] = STATUS_OK;
					} else {
					    buf[0] = STATUS_EXEC_ERROR;
					}
				return;
			default:
				buf[0] = STATUS_INVALID_CMD;
				return;
		}
}

void massage_rs232(uint8_t* buf)
{
	uint8_t rs_232_tx [RS_232] = "RS_232!";
	uint8_t rs_232_rx [RS_232];
	uart_tx_rx(&UART_RS_232, buf, rs_232_tx, rs_232_rx, RS_232);

	if (buf[0] == STATUS_TIMED_OUT){ return; }

	if (compare_arrays(rs_232_tx, rs_232_rx, RS_232) == 0){
		buf[0] = STATUS_OK;
	}
	else {
		buf[0] = STATUS_EXEC_ERROR;
	}
	for (int i = 1; i < 5; i++){
		buf[i] = 0;
	}
}

void massage_gps(uint8_t* buf)
{
	uint8_t gps_tx [GPS_SIZE] = "$GNGLL,5502.49000,N,08256.07600,E,1235  .000,A,A*"; // GLL, version 4.1 and 4.2, NMEA 0183
	uint8_t gps_rx [GPS_SIZE];
	gps_tx[38] = (buf[1]/ 10) + '0';
	gps_tx[39] = (buf[1] % 10) + '0';
	uart_tx_rx(&UART_GPS, buf, gps_tx, gps_rx, GPS_SIZE);

	if (buf[0] == STATUS_TIMED_OUT){ return; }

	if (compare_arrays(gps_tx, gps_rx, GPS_SIZE) == 0){
			buf[0] = STATUS_OK;
		}
		else {
			buf[0] = STATUS_EXEC_ERROR;
		}
}
