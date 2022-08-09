/*
 * canfilter.c
 *
 *  Created on: Jan 15, 2020
 *      Author: eko
 */
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
uint32_t TxMailbox;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
uint16_t rpm;

const uint8_t msgFilOrig[3][8] = {"!VA2CN6G", "\"H104051", "xxxxxSAL"};

const uint8_t msgFilMask[3][8] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
							{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
							{0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF}};

const uint8_t msgReplace[3][8] = {"Test1234", "Test5678", "Test9876"};
/* USER CODE END PD */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void copyData(void);
void filtercan(int airbid, uint8_t data[8]);
/* USER CODE END PFP */

void canloop(CAN_HandleTypeDef *can1, CAN_HandleTypeDef *can2) {
	while (1) {
		// Receive Message from Can1 & send to CAN2:
		if (HAL_CAN_GetRxFifoFillLevel(can1, CAN_RX_FIFO0) != 0) {
			if (HAL_CAN_GetRxMessage(can1, CAN_RX_FIFO0, &RxHeader, RxData)
					!= HAL_OK) {
				/* Reception Error */
				Error_Handler();
			}
			copyData();
			if (HAL_CAN_GetTxMailboxesFreeLevel(can2) != 0) {
				if (HAL_CAN_AddTxMessage(can2, &TxHeader, TxData, &TxMailbox)
						!= HAL_OK) {
					/* Transmission request Error */
					HAL_CAN_ResetError(can2);
					//Error_Handler();
				}
			}

		}
		// Do same on Can2:
		if (HAL_CAN_GetRxFifoFillLevel(can2, CAN_RX_FIFO1) != 0) {
			if (HAL_CAN_GetRxMessage(can2, CAN_RX_FIFO1, &RxHeader, RxData)
					!= HAL_OK) {
				/* Reception Error */
				Error_Handler();
			}
			copyData();
			if (HAL_CAN_GetTxMailboxesFreeLevel(can1) != 0) {
				if (HAL_CAN_AddTxMessage(can1, &TxHeader, TxData, &TxMailbox)
						!= HAL_OK) {
					/* Transmission request Error */
					HAL_CAN_ResetError(can1);
					//Error_Handler();
				}
			}

		}

	}
}

void copyData() {
	memcpy(TxData, RxData, 8);
	TxHeader.DLC = RxHeader.DLC;
	TxHeader.StdId = RxHeader.StdId;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	filtercan(RxHeader.StdId, TxData);

	
}
void filtercan(int airbid, uint8_t data[8]) {
	uint8_t LvMatch = 1;
	for(uint8_t i=0; i < 3; i++){
		for(uint8_t j=0; j < 8; j++){
			// Check if content of data[] masked by msgFilMask matches exactly msgFilOrig[]
			if((msgFilMask[i][j]) & (data[j] ^ msgFilOrig[i][j])){
				// Set LvMatch=0 as soon as one bit does not match
				LvMatch = 0;
			}
		}
		// Replace masked content if whole string matches the target string
		if(LvMatch){
			for(uint8_t j=0; j < 8; j++){
				data[j] = (msgFilMask[i][j] & msgReplace[i][j]) | (data[j]& (~msgFilMask[i][j]));
			}
		}
	}
}
