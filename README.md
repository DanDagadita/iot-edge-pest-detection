# IoT Edge Pest Detection

## Setting up

The code expects a generic ESP32 board, connected via USB cable to the computer the flashing will be done from. To run the code, the ESP32 board needs to be powered, and its D32 pin connected to the DO pin of the KY-038 microphone sound sensor. The ground and + pins of the KY-038 can be connected to the ESP32 board, to GND and 3V3 respectively. The sensor needs its potentiometer tuned so that the KY-038's LED2 is off, and when talking into it or detecting sound, it turns on.

## Save audio data to csv

In order to save audio as a csv in order to train a model on it, you first need to connect the ESP32 via an USB port, upload the code, then run this command in a terminal: `tio --log --log-file output.csv --log-strip /dev/ttyUSB0`. After that, you can start audio playback and it will automatically be written to `output.csv`.
