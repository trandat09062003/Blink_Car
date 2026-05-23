// Test inference for DriverSafetyTinyML with float model
#include <Arduino.h>
#include <Driver_Safety_TinyML.h>

DriverSafetyTinyML ai;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println(F("[Test] Starting inference test..."));
  if (!ai.begin()) {
    Serial.println(F("[Test] Failed to initialize AI model"));
    return;
  }
  // Generate dummy data: steady values
  for (int i = 0; i < 100; i++) {
    ai.addSample(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  }
  if (ai.isReady()) {
    float prob = ai.predictSwerving();
    Serial.print(F("[Test] Swerving probability: "));
    Serial.println(prob, 4);
  } else {
    Serial.println(F("[Test] Not enough samples"));
  }
}

void loop() {
  // nothing
}
