# midi_pulse
<img src="https://raw.githubusercontent.com/jaredellison/midi_pulse/master/Prototype.png" alt="Module Prototype" width="400px"/>

The Midi Pulse Eurorack Module is a utility module which takes a midi input and outputs 8 channels of velocity 
sensitive pulses. It was originally built to trigger analog drum synthesizer modules, emulating the function of a 
piezo drum pad's trigger signal. However, these pulses are likely to have other applications in combination with
different modules.

Midi notes are selected in the module's firmware to be mapped to one of 8 output channels. The Midi note's velocity value is used 
to change the amplitude of an output pulse roughly between 0V and 10V peak to peak.

A Texas Instruments DAC5578 is used to create the pulses, and an Arduino compatible Adafruit Metromini microcontroller was used 
for the project. Please see the datasheets linked below.

This source code and hardware described here comprise a functional yet -very- barebones design. If you build something
based on this design, please reach out!
   
## Key Parts

* [Adafruit Metromini](https://www.adafruit.com/products/2590) - Perf & Breadboard friendly microcontroller
* [TI DAC5578](www.ti.com/lit/pdf/sbas496) - 8 Bit Octal DAC 

## Tools Used

* [Diptrace](https://diptrace.com/) - Schematic and PCB design software
* [Arduino](https://www.arduino.cc/) - Microcontroller and Software environment

## Authors

* **Jared Ellison** - [jaredellison.net](http://jaredellison.net)

## Acknowledgments And Sources

* **Adafruit** - *MetroMini Arduino Style Microcontroller.* - [Adafruit](https://www.adafruit.com/)
* **CTRL Mod** - *Synthesizer Goods and Eurorack Panels Available in NYC.* - [Control](https://www.ctrl-mod.com/)
* **Akizukidenshi** - *Sweet, reasonably priced perfboard. Excellent selection. Akihabara, Tokyo.* - [Akizukidenshi](http://akizukidenshi.com/catalog/)
