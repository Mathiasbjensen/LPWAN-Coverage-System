/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SARA_LTEM 7
#define SARA_NBIOT 8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
uint8_t randSeq[2];
uint8_t stry[2];

uint8_t testSara[3];

uint8_t SARAresult[128];
uint8_t SARAtech[50];
uint8_t testing[] = ".";
uint8_t SARAate0[] = "ATE0\r\n";
uint8_t SARAumnoprof[] = "AT+UMNOPROF=100\r\n";
uint8_t SARAcops[] = "AT+COPS=0,2\r\n";
uint8_t SARAconnLTE[] = "AT+URAT=7\r\n";
uint8_t SARAconnNB[] = "AT+URAT=8\r\n";
uint8_t SARAcopsCheck[] = "AT+COPS?\r\n";
uint8_t SARAcsq[] = "AT+CSQ\r\n";
uint8_t SARAcesq[] = "AT+CESQ\r\n";
uint8_t SARAcfun15[] = "AT+CFUN=15\r\n";
const uint8_t crlf[] = "\r\n";
uint8_t SARATechnology[1];
uint8_t SARARsrpRsrq[6];
uint8_t SARAcsqResult[4];
uint8_t SaraMeasurements[128];
uint8_t saraCESQmessage[70];
uint8_t saraCSQmessage[50];
uint8_t LTEMTechName[] = "lte-m,";
uint8_t NBIoTTechName[] = "nb-iot,";


uint8_t syncLora[] = "AT+MAC=?\r\n";
//uint8_t beginLora[] = "AT+MAC=ON\r\n";
uint8_t beginLora[] = "AT+MAC=ON,3,A,1\r\n";
uint8_t getLoraLCR[] = "AT+MAC=SNDLCR\r\n";
uint8_t beginSigfox[] = "AT+SF=ON\r\n";
uint8_t sigfoxSendBinCommand[] = "AT+SF=SNDBIN,";
uint8_t sigfoxEnd[] = ",0\r\n";
uint8_t getGPSCoordsCommand[] = "<G>\r\n";
uint8_t loraTechName[] = "lora,";
uint8_t sigfoxTechName[] = "sigfox,";
uint8_t currentGPSCoords[80];
uint8_t currentLoraSignalQuality[69];
uint8_t trash[128];
uint8_t loraMeasurements[128];
uint8_t sigfoxMeasurements[128];

int sigfoxSeq = 0;
uint8_t myInt[5];
char* sigfoxArr;

uint8_t beginDelim[] = "<";
uint8_t endDelim[] = ">";

typedef enum {
  LTEM,
  NBIOT,
  LORA,
  SIGFOX
} lpwan_technology ;

HAL_StatusTypeDef  HAL_STATUS_UART1;
HAL_StatusTypeDef  HAL_STATUS_UART3;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void SARA_Init() {
	HAL_UART_Transmit(&huart1, SARAate0, strlen(SARAate0), 50);
	HAL_UART_Receive(&huart1, trash, 128, 100);
	osDelay(500);
	HAL_UART_Transmit(&huart1, SARAumnoprof, strlen(SARAumnoprof), 50);
	HAL_UART_Receive(&huart1, trash, 128, 100);
	osDelay(500);

	HAL_UART_Transmit(&huart1, SARAcops, strlen(SARAcops), 50);
	HAL_UART_Receive(&huart1, trash, 128, 100);
	osDelay(500);
	HAL_UART_Transmit(&huart1, SARAconnLTE, strlen(SARAconnLTE), 50);
	HAL_UART_Receive(&huart1, trash, 128, 100);

	HAL_UART_Transmit(&huart1, SARAate0, strlen(SARAate0), 50);
	HAL_UART_Receive(&huart1, trash, 128, 100);

	osDelay(500);
	HAL_UART_Receive(&huart1, trash, 128, 100);

}

