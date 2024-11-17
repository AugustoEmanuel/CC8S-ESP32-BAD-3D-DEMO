#include "n64controller.hpp"
#include <Arduino.h>
#include <Wire.h>
#include <queue>
#include "graphics.hpp"
#include <esp_ota_ops.h>

#define C_BUFFER_SIZE   4

#define SLAVE_ADDR 0x08
#define ARDUINO_BUSY_PIN		39


const unsigned long timeoutMillis = 1000;
volatile unsigned int delayC;

static int8_t cDataBuffer[C_BUFFER_SIZE];

N64C_D actualState;
std::queue<N64C_D> inputQueueOld = {};

N64CCallbackFunction n64callback = NULL;

static N64C_D convertDataToStruct();

void N64C::init(){
    //RESET ARDUINO
    Wire.begin();
    // pinMode(ARDUINO_RESET_PIN, OUTPUT);
    // digitalWrite(ARDUINO_RESET_PIN, LOW);
    // delay(5);
    // digitalWrite(ARDUINO_RESET_PIN, HIGH);
    delay(3000); //GIVE SOME TIME TO ARDUINO TO RESTART

    pinMode(ARDUINO_BUSY_PIN, INPUT);
}

//TA TENDO UMAS CORRUPÇÕES USANDO O JOYSTICK MAS FDS (FIM DE SEMANA) NEM VO USA PORCARIA
void N64C::updateInputBuffer()
{

    // Serial.println("WAITING ARDUINO");

    // Wait for ARDUINO_BUSY_PIN to go low (or high), with timeout
    unsigned long startMillis = millis();
    while (digitalRead(ARDUINO_BUSY_PIN) == 1) {
        if (millis() - startMillis >= timeoutMillis) {
            // Handle timeout (e.g., break, log an error, etc.)
            Serial.println("Timeout waiting for ARDUINO_BUSY_PIN to go low.");
            break;
        }
        vTaskDelay(1);
    }

    // Request data from I2C slave
    Wire.requestFrom(SLAVE_ADDR, C_BUFFER_SIZE);

    int i = 0;

    // Wait for at least 4 bytes to become available on I2C, with timeout
    startMillis = millis();
    while (Wire.available() < 4) {
        if (millis() - startMillis >= timeoutMillis) {
            // Handle timeout (e.g., break, log an error, etc.)
            Serial.println("Timeout waiting for I2C data.");
            break;
        }
        vTaskDelay(1);
    }

    // Read the available bytes if the timeout didn't occur
    if (Wire.available() >= 4) {
        Wire.readBytes((uint8_t *)cDataBuffer, 4);
    }

    // Wait for ARDUINO_BUSY_PIN to go high (or low), with timeout
    startMillis = millis();
    while (digitalRead(ARDUINO_BUSY_PIN) == 0) {
        if (millis() - startMillis >= timeoutMillis) {
            // Handle timeout (e.g., break, log an error, etc.)
            Serial.println("Timeout waiting for ARDUINO_BUSY_PIN to go high.");
            break;
        }
        vTaskDelay(1);
    }


    N64C_D newState = convertDataToStruct();

    N64C_D changed;
    changed.start = !newState.start && actualState.start;
    changed.Z = !newState.Z && actualState.Z;
    changed.B = !newState.B && actualState.B;
    changed.A = !newState.A && actualState.A;

    changed.R = !newState.R && actualState.R;
    changed.L = !newState.L && actualState.L;

    changed.C_up = !newState.C_up && actualState.C_up;
    changed.C_down = !newState.C_down && actualState.C_down;
    changed.C_right = !newState.C_right && actualState.C_right;
    changed.C_left = !newState.C_left && actualState.C_left;

    changed.up = !newState.up && actualState.up;
    changed.down = !newState.down && actualState.down;
    changed.right = !newState.right && actualState.right;
    changed.left = !newState.left && actualState.left;

    changed.x = 0;
    changed.y = 0;

    actualState = newState;


    if(!inputQueueOld.empty() && inputQueueOld.back() == changed) return;

    if(inputQueueOld.size() > 2048) {
        inputQueueOld.pop();
    } //SE ISSO ACONTECER FOI ALGUMA CAGADA PESADA
    inputQueueOld.push(changed);
    // Serial.println("NEW INPUT");
}

void N64C::update()
{
    if(n64callback == NULL) return;


    N64C_D changed = {};
    if(!inputQueueOld.empty()){
        changed = inputQueueOld.front();
        inputQueueOld.pop();
    }
    n64callback(actualState, changed);
}

static N64C_D convertDataToStruct()
{
    N64C_D controlerData;

    controlerData.start = cDataBuffer[0] & 16 ? 1:0;
    controlerData.Z = cDataBuffer[0] & 32 ? 1:0;
    controlerData.B = cDataBuffer[0] & 64 ? 1:0;
    controlerData.A = cDataBuffer[0] & 128 ? 1:0;

    controlerData.R = cDataBuffer[1] & 16 ? 1:0;
    controlerData.L = cDataBuffer[1] & 32 ? 1:0;

    controlerData.C_up = cDataBuffer[1] & 0x08 ? 1:0;
    controlerData.C_down = cDataBuffer[1] & 0x04 ? 1:0;
    controlerData.C_right = cDataBuffer[1] & 0x01 ? 1:0;
    controlerData.C_left = cDataBuffer[1] & 0x02 ? 1:0;

    controlerData.up = cDataBuffer[0] & 0x08 ? 1:0;
    controlerData.down = cDataBuffer[0] & 0x04 ? 1:0;
    controlerData.right = cDataBuffer[0] & 0x01 ? 1:0;
    controlerData.left = cDataBuffer[0] & 0x02 ? 1:0;

    controlerData.x = cDataBuffer[2];
    controlerData.y = cDataBuffer[3];

    return controlerData;
}

void N64C::setControllerCallback(N64CCallbackFunction callback)
{
    n64callback = callback;
}

void N64C::checkForFactoryStart(){
    for(int i = 0; i < 60; i++){
        updateInputBuffer();
        if(actualState.start){
            const esp_partition_t* factory_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
            if (factory_partition) {
                esp_ota_set_boot_partition(factory_partition);
            }
            ESP.restart();
        }
    }
}