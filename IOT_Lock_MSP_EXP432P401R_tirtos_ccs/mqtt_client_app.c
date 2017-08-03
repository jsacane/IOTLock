/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************

 Application Name     -   MQTT Client
 Application Overview -   The device is running a MQTT client which is
                           connected to the online broker. Three LEDs on the 
                           device can be controlled from a web client by
                           publishing msg on appropriate topics. Similarly, 
                           message can be published on pre-configured topics
                           by pressing the switch buttons on the device.
 
 Application Details  - Refer to 'MQTT Client' README.html
 
 *****************************************************************************/
//*****************************************************************************
//
//! \addtogroup mqtt_server
//! @{
//
//*****************************************************************************
/* Standard includes                                                          */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* MSP432 includes                                                            */
#include <ti/devices/msp432p4xx/inc/msp432.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* TI-Driver includes                                                         */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>

/* SimpleLink includes                                                        */
#include <ti/drivers/net/wifi/simplelink.h>

/* MQTT Library includes                                                      */
#include <ti/net/mqtt/mqtt_client.h>

/* GrLib Includes */
#include <ti/grlib/grlib.h>
#include <ti/grlib/button.h>
#include <ti/grlib/imageButton.h>
#include <ti/grlib/radioButton.h>
#include <ti/grlib/checkbox.h>
#include "LcdDriver/kitronix320x240x16_ssd2119_spi.h"
#include "images/images.h"
#include "touch_P401R.h"

/* Common interface includes                                                  */
#include "network_if.h"
#include "uart_term.h"

/* Application includes                                                       */
#include "Board.h"
#include "client_cbs.h"
#include "pthread.h"
#include "mqueue.h"
#include "time.h"
#include "unistd.h"

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
/* enables secured client                                                     */
//#define SECURE_CLIENT

/* enables client authentication by the server                                */
//#define CLNT_USR_PWD

#define CLIENT_INIT_STATE        (0x01)
#define MQTT_INIT_STATE          (0x04)

#define APPLICATION_VERSION      "1.1.1"
#define APPLICATION_NAME         "IOTLock - MQTT client"

/* Operate Lib in MQTT 3.1 mode.                                              */
#define MQTT_3_1_1               false
#define MQTT_3_1                 true

#define WILL_TOPIC               "Client"
#define WILL_MSG                 "Client Stopped"
#define WILL_QOS                 MQTT_QOS_2
#define WILL_RETAIN              false

/* Defining Broker IP address and port Number                                 */
//#define SERVER_ADDRESS           "messagesight.demos.ibm.com"
//#define SERVER_ADDRESS           "m2m.eclipse.org"
#define SERVER_ADDRESS           "test.mosquitto.org"
//#define SERVER_IP_ADDRESS        "192.168.178.67"
#define SERVER_IP_ADDRESS        "37.187.106.16"
#define PORT_NUMBER              1883
#define SECURED_PORT_NUMBER      8883
#define LOOPBACK_PORT            1882

/* Clean session flag                                                         */
#define CLEAN_SESSION            true

/* Retain Flag. Used in publish message.                                      */
#define RETAIN_ENABLE            1

/* Defining Publish Topic Values                                              */
#define PUBLISH_TOPIC_SENDCODE        "IOTLock/SendCode"
#define PUBLISH_TOPIC_SUCCESS         "IOTLock/Success"
#define PUBLISH_TOPIC_FAILURE         "IOTLock/Failure"

#define PUBLISH_TOPIC_SUCCESS_DATA    "SUCCESS"
#define PUBLISH_TOPIC_FAILURE_DATA    "FAILURE"

/* Spawn task priority and Task and Thread Stack Size                         */
#define TASKSTACKSIZE            2048
#define RXTASKSIZE               4096
#define MQTTTHREADSIZE           2048
#define SPAWN_TASK_PRIORITY      9

/* secured client requires time configuration, in order to verify server      */
/* certificate validity (date).                                               */

/* Day of month (DD format) range 1-31                                        */
#define DAY                      1
/* Month (MM format) in the range of 1-12                                     */
#define MONTH                    5
/* Year (YYYY format)                                                         */
#define YEAR                     2017
/* Hours in the range of 0-23                                                 */
#define HOUR                     12
/* Minutes in the range of 0-59                                               */
#define MINUTES                  33
/* Seconds in the range of 0-59                                               */
#define SEC                      21   

/* Number of files used for secure connection                                 */
#define CLIENT_NUM_SECURE_FILES  1

/* Expiration value for the timer that is being used to toggle the Led.       */
#define TIMER_EXPIRATION_VALUE   100 * 1000000

/* Number of digits in the secret passcode                                    */
#define PASSCODE_LEN             6

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
void pushButtonInterruptHandler2(uint_least8_t index);
void pushButtonInterruptHandler3(uint_least8_t index);
void TimerPeriodicIntHandler(sigval val);
void LedTimerConfigNStart();
void LedTimerDeinitStop();
static void DisplayBanner(char * AppName);

void *MqttClient(void *pvParameters);
void Mqtt_ClientStop(uint8_t disconnect);
void Mqtt_ServerStop();
void Mqtt_Stop();
void Mqtt_start();
int32_t Mqtt_IF_Connect();
int32_t MqttServer_start();
int32_t MqttClient_start();
int32_t MQTT_SendMsgToQueue(struct msgQueue *queueElement);

void Delay(uint16_t msec);
void boardInit(void);
void clockInit(void);
void initializeKeypadButtons(void);
void drawLock(void);
void drawKeypad(void);

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************

/* Connection state: (0) - connected, (negative) - disconnected               */
int32_t  gApConnectionState = -1;
uint32_t gInitState = 0;
uint32_t memPtrCounterfree = 0;
bool     gResetApplication = false;
static NetAppHandle_t gMqttClient;
MQTTClient_Attrib_t  MqttClientExmple_params;
unsigned short g_usTimerInts;

/* Receive task handle                                                        */
pthread_t g_rx_task_hndl = (pthread_t) NULL;
uint32_t gUiConnFlag = 0;

