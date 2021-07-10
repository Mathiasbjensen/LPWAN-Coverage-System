# LPWAN-Coverage-System

developing a RF performance device for measuring the performance of LPWAN technologies

## needed modifications

### .bashrc

add the line to .bashrc
`export PATH="/opt/gcc-arm/bin:$PATH"`

It is MAYBE crucial to add the following to .bashrc as well
`export LD_LIBRARY_PATH=/usr/local/lib`

## STM 32

launch CUBE IDE
`sudo /home/rander/st/stm32cubeide_1.5.1/stm32cubeide %F`

### RESETTING STM32

This step perhaps needed before upload

1. Move bootloader pin bridge from 0 to 1 (both jumpers should be closest to 1)
2. hold reset button while running `st-flash erase`
3. Move bootloader pin bridge back from 1 to 0 (both jumpers should be closest to 0)
4. Now it is possible to run a program from e.g. CubeIDE onto the stm32

### Reading and writing IMAGES from/to STM32

#### Reading (saving) 

`st-flash read FILENAME ADDRESS SIZE` 

Alternatively the backup.sh program can be used for default configs (saves image in BackupImages folder)

#### Writing

`st-flash write FILENAME ADDRESS`


#### Erase flash

`st-flash erase` 

This will erase all images from the device

### Building and flashing program to STM32

This sections builds on the miniblink example found on page 57 of the book beginning STM32

1. Go to dir with program `cd ~/stm32f103c8t6/miniblink`
2. Build program (force rebuild) `make clobber` then `make`
3. Flash program `make flash`

### Enabling Pin on Board

To enable a pin on the Board from Anas, simply use the following code in the STM32 Setup():

```
rcc_periph_clock_enable(RCC_GPIOA);
gpio_set_mode(
    GPIOA,
    GPIO_MODE_OUTPUT_2_MHZ,
    GPIO_CNF_OUTPUT_PUSHPULL,
    GPIO4);
```

For this particular example the wire should be connected from A4 on the STM to GPS_E on the Board for instance.

The code to keeping the GPIO pin high is:

```
gpio_set(GPIOA,GPIO4);
```

### Troubleshooting

Check and find ST-LINK V2 and stm32 device: 
`st-info --probe`


## LoRa RN2483
### MAC connection guide (LoRaWAN)

Setup minicom: 

1. bps/par/bits = 57600 8N1
2. hardware flow control = no 
3. Local Echo = yes

**enter then press ctrl + j in order to send command with \r\n** 

Setup lorawan connection: 

1. `sys factoryRESET`
2. `mac pause`
3. `sys get hweui` - get eui for device
4. Generate APPKEY (128 bit aes to HEX) with `openssl enc -aes-128-cbc -k secret -P -md sha1`
5. Setup teracom (OTAA, do not generate) with hweui, APPEUI(if app id = BE7A1383 then APPEUI = BE7A000000001383) and APPKEY   https://iotnet.cibicom.dk/application/ 
6. `mac set appeui XXX` 
7. `mac set appkey XXX`
8. `mac set adr on`
9.  `mac save`
10. `mac resume`
11. `mac join otaa` 
12. `mac tx uncnf 1 0E`
    
In order to set watchdog timeout: 
`radio set wdt XXX`

### STM32 control LoRa RN2483
uart.c file in project uart_lora shows the way

ensure that mac commands for lora device has been run


## Projects folder
files in projects folder needs to be added to own project folder in order to be run.

e.g. making new project for uart_lora: 
1. `cd ~/stm32/rtos`
2. `cp -r uart uart_lora`
3. `cd uart_lora`
4. `cp ~/Documents/thesis/Projects/StmToLora_UART/uart.c uart.c`


## NEMEUS stuff:
- NEMEUS breakoutboard appears to have RX and TX switched, so RX on the NEMEUS breakout board has to be
put into RX.
- See [here](https://wiki.nemeus.fr/index.php/MM002-xx-EU_AT_Commands) for NEMEUS commands and configuration.

- In order to circumvent the duty cycle limitations that are on the NEMEUS MM002 board, a simple power cycle is sufficient to avoid having to wait upwards of 10 minutes.
Since the internal timer for keeping track of the Duty Cycle is stored in the RAM, a power cycle will reset this timer and allow us to send packets more often, which in return
allows for more measurements.

### LORA connection):

These steps will set up LoRa OTAA.

- AT+MAC=ON,3,A,1 (Turns on OTAA)
- AT+MAC=RAPPKEY
- AT+MAC=RAPPUID
- AT+MAC=RDEVUID
After these have been set up in the teracom etc, then:
- AT+MAC=?
- AT+MAC=ON (if mac layer is set to off)
- AT+MAC=RDEVADDR (**This may take 10-15 min to actually connect?????**)

**If you want to receive on the NEMEUS, you have to "queue" a packet in Teracom, which will then be sent to the NEMEUS**.

### Sigfox Connection:

- AT+SF=? will give you both DevUID and Initial PAC.
-
- AT+SF=SNDBIN,CAFE,1 Will send a message

### SARA R412

- Requires power from 2 power sources, or at least on 2 pins.

