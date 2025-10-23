# Pico EPD Audio Player

**DAC Implementation is by the amazing [Elehobica](https://github.com/elehobica) [repo](https://github.com/elehobica/pico_audio_i2s_32b)**  
**WAV/MP3 Decoding is _currently_ done with [dr_libs](https://github.com/mackron/dr_libs) implementations**

This Project utilises a few custom file-formats to store and manage Audio-/Meta- and Font Data

Font-Data is saved in a binary file, containing a header where each 3 bytes of the Header with 0x00 to 0xFFFF elements (each 3 bytes long), covering the most important glyphs of UTF-8. 
Each Element corresponds to a in-file offset to get to the Bitmap for that Glyph.

The first byte of each Bitmap then encodes the width and height (lower nibble -> width, higher nibble -> height) and from then on it's a simple 1bpp bitmap

Audio, Image and Meta-Data is saved in .db files, which are binary databases which use the same header-lookup principle

_TO BE COMPLETED_

These Database Files need another Tool to properly encode them onto a Micro-SD Card, which will be uploaded later