/* AP Security Parameters                                                     */
SlWlanSecParams_t SecurityParams = { 0 };

/* Client ID                                                                  */
/* If ClientId isn't set, the MAC address of the device will be copied into   */
/* the ClientID parameter.                                                    */
char ClientId[13] = {'\0'};

/* Randomly generated 6-digit passcode                                        */
char randomCode[PASSCODE_LEN] = {0};

/* ASCII string of 6-digit passcode + null terminator                         */
char randomCodeStr[PASSCODE_LEN + 1] = {'\0'};

/* Passcode being entered by the user                                         */
char enteredCode[PASSCODE_LEN] = {0};

/* Keep track if the device is locked or unlocked                             */
bool locked;

/* Client User Name and Password                                              */
const char *ClientUsername = "username1";
const char *ClientPassword = "pwd1";

/* Publishing topics and messages                                             */
const char *publish_topic_sendcode = { PUBLISH_TOPIC_SENDCODE };
const char *publish_topic_success  = { PUBLISH_TOPIC_SUCCESS  };
const char *publish_topic_failure  = { PUBLISH_TOPIC_FAILURE  };

const char *publish_topic_sendcode_data = randomCodeStr;
const char *publish_topic_success_data  = { PUBLISH_TOPIC_SUCCESS_DATA };
const char *publish_topic_failure_data  = { PUBLISH_TOPIC_FAILURE_DATA };

/* Message Queue                                                              */
mqd_t g_PBQueue;
pthread_t mqttThread = (pthread_t) NULL;
pthread_t appThread = (pthread_t) NULL;
timer_t g_timer;

/* Printing new line                                                          */
char lineBreak[] = "\n\r";

/* Touch screen context                                                       */
touch_context g_sTouchContext;

/* Images and buttons                                                         */
Graphics_ImageButton oneButton;
Graphics_ImageButton twoButton;
Graphics_ImageButton threeButton;
Graphics_ImageButton fourButton;
Graphics_ImageButton fiveButton;
Graphics_ImageButton sixButton;
Graphics_ImageButton sevenButton;
Graphics_ImageButton eightButton;
Graphics_ImageButton nineButton;

/* Graphics library context                                                   */
Graphics_Context g_sContext;

/* Flag to know if a reset was triggered                                      */
bool reset = false;

//*****************************************************************************
//                 Banner VARIABLES
//*****************************************************************************
#ifdef  SECURE_CLIENT

char *Mqtt_Client_secure_files[CLIENT_NUM_SECURE_FILES] = {"ca-cert.pem"};

/* Initialization structure to be used with sl_ExtMqtt_Init API. In order to  */ 
/* use secured socket method, the flag MQTTCLIENT_NETCONN_SEC, cipher,        */
/* n_files and secure_files must be configured.                               */
/* certificates also must be programmed  ("ca-cert.pem").                     */
/* The first parameter is a bit mask which configures server address type and */
/* security mode.                                                             */
/* Server address type: IPv4, IPv6 and URL must be declared with The          */
/* corresponding flag.                                                        */
/* Security mode: The flag MQTTCLIENT_NETCONN_SEC enables the security (TLS)  */
/* which includes domain name verification and certificate catalog            */
/* verification, those verifications can be disabled by adding to the bit mask*/
/* MQTTCLIENT_NETCONN_SKIP_DOMAIN_NAME_VERIFICATION and                       */
/* MQTTCLIENT_NETCONN_SKIP_CERTIFICATE_CATALOG_VERIFICATION flags             */
/* Example: MQTTCLIENT_NETCONN_IP6 | MQTTCLIENT_NETCONN_SEC |                 */
/* MQTTCLIENT_NETCONN_SKIP_CERTIFICATE_CATALOG_VERIFICATION                   */
/* For this bit mask, the IPv6 address type will be in use, the security      */
/* feature will be enable and the certificate catalog verification will be    */
/* skipped.                                                                   */
/* Note: The domain name verification requires URL Server address type        */
/*       otherwise, this verification will be disabled.                       */
MQTTClient_NetAppConnParams_t Mqtt_ClientCtx =
{
     MQTTCLIENT_NETCONN_IP4 | MQTTCLIENT_NETCONN_SEC,
     SERVER_IP_ADDRESS, //SERVER_ADDRESS,
     SECURED_PORT_NUMBER,//  PORT_NUMBER
     SL_SO_SEC_METHOD_SSLv3_TLSV1_2,
     SL_SEC_MASK_SECURE_DEFAULT,
     CLIENT_NUM_SECURE_FILES,
     Mqtt_Client_secure_files
};

void setTime()
{
    SlDateTime_t dateTime = {0};
    dateTime.tm_day  = (uint32_t)DAY;
    dateTime.tm_mon  = (uint32_t)MONTH;
    dateTime.tm_year = (uint32_t)YEAR;
    dateTime.tm_hour = (uint32_t)HOUR;
    dateTime.tm_min  = (uint32_t)MINUTES;
    dateTime.tm_sec  = (uint32_t)SEC;
    sl_DeviceSet(SL_DEVICE_GENERAL, SL_DEVICE_GENERAL_DATE_TIME, sizeof(SlDateTime_t), (uint8_t *)(&dateTime));
}
#else
MQTTClient_NetAppConnParams_t Mqtt_ClientCtx =
{
    MQTTCLIENT_NETCONN_URL,
    SERVER_ADDRESS,
    PORT_NUMBER, 0, 0, 0,
    NULL
};
#endif

/* Initialize the will_param structure to the default will parameters         */
MQTTClient_Will_t will_param =
{
    WILL_TOPIC,
    WILL_MSG,
    WILL_QOS,
    WILL_RETAIN 
};

