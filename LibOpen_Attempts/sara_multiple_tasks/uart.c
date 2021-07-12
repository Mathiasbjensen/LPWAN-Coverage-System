/* Task based USART demo:
 * Warren Gay VE3WWG
 *
 * This simple demonstration runs from task1, writing 012...XYZ lines
 * one after the other, at a rate of 5 characters/second. This demo
 * uses usart_send_blocking() to write characters.
 *
 * STM32F103C8T6:
 *	TX:	A9  <====> RX of TTL serial
 *	RX:	A10 <====> TX of TTL serial
 *	CTS:	A11 (not used)
 *	RTS:	A12 (not used)
 *	Config:	8N1
 *	Baud:	38400
 * Caution:
 *	Not all GPIO pins are 5V tolerant, so be careful to
 *	get the wiring correct.
 */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#define MSG_LENGTH 256

static QueueHandle_t uart_txq_sara;
static QueueHandle_t uart_rxq_sara;
static QueueHandle_t uart_txq_esp;
static QueueHandle_t uart_rxq_esp;

/*********************************************************************
 * Setup the UART
 *********************************************************************/
static void
uart_setup(void) {

	// TTL SETUP
	
	//vTaskDelay(pdMS_TO_TICKS(8000));
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART2);

	gpio_set_mode(GPIOA,
		      GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		      GPIO_USART2_TX);

	usart_set_baudrate(USART2,115200);
	usart_set_databits(USART2,8);
	usart_set_stopbits(USART2,USART_STOPBITS_1);
	usart_set_mode(USART2,USART_MODE_TX_RX);
	usart_set_parity(USART2,USART_PARITY_NONE);
	usart_set_flow_control(USART2,USART_FLOWCONTROL_NONE);
	usart_enable(USART2);


	// ESP SETUP
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_USART3);

	gpio_set_mode(GPIOB,
		      GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		      GPIO_USART3_TX);

	usart_set_baudrate(USART3,115200); // ESP_A
	usart_set_databits(USART3,8);
	usart_set_stopbits(USART3,USART_STOPBITS_1);
	usart_set_mode(USART3,USART_MODE_TX_RX);
	usart_set_parity(USART3,USART_PARITY_NONE);
	usart_set_flow_control(USART3,USART_FLOWCONTROL_NONE);
	usart_enable(USART3);

	// Create a queue for data to transmit from UART
	uart_txq_sara = xQueueCreate(256,sizeof(char));
	uart_txq_esp = xQueueCreate(256,sizeof(char));


/*
	// Pin Enable:
	gpio_set_mode(
	    GPIOA,
	    GPIO_MODE_OUTPUT_2_MHZ,
	    GPIO_CNF_INPUT_FLOAT,
	    GPIO4); //RESET_N
	gpio_set_mode(
	    GPIOA,
	    GPIO_MODE_OUTPUT_2_MHZ,
	    GPIO_CNF_OUTPUT_PUSHPULL,
	    GPIO5); //PWR_ON
*/
}

/*********************************************************************
 * Send one character to the UART
 *********************************************************************/

static void
uart_putsQ(const char *s, QueueHandle_t queue) {
	
	for ( ; *s; ++s ) {
		// blocks when queue is full
		xQueueSend(queue,s,portMAX_DELAY); 
	}
}


static inline void
uart_putc(uint8_t ch, uint32_t usart_port) {
	//usart_send_blocking(USART1,ch);
	usart_send(usart_port,ch);
	//usart_send_blocking(usart_port,'\n');
	
}

static inline void
uart_putc_blocking(uint8_t ch, uint32_t usart_port) {
	//usart_send_blocking(USART1,ch);
	usart_send_blocking(usart_port,ch);
	//usart_send_blocking(usart_port,'\n');
	
}

static inline void uart_puts(uint8_t *s, uint32_t usart_port)
{
	while (*s != '\0') { 
	  uart_putc_blocking(*s, usart_port);
	  gpio_toggle(GPIOC,GPIO13);
	  vTaskDelay(pdMS_TO_TICKS(100));
	  s++;
	} 
	  uart_putc_blocking('\r',usart_port);
	  //vTaskDelay(pdMS_TO_TICKS(100));
	  uart_putc_blocking('\n',usart_port);
	
}


static void usart_get_string(uint32_t usartSrc, uint32_t usartDst, uint8_t * outString, uint16_t str_max_size)
{
  //uint16_t received;
  uint8_t received = 'V';
  uint16_t itr = 0;
  uint8_t test5[] = "e\n";
  uint8_t test6[] = "a\n";
  uint8_t test7[] = "b\n";
  uint8_t upper, lower;
  uint16_t newLines = 0;
  uint8_t recvPrev;
  char recvChar;
  uint8_t itr2 = '0';
  



  while (itr < str_max_size)
    { 
	
	//received = usart_recv_blocking(usartSrc);
	if (!((USART_SR(USART2) & USART_SR_RXNE) == 0)){
		received = usart_recv_blocking(usartSrc);	
	}
	uart_putc_blocking(received, usartDst);
	

	if (recvPrev == 'O' && received == 'K'){
		uart_putc_blocking('X', usartDst);
		break;
	}
	recvPrev = received;
	itr++;
    }

}




/*********************************************************************
 * Send characters to the UART, slowly
 *********************************************************************/
