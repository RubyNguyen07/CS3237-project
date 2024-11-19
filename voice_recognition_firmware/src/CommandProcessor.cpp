#include <Arduino.h>
#include "CommandProcessor.h"
#include <HardwareSerial.h>

#define CHICKEN 18
#define FISH 21
#define SALAD 19
#define ACK 4

int chic_state = 0;
int fish_state = 0;
int sal_state = 0;

const char *words[] = {
    "chickenrice",
    "salad",
    "fishsoup",
    "_invalid",
};


void commandQueueProcessorTask(void *param)
{
    CommandProcessor *commandProcessor = (CommandProcessor *)param;
    while (true)
    {
        uint16_t commandIndex = 0;
        if (xQueueReceive(commandProcessor->m_command_queue_handle, &commandIndex, portMAX_DELAY) == pdTRUE)
        {
            commandProcessor->processCommand(commandIndex);
        }
    }
}

int calcDuty(int ms)
{
    // 50Hz = 20ms period
    return (65536 * ms) / 20000;
}

const int leftForward = 1600;
const int leftBackward = 1400;
const int leftStop = 1500;
const int rightBackward = 1600;
const int rightForward = 1445;
const int rightStop = 1500;

void CommandProcessor::processCommand(uint16_t commandIndex)
{
    if (commandIndex == 0) {
        digitalWrite(CHICKEN, LOW);
        int cnt = 0; 
        while (cnt < 250 && digitalRead(ACK) == LOW) {
            cnt++;
            delay(10);
        }
        digitalWrite(CHICKEN, HIGH);
        digitalWrite(SALAD, HIGH);
        digitalWrite(FISH, HIGH);
    } else if (commandIndex == 1) {
        digitalWrite(SALAD, LOW);
        int cnt = 0; 
        while (cnt < 250 && digitalRead(ACK) == LOW) {
            cnt++;
            delay(20);
        }
        digitalWrite(CHICKEN, HIGH);
        digitalWrite(SALAD, HIGH);
        digitalWrite(FISH, HIGH);
        
    } else if (commandIndex == 2) {
        digitalWrite(FISH, LOW);
        int cnt = 0; 
        while (cnt < 250 && digitalRead(ACK) == LOW) {
            cnt++;
            delay(20);
        }
        digitalWrite(CHICKEN, HIGH);
        digitalWrite(SALAD, HIGH);
        digitalWrite(FISH, HIGH);
    } 
}

CommandProcessor::CommandProcessor()
{
    // allow up to 5 commands to be in flight at once
    m_command_queue_handle = xQueueCreate(5, sizeof(uint16_t));
    if (!m_command_queue_handle)
    {
        Serial.println("Failed to create command queue");
    }
    // kick off the command processor task
    TaskHandle_t command_queue_task_handle;
    xTaskCreate(commandQueueProcessorTask, "Command Queue Processor", 1024, this, 1, &command_queue_task_handle);
}

void CommandProcessor::queueCommand(uint16_t commandIndex, float best_score)
{
    // unsigned long now = millis();
    // Serial.printf("***** %ld Command %d(%f)\n", millis(), commandIndex, best_score);
    if (commandIndex != 5 && commandIndex != -1)
    {
        Serial.printf("***** %ld Detected command %s(%f)\n", millis(), words[commandIndex], best_score);
        if (xQueueSendToBack(m_command_queue_handle, &commandIndex, 0) != pdTRUE)
        {
            Serial.println("No more space for command");
        }
    }
}