//*****************************************************************************
//
//! MQTT_SendMsgToQueue - Utility function that receives msgQueue parameter and
//! tries to push it to the queue with minimal time for timeout of 0.
//! If the queue isn't full, the parameter will be stored and the function
//! will return 0.
//! If the queue is full and the timeout expired (because the timeout parameter
//! is 0 it will expire immediately), the parameter is thrown away and the
//! function will return -1 as an error for full queue.
//!
//! \param[in] struct msgQueue *queueElement
//!
//! \return 0 on success, -1 on error
//
//*****************************************************************************
int32_t MQTT_SendMsgToQueue(struct msgQueue *queueElement)
{
    struct timespec abstime = {0};

    clock_gettime(CLOCK_REALTIME, &abstime);

    if(g_PBQueue)
    {
        /* send message to the queue                                         */
        if (mq_timedsend(g_PBQueue, (char *) queueElement, sizeof(struct msgQueue), 0, &abstime) == 0)
        {
            return 0;
        }
    }
    return -1;
}

//*****************************************************************************
//
//! Push Button Handler1(GPIOSW2). Press push button1 (GPIOSW2) Whenever user
//! wants to publish a message. Write message into message queue signaling the
//! event publish messages
//!
//! \param none
//!
//! return none
//
//*****************************************************************************
void pushButtonInterruptHandler2(uint_least8_t index)
{
    struct msgQueue queueElement;

    /* Disable the SW2 interrupt */
    GPIO_disableInt(Board_BUTTON0); // SW2

    queueElement.event = PUBLISH_PUSH_BUTTON_PRESSED;
    queueElement.msgPtr = NULL;

    /* write message indicating publish message                              */
    if (MQTT_SendMsgToQueue(&queueElement))
    {
        UART_PRINT("\n\n\rQueue is full\n\n\r");
    }

}

//*****************************************************************************
//
//! Push Button Handler2(GPIOSW3). Press push button3 Whenever user wants to
//! disconnect from the remote broker. Write message into message queue
//! indicating disconnect from broker.
//!
//! \param none
//!
//! return none
//
//*****************************************************************************
void pushButtonInterruptHandler3(uint_least8_t index)
{
    reset = true;

    GPIO_write(Board_LED0, Board_LED_ON);
    P2OUT &= ~BIT1;
    locked = true;

}

//*****************************************************************************
//
//! Periodic Timer Interrupt Handler
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void TimerPeriodicIntHandler(sigval val)
{
    /* Increment our interrupt counter.                                       */
    g_usTimerInts++;

    if (!(g_usTimerInts & 0x1))
    {
        /* Turn Led Off                                                       */
        GPIO_write(Board_LED0, Board_LED_OFF);
    }
    else
    {
        /* Turn Led On                                                        */
        GPIO_write(Board_LED0, Board_LED_ON);
    }
}

//*****************************************************************************
//
//! Function to configure and start timer to blink the LED while device is
//! trying to connect to an AP
//!
//! \param none
//!
//! return none
//
//*****************************************************************************
void LedTimerConfigNStart()
{
    struct itimerspec value;
    sigevent sev;

    /* Create Timer                                                           */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_notify_function = &TimerPeriodicIntHandler;
    timer_create(2, &sev, &g_timer);

    /* start timer                                                            */
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = TIMER_EXPIRATION_VALUE;
    value.it_value.tv_sec = 0;
    value.it_value.tv_nsec = TIMER_EXPIRATION_VALUE;

    timer_settime(g_timer, 0, &value, NULL);
}

