#include <Arduino.h>
#include "model.h"
#include "model_config.h"

// Instantiate the single, dynamically generated model
Eloquent::ML::Port::SenseWeaveModel ml_model;

void run_universal_benchmark()
{
    // The Python tool listens for this specific start token
    Serial.println("---HIL_BENCHMARK_START---");
    
    // 1. Measure Free RAM
    uint32_t free_ram = ESP.getFreeHeap();
    
    // 2. Cache Warmup
    float dummy_vec[NUM_FEATURES];
    for(int j=0; j<NUM_FEATURES; j++) dummy_vec[j] = senseweave::kTestVectors[0][j];
    ml_model.predict(dummy_vec); 
    
    // 3. True Average Latency (1000 iterations)
    const int iterations = 1000;
    uint32_t t_start = micros();
    
    for(int i = 0; i < iterations; i++)
    {
        for(int j=0; j<NUM_FEATURES; j++) dummy_vec[j] = senseweave::kTestVectors[i % NUM_TEST_VECTORS][j];
        volatile int discard = ml_model.predict(dummy_vec);
        (void)discard;
    }
    
    uint32_t t_end = micros();
    float avg_latency = (float)(t_end - t_start) / iterations;
    
    // 4. Output structured data for the Python parser
    Serial.printf("CANDIDATE:%s\n", MODEL_NAME);
    Serial.printf("FEATURES:%d\n", NUM_FEATURES);
    Serial.printf("RAM_BYTES:%lu\n", free_ram);
    Serial.printf("LATENCY_US:%.2f\n", avg_latency);
    
    // The Python tool listens for this end token
    Serial.println("---HIL_BENCHMARK_END---");
}

void setup()
{
    Serial.begin(115200);
    // Give the USB Serial a moment to stabilize
    delay(2000); 
    Serial.println("ESP32-S3 HIL Harness Ready. Waiting for trigger...");
}

void loop()
{
    // Listen for the 'B' trigger from Python
    if(Serial.available() > 0)
    {
        char cmd = Serial.read();
        if(cmd == 'B') 
        {
            run_universal_benchmark();
        }
    }
}