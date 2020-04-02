/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "../../Systick/systick.h"
#include "../../GSM/gsm.h"
#include "../../MQTT/MQTTClient.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
Network network;

GSMModuleState gsmModuleState;

Timer *mqttTimer = {0};

MQTTState mqttConnectionState;
MQTTClient mqttClient;
MQTTMessage mqttMessage = {0};
MQTTString mqttTopic;
MQTTPacket_connectData mqttPacketConnectData = MQTTPacket_connectData_initializer;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t apn_netw[30] = {"iot.safaricom.com"};
uint8_t apn_user[10] = {"none"};
uint8_t apn_pass[10] = {"none"};

int sent_failed_count = 0;
uint8_t tcp_connection_failed_count = 0;
uint32_t sim_disconnected = 0;
uint8_t command_failed_count = 0;
int sim_module_on_counter = 0;

const char *broker_url = "broker.africastalking.com";
int broker_port = 1883;

unsigned char mqtt_outbound_buffer[80];
unsigned char mqtt_inbound_buffer[80];
char mqttTopicName[100];

int publish_birth_message = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  UART1FIFOInit();
  UART2FIFOInit();
  serialPrint("\nStarting up\r\n");
  resetSIMModule(GSM_POWER_PORT, GSM_POWER_PIN, 3000);
  MQTTClientInit(&mqttClient, &network, 3000, mqtt_outbound_buffer, sizeof(mqtt_outbound_buffer), mqtt_inbound_buffer, sizeof(mqtt_inbound_buffer));
  mqttConnectionState = MQTTOff;
  mqttPacketConnectData.MQTTVersion = 4;
  mqttPacketConnectData.keepAliveInterval = 60;
  mqttPacketConnectData.cleansession = 0;
  mqttPacketConnectData.clientID.cstring = (char *)DEVICE_ID;
  mqttPacketConnectData.username.cstring = (char *)AUTH_CREDENTIALS;
  mqttPacketConnectData.password.cstring = (char *)MQTT_PASSWORD;
  mqttPacketConnectData.will.topicName.cstring = (char *)DEVICE_WILL_TOPIC;
  mqttPacketConnectData.will.message.cstring = (char *)WILL_MESSAGE;
  mqttPacketConnectData.will.qos = MQTT_QOS;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    if (gsmModuleState == Off)
    {
      serialPrint("\nSIM State is Off.Initializing\r\n");
      if (initializeSIMModule())
      {
        gsmModuleState = On;
        serialPrint("\nSIM Module initilized\r\n");
      }
    }
    if (gsmModuleState == On)
    {
      serialPrint("\nSIM module is on\r\n");
      sim_module_on_counter++;
      if (sim_module_on_counter > 300)
      {
        serialPrint("\nReseting...\r\n");
        resetSIMModule(GSM_POWER_PORT, GSM_POWER_PIN, 3000);
        sim_module_on_counter = 0;
        gsmModuleState = Off;
        sent_failed_count = 0;
        tcp_connection_failed_count = 0;
      }
      if (checkSIMNetworkState())
      {
        if (setupTCP())
        {
          gsmModuleState = Registered;
          serialPrint("\nNetwork registered\r\n");
        }
      }
    }
    if (sent_failed_count > 10)
    {
      gsmModuleState = On;
      sent_failed_count = 0;
      tcp_connection_failed_count = 0;
      serialPrint("\nGPRS Connection failed\r\n");
    }
    if (tcp_connection_failed_count > 20)
    {
      gsmModuleState = On;
      tcp_connection_failed_count = 0;
      sent_failed_count = 0;
      serialPrint("\nTCP Connection failed\r\n");
    }
    if (gsmModuleState == Registered)
    {
      gsmKeepAlive();
      getTCPStatus(300);
      if (tcpConnectionObject.state == CONNECTED)
      {
        serialPrint("\nConnected to TCP Endpoint.\r\n");
        cycle(&mqttClient, mqttTimer);
      }
      else if ((tcpConnectionObject.state == CLOSED) || (tcpConnectionObject.state == INITIAL))
      {
        uint8_t data_length;
        uint8_t data_bucket[100];
        data_length = snprintf((char *)data_bucket, 64, "AT+CIPSTART=0,\"TCP\",\"%s\",\"%i\"\r", broker_url, broker_port);
        if (sendATCommand(data_bucket, data_length, "CONNECT OK", 300))
        {
          serialPrint("\nConnected to endpoint\r\n");
        }
        else
        {
          serialPrint("\nTCP Connection failed\r\n");
          sent_failed_count++;
        }
        publish_birth_message = 1;
      }
    }
    if (tcpConnectionObject.state == CONNECTED)
    {
      serialPrint("\nStill connected.\r\n");
      MQTTConnect(&mqttClient, &mqttPacketConnectData);
      char birthMessage[100];
      mqttMessage.dup = 0;
      mqttMessage.qos = MQTT_QOS;
      mqttMessage.id = 0;
      mqttMessage.retained = 0;
      mqttMessage.payload = birthMessage;
      sprintf(birthMessage, (char *)BIRTH_MESSAGE);
      mqttMessage.payloadlen = strlen((char *)mqttMessage.payload) + 1;
      if (publish_birth_message)
      {
        if (MQTTPublish(&mqttClient, (char *)DEVICE_BIRTH_TOPIC, &mqttMessage) == 0)
        {
          serialPrint("Birth message published \r\n");
          sent_failed_count = 0;
          publish_birth_message = 0;
        }
        else
        {
          serialPrint("Unable to publish birth message \r\n");
          sent_failed_count++;
          publish_birth_message = 1;
        }
        if (mqttConnectionState != MQTTSubscribed)
        {
          sprintf(mqttTopicName, (char *)DEVICE_DOWNSTREAM_TOPIC);
          if (MQTTSubscribe(&mqttClient, mqttTopicName, MQTT_QOS, downStreamCommandHandler) == 0)
          {
            serialPrint("Subscribed to downstream topic \r\n");
            mqttConnectionState = MQTTSubscribed;
          }
          else
          {
            serialPrint("Unable to subscribe to Topic \r\n");
          }
        }
      }
    }
    else
    {
      serialPrint("\nDisconnected\r\n");
      mqttConnectionState = MQTTOff;
    }
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void downStreamCommandHandler(MessageData *messageData)
{
  serialPrint("Received a message %s : \r\n", messageData->message->payload);
  serialPrint("Topic is %s \r\n", messageData->topicName);
  memset((char *)messageData->message->payload, '\0', sizeof(uint8_t));
  memset((char *)messageData->topicName, '\0', sizeof(uint8_t));
  serialPrint("Sending message \r\n");

  char dataMessage[100];
  mqttMessage.dup = 0;
  mqttMessage.qos = MQTT_QOS;
  mqttMessage.id = 0;
  mqttMessage.retained = 0;
  mqttMessage.payload = dataMessage;
  sprintf(dataMessage, "Done");
  mqttMessage.payloadlen = strlen((char *)mqttMessage.payload) + 1;
  if (MQTTPublish(&mqttClient, (char *)DEVICE_UPSTREAM_TOPIC, &mqttMessage) == 0)
  {
    serialPrint("Message published \r\n");
  }
  else
  {
    serialPrint("Unable to publish message \r\n");
    sent_failed_count++;
  }
}

int serialPrint(const char *string_format, ...)
{
  if (DEVEL)
  {
    return printf(string_format);
  }
  else
  {
    return 0;
  }
}

int __io_putchar(uint8_t ch)
{
  if (ch)
  {
    UART1PutChar(ch);
    return ch;
  }
  else
  {
    return 0;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