void nemeus_Power_Cycle() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    osDelay(1500);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

    HAL_UART_Receive(&huart3, trash, 128, 150);

	HAL_UART_Transmit(&huart3, syncLora, strlen(syncLora), 10);
	HAL_UART_Receive(&huart3, trash, 128, 100);
	osDelay(500);

	HAL_UART_Transmit(&huart3, beginLora, strlen(beginLora), 10);
	HAL_UART_Receive(&huart3, trash, 128, 100);

	memset(trash, '\0', 128);

	HAL_UART_Receive(&huart3, trash, 10, 40000);
	HAL_UART_Receive(&huart3, trash, 128, 100);

	HAL_UART_Transmit(&huart3, beginSigfox, strlen(beginSigfox), 10);
	HAL_UART_Receive(&huart3, trash, 128, 100);
	osDelay(50);
	HAL_UART_Receive(&huart3, trash, 128, 100);


}

void SARA_Get_Measurement(uint8_t * cmd){
	HAL_UART_Transmit(&huart1, cmd, strlen(cmd), 10);
	HAL_UART_Receive(&huart1, SARAresult, 128, 100);
}


void SARA_ChangeTech(uint8_t tech){ //tech should be 9 for NB
	uint8_t lpwanTechnology[12];
	if(tech == '7'){
		strcpy(lpwanTechnology,SARAconnLTE);
	}
	else if (tech == '9') {
		strcpy(lpwanTechnology,SARAconnNB);
	}
	HAL_UART_Transmit(&huart1, lpwanTechnology, strlen(lpwanTechnology), 50);
	HAL_UART_Receive(&huart1, trash, 128, 100);

	HAL_UART_Transmit(&huart1, SARAcfun15, strlen(SARAcfun15), 50);
	HAL_UART_Receive(&huart1, trash, 128, 100);

	// Wait for device to restart
	osDelay(7500);

	HAL_UART_Transmit(&huart1, SARAate0, strlen(SARAate0), 50);
	HAL_UART_Receive(&huart1, trash, 128, 250);

	if (tech == '9'){ // give NB-IoT additional boot-up time.
		osDelay(2000);
	}

	HAL_UART_Transmit(&huart1, SARAate0, strlen(SARAate0), 50);
	HAL_UART_Receive(&huart1, trash, 128, 250);

	int i = 0;
	int msgLength;
	do {
		SARA_CheckTech();
		msgLength = strlen(SARAtech);
		getResultParameterURAT(3, SARAtech, msgLength);
		osDelay(200);
		osDelay(1500);
		i++;
		if(i > 3){
			__HAL_UART_FLUSH_DRREGISTER(&huart1);
			if(&huart1.ErrorCode == HAL_UART_ERROR_ORE){
				HAL_UART_Receive(&huart1, trash, 1, 20);
			}

			HAL_UART_Transmit(&huart1, SARAate0, strlen(SARAate0), 50);
			HAL_UART_Receive(&huart1, trash, 128, 250);
		}
	} while (SARATechnology[0] != tech && i < 5);

}

void SARA_CheckTech(){
	memset(SARAtech,'\0',50);
	HAL_UART_Transmit(&huart1, SARAcopsCheck, strlen(SARAcopsCheck), 10);
	HAL_UART_Receive(&huart1, SARAtech, 50, 1500);
	osDelay(200);//sendToESP(SARAtech);
}

void getResultParameterURAT(int nParam, uint8_t * msg, int msgLength){
	memset(SARATechnology,'\0',1);
	int commaCnt = 0;
	int i = 0;
	osDelay(200);
	while (msg[i] != '\0' && i < msgLength){
		osDelay(50);
		if(msg[i] == ',' && commaCnt == nParam-1){
			SARATechnology[0] = msg[i+1];
			return;
		} else if(msg[i] == ','){
			commaCnt++;
		}
		i++;	
	}
}


void getResultParameterCESQ(int nParam, uint8_t * msg){
	int commaCnt = 0;
	int i = 0;
	int j = 1;
	while (msg[i] != '\0'){
		if(msg[i] == ',' && commaCnt == nParam-1){
			while (j <= 5 && msg[i+j] != '\r' && msg[i+j] != '\n'){
				SARARsrpRsrq[j-1] = msg[i+j];
				j++;
			}
			return;
		} else if(msg[i] == ','){
			commaCnt++;
		}
		i++;
	}
}

