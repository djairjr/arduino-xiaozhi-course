#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

SemaphoreHandle_t semaphore;

void count(void* pvParameters)
{
    Serial.println("Start counting...")nting...");
    for (int i = 1; i <= 10; i++)
    {
        Serial.printf("Current report: %d\n", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    xSemaphoreGive(semaphore);  // The semaphore is released after the report. Only when the number is released here can the setup function take successfully.
    vTaskDelete(nullptr);
}

void setup()
{
    Serial.begin(9600);
    semaphore = xSemaphoreCreateBinary();

    // Create a counting task, counting from 1 to 10 counting
    xTaskCreate(count,
        "count",
        2048,
        nullptr,
        1,
        nullptr);

    // Wait for the semaphore to be obtained (the semaphore will be released only if the previous task count is completed), if it cannot be obtained, it will be waiting.
    xSemaphoreTake(semaphore, portMAX_DELAY);
    Serial.println("Acquisition of semaphore successfully")on of semaphore successfully");
}

void loop()
{
}
