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
    "宫保鸡丁", "鱼香肉丝", "麻婆豆腐", "糖醋排骨", "回锅肉"
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

        Serial.print("顾客 #");
        Serial.print(newOrder.customerId);
        Serial.print(" 点了: ");
        Serial.print(newOrder.dishName);
        Serial.print(", VIP: ");
        Serial.println(newOrder.isVip ? "是" : "否");

        // Decide the location of the queue based on whether the VIP is used.
        if (newOrder.isVip)
        {
            xQueueSendToFront(orderQueue, &newOrder, portMAX_DELAY);
            Serial.println("VIP订单已优先处理");
        }
        else
        {
            xQueueSendToBack(orderQueue, &newOrder, portMAX_DELAY);
            Serial.println("普通订单已加入队列");
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
            Serial.print("厨师开始制作: ");
            Serial.print(currentOrder.dishName);
            Serial.print(" 给顾客 #");
            Serial.println(currentOrder.customerId);

            // Simulated cooking time (2-4 seconds)
            vTaskDelay(pdMS_TO_TICKS(random(2000, 4000)));

            Serial.print("厨师完成了顾客 #");
            Serial.print(currentOrder.customerId);
            Serial.print(" 点的: ");
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
        Serial.println("创建队列失败!");
    }
}

void loop()
{
}
