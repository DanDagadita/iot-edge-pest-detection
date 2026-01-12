# IoT Edge Pest Detection

## Setting up

The code expects a generic ESP32 board, connected via USB cable to the computer the flashing will be done from. In order to build and upload the code, I used the PlatformIO extension for VSCode.

To run the code, the ESP32 board needs to be powered, and its D32 pin connected to the DO pin of the KY-038 microphone sound sensor. The ground and + pins of the KY-038 can be connected to the ESP32 board, to GND and 3V3 respectively. The sensor needs its potentiometer tuned so that the KY-038's LED2 is off, and when talking into it or detecting sound, it turns on.

LCD1602 I2C Display: connect GND and VCC to GND and 3V3 respectively, SDA to D21, and SCL to D22.

## Save audio data to csv

In order to save audio as a csv in order to train a model on it, you first need to connect the ESP32 via an USB port, upload the code, then run this command in a terminal: `tio --log --log-file output.csv --baud 921600 --log-strip /dev/ttyUSB0`. After that, you can start audio playback and it will automatically be written to `output.csv`.

## Prerequisites for training

This project requires a machine with an Nvidia GPU and its drivers, Docker Compose, and the Nvidia Container Toolkit. It has been tested only on an Arch Linux machine, for more information about Google Colab local runtimes, click [here](https://research.google.com/colaboratory/local-runtimes.html).

## Starting training

**1.** In a terminal, clone this repository: `git clone https://github.com/DanDagadita/iot-pest-detection.git`

**2.** Enter the cloned repository's folder, then run `docker compose up -d`, which starts the container containing the Google Colab Jupyter environment.

**3.** Upon opening this project in your text editor/IDE of choice, and opening the `notebook.ipynb` file, you have the choice to connect to the local runtime using `http://127.0.0.1:9000`. I used VSCode, using the suggested Jupyter and Python extensions.

## Benchmarks

* 0.5ms -> optimized serial logging
* 0.8ms -> custom serial logging
* 5ms -> publish to MQTT
* 62ms -> print to LCD