//*****************************************************************************
//
//! Disable the LED blinking Timer as Device is connected to AP
//!
//! \param none
//!
//! return none
//
//*****************************************************************************
void LedTimerDeinitStop()
{ 
    /* Disable the LED blinking Timer as Device is connected to AP.           */
    timer_delete(g_timer);
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t    MSP432 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

void *MqttClientThread(void * pvParameters)
{
    struct msgQueue queueElement;
    struct msgQueue queueElemRecv;

    MQTTClient_run((NetAppHandle_t)pvParameters);

    queueElement.event  = LOCAL_CLIENT_DISCONNECTION;
    queueElement.msgPtr = NULL;

    /* write message indicating disconnect Broker message.                    */
    if (MQTT_SendMsgToQueue(&queueElement))
    {
        UART_PRINT("\n\n\rQueue is full, throw first msg and send the new one\n\n\r");
        mq_receive(g_PBQueue, (char*) &queueElemRecv, sizeof(struct msgQueue), NULL);
        MQTT_SendMsgToQueue(&queueElement);
    }

    pthread_exit(0);

    return NULL;
}

//*****************************************************************************
//
//! Task implementing MQTT Server plus client bridge
//!
//! This function
//!    1. Initializes network driver and connects to the default AP
//!    2. Initializes the mqtt client and server libraries and sets up MQTT
//!       with the remote broker.
//!    3. Set up the button events and their callbacks (for publishing)
//!    4. Handles the callback signals
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void * MqttClient(void *pvParameters)
{

    struct msgQueue queueElemRecv;
    long lRetVal = -1;

    /* Initializing Client and Subscribing to the Broker.                     */
    if (gApConnectionState >= 0)
    {
        lRetVal = MqttClient_start();
        if (lRetVal == -1)
        {
            UART_PRINT("MQTT Client lib initialization failed\n\r");
            pthread_exit(0);
            return NULL;
        }
    }

    /* handling the signals from various callbacks including the push button     */
    /* prompting the client to publish a msg on PUB_TOPIC OR msg received by     */
    /* the server on enrolled topic (for which the on-board client has enrolled) */
    /* from a local client (will be published to the remote broker by the        */
    /* client) OR msg received by the client from the remote broker (need to     */
    /* be sent to the server to see if any local client has subscribed on the    */
    /* same topic).                                                              */

    uint8_t digit;
    uint8_t randNum;
    uint8_t keysPressed;
    for (;;)
    {
        /* waiting for signals                                                */
        mq_receive(g_PBQueue, (char*) &queueElemRecv, sizeof(struct msgQueue), NULL);

        switch (queueElemRecv.event)
        {
            case PUBLISH_PUSH_BUTTON_PRESSED:
                /* Generate new 6-digit random passcode to send to the user   */
                for (digit=0; digit<PASSCODE_LEN; digit++) {
                    randNum = rand() % 10;
                    // make sure the code has no sevens in it since the
                    // seven key is broken...
                    while (randNum == 7 || randNum == 0) {
                        randNum = rand() % 10;
                    }
                    randomCode[digit] = randNum;
                    randomCodeStr[digit] = '0' + randNum;
                }

                /* Publish the secret code to MQTT to text it to the user's phone */
                lRetVal = MQTTClient_publish(gMqttClient, (char*) publish_topic_sendcode, strlen((char*)publish_topic_sendcode),
                                             (char*)publish_topic_sendcode_data, strlen((char*) publish_topic_sendcode_data),
                                             MQTT_QOS_2 | ((RETAIN_ENABLE)?MQTT_PUBLISH_RETAIN:0) );

                UART_PRINT("\n\rMSP432 Publishes the following message:\n\r");
                UART_PRINT("Topic: %s\n\r", publish_topic_sendcode);
                UART_PRINT("Data: %s\n\n\r", publish_topic_sendcode_data);

                /* Clear and enable again the SW2 interrupt */
                GPIO_clearInt(Board_BUTTON0);  // SW2
                GPIO_enableInt(Board_BUTTON0); // SW2

                keysPressed = 0;
                //drawKeypad();

                while (keysPressed < PASSCODE_LEN) {
                    touch_updateCurrentTouch(&g_sTouchContext);

                    if(g_sTouchContext.touch)
                    {
                        if(Graphics_isImageButtonSelected(&oneButton,
                                                          g_sTouchContext.x,
                                                          g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext, &oneButton);
                            enteredCode[keysPressed] = 1;
                        }
                        else if(Graphics_isImageButtonSelected(&twoButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext,&twoButton);
                            enteredCode[keysPressed] = 2;
                        }
                        else if(Graphics_isImageButtonSelected(&threeButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext,&threeButton);
                            enteredCode[keysPressed] = 3;
                        }
                        else if(Graphics_isImageButtonSelected(&fourButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext,&fourButton);
                            enteredCode[keysPressed] = 4;
                        }
                        else if(Graphics_isImageButtonSelected(&fiveButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext,&fiveButton);
                            enteredCode[keysPressed] = 5;
                        }
                        else if(Graphics_isImageButtonSelected(&sixButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext,&sixButton);
                            enteredCode[keysPressed] = 6;
                        }
                        else if(Graphics_isImageButtonSelected(&sevenButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            // Button seven is broken
                            //Graphics_drawSelectedImageButton(&g_sContext,&sevenButton);
                            keysPressed--;
                        }
                        else if(Graphics_isImageButtonSelected(&eightButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext,&eightButton);
                            enteredCode[keysPressed] = 8;
                        }
                        else if(Graphics_isImageButtonSelected(&nineButton,
                                                               g_sTouchContext.x,
                                                               g_sTouchContext.y))
                        {
                            Graphics_drawSelectedImageButton(&g_sContext,&nineButton);
                            enteredCode[keysPressed] = 9;
                        }

                        keysPressed++;
                    }
                }

                if (memcmp(enteredCode, randomCode, 6) == 0) {
                    lRetVal = MQTTClient_publish(gMqttClient, (char*) publish_topic_success, strlen((char*)publish_topic_success),
                                                 (char*)publish_topic_success_data, strlen((char*) publish_topic_success_data),
                                                 MQTT_QOS_2 | ((RETAIN_ENABLE)?MQTT_PUBLISH_RETAIN:0) );

                    UART_PRINT("\n\rMSP432 Publishes the following message:\n\r");
                    UART_PRINT("Topic: %s\n\r", publish_topic_success);
                    UART_PRINT("Data: %s\n\n\r", publish_topic_success_data);

                    GPIO_write(Board_LED0, Board_LED_OFF);
                    P2OUT |= BIT1;

                    locked = false;
                    //drawLock();
                }
                else {
                    lRetVal = MQTTClient_publish(gMqttClient, (char*) publish_topic_failure, strlen((char*)publish_topic_failure),
                                                 (char*)publish_topic_failure_data, strlen((char*) publish_topic_failure_data),
                                                 MQTT_QOS_2 | ((RETAIN_ENABLE)?MQTT_PUBLISH_RETAIN:0) );

                    UART_PRINT("\n\rMSP432 Publishes the following message:\n\r");
                    UART_PRINT("Topic: %s\n\r", publish_topic_failure);
                    UART_PRINT("Data: %s\n\n\r", publish_topic_failure_data);

                    GPIO_write(Board_LED0, Board_LED_ON);
                    P2OUT &= ~BIT1;

                    locked = true;
                    //drawLock();
                }

                break;

                /* On-board client disconnected from remote broker, only      */
                /* local MQTT network will work                               */
            case LOCAL_CLIENT_DISCONNECTION:
                UART_PRINT("\n\rOn-board Client Disconnected\n\r\r\n");
                gUiConnFlag = 0;
                break;

                /* Push button for full restart check                         */
            case DISC_PUSH_BUTTON_PRESSED:
                gResetApplication = true;
                break;
                
            case THREAD_TERMINATE_REQ:
                gUiConnFlag = 0;
                pthread_exit(0);
                return NULL;
                
            default:
                sleep(1);
                break;
        }
    }
}

//*****************************************************************************
//
//! This function connects the MQTT device to an AP with the SSID which was
//! configured in SSID_NAME definition which can be found in Network_if.h file, 
//! if the device can't connect to to this AP a request from the user for other 
//! SSID will appear.
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
int32_t Mqtt_IF_Connect()
{
    int32_t lRetVal;
    char SSID_Remote_Name[32];
    int8_t Str_Length;

    memset(SSID_Remote_Name, '\0', sizeof(SSID_Remote_Name));
    Str_Length = strlen(SSID_NAME);

    if (Str_Length)
    {
        /* Copy the Default SSID to the local variable                        */
        strncpy(SSID_Remote_Name, SSID_NAME, Str_Length);
    }

    /* Display Application Banner                                             */
    DisplayBanner(APPLICATION_NAME);

    GPIO_write(Board_LED0, Board_LED_OFF);
    GPIO_write(Board_LED1, Board_LED_OFF);
    P2OUT &= ~BIT1;

    /* Reset The state of the machine                                         */
    Network_IF_ResetMCUStateMachine();

    /* Start the driver                                                       */
    lRetVal = Network_IF_InitDriver(ROLE_STA);
    if (lRetVal < 0)
    {
        UART_PRINT("Failed to start SimpleLink Device\n\r", lRetVal);
        return -1;
    }

    /* Toggle the Green LED to indicate Simplelink is properly up.             */
    GPIO_write(Board_LED2, Board_LED_ON);

    /* Start Timer to blink Red LED till AP connection                        */
    LedTimerConfigNStart();

    /* Initialize AP security params                                          */
    SecurityParams.Key = (signed char *) SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    /* Connect to the Access Point                                            */
    lRetVal = Network_IF_ConnectAP(SSID_Remote_Name, SecurityParams);
    if (lRetVal < 0)
    {
        UART_PRINT("Connection to an AP failed\n\r");
        return -1;
    }

    /* Disable the LED blinking Timer as Device is connected to AP.           */
    LedTimerDeinitStop();

    /* Switch ON RED LED to indicate that Device acquired an IP.              */
    GPIO_write(Board_LED0, Board_LED_ON);

    sleep(1);

    GPIO_write(Board_LED0, Board_LED_OFF);
    GPIO_write(Board_LED1, Board_LED_OFF);
    P2OUT &= ~BIT1;

    return 0;
}

//*****************************************************************************
//!
//! MQTT Start - Initialize and create all the items required to run the MQTT
//! protocol
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void Mqtt_start()
{
    int32_t threadArg = 100;
    pthread_attr_t pAttrs;
    struct sched_param priParam;
    int32_t retc = 0;
    mq_attr attr;
    unsigned mode = 0;

    /* sync object for inter thread communication                             */
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct msgQueue);
    g_PBQueue = mq_open("g_PBQueue", O_CREAT, mode, &attr);

    if (g_PBQueue == NULL)
    {
        UART_PRINT("MQTT Message Queue create fail\n\r");
        gInitState &= ~MQTT_INIT_STATE;
        return;
    }

    /* Set priority and stack size attributes                                 */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 2;
    retc = pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, MQTTTHREADSIZE);
    retc |= pthread_attr_setdetachstate(&pAttrs, PTHREAD_CREATE_DETACHED);

    if (retc != 0)
    {
        gInitState &= ~MQTT_INIT_STATE;
        UART_PRINT("MQTT thread create fail\n\r");
        return;
    }

    retc = pthread_create(&mqttThread, &pAttrs, MqttClient, (void *) &threadArg);
    if (retc != 0)
    {
        gInitState &= ~MQTT_INIT_STATE;
        UART_PRINT("MQTT thread create fail\n\r");
        return;
    }

    /* enable interrupt for the GPIO 13 (SW3) and GPIO 22 (SW2).              */
    GPIO_setCallback(Board_BUTTON0, pushButtonInterruptHandler2);
    GPIO_enableInt(Board_BUTTON0); // SW2

    GPIO_setCallback(Board_BUTTON1, pushButtonInterruptHandler3);
    GPIO_enableInt(Board_BUTTON1); // SW3

    gInitState &= ~MQTT_INIT_STATE;

}

