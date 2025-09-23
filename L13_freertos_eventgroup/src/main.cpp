#include <Arduino.h>
#include <event_groups.h>

// Define some event bits during washing machine washing
#define DOOR_CLOSED   (1 << 0)  // Indicates that the washing machine door is closed
#define WATER_READY   (1 << 1)  // Indicates whether the water level meets the conditions
#define POWER_ON      (1 << 2)  // Indicates that the power supply is turned on

// Create event group handle
EventGroupHandle_t xWashingEventGroup;

// Sensor task to detect whether the hatch door is closed
void vDoorSensorTask(void *pvParameters);
// Sensor task to detect whether the water level reaches a predetermined position
void vWaterSensorTask(void *pvParameters);
// Task to detect whether the power supply is turned on
void vPowerMonitorTask(void *pvParameters);
// The laundry task officially begins
void vWashingMachineTask(void *pvParameters);

void setup() {
  Serial.begin(9600);
  vTaskDelay(pdMS_TO_TICKS(3000));

  // Create event group
  xWashingEventGroup = xEventGroupCreate();

  // Create individual subtasks
  xTaskCreate(vDoorSensorTask, "Door", 1024, nullptr, 1, nullptr);
  xTaskCreate(vWaterSensorTask, "Water", 1024, nullptr, 1, nullptr);
  xTaskCreate(vPowerMonitorTask, "Power", 1024, nullptr, 1, nullptr);
  xTaskCreate(vWashingMachineTask, "Washing", 1024, nullptr, 2, nullptr);

  Serial.println("洗衣机已就绪，请准备洗衣...");
}

void loop() {
}

// Simulated door sensor detection task
void vDoorSensorTask(void *pvParameters) {
  while (true) {
    // Simulation gate state changes
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Whether the random simulation door is closed
    if (random(0, 2) == 1) {
      Serial.println("洗衣机门已关闭");
      xEventGroupSetBits(xWashingEventGroup, DOOR_CLOSED);
    } else {
      Serial.println("洗衣机门未关闭，请关闭门");
      xEventGroupClearBits(xWashingEventGroup, DOOR_CLOSED);
    }
  }
}

// Water level sensor task
void vWaterSensorTask(void *pvParameters) {
  while (true) {
    // Simulate water level changes
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Random simulation of whether the water level meets the requirements
    if (random(0, 2) == 1) {
      Serial.println("水位已达到要求");
      xEventGroupSetBits(xWashingEventGroup, WATER_READY);
    } else {
      Serial.println("水位不足，正在进水...");
      xEventGroupClearBits(xWashingEventGroup, WATER_READY);
    }
  }
}

// Power monitoring tasks
void vPowerMonitorTask(void *pvParameters) {
  while (true) {
    // Analog power supply state changes
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Whether the random analog power supply is turned on
    if (random(0, 2) == 1) {
      Serial.println("电源已接通");
      xEventGroupSetBits(xWashingEventGroup, POWER_ON);
    } else {
      Serial.println("电源未接通，请检查电源...");
      xEventGroupClearBits(xWashingEventGroup, POWER_ON);
    }
  }
}

// Washing Machine Task - Wait until all conditions are met to start work
void vWashingMachineTask(void *pvParameters) {
  while (true) {
    // Wait for all conditions to be met (door close, water level meets standards, power supply is turned on)
    constexpr EventBits_t waitBits = (DOOR_CLOSED | WATER_READY | POWER_ON);

    const EventBits_t uxBits = xEventGroupWaitBits(
      xWashingEventGroup, // Event group that needs to be listened to
      waitBits, // Which events need to be listened to, multiple events use bitwise or operation combinations
      pdFALSE, // After the condition is met, the waiting event bit is not cleared
      pdTRUE, // You need to wait until all conditions are met. If false, it means that any conditions are met.
      portMAX_DELAY // Unlimited waiting
    );

    // Check if all conditions are met
    if ((uxBits & waitBits) == waitBits) {
      Serial.println("=== 开始洗衣程序 ===");
      Serial.println("洗衣模式：标准");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("洗涤中...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("漂洗中...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("脱水中...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("洗衣完成！");
      Serial.println("===================");
    }
  }
}
