#include "lcd.hpp"

LCD::LCD(int rs, int en, int d4, int d5, int d6, int d7)
    : alarms{nullptr}, lcd{rs, en, d4, d5, d6, d7} {}

void LCD::begin(std::deque<Alarm>* alarms) {
    this->alarms = alarms;
    this->lcd.begin(20, 2);
    this->lcd.clear();
}

void LCD::clear() {
    this->lcd.clear();
}

void LCD::pair() {
    this->lcd.clear();
    this->lcd.setCursor(0, 0);
    this->lcd.print("Pairing...");
}

void LCD::printId(DeviceID id) {
    this->lcd.print(ID_STRING(id));
}

void LCD::pairedId(DeviceID id) {
    this->lcd.print(" ");
    this->printId(id);
}

void LCD::emplace(AlarmType type, DeviceID id) {
    this->enqueue(Alarm{type, id});
}

void LCD::enqueue(Alarm alarm){
    this->alarms->push_back(alarm);

    std::size_t size = this->alarms->size();
    if (size > 1) {
        // If there's already an alarm, just update the pending count display
        this->lcd.setCursor(0, 1);
        this->lcd.print(size - 1);
        this->lcd.print(" pending alarms");
    } else {
        // Just print first line if there's one or less alarm
        this->lcd.clear();
        this->lcd.setCursor(0, 0);
        this->lcd.print(alarm.name());
        this->lcd.print(" room ");
        this->printId(alarm.id);
    }
}


Alarm LCD::dequeue() {
    Alarm top = this->alarms->front();
    this->alarms->pop_front();
    Alarm next = this->alarms->front();

    std::size_t size = this->alarms->size();
    if (size > 0) {
        this->lcd.clear();
        this->lcd.setCursor(0, 0);
        this->lcd.print(next.name());
        this->lcd.print(" room ");
        this->printId(next.id);
        this->lcd.setCursor(0, 1);
        this->lcd.print(size);
        this->lcd.print(" pending alarms");
    } else {
        this->lcd.clear();
    }

    return top;
}