static void
task1(void *args __attribute__((unused))) {
        //char otaaJoin[] = "mac join otaa\r\n"; // Message to join
	//uint8_t atCommand[] = "AT+MAC=SAPPUID,BE7A000000001383"; // transmit FF 
	uint8_t atSync[] = "AT+MAC=?";
	uint8_t lora_macOff[] = "AT+MAC=OFF";
	uint8_t lora_macOn[] = "AT+MAC=ON,3,A,1";
	uint8_t atCommand2[] = "AT+MAC=RDEVADDR";
	uint8_t atCommand3[] = "AT+MAC=SNDBIN,CAFE,0,3,0"; 
	uint8_t sysgetver[] = "sys get ver";
	char esp[] = "Hej ESP\r\n";
	//uint8_t received[MSG_LENGTH];
	uint8_t str_send[MSG_LENGTH];
	uint8_t gps[] = "PSTMGETRTCTIME";
	uint8_t initGps[] = "PSTMINITGPS";
	uint8_t coldStart[] = "PSTMSRR";
	uint8_t coldStart2[] = "PSTMGPSRESTART";
	uint8_t getPar[] = "PSTMGETPAR,000";
	uint8_t AT[] = "ATI\r\n";
	
	
	uart_putsQ(esp,uart_txq_sara);
	vTaskDelay(pdMS_TO_TICKS(500));
	gpio_clear(GPIOA,GPIO5);
	vTaskDelay(pdMS_TO_TICKS(5000));
	gpio_set(GPIOA,GPIO5);


		// Pin Enable:
	gpio_set_mode(
	    GPIOA,
	    GPIO_MODE_OUTPUT_2_MHZ,
	    GPIO_CNF_INPUT_FLOAT,
	    GPIO4); //RESET_N
	gpio_set_mode(
	    GPIOA,
	    GPIO_MODE_OUTPUT_2_MHZ,
	    GPIO_CNF_OUTPUT_PUSHPULL,
	    GPIO5); //PWR_ON
 	
	
	

	vTaskDelay(pdMS_TO_TICKS(10000));

	for(;;) {
	  uart_putsQ(esp,uart_txq_sara);
	  vTaskDelay(pdMS_TO_TICKS(500));

	  uart_putsQ(AT, uart_txq_esp);
	  //uart_puts(AT, USART2);
	  //vTaskDelay(pdMS_TO_TICKS(500));

	  //usart_get_string(USART2, USART3, str_send, MSG_LENGTH);
	  vTaskDelay(pdMS_TO_TICKS(10000));

	  memset(str_send, 0, MSG_LENGTH);

	}

	


}

static void
task_sara(void *args __attribute__((unused))) 
{
	char ch;

	for (;;) {
		// Receive char to be TX
		if ( xQueueReceive(uart_txq_sara,&ch,500) == pdPASS ) {
			while ( !usart_get_flag(USART3,USART_SR_TXE) )
				taskYIELD();	// Yield until ready
			usart_send(USART3,ch);
		}
		// Toggle LED to show signs of life
		gpio_toggle(GPIOC,GPIO13);
	}
}

static void
task_esp(void *args __attribute__((unused))) 
{
	char ch;

	for (;;) {
		// Receive char to be TX
		if ( xQueueReceive(uart_txq_esp,&ch,500) == pdPASS ) {
			while ( !usart_get_flag(USART2,USART_SR_TXE) )
				taskYIELD();	// Yield until ready
			usart_send(USART2,ch);
		}
		// Toggle LED to show signs of life
		gpio_toggle(GPIOC,GPIO13);
	}
}

static void
task_sara_recv(void *args __attribute__((unused))) 
{
	char recv;

	for (;;) {
		// Receive char to be TX
		while ( !usart_get_flag(USART2,USART_SR_RXNE) )
		{
			taskYIELD();	// Yield until ready
		}

		recv = usart_recv(USART2);
		xQueueSend(uart_txq_sara,recv,portMAX_DELAY);
		//uart_putsQ("XQX",uart_txq_sara);

		// Toggle LED to show signs of life
		gpio_toggle(GPIOC,GPIO13);
	}
}


static void
task_esp_recv(void *args __attribute__((unused)))  
{
	char recv;

	for (;;) {
		// Receive char to be TX
		while ( usart_get_flag(USART3,USART_SR_TXE) && usart_get_flag(USART2,USART_SR_TXE) )
			taskYIELD();	// Yield until ready

		recv = usart_recv_blocking(USART2);
		xQueueSend(uart_txq_esp,recv,portMAX_DELAY);

		// Toggle LED to show signs of life
		gpio_toggle(GPIOC,GPIO13);
	}
}




/*********************************************************************
 * Main program
 *********************************************************************/
int
main(void) {

	rcc_clock_setup_in_hse_8mhz_out_72mhz(); // Blue pill
	//usart_recv
	// PC13:
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(
		GPIOC,
                GPIO_MODE_OUTPUT_2_MHZ,
                GPIO_CNF_OUTPUT_PUSHPULL,
                GPIO13);

	uart_setup();
	//gpio_clear(GPIOA,GPIO4);
	
	xTaskCreate(task1,"task1",100,NULL,configMAX_PRIORITIES-1,NULL);
	xTaskCreate(task_sara,"SaraTX",100,NULL,configMAX_PRIORITIES-1,NULL);
	xTaskCreate(task_esp,"EspTX",100,NULL,configMAX_PRIORITIES-1,NULL);
	xTaskCreate(task_sara_recv,"SaraRX",100,NULL,configMAX_PRIORITIES-1,NULL);
	//xTaskCreate(task_esp_recv,"EspRX",100,NULL,configMAX_PRIORITIES-1,NULL);

	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End