//*****************************************************************************
//!
//! MQTT Stop - Close the client instance and free all the items required to 
//! run the MQTT protocol
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************

void Mqtt_Stop()
{
    struct msgQueue queueElement;
    struct msgQueue queueElemRecv;

    if (gApConnectionState >= 0)
    {
        Mqtt_ClientStop(1);
    }

    queueElement.event = THREAD_TERMINATE_REQ;
    queueElement.msgPtr = NULL;

    /* write message indicating publish message                               */
    if (MQTT_SendMsgToQueue(&queueElement))
    {
        UART_PRINT("\n\n\rQueue is full, throw first msg and send the new one\n\n\r");
        mq_receive(g_PBQueue, (char*) &queueElemRecv, sizeof(struct msgQueue), NULL);
        MQTT_SendMsgToQueue(&queueElement);
    }

    sleep(2);

    mq_close(g_PBQueue);
    g_PBQueue = NULL;

    sl_Stop(200);
    UART_PRINT("\n\r Client Stop completed\r\n");

    /* Disable the SW2 and SW3 interrupt */
    GPIO_disableInt(Board_BUTTON0); // SW2
    GPIO_disableInt(Board_BUTTON1); // SW3
}

int32_t MqttClient_start()
{
    int32_t lRetVal = -1;

    int32_t threadArg = 100;
    pthread_attr_t pAttrs;
    struct sched_param priParam;

    MqttClientExmple_params.clientId = ClientId;
    MqttClientExmple_params.connParams = &Mqtt_ClientCtx;
    MqttClientExmple_params.mqttMode31 = MQTT_3_1;
    MqttClientExmple_params.blockingSend = true;

    gInitState |= CLIENT_INIT_STATE;

    /* Initialize MQTT client lib                                             */
    gMqttClient = MQTTClient_create(MqttClientCallback, &MqttClientExmple_params);
    if( gMqttClient == NULL )
    {
        /* lib initialization failed                                          */
        gInitState &= ~CLIENT_INIT_STATE;
        return -1;
    }

    /* Open Client Receive Thread start the receive task. Set priority and    */
    /* stack size attributes                                                  */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 2;
    lRetVal = pthread_attr_setschedparam(&pAttrs, &priParam);
    lRetVal |= pthread_attr_setstacksize(&pAttrs, RXTASKSIZE);
    lRetVal |= pthread_attr_setdetachstate(&pAttrs, PTHREAD_CREATE_DETACHED);
    lRetVal |= pthread_create(&g_rx_task_hndl, &pAttrs, MqttClientThread, (void *) &threadArg);
    if (lRetVal != 0)
    {
        UART_PRINT("Client Thread Create Failed failed\n\r");
        gInitState &= ~CLIENT_INIT_STATE;
        return -1;
    }
#ifdef SECURE_CLIENT
    setTime();
#endif

    /* setting will parameters                                                */
    MQTTClient_set(gMqttClient, MQTT_CLIENT_WILL_PARAM, &will_param, sizeof(will_param));

#ifdef CLNT_USR_PWD
    /* Set user name for client connection                                    */
    MQTTClient_set(gMqttClient, MQTT_CLIENT_USER_NAME, (void *)ClientUsername, strlen((char*)ClientUsername));

    /* Set password                                                           */
    MQTTClient_set(gMqttClient, MQTT_CLIENT_PASSWORD, (void *)ClientPassword, strlen((char*)ClientPassword));
#endif
    /* Initiate MQTT Connect                                                  */
    if (gApConnectionState >= 0)
    {
#if CLEAN_SESSION == false
        bool clean = CLEAN_SESSION;
        MQTTClient_set(gMqttClient, MQTT_CLIENT_CLEAN_CONNECT, (void *)&clean, sizeof(bool));
#endif
        /* The return code of MQTTClient_connect is the ConnACK value that
           returns from the server */
        lRetVal = MQTTClient_connect(gMqttClient);

        /* negative lRetVal means error,
           0 means connection successful without session stored by the server,
           greater than 0 means successful connection with session stored by
           the server */
        if (0 > lRetVal)
        {
            /* lib initialization failed                                      */
            UART_PRINT("Connection to broker failed\n\r");

            gUiConnFlag = 0;
        }
        else
        {
            gUiConnFlag = 1;
        }
    }

    gInitState &= ~CLIENT_INIT_STATE;

    return 0;
}

