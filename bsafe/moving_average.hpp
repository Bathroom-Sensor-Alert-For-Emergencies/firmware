/*
  Define the moving average class to average out the energy level erading
*/

class MovingAverage {
private:
    int* _readings;      // Array to store readings
    int _numReadings;    // The window size
    int _count;           // count
    int _index;
    long _total;         // Running total
    int _average;

public:
    MovingAverage(int windowSize = 10) {
        _numReadings = windowSize;
        _readings = new int[_numReadings];
        _index = 0;
        _total = 0;
        _average = 0;
        _count = 0;

        // Initialize all readings to 0
        for (int i = 0; i < _numReadings; i++) {
            _readings[i] = 0;
        }
    }

    // Add a new sensor reading to the filter
    void add(int newReading) {
        _total = _total - _readings[_index];   // Subtract the tail reading, slide the window
        _readings[_index] = newReading;
        _total = _total + _readings[_index];
        _index = _index + 1;                  // slide the tail

        // Wrap around
        if (_index >= _numReadings) {
            _index = 0;
        }

        if (_count< _numReadings) {
            _count++;
        }

        _average = _total /_count;
    }

    int get() {
        return _average;
    }

    int getCount(){
        return _count;
    }

    int isFilled() {
        return _count == _numReadings;
    }
};
