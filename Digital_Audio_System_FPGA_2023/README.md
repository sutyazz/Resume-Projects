# Digital Audio System FPGA (2023)

## Project Overview
Designed a multi-feature digital audio synthesizer in Verilog on FPGA, implementing chord synthesis, ADSR modulation, echo effects, and waveform visualization.

## Skills Demonstrated
- **Hardware Description Languages**: Verilog programming
- **FPGA Development**: Digital logic design and implementation
- **Digital Signal Processing**: Audio synthesis, filtering, effects processing
- **Audio Engineering**: Chord synthesis, ADSR envelope generation
- **Real-time Systems**: Real-time audio processing and visualization
- **Hardware Design**: FPGA constraint management and optimization

## Key Features
- Multi-feature digital audio synthesizer
- Chord synthesis capabilities
- ADSR (Attack, Decay, Sustain, Release) modulation
- Real-time echo effects processing
- Waveform visualization and display
- FPGA-optimized digital signal processing

## Technical Implementation
- Designed digital oscillators and waveform generators in Verilog
- Implemented ADSR envelope generation for dynamic amplitude control
- Created echo and delay effects using circular buffer techniques
- Developed real-time waveform visualization system
- Optimized for FPGA resource utilization and timing constraints

## Audio Processing Features
- **Synthesis Engine**: Multiple waveform generation (sine, square, sawtooth, triangle)
- **Chord Processing**: Simultaneous multi-note synthesis
- **Envelope Shaping**: Configurable ADSR parameters
- **Effects Processing**: Digital echo and reverberation
- **Visualization**: Real-time waveform and spectrum display

## Architecture Components
- **Top-level Module**: `lab5_top.v` - Main system integration
- **Audio Core**: `music_player.v` - Primary audio synthesis engine
- **Note Processing**: `note_player_with_echo.v` - Individual note synthesis with effects
- **Waveform Generation**: `sine_reader.v`, `sine_rom.v` - Digital oscillators
- **Effects Processing**: `dynamics.v` - ADSR and dynamics processing
- **Memory Systems**: `frequency_rom.v`, `song_rom.v` - Audio data storage
- **Visualization**: `wave_display.v`, `smooth_wave_display.v` - Real-time waveform display
- **Control Interface**: `button_press_unit.v`, `mcu.v` - User interaction
- **Signal Conditioning**: `codec_conditioner.v` - Audio I/O interface

## File Structure
- `final_project/` - Complete Verilog source code
  - `lab5_top.v` - Top-level system module
  - `music_player.v` - Audio synthesis engine
  - `note_player_with_echo.v` - Note synthesis with echo effects
  - `dynamics.v` - ADSR envelope generation
  - `sine_reader.v` / `sine_rom.v` - Waveform generation
  - `wave_display.v` - Waveform visualization
  - `song_reader.v` / `song_rom.v` - Song data management
  - `beat_generator.v` - Timing and rhythm control
  - `ram_1w2r.v` - Dual-port memory for audio buffering
- `rom_generator.xls` - ROM data generation utilities
- Testbenches: `dynamics_tb.v`, `test_smooth_dots.v`

## Technology Stack
- **Hardware Description Language**: Verilog HDL
- **Platform**: FPGA development board
- **Tools**: FPGA synthesis and timing analysis tools
- **Audio Processing**: Digital signal processing algorithms
- **Memory**: On-chip RAM and ROM for audio data storage

## Results
Successfully created a fully functional digital audio synthesizer on FPGA with professional-quality audio synthesis, real-time effects processing, and interactive waveform visualization capabilities. The system demonstrates advanced FPGA design skills with optimized resource utilization and real-time performance.
