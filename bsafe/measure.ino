const uint32_t vl_pin = 22;
const uint32_t vr_pin = 23;
const uint32_t width = 12;

void setup() {
    Serial.begin(115200);
    delay(2000);

    pinMode(vl_pin, INPUT);
    pinMode(vr_pin, INPUT);
    analogSetWidth(width);

    Serial.println("Initialized");
    Serial.println("Time (ms),Vl (V),Vr (V)");
}

double readV(uint32_t pin) {
    double raw = analogRead(pin);
    // scale from 0..4095 to 0..3.3
    // * 2 to account for voltage divider
    return raw * 3.3 / ((1 << width) - 1) * 2.0;
}

void loop() {
    Serial.printf("%ld,%f,%f\n", millis(), readV(vl_pin), readV(vr_pin));
}