//*****************************************************************************
//!
//! MQTT Client stop - Unsubscribe from the subscription topics and exit the 
//! MQTT client lib.
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************

void Mqtt_ClientStop(uint8_t disconnect)
{
    /* exiting the Client library                                             */
    MQTTClient_delete(gMqttClient);

}

//*****************************************************************************
//!
//! Utility function which prints the borders
//!
//! \param[in] ch  -  hold the charater for the border.
//! \param[in] n   -  hold the size of the border.
//!
//! \return none.
//!
//*****************************************************************************

void printBorder(char ch, int n)
{
    int        i = 0;

    for(i=0; i<n; i++)    putch(ch);
}

//*****************************************************************************
//!
//! Set the ClientId with its own mac address
//! This routine converts the mac address which is given
//! by an integer type variable in hexadecimal base to ASCII
//! representation, and copies it into the ClientId parameter.
//!
//! \param  macAddress  -   Points to string Hex.
//!
//! \return void.
//!
//*****************************************************************************
void SetClientIdNamefromMacAddress(uint8_t *macAddress)
{
    uint8_t Client_Mac_Name[2];
    uint8_t Index;

    /* When ClientID isn't set, use the mac address as ClientID               */
    if (ClientId[0] == '\0')
    {
        /* 6 bytes is the length of the mac address                           */
        for (Index = 0; Index < SL_MAC_ADDR_LEN; Index++)
        {
            /* Each mac address byte contains two hexadecimal characters      */
            /* Copy the 4 MSB - the most significant character                */
            Client_Mac_Name[0] = (macAddress[Index] >> 4) & 0xf;
            /* Copy the 4 LSB - the least significant character               */
            Client_Mac_Name[1] = macAddress[Index] & 0xf;

            if (Client_Mac_Name[0] > 9)
            {
                /* Converts and copies from number that is greater than 9 in  */
                /* hexadecimal representation (a to f) into ascii character   */
                ClientId[2 * Index] = Client_Mac_Name[0] + 'a' - 10;
            }
            else
            {
                /* Converts and copies from number 0 - 9 in hexadecimal       */
                /* representation into ascii character                        */
                ClientId[2 * Index] = Client_Mac_Name[0] + '0';
            }
            if (Client_Mac_Name[1] > 9)
            {
                /* Converts and copies from number that is greater than 9 in  */
                /* hexadecimal representation (a to f) into ascii character   */
                ClientId[2 * Index + 1] = Client_Mac_Name[1] + 'a' - 10;
            }
            else
            {
                /* Converts and copies from number 0 - 9 in hexadecimal       */
                /* representation into ascii character                        */
                ClientId[2 * Index + 1] = Client_Mac_Name[1] + '0';
            }
        }
    }
}

//*****************************************************************************
//!
//! Utility function which Display the app banner
//!
//! \param[in] appName     -  holds the application name.
//! \param[in] appVersion  -  holds the application version.
//!
//! \return none.
//!
//*****************************************************************************

int32_t DisplayAppBanner(char* appName, char* appVersion)
{
    int32_t            ret = 0;
    uint8_t            macAddress[SL_MAC_ADDR_LEN];
    uint16_t           macAddressLen = SL_MAC_ADDR_LEN;
    uint16_t           ConfigSize = 0;
    uint8_t            ConfigOpt = SL_DEVICE_GENERAL_VERSION;
    SlDeviceVersion_t  ver = {0};

    ConfigSize = sizeof(SlDeviceVersion_t);

    /* Print device version info. */
    ret = sl_DeviceGet(SL_DEVICE_GENERAL, &ConfigOpt, &ConfigSize, (uint8_t*)(&ver));

    /* Print device Mac address */
    ret = sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, 0, &macAddressLen, &macAddress[0]);

    UART_PRINT(lineBreak);
    UART_PRINT("\t");
    printBorder('=', 44);
    UART_PRINT(lineBreak);
    UART_PRINT("\t   %s Example Ver: %s",appName, appVersion);
    UART_PRINT(lineBreak);
    UART_PRINT("\t");
    printBorder('=', 44);
    UART_PRINT(lineBreak);
    UART_PRINT(lineBreak);
    UART_PRINT("\t CHIP: 0x%x",ver.ChipId);
    UART_PRINT(lineBreak);
    UART_PRINT("\t MAC:  %d.%d.%d.%d",ver.FwVersion[0],ver.FwVersion[1],ver.FwVersion[2],ver.FwVersion[3]);
    UART_PRINT(lineBreak);
    UART_PRINT("\t PHY:  %d.%d.%d.%d",ver.PhyVersion[0],ver.PhyVersion[1],ver.PhyVersion[2],ver.PhyVersion[3]);
    UART_PRINT(lineBreak);
    UART_PRINT("\t NWP:  %d.%d.%d.%d",ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3]);
    UART_PRINT(lineBreak);
    UART_PRINT("\t ROM:  %d",ver.RomVersion);
    UART_PRINT(lineBreak);
    UART_PRINT("\t HOST: %s", SL_DRIVER_VERSION);
    UART_PRINT(lineBreak);
    UART_PRINT("\t MAC address: %02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    UART_PRINT(lineBreak);
    UART_PRINT(lineBreak);
    UART_PRINT("\t");
    printBorder('=', 44);
    UART_PRINT(lineBreak);
    UART_PRINT(lineBreak);

    SetClientIdNamefromMacAddress(macAddress);

    return ret;
}

