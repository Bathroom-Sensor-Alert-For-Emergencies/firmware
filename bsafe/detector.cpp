#include "detector.hpp"
#include "moving_average.hpp"
#include <Arduino.h>

// MovingAverage object
const int windowSize = 10;
MovingAverage energyMovingAverage(windowSize);
// "Report Mode" Total 45 bytes per packet
const byte REPORT_HEADER[4] = { 0xF4, 0xF3, 0xF2, 0xF1 };
const byte REPORT_TAIL[4] = { 0xF8, 0xF7, 0xF6, 0xF5 };

// Read the 41 bytes AFTER the header
const int PACKET_PAYLOAD_LENGTH = 41;

// ALARM THRESHOLD (NEEDS TUNING!)
// Note: A person is present, but their energy is as low as an empty room
const long UNCONSCIOUS_THRESHOLD = 1200;
// Time before sending alert
const unsigned long UNCONSCIOUS_TIME = 5 * 1000;
const int MAX_DISTANCE_CM = 300;

static int parserState = 0;  // 0-3 = Header, 4 = Reading Message/Payload
static int dataCounter = 0;
// Buffer to hold the 41 bytes *after* the header
static byte payloadBuffer[PACKET_PAYLOAD_LENGTH];

static unsigned long last_conscious_time = 0;

static bool isTargetDetected = false;

Detector::Detector(std::uint32_t otz, std::uint32_t tx, std::uint32_t rx)
    : serial{1}, detected{false}, otz{otz}, tx{tx}, rx{rx} {}

bool Detector::begin() {
    pinMode(this->otz, INPUT);
    this->serial.begin(115200, SERIAL_8N1, this->rx, this->tx);
    if (!this->serial) {
        Serial.println("Error initializing serial");
        return false;
    }

    // Send the "REPORT MODE" command
    // Wiki: FDFCFBFA 0800 1200 0000 0400 0000 04030201
    String hex_to_send = "FDFCFBFA0800120000000400000004030201";

    Serial.println("Sending 'Report Mode' command over this->serial...");
    this->sendHexData(hex_to_send);
    Serial.println("Report command sent.");
    Serial.println("Waiting for 'Report Mode' packets...");

    return true;
}

