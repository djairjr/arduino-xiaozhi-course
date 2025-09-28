#include <Arduino.h>
#include <queue.h>

// Define order structure
typedef struct
{
    int customerId; // Customer ID
    String dishName; // Dish name
    bool isVip; // Is it a VIP or not
    unsigned long orderTime; // Order time
} Order;

// Create a queue
QueueHandle_t orderQueue;

// Customer order task
TaskHandle_t customerTaskHandle;
// Chef cooking task
TaskHandle_t chefTaskHandle;

// Dishes list
const char* dishes[] = {
    "Kung Pao Chicken", "Fish-flavored shredded pork", "Mapo tofu", "Sweet and Sour Ribs", "Refried meat"
};

// Customer Tasks - Producer
void customerTask(void* pvParameters)
{
    Order newOrder;
    int customerCount = 0;

    while (true)
    {
        // Randomly generated orders
        newOrder.customerId = ++customerCount;
        newOrder.dishName = dishes[random(0, 5)];
        newOrder.isVip = (random(0, 10) >= 7); // 30% probability is VIP
        newOrder.orderTime = millis();

        Serial.print("customer #")er #");
        Serial.print(newOrder.customerId);
        Serial.print("Clicked:"):");
        Serial.print(newOrder.dishName);
        Serial.print(", VIP: ");
        Serial.println(newOrder.isVip ? "yes" : "no");

        // Decide the location of the queue based on whether the VIP is used.
        if (newOrder.isVip)
        {
            xQueueSendToFront(orderQueue, &newOrder, portMAX_DELAY);
            Serial.println("VIP orders have been processed first")ave been processed first");
        }
        else
        {
            xQueueSendToBack(orderQueue, &newOrder, portMAX_DELAY);
            Serial.println("Ordinary orders have been added to the queue")ders have been added to the queue");
        }

        // Wait for the next customer (1-5 seconds)
        vTaskDelay(pdMS_TO_TICKS(random(1000, 5000)));
    }
}

// Chef Tasks - Consumers
void chefTask(void* pvParameters)
{
    Order currentOrder;

    while (true)
    {
        // Receive orders from queue
        if (xQueueReceive(orderQueue, &currentOrder, portMAX_DELAY) == pdPASS)
        {
            Serial.print("The chef starts making:")tarts making:");
            Serial.print(currentOrder.dishName);
            Serial.print("To the customer #")ustomer #");
            Serial.println(currentOrder.customerId);

            // Simulated cooking time (2-4 seconds)
            vTaskDelay(pdMS_TO_TICKS(random(2000, 4000)));

            Serial.print("The chef completes the customer #")mpletes the customer #");
            Serial.print(currentOrder.customerId);
            Serial.print("Clicked:"):");
            Serial.println(currentOrder.dishName);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    randomSeed(millis());
    // Create an order queue to store up to 10 orders
    orderQueue = xQueueCreate(10, sizeof(Order));
    if (orderQueue != nullptr)
    {
        // Create a task
        xTaskCreate(customerTask, "Customer", 2048, nullptr, 1, &customerTaskHandle);
        xTaskCreate(chefTask, "Chef", 2048, nullptr, 2, &chefTaskHandle);
    }
    else
    {
        Serial.println("Failed to create a queue!") create a queue!");
    }
}

void loop()
{
}