void mainThread(void * args)
{
    uint32_t count = 0;
    pthread_t spawn_thread = (pthread_t) NULL;
    pthread_attr_t pAttrs_spawn;
    struct sched_param priParam;
    int32_t retc = 0;
    UART_Handle tUartHndl;

    Board_initGPIO();
    Board_initSPI();

    /* Initialize the demo. */
    boardInit();
    //clockInit();
    initializeKeypadButtons();

    /* Globally enable interrupts. */
    __enable_interrupt();

    // LCD setup using Graphics Library API calls
    Kitronix320x240x16_SSD2119Init();
    Graphics_initContext(&g_sContext, &g_sKitronix320x240x16_SSD2119,
                         &g_sKitronix320x240x16_SSD2119_funcs);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setFont(&g_sContext, &g_sFontCmss20b);
    Graphics_clearDisplay(&g_sContext);

    /* Calibrate the LCD and draw the keypad                                 */
    touch_initInterface();
    locked = true;
    drawKeypad();
    //drawLock();

    /* Seed the RNG                                                           */
    srand(TLV->RANDOM_NUM_1);

    /* Configure the UART                                                     */
    tUartHndl = InitTerm();
    /* remove uart receive from LPDS dependency                               */
    UART_control(tUartHndl, UART_CMD_RXDISABLE, NULL);

    /* Create the sl_Task                                                     */
    pthread_attr_init(&pAttrs_spawn);
    priParam.sched_priority = SPAWN_TASK_PRIORITY;
    retc = pthread_attr_setschedparam(&pAttrs_spawn, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs_spawn, TASKSTACKSIZE);
    retc |= pthread_attr_setdetachstate(&pAttrs_spawn, PTHREAD_CREATE_DETACHED);

    retc = pthread_create(&spawn_thread, &pAttrs_spawn, sl_Task, NULL);

    if (retc != 0)
    {
        UART_PRINT("could not create simplelink task\n\r");
        while (1);
    }

    retc = sl_Start(0, 0, 0);
    if (retc < 0)
    {
        /* Handle Error */
        UART_PRINT("\n sl_Start failed\n");
        while(1);
    }

    /* Output device information to the UART terminal */
    retc = DisplayAppBanner(APPLICATION_NAME, APPLICATION_VERSION);

    retc = sl_Stop(SL_STOP_TIMEOUT );
    if (retc < 0)
    {
        /* Handle Error */
        UART_PRINT("\n sl_Stop failed\n");
        while(1);
    }

    if(retc < 0)
    {
        /* Handle Error */
        UART_PRINT("mqtt_client - Unable to retrieve device information \n");
        while (1);
    }


    while (1)
    {
        gResetApplication = false;
        gInitState = 0;

        /* Connect to AP                                                      */
        gApConnectionState = Mqtt_IF_Connect();

        gInitState |= MQTT_INIT_STATE;
        /* Run MQTT Main Thread (it will open the Client and Server)          */
        Mqtt_start();

        /* Wait for init to be completed!!!                                   */
        while (gInitState != 0)
        {
            UART_PRINT(".");
            sleep(1);
        }
        UART_PRINT(".\r\n");

        GPIO_write(Board_LED0, Board_LED_ON);
        while (gResetApplication == false) {
            if (reset) {
                reset = false;

                MQTTClient_publish(gMqttClient, (char*) publish_topic_failure, strlen((char*)publish_topic_failure),
                                             (char*)publish_topic_failure_data, strlen((char*) publish_topic_failure_data),
                                             MQTT_QOS_2 | ((RETAIN_ENABLE)?MQTT_PUBLISH_RETAIN:0) );

                UART_PRINT("\n\rMSP432 Publishes the following message:\n\r");
                UART_PRINT("Topic: %s\n\r", publish_topic_failure);
                UART_PRINT("Data: %s\n\n\r", publish_topic_failure_data);
            }
        }

        UART_PRINT("TO Complete - Closing all threads and resources\r\n");

        /* Stop the MQTT Process                                              */
        Mqtt_Stop();

        UART_PRINT("reopen MQTT # %d  \r\n", ++count);

    }
}