void Detector::sendHexData(String hexString) {
    int hexStringLength = hexString.length();
    if (hexStringLength % 2 != 0) {
        Serial.println("Error: Hex string must have an even number of characters.");
        return;
    }
    int byteCount = hexStringLength / 2;
    byte hexBytes[byteCount];
    for (int i = 0; i < hexStringLength; i += 2) {
        String byteString = hexString.substring(i, i + 2);
        byte hexByte = (byte)strtoul(byteString.c_str(), NULL, 16);
        hexBytes[i / 2] = hexByte;
    }
    Serial.print("Sending ");
    Serial.print(byteCount);
    Serial.print(" bytes: ");
    for (int i = 0; i < byteCount; i++) {
        if (hexBytes[i] < 16) Serial.print("0");
        Serial.print(hexBytes[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    this->serial.write(hexBytes, byteCount);
}

void Detector::update() {
    // TODO: Placeholder
    if (random(1000000) == 0) this->detected = true;
    return;

    // Serial.println("Updating");
    while (this->serial.available() > 0) {
        byte incomingByte = this->serial.read();

        // --- DEBUG PRINT ---
        // Serial.print("Byte: ");
        // if (incomingByte < 16) Serial.print("0");
        // Serial.print(incomingByte, HEX);
        // Serial.print(" | State: ");
        // Serial.println(parserState);
        // --- END DEBUG ---

        if (parserState < 4) {
            //STATE 0-3: LOOKING FOR HEADER
            if (incomingByte == REPORT_HEADER[parserState]) {
                parserState++;  // advance state if matched
            } else {
                parserState = 0;  // Mismatch, reset (if neg)
                if (incomingByte == REPORT_HEADER[0]) {
                    parserState = 1;
                }
            }
        } else if (parserState == 4) {
            // --> STATE 4: READING PACKET PAYLOAD (41 bytes)
            payloadBuffer[dataCounter] = incomingByte;
            dataCounter++;

            if (dataCounter == PACKET_PAYLOAD_LENGTH) {
                // all 41 payload bytes are read

                // Check the TAIL
                // (Indices 37, 38, 39, 40)
                if (payloadBuffer[37] == REPORT_TAIL[0] && payloadBuffer[38] == REPORT_TAIL[1] && payloadBuffer[39] == REPORT_TAIL[2] && payloadBuffer[40] == REPORT_TAIL[3]) {

                    // Have a valid packet

                    // 1. PARSE PRESENCE
                    // Full Packet Index: 6. Payload Index: 2
                    byte presenceByte = payloadBuffer[2];
                    bool isPresent_Packet = (presenceByte == 0x01);

                    // 2. PARSE DISTANCE
                    // Full Packet Indices: 7, 8. Payload Indices: 3, 4
                    // Little-endian: low byte first, so reversed
                    int distance = (int)payloadBuffer[4] << 8 | payloadBuffer[3]; // shift the high byte forward
                    // int distance = (x | payloadBuffer[3]) - 0x01;  // merge to the real original distance, may need to tweak

                    // 3. PARSE ENERGY
                    // Full Packet Indices: 9-40. Payload Indices: 5-36 (32 bytes total)
                    int singleEnergy = 0;
                    for (int i = 5; i <= 36; i++) {
                        singleEnergy += payloadBuffer[i];
                    }

                    // starting fill the buffer
                    energyMovingAverage.add((int)singleEnergy);

                    int smoothEnergy = energyMovingAverage.get();

                    // 4. READ 0TZ PIN to verify
                    bool isPresent_0TZ = (digitalRead(this->otz) == HIGH);

                    // distance check, if fail --> turn off th LED and break out of if
                    if (distance > MAX_DISTANCE_CM) {
                        bool isPresent_Packet = 0;  // if detect motion farther than 1 m, no detection
                        // need assign a energy floor
                    }

                    // 4.5. Final range check
                    if (isPresent_Packet && (distance <= MAX_DISTANCE_CM)) {
                        // only if person is present AND within 3 meter
                        if (!isTargetDetected) last_conscious_time = millis();
                        isTargetDetected = true;
                    } else {
                        isTargetDetected = false;
                    }

                    if (isTargetDetected && smoothEnergy >= UNCONSCIOUS_THRESHOLD) {
                        last_conscious_time = millis();
                        Serial.printf("Person responsive at %ld with energy %d\n", last_conscious_time, smoothEnergy);
                    } else {
                        Serial.printf("It has been %ld since last movement\n", millis() - last_conscious_time);
                    }

                    if (isTargetDetected && millis() >= (last_conscious_time + UNCONSCIOUS_TIME)) {
                        // Serial.println("EMERGENCY! Person unresponsive for two minutes!");
                        // while (true) {}
                        // // why the fuck does this work??? ofc it does
                        // for (;;);
                        this->detected = true;
                    }

                    // 5. PRINT EVERYTHING
                    // time, distance, movement energy, presence packet, presence otz pin, detected
                    // Serial.printf("%ld,%d,%d,%d,%d,%d,%d\n", millis(), distance, singleEnergy, smoothEnergy, isPresent_Packet, isPresent_0TZ, isTargetDetected);
                    // Serial.println("----------------------------------------------------");
                    // Serial.println("+++ VALID PACKET RECEIVED +++");
                    //
                    // Serial.print("  Distance: ");
                    // Serial.print(distance);
                    // Serial.println(" cm");
                    //
                    // Serial.print("  Total Movement Energy: ");
                    // Serial.println(totalEnergy);
                    //
                    // Serial.print("  Presence (from Packet): ");
                    // Serial.println(isPresent_Packet ? "PRESENT (1)" : "Absent (0)");
                    //
                    // Serial.print("  Presence (from 0TZ Pin): ");
                    // Serial.println(isPresent_0TZ ? "PRESENT (HIGH)" : "Absent (LOW)");
                    //
                    // Serial.print("  FINAL DETECTION (Present & <= 1m):  ");
                    // Serial.println(isTargetDetected ? "TARGET DETECTED" : "No Target / Out of Range");

                    // 6. FINAL ALARM LOGIC (need testing)
                    // if (isPresent_0TZ && (smoothEnergy < UNCONSCIOUS_THRESHOLD)) {
                    //   Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                    //   Serial.println("!!! ALERT: Person Present but NOT MOVING !!!");
                    //   Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                    // }
                    // Serial.println("----------------------------------------------------");


                } else {
                    // Tail mismatch, packet was corrupt
                    // Serial.println("Packet tail mismatch!");
                }

                // Reset for the next packet
                parserState = 0;
                dataCounter = 0;
            }
        }
    }
}

bool Detector::isUnresponsive() {
    return this->detected;
}