void getCSQResult(uint8_t * msg){
	int i = 8;
	int j = 0;
	while (msg[i] != ',' && msg[i] != '\0' && msg[i] != '\r' && msg[i] != '\n' && j < 4){
		SARAcsqResult[j] = msg[i];
		i++;
		j++;
	}
}

void getGPSCoordinates(){
	int cnt = 0;
	memset(currentGPSCoords,'\0',80);
    HAL_UART_Transmit(&huart1, getGPSCoordsCommand, strlen(getGPSCoordsCommand), 50);
    HAL_UART_Receive(&huart1, currentGPSCoords, 80, 250);

    while (strlen(currentGPSCoords) < 18 && cnt < 5){ 
    	memset(currentGPSCoords,'\0',80);
    	HAL_UART_Receive(&huart1, trash, 128, 250);
        HAL_UART_Transmit(&huart1, getGPSCoordsCommand, strlen(getGPSCoordsCommand), 50);
        HAL_UART_Receive(&huart1, currentGPSCoords, 80, 500);
        osDelay(250);
        cnt++;
    }

}

void prepareSaraMeasurement(int technology){
	memset(SaraMeasurements,'\0',128);
	if (technology == SARA_LTEM){
		strcpy(SaraMeasurements,LTEMTechName);
	}
	if (technology == SARA_NBIOT){
		strcpy(SaraMeasurements,NBIoTTechName);
	}

	strcat(SaraMeasurements,currentGPSCoords);
	strcat(SaraMeasurements,SARARsrpRsrq);
	strcat(SaraMeasurements,",");
	strcat(SaraMeasurements,SARAcsqResult);

	memset(currentGPSCoords,'\0',80);
	memset(SARARsrpRsrq,'\0',6);
	memset(SARAcsqResult,'\0',4);

	memset(saraCESQmessage,'\0',70);
	memset(saraCSQmessage,'\0',50);
}

void NEMEUS_Extract_Lora_Measurements(uint8_t * cmd){
	int i = 8; // start after '+MAC: '
	int j = 0;

	while(i < 69 && cmd[i] != '\n' && cmd[i] != '\0' && cmd[i] != '\r'){
		currentLoraSignalQuality[j] = cmd[i];
		i++;
		j++;
	}
}

void NEMEUS_Prepare_Lora_Measurements(){

	strcpy(loraMeasurements,loraTechName);
	strcat(loraMeasurements,currentGPSCoords);
	strcat(loraMeasurements,currentLoraSignalQuality);

	memset(currentGPSCoords,'\0',80);
}

void NEMEUS_Prepare_Sigfox_Measurements(){
	strcpy(sigfoxMeasurements,sigfoxTechName);
	strcat(sigfoxMeasurements,currentGPSCoords);
	strcat(sigfoxMeasurements,randSeq);
	strcat(sigfoxMeasurements,sigfoxArr);
}

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
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);


  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  srand(DWT->CYCCNT-HAL_GetTick());
  int x = rand();
  int y = rand();
  sprintf(randSeq, "%X", x%16);
  sprintf(stry, "%X", y%16);
  strcat(randSeq, stry);


  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 57600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 57600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 38400;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void sendToESP(uint8_t * msg) {
	HAL_UART_Transmit(&huart2, beginDelim, 1, 50);
	HAL_UART_Transmit(&huart2, msg, strlen(msg), 50);
	HAL_UART_Transmit(&huart2, endDelim, 1, 50);
}


