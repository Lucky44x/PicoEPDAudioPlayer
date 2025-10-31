# PICO-EPD-WAV-PLAYER

**This Project is heavily inspired by [Elehobica's Pico-WAV-Player](https://github.com/elehobica/RPi_Pico_WAV_Player) and borrows many design- and technical aspects from it**
_(Or in other words: Why reinvent the Wheel when it is already there? ʕ •ᴥ•ʔ )_ 

### Libraries used
- [Elehobica's](https://github.com/elehobica) [pico_audio_i2s_32b](https://github.com/elehobica/pico_audio_i2s_32b)
- [Elehobica's](https://github.com/elehobica) [pcio_fatfs implementation](https://github.com/elehobica/pico_fatfs)
- [WaveShare's](https://github.com/waveshareteam) [e-Paper Library](https://github.com/waveshareteam/e-Paper) _(Heavily modified to suit my needs and naming-convention)_

This is the second attempt at writing this Project...
The first can be seen on the branch _1.0-incomplete_ and implements a full EPD-Drawing and Custom Database-File reading system, however, due to the realization, that the I2S DAC I use, will need to run on the pico's second core, I decided to rebuild the project, this time focusing on better documentation, planning and code-quality as well as sticking to my code-convention.  
  
The current Plan is as follows:
- Core0 Will serve as the File-Reader and UI Core
- Core1 Will serve as an exclusive Audio Core

- Core0 Will read the Audio-File from the SD-Card and write the read PCM-Frames into a lock-free ring-buffer in memory
- Core1 Will read those frames from the ring-buffer in shared memory
- Core1 Will feed the DAC via a PIO programm and hardware interrupt, to facillitate the high frequencies needed for wav playback  
  
This does solve the stalling issue I have experienced on the 1st. implementation, however it comes with one drawback:  
  
Due to the nature of E-Ink-Displays/EPDs the library I am using and modified to fit my needs, will block execution until a command to the EPD is done. This means screen refreshes (which can take up to a second) **will** halt all other code on that core.  
Since I do not want the Audio-Playback to halt, I will need the UI-Calls to come from Core0, but if these happened during playback, the file-reader would still be halted...  
  
**The Solution** to this problem is relatively simple: Before Audio-Playback we intialize the Screen with the image and Meta-Data of the playing Audio-File and from then on, we disallow any and all interaction with the UI.
The UI will only be _unlocked_ once the playback of a Song has finished and core0 can resume it's normal operations.  
  
In my opinion, this is, while annoying, a necessary tradeoff, to keep the code-base consice and understandable, as **The Alternative** would be to write the EPD-Driver non-blocking which would involve a Queue or Buffer of commands, which then execute in sequence, but this in turn, could lead to timing issues, or in a **worst-case** scenario even to a _command-pileup_ where new commands to the EPD pile up in the buffer, faster than the Hardware can consume them  
  
Thus I decided on the former Implementation