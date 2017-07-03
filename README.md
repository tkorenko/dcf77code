### About dcf77code

  **dcfcode** is a simple command-line utility that aims to produce (and decipher, too) strings of bits put together in accordance to [DCF77](https://en.wikipedia.org/wiki/DCF77) [Timecode](https://en.wikipedia.org/wiki/DCF77#Time_code_interpretation) specification.

### What for?

  There *does* exist hardware capable of making radiowaves using these strings
of bits as source of modulation, thus data created by this utility may be
fed into [Radio Controlled Clocks](https://en.wikipedia.org/wiki/Radio_clock).

### Features

- Produced timecode depends on localtime (not UTC), thus the [DST](https://en.wikipedia.org/wiki/Daylight_saving_time) flag of localtime drives selection of Z1/Z2 bits of timecode.

### Bugs

- There is no way to set 'leap second' flag of timecode.
- Summer time announcement (A1) prediction depends on the change of DST flag of localtime.

### Definitions

Timecode spans 60 bits.  Within this project the timecode is represented as 8-byte chunk of data, which also has a local name -- a *block*.

### Examples

By default, a timecode is created for current time:

    % dcfcode -c
    0000D2B86A2A5D00
    % date
    Tue Sep 26 15:46:35 EEST 2017

To decipher timecode contents, run:

    % dcfcode -d 0000D2B86A2A5D00
    0000D2B86A2A5D00 -> Tue Sep 26 15:46:00 2017 (MSD)

Note: DCF77 timecode is originally defined for [Central European Time](https://en.wikipedia.org/wiki/Central_European_Time) and carries no timezone information as such.

It's possible to define offset in minutes from current time (watch inner call of dcfcode):

    % dcfcode -d `dcfcode -c -s +3`
    000032B96A2A5D00 -> Tue Sep 26 15:49:00 2017 (MSD)

It's possible to produce multiple blocks (timecodes) at once:

    % dcfcode -d `dcfcode -c -n 4`
    0000D2B86A2A5D00 -> Tue Sep 26 15:46:00 2017 (MSD)
    0000F2A86A2A5D00 -> Tue Sep 26 15:47:00 2017 (MSD)
    000012A96A2A5D00 -> Tue Sep 26 15:48:00 2017 (MSD)
    000032B96A2A5D00 -> Tue Sep 26 15:49:00 2017 (MSD)

It's possible partially or fully redefine current timestamp (for instance: lets set hour and minute to 22:33 while leaving year, month, day of month at their defaults (at the moment of this writing)):

    % dcfcode -d `dcfcode -c -t 2233`
    00007246642A5D00 -> Tue Sep 26 22:33:00 2017 (MSD)

And it is possible to use a block as a timestamp:

    % dcfcode -c -t 0000D2B86A2A5D00 -s +1 -n 2
    0000F2A86A2A5D00
    000012A96A2A5D00
    % dcfcode -d 0000D2B86A2A5D00 0000F2A86A2A5D00 000012A96A2A5D00
    0000D2B86A2A5D00 -> Tue Sep 26 15:46:00 2017 (MSD)
    0000F2A86A2A5D00 -> Tue Sep 26 15:47:00 2017 (MSD)
    000012A96A2A5D00 -> Tue Sep 26 15:48:00 2017 (MSD)

Digging into timecode:

    % dcfcode -D 0000D2B86A2A5D00
    # 0000D2B86A2A5D00
                 0 :    0 : M       : Start of minute
    00000000000000 :    0 : weather : Weather info
                 0 :    0 : R       : Abnormal transmitter operation
                 0 :    0 : A1      : Summer time announcement
                 1 :    1 : Z1      : CEST in effect
                 0 :    0 : Z2      : CET in effect
                 0 :    0 : A2      : Leap second announcement
                 1 :    1 : S       : Start of encoded time
           0110001 :   46 : min     : Minutes 00-59
                 1 :    1 : P1      : Even parity over minute bits
            101010 :   15 : hour    : Hours 00-23
                 1 :    1 : P2      : Even parity over hour bits
            011001 :   26 : dom     : Day of month
               010 :    2 : dow     : Day of week (Mon=1, Sun=7)
             10010 :    9 : month   : Month number 01-12
          11101000 :   17 : year    : Year within century 00-99
                 0 :    0 : P3      : Parity over date bits
                 0 :    0 : -       : Minute Mark (no AM)

*There are two counter-intuitive things in the output shown above: 1. binary values in the first column have their least significant bit on the left; 2. don't try to convert multibit fields to decimal form directly, as they are BCD-encoded numbers and are easily interpreted when stated in hex form.* Lets read, for instance, day of month field:

    (lsb)011001 -{reversed}-> 100110(lsb) -{padded with zeros}-> 0010 0110 -{as hex}-> 26h.


### Encoding Details

Sixty bits of the DCF77 timecode are laid down into 7+0.5 bytes of the block (lsb first):

- Bit 0 of DCF77 timecode corresponds to (byte 0, bit 0) of the block;
- Bit 1 of DCF77 timecode -> (byte 0, bit 1) of the block;
- ...
- Bit 59 -> (byte 7, bit 3) of the block.
