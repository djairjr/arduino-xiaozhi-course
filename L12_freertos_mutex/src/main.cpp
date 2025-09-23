#include <Arduino.h>

volatile int totalCount;
SemaphoreHandle_t semaphore1;
SemaphoreHandle_t semaphore2;
SemaphoreHandle_t mutex;

void count1(void* ptr)
{
    Serial.println("任务1开始计数");
    for (int i = 0; i < 10000; i++)
    {
        // if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) == pdTRUE)
        // {
            totalCount++;
            // xSemaphoreGiveRecursive(mutex);
        // }
        // vTaskDelay(pdMS_TO_TICKS(1));
    }
    xSemaphoreGive(semaphore1);
    vTaskDelete(nullptr);
}

void count2(void* ptr)
{
    Serial.println("任务2开始计数");
    for (int i = 0; i < 10000; i++)
    {
        // if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) == pdTRUE)
        // {
            totalCount++;
        //     xSemaphoreGiveRecursive(mutex);
        // }
        // vTaskDelay(pdMS_TO_TICKS(1));
    }
    xSemaphoreGive(semaphore2);
    vTaskDelete(nullptr);
}

void setup()
{
    Serial.begin(9600);
    vTaskDelay(pdMS_TO_TICKS(3000));

    totalCount = 0;
    semaphore1 = xSemaphoreCreateBinary();
    semaphore2 = xSemaphoreCreateBinary();
    mutex = xSemaphoreCreateMutex();

    // Create two counting tasks, each task accumulates 10,000 on totalCount respectively
    xTaskCreatePinnedToCore(count1, "count-task1", 1024, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(count2, "count-task2", 1024, nullptr, 1, nullptr, 1);

    // Wait for both tasks to end
    xSemaphoreTake(semaphore1, portMAX_DELAY);
    xSemaphoreTake(semaphore2, portMAX_DELAY);

    // Print the final count value (expectedly 20000)
    Serial.printf("total count = %d\n", totalCount);
}

void loop()
{
}
