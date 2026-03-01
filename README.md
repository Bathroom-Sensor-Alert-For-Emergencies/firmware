# BSAFE
**Bathroom Safety Alert for Emergencies**

This is our Rose-Hulman senior design project. Our client tasked us with developing a system for detecting when someone becomes unresponsive while using a bathroom.
It will be installed at our client's facility, a healthcare center in Indianapolis.


## Design

Our design consists of two devices: a sensor device mounted in the single-stall bathrooms and a receiver device that the staff carry.
The sensor device uses a millimeter wave radar to detect motion and sends an alarm once it detects no movement for two minutes.
The receiver device is equipped with an LCD screen and a speaker to display alerts.
We use LoRa radios to communicate between devices wirelessly.
To ensure that our system is fault tolerant, receiver devices maintain a list of paired sensors.
All sensors periodically send out special heartbeat packets, and if a receiver does not receive one from a sensor for long enough, it will display an alert.


## Installation

To upload, install [Nix](https://nixos.org/) and run the following command in the project's directory:

```bash
# Upload the sensor device's firmware
nix run .#sensor
# Upload the receiver device's firmware
nix run .#receiver
```

By default, it will be uploaded to `/dev/ttyUSB0`. To change this, replace the value with the desired one in `flake.nix`.