void collectAndTransmitSARAMeasurement(){

    HAL_UART_Transmit(&huart1, SARAcesq, strlen(SARAcesq), 50);
    HAL_UART_Receive(&huart1, saraCESQmessage, 70, 150);
    getResultParameterCESQ(4, saraCESQmessage);

    HAL_UART_Transmit(&huart1, SARAcsq, strlen(SARAcsq), 50);
    HAL_UART_Receive(&huart1, saraCSQmessage, 50, 150);

    getCSQResult(saraCSQmessage);

}
char* convertIntegerToChar(int N)
{

    int m = N;
    int digit = 0;
    while (m) {
        digit++;
        m /= 10;
    }

    char* arr;
    char arr1[digit];

    arr = (char*)malloc(digit);

    int index = 0;
    while (N) {
        arr1[++index] = N % 10 + '0';
        N /= 10;
    }

    if (index < 4){
        for (int j = 0; j < 4-index; j++){
            arr[j] = '0';
        }
    }

    int i;
    int k = 0;
    for (i = 4-index; i < 4; i++){
        arr[i] = arr1[index-k];
        k++;
    }
    arr[i] = '\0';
    return (char*)arr;
}



/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */

  uint8_t debugTest[] = "AT+DEBUG=ME?";
  osDelay(4500);
  SARA_Init();
  nemeus_Power_Cycle();

  uint8_t sigfoxSend[30];
  uint8_t LoRaMessage[69];
  uint8_t SigFoxMessage[69];




  for(;;)
  {
    osDelay(1000);

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);



    // **** SARA STUFF ****
    // ********************

    SARA_ChangeTech('7');

    collectAndTransmitSARAMeasurement();
    getGPSCoordinates();
    prepareSaraMeasurement(SARA_LTEM);
    sendToESP(SaraMeasurements);

    memset(SaraMeasurements, '\0', 128);

	SARA_ChangeTech('9');

    collectAndTransmitSARAMeasurement();
    getGPSCoordinates();
    prepareSaraMeasurement(SARA_NBIOT);
    sendToESP(SaraMeasurements);

    memset(SaraMeasurements, '\0', 128);

    // **** NEMEUS STUFF ****
    // **********************
    HAL_UART_Receive(&huart3,trash,10,100);
    HAL_UART_Transmit(&huart3, getLoraLCR, strlen(getLoraLCR), 50);
    HAL_UART_Receive(&huart3, LoRaMessage, 69, 5000);


    if(strstr("ERROR",LoRaMessage) != NULL){
    	nemeus_Power_Cycle();
    	memset(LoRaMessage, '\0', 69);
    	HAL_UART_Receive(&huart3, trash, 128, 150);
        HAL_UART_Transmit(&huart3, getLoraLCR, strlen(getLoraLCR), 50);
        HAL_UART_Receive(&huart3, LoRaMessage, 69, 10000);
    }
    NEMEUS_Extract_Lora_Measurements(LoRaMessage);

    getGPSCoordinates();

    NEMEUS_Prepare_Lora_Measurements();

    sendToESP(loraMeasurements);

	// Sigfox:

    sigfoxArr = convertIntegerToChar(sigfoxSeq);
    strcpy(sigfoxSend,sigfoxSendBinCommand);
    strcat(sigfoxSend,randSeq);
    strcat(sigfoxSend,sigfoxArr);
    strcat(sigfoxSend,sigfoxEnd);


    HAL_UART_Transmit(&huart3, sigfoxSend, 30, 50);
	HAL_UART_Receive(&huart3, SigFoxMessage, 69, 1500);

	getGPSCoordinates();
    NEMEUS_Prepare_Sigfox_Measurements();

    sendToESP(sigfoxMeasurements);



    HAL_UART_Transmit(&huart2, crlf, strlen(crlf), 50);


	memset(SigFoxMessage, '\0', 69);
	memset(LoRaMessage, '\0', 69);
	memset(currentGPSCoords,'\0',80);
	memset(currentLoraSignalQuality,'\0',69);
	memset(loraMeasurements,'\0',128);
	memset(sigfoxMeasurements,'\0',128);
	memset(sigfoxSend,'\0',30);
	free(sigfoxArr);
	sigfoxSeq++;

	nemeus_Power_Cycle(); 


  }
  /* USER CODE END 5 */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
