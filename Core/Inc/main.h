/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include "../../MQTT/MQTTPacket.h"
#include "../../MQTT/MQTTClient.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
  Off        = 0,
  On         = 1,
  Registered = 2
}GSMModuleState;

typedef enum { 
  MQTTOff        = 0,
  MQTTConnected  = 1,
  MQTTSubscribed = 2
 }MQTTState;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern GSMModuleState gsmModuleState;

extern uint8_t apn_netw[30]; // say iot.safaricom.com or safaricom
extern uint8_t apn_user[10]; // something like saf or empty
extern uint8_t apn_pass[10]; // something like data or empty

extern uint32_t sim_disconnected;
extern uint8_t command_failed_count;

extern MQTTState mqttConnectionState;
extern MQTTString mqttTopic;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
int serialPrint(const char *string_format, ...);

int __io_putchar(uint8_t ch);

void downStreamCommandHandler(MessageData *messageData);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define POWER_KEY_Pin GPIO_PIN_15
#define POWER_KEY_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define DEVEL 1

#define GSM_POWER_PORT GPIOB
#define GSM_POWER_PIN  GPIO_PIN_15

// MQTT-Specific
#define MQTT_USERNAME "<CoolPerson>"
#define MQTT_DEVICE_GROUP "<cooldevicegroup>"
#define MQTT_PASSWORD "<strongpassword>" // Mandatory field
#define AUTH_CREDENTIALS MQTT_USERNAME ":" MQTT_DEVICE_GROUP // Mandatory field; resolves to username:devicegroup
#define MQTT_QOS 1 // Can be 0 or 1 or 2

#define DEVICE_ID "Eris" // Mandatory Field

#define MQTT_BIRTH_TOPIC "birth"
#define MQTT_WILL_TOPIC "will"
#define MQTT_SEND_DATA_TOPIC "send"
#define MQTT_REC_DATA_TOPIC "rec"

#define BASE_TOPIC MQTT_USERNAME "/" MQTT_DEVICE_GROUP "/" DEVICE_ID // Caveat: Your topics should always be in the format username/devicegroup example username/devicegroup/some/thing/else

#define DEVICE_WILL_TOPIC BASE_TOPIC "/" MQTT_WILL_TOPIC
#define DEVICE_BIRTH_TOPIC BASE_TOPIC "/" MQTT_BIRTH_TOPIC
#define DEVICE_UPSTREAM_TOPIC BASE_TOPIC "/" MQTT_SEND_DATA_TOPIC
#define DEVICE_DOWNSTREAM_TOPIC BASE_TOPIC "/" MQTT_REC_DATA_TOPIC 

#define WILL_MESSAGE "DISCONNECTED"
#define BIRTH_MESSAGE "CONNECTED"
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
