#include <Arduino.h>

TaskHandle_t printMessageTask1;
TaskHandle_t printMessageTask2;
TaskHandle_t printMessageTask3;
TaskHandle_t printMessageTask4;

void printMessage(void* ptr)
{
    Serial.println(static_cast<char*>(ptr));
    while (true)
    {
<<<<<<< HEAD
        Serial.printf("Current task parameters: %s, Running CPU: %d\n", static_cast<char*>(ptr), xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    // This is the first permanent task, so the line of code will not be run. If it is a task that will end,
    // Then you need to delete the current task at the end of the task. The parameter is NULL to indicate that you delete yourself.
=======
        Serial.printf("Current task parameter: %s, Running CPU: %d\n", static_cast<char*>(ptr), xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    // This is the first permanently running task, so this line of code will not be executed. If it were a task that would end,
    // it would need to delete the current task at the end, with NULL as the parameter indicating to delete itself.
>>>>>>> aa37e20ed286b71ae060ecd39c107b7b1171a68e
    vTaskDelete(nullptr);
}

void setup()
{
    Serial.begin(9600);
    vTaskDelay(pdMS_TO_TICKS(3000));
    xTaskCreate(printMessage, // Task function pointer
                "printMessage1", // Task name
<<<<<<< HEAD
                2048, // Task stack size, simple task stack can be set smaller, complex tasks need to be set larger to get the stack, otherwise the stack will be easily overflowed.
                const_cast<char*>("This is the first task parameter"), // What parameters need to be passed into the task processing function
                1, // Task priority, optional range is 0-24, the larger the number, the higher the priority
                &printMessageTask1); // Task handle. If there is a control requirement for the task in the future, you can set this parameter, TaskHandle_t type

    xTaskCreate(printMessage, // Task function pointer
                "printMessage2", // Task name
                4096, // Task stack size, simple task stack can be set smaller, complex tasks need to be set larger to get the stack, otherwise the stack will be easily overflowed.
                const_cast<char*>("This is the second task parameter"), // What parameters need to be passed into the task processing function
                2, // Task priority, optional range is 0-24, the larger the number, the higher the priority
                &printMessageTask2); // Task handle. If there is a control requirement for the task in the future, you can set this parameter, TaskHandle_t type

    vTaskDelay(pdMS_TO_TICKS(5000));
    // After 5s, delete the above two tasks
=======
                2048, // Task stack size, simple tasks can have a smaller stack, complex tasks need a larger stack to avoid stack overflow
                const_cast<char*>("This is the first task parameter"), // What parameter to pass to the task handler function
                1, // Task priority, optional range 0-24, higher number means higher priority
                &printMessageTask1); // Task handle, if there is a need to control this task later, set this parameter, TaskHandle_t type

    xTaskCreate(printMessage, // Task function pointer
                "printMessage2", // Task name
                4096, // Task stack size, simple tasks can have a smaller stack, complex tasks need a larger stack to avoid stack overflow
                const_cast<char*>("This is the second task parameter"), // What parameter to pass to the task handler function
                2, // Task priority, optional range 0-24, higher number means higher priority
                &printMessageTask2); // Task handle, if there is a need to control this task later, set this parameter, TaskHandle_t type

    vTaskDelay(pdMS_TO_TICKS(5000));
    // After 5 seconds, delete the above two tasks
>>>>>>> aa37e20ed286b71ae060ecd39c107b7b1171a68e
    vTaskDelete(printMessageTask1);
    vTaskDelete(printMessageTask2);

    xTaskCreatePinnedToCore(printMessage,
                            "printMessage3",
                            2048,
<<<<<<< HEAD
                            const_cast<char*>("The third task parameter"),
=======
                            const_cast<char*>("Third task parameter"),
>>>>>>> aa37e20ed286b71ae060ecd39c107b7b1171a68e
                            1,
                            &printMessageTask3,
                            0);
    xTaskCreatePinnedToCore(printMessage,
                            "printMessage4",
                            2048,
<<<<<<< HEAD
                            const_cast<char*>("The 4th task parameter"),
=======
                            const_cast<char*>("Fourth task parameter"),
>>>>>>> aa37e20ed286b71ae060ecd39c107b7b1171a68e
                            2,
                            &printMessageTask4,
                            1);
    vTaskDelay(pdMS_TO_TICKS(5000));
<<<<<<< HEAD
    // After 5s, delete the above two tasks
=======
    // After 5 seconds, delete the above two tasks
>>>>>>> aa37e20ed286b71ae060ecd39c107b7b1171a68e
    vTaskDelete(printMessageTask3);
    vTaskDelete(printMessageTask4);
}

void loop()
{
}
