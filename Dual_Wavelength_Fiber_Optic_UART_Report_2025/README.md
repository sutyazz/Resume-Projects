
# Dual-Wavelength Fiber Optic UART (2025)

## Project Overview
This project demonstrates a novel full-duplex UART communication system over a single fiber optic cable, utilizing dual-wavelength modulation with blue and infrared LEDs. Custom analog front-end circuits, including transimpedance amplifiers (TIA) and comparators, enable reliable signal decoding and separation of simultaneous bidirectional data streams.

## Skills Demonstrated
- **Optical Communication:** Fiber optic transmission and reception using wavelength multiplexing
- **Analog Circuit Design:** Custom TIA, comparator, and filter design for photodetector signal processing
- **Digital Communication:** UART protocol implementation and bit error rate (BER) analysis
- **Signal Processing:** Optimization of signal integrity, gain, and noise performance
- **Hardware Design:** LED driver circuits, phototransistor receivers, PCB layout

## Key Features
- Full-duplex UART over a single fiber using blue and IR LEDs
- Custom analog front-end for high-speed, low-error communication
- Real-time BER measurement and performance optimization
- Modular design for easy integration with microcontrollers and embedded systems

## Technical Details
- Blue and IR LEDs transmit independent UART signals simultaneously
- Phototransistor receivers with TIA and comparator circuits decode each wavelength
- System tested for data rates, BER, and signal integrity under various conditions
- Includes schematics, PCB layouts, and waveform analysis

## Repository Contents
- Schematic files and PCB designs (KiCad)
- Test data, waveform images, and performance analysis
- Python scripts for data acquisition and BER calculation
- Documentation and project proposal

## Results
Achieved robust full-duplex UART communication with low BER and high signal integrity. The system demonstrates practical fiber optic multiplexing for embedded applications and provides a foundation for further research in optical data links.
