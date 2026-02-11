#include "lcd.hpp"

LCD::LCD(int rs, int en, int d4, int d5, int d6, int d7)
    : alarms{}, lcd{rs, en, d4, d5, d6, d7} {}

void LCD::begin() {
    this->lcd.begin(20, 2);
    this->lcd.clear();
}

void LCD::clear() {
    this->lcd.clear();
}

void LCD::pair() {
    this->lcd.clear();
    this->lcd.setCursor(0, 1);
    this->lcd.print("Pairing...");
}

void LCD::emplace(AlarmType type, DeviceID id) {
    this->enqueue(Alarm{type, id});
}

void LCD::enqueue(Alarm alarm){
    this->alarms.push_back(alarm);

    this->lcd.setCursor(0, 1);
    std::size_t size = this->alarms.size();
    if (size > 1) {
        this->lcd.print(size - 1);
        this->lcd.print(" pending alarms");
    } else {
        // Just print first line if there's one or less alarm
        this->lcd.clear();
        this->lcd.setCursor(0, 0);
        this->lcd.print(alarm.name());
        this->lcd.print(" room ");
        this->lcd.print(alarm.id);
    }
}


void LCD::dequeue() {
    this->alarms.pop_front();
    std::size_t size = this->alarms.size();
    if (size > 0) {
        Alarm alarm = this->alarms.front();
        this->lcd.clear();
        this->lcd.setCursor(0, 0);
        this->lcd.print(alarm.name());
        this->lcd.print(" room ");
        this->lcd.print(alarm.id);
        this->lcd.setCursor(0, 1);
        this->lcd.print(size);
        this->lcd.print(" pending alarms");
    } else {
        this->lcd.clear();
    }
}
