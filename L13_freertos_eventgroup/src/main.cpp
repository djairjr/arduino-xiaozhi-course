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

  Serial.println("The washing machine is ready, please prepare the laundry...")ne is ready, please prepare the laundry...");
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
      Serial.println("Washing machine door closed")achine door closed");
      xEventGroupSetBits(xWashingEventGroup, DOOR_CLOSED);
    } else {
      Serial.println("The washing machine door is not closed, please close the door")chine door is not closed, please close the door");
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
      Serial.println("The water level has reached the requirements") level has reached the requirements");
      xEventGroupSetBits(xWashingEventGroup, WATER_READY);
    } else {
      Serial.println("The water level is insufficient, water is inflowing...")l is insufficient, water is inflowing...");
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
      Serial.println("The power supply is turned on")er supply is turned on");
      xEventGroupSetBits(xWashingEventGroup, POWER_ON);
    } else {
      Serial.println("The power supply is not turned on, please check the power supply...") is not turned on, please check the power supply...");
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
      Serial.println("=== Start the laundry program ===")undry program ===");
      Serial.println("Laundry mode: Standard")ode: Standard");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Washing...")..");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Rinse...")");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Dehydration...")ion...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      Serial.println("Laundry is finished!") is finished!");
      Serial.println("===================");
    }
  }
}
