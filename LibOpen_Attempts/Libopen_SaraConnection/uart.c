#include <FreeRTOS.h>
#include <task.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#define MSG_LENGTH 512

static void
uart_setup(void)
{

    // SARA SETUP
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);

    gpio_set_mode(GPIOA,
                  GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO_USART2_TX);

    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_enable(USART2);

    // ESP SETUP
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART3);

    gpio_set_mode(GPIOB,
                  GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO_USART3_TX);

    usart_set_baudrate(USART3, 115200);
    usart_set_databits(USART3, 8);
    usart_set_stopbits(USART3, USART_STOPBITS_1);
    usart_set_mode(USART3, USART_MODE_TX_RX);
    usart_set_parity(USART3, USART_PARITY_NONE);
    usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);
    usart_enable(USART3);
}

static inline void
uart_putc(uint8_t ch, uint32_t usart_port)
{
    usart_send(usart_port, ch);
}

static inline void
uart_putc_blocking(uint8_t ch, uint32_t usart_port)
{
    usart_send_blocking(usart_port, ch);
}

static inline void uart_puts(uint8_t *s, uint32_t usart_port)
{
    while (*s != '\0')
    {
        uart_putc_blocking(*s, usart_port);
        gpio_toggle(GPIOC, GPIO13);
        vTaskDelay(pdMS_TO_TICKS(100));
        s++;
    }
    uart_putc_blocking('\r', usart_port);
    uart_putc_blocking('\n', usart_port);
}


tatic bool usart_get_string(uint32_t usartSrc, uint32_t usartDst, uint16_t str_max_size)
{
    uint8_t received = 'V';
    uint16_t itr = 0;
    uint8_t recvPrev = 'Q';
    bool timeoutflag;



    //restart timer:
    if ( xTimerReset(timerHndl1, 10 ) != pdPASS )
    {
        timeoutflag = true;
    }
    else
    {
        timeoutflag = false;
    }

    while ((itr < str_max_size) && (!timeoutflag))
    {
        while ( (!((USART_SR(USART2) & USART_SR_RXNE) == 0)) &&
                (itr < str_max_size) )
        {
            received = usart_recv_blocking(usartSrc);
            uart_putc(received, usartDst);

            if (recvPrev == 'O' && received == 'K')
            {
                return;
            }
            recvPrev = received;
            itr++;
        }


        // if timer times out
        if ( xTimerIsTimerActive(timerHndl1) == pdFALSE )
        {
            timeoutflag = true;
        }
    }

    return !timeoutflag;
}

static void
task1(void *args __attribute__((unused)))
{

    uint8_t esp[] = "Hi ESP";
    uint8_t AT[] = "ATI";

    uart_puts(esp, USART3);

    // Power_on Start for SARA module
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_clear(GPIOA, GPIO5);
    vTaskDelay(pdMS_TO_TICKS(5000));
    gpio_set(GPIOA, GPIO5);

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

    for (;;)
    {
        uart_puts(esp, USART3);
        vTaskDelay(pdMS_TO_TICKS(500));

        uart_puts(AT, USART2);

        usart_get_string(USART2, USART3, MSG_LENGTH);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

int main(void)
{

    rcc_clock_setup_in_hse_8mhz_out_72mhz(); // Blue pill
    // PC13:
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(
        GPIOC,
        GPIO_MODE_OUTPUT_2_MHZ,
        GPIO_CNF_OUTPUT_PUSHPULL,
        GPIO13);

    uart_setup();

 	timerHndl1 = xTimerCreate(
      "timer1", /* name */
      pdMS_TO_TICKS(400), /* period/time */
      pdFALSE, /* auto reload */
      (void*)0, /* timer ID */
      vTimerCallback1SecExpired); /* callback */

    xTaskCreate(task1, "task1", 100, NULL, configMAX_PRIORITIES - 1, NULL);
    vTaskStartScheduler();

    for (;;)
        ;
    return 0;
}