void initializeKeypadButtons(void)
{
    oneButton.xPosition = 0;
    oneButton.yPosition = 0;
    oneButton.borderWidth = 1;
    oneButton.selected = false;
    oneButton.imageWidth = one8BPP_UNCOMP.xSize;
    oneButton.imageHeight = one8BPP_UNCOMP.ySize;
    oneButton.borderColor = GRAPHICS_COLOR_WHITE;
    oneButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    oneButton.image = &one8BPP_UNCOMP;

    twoButton.xPosition = 106;
    twoButton.yPosition = 0;
    twoButton.borderWidth = 1;
    twoButton.selected = false;
    twoButton.imageWidth = two8BPP_UNCOMP.xSize;
    twoButton.imageHeight = two8BPP_UNCOMP.ySize;
    twoButton.borderColor = GRAPHICS_COLOR_WHITE;
    twoButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    twoButton.image = &two8BPP_UNCOMP;

    threeButton.xPosition = 212;
    threeButton.yPosition = 0;
    threeButton.borderWidth = 1;
    threeButton.selected = false;
    threeButton.imageWidth = three8BPP_UNCOMP.xSize;
    threeButton.imageHeight = three8BPP_UNCOMP.ySize;
    threeButton.borderColor = GRAPHICS_COLOR_WHITE;
    threeButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    threeButton.image = &three8BPP_UNCOMP;

    fourButton.xPosition = 0;
    fourButton.yPosition = 61;
    fourButton.borderWidth = 1;
    fourButton.selected = false;
    fourButton.imageWidth = four8BPP_UNCOMP.xSize;
    fourButton.imageHeight = four8BPP_UNCOMP.ySize;
    fourButton.borderColor = GRAPHICS_COLOR_WHITE;
    fourButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    fourButton.image = &four8BPP_UNCOMP;

    fiveButton.xPosition = 106;
    fiveButton.yPosition = 61;
    fiveButton.borderWidth = 1;
    fiveButton.selected = false;
    fiveButton.imageWidth = five8BPP_UNCOMP.xSize;
    fiveButton.imageHeight = five8BPP_UNCOMP.ySize;
    fiveButton.borderColor = GRAPHICS_COLOR_WHITE;
    fiveButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    fiveButton.image = &five8BPP_UNCOMP;

    sixButton.xPosition = 212;
    sixButton.yPosition = 61;
    sixButton.borderWidth = 1;
    sixButton.selected = false;
    sixButton.imageWidth = six8BPP_UNCOMP.xSize;
    sixButton.imageHeight = six8BPP_UNCOMP.ySize;
    sixButton.borderColor = GRAPHICS_COLOR_WHITE;
    sixButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    sixButton.image = &six8BPP_UNCOMP;

    sevenButton.xPosition = 0;
    sevenButton.yPosition = 122;
    sevenButton.borderWidth = 1;
    sevenButton.selected = false;
    sevenButton.imageWidth = seven8BPP_UNCOMP.xSize;
    sevenButton.imageHeight = seven8BPP_UNCOMP.ySize;
    sevenButton.borderColor = GRAPHICS_COLOR_WHITE;
    sevenButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    sevenButton.image = &seven8BPP_UNCOMP;

    eightButton.xPosition = 106;
    eightButton.yPosition = 122;
    eightButton.borderWidth = 1;
    eightButton.selected = false;
    eightButton.imageWidth = eight8BPP_UNCOMP.xSize;
    eightButton.imageHeight = eight8BPP_UNCOMP.ySize;
    eightButton.borderColor = GRAPHICS_COLOR_WHITE;
    eightButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    eightButton.image = &eight8BPP_UNCOMP;

    nineButton.xPosition = 212;
    nineButton.yPosition = 122;
    nineButton.borderWidth = 1;
    nineButton.selected = false;
    nineButton.imageWidth = nine8BPP_UNCOMP.xSize;
    nineButton.imageHeight = nine8BPP_UNCOMP.ySize;
    nineButton.borderColor = GRAPHICS_COLOR_WHITE;
    nineButton.selectedColor = GRAPHICS_COLOR_ROYAL_BLUE;
    nineButton.image = &nine8BPP_UNCOMP;
}

void drawLock(void)
{
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_clearDisplay(&g_sContext);

    /* Draw text in center of screen */
    Graphics_drawStringCentered(&g_sContext, "IOTLock",
                                AUTO_STRING_LENGTH,
                                159,
                                15,
                                TRANSPARENT_TEXT);

    /* Draw TI banner at the bottom of screen */
    Graphics_drawImage(&g_sContext,
                       &TI_platform_bar_red4BPP_UNCOMP,
                       0,
                       Graphics_getDisplayHeight(&g_sContext) -
                       TI_platform_bar_red4BPP_UNCOMP.ySize);

    if (locked)
    {
        /* Draw Lock */
        Graphics_drawImage(&g_sContext, &locked8BPP_UNCOMP, 80, 32);
    }
    else
    {
        /* Draw Unlock */
        Graphics_drawImage(&g_sContext, &unlocked8BPP_UNCOMP, 80, 32);
    }
}

void drawKeypad(void)
{
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_clearDisplay(&g_sContext);

    // Draw TI banner at the bottom of screen
    Graphics_drawImage(&g_sContext,
                       &TI_platform_bar_red4BPP_UNCOMP,
                       0,
                       Graphics_getDisplayHeight(
                           &g_sContext) - TI_platform_bar_red4BPP_UNCOMP.ySize);

    // Draw keypad buttons
    Graphics_drawImageButton(&g_sContext, &oneButton);
    Graphics_drawImageButton(&g_sContext, &twoButton);
    Graphics_drawImageButton(&g_sContext, &threeButton);
    Graphics_drawImageButton(&g_sContext, &fourButton);
    Graphics_drawImageButton(&g_sContext, &fiveButton);
    Graphics_drawImageButton(&g_sContext, &sixButton);
    Graphics_drawImageButton(&g_sContext, &sevenButton);
    Graphics_drawImageButton(&g_sContext, &eightButton);
    Graphics_drawImageButton(&g_sContext, &nineButton);
}

void boardInit()
{
    FPU_enableModule();
}

void clockInit(void)
{
    /* 2 flash wait states, VCORE = 1, running off DC-DC, 48 MHz */
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);
    PCM_setPowerState(PCM_AM_DCDC_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_setDCOFrequency(48000000);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, 1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, 1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, 1);

    return;
}

void Delay(uint16_t msec){
    uint32_t i = 0;
    uint32_t time = (msec / 1000) * (SYSTEM_CLOCK_SPEED / 15);

    for(i = 0; i < time; i++)
    {
        ;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
