# Introduction #

A common issue with dsd -> pcm conversion is the occurrence of click at the start and ends of the tracks. These clicks can sometimes be very loud and not just annoying but also potentially damaging of equipment. Why do these occur? How can they be prevented?


# A Simple Example #

The image below is taken from Audacity and shows a DSD64 track which has been converted to 88.2kHz pcm. I've zoomed right in so that the click at the start can be seen.

The highest point of the click occurs at 9 samples into the pcm data. In this particular case I converted the track using a FIR filter with 575 taps (575 samples long). This filter operates on the raw DSD data at a sampling rate of 2282.4kHz. The FIR filter is a linear phase type, this means that all frequencies pass through the filter with their relative timings unchanged. However, the filter does change the absolute timing: the output is delayed by half of the length of the filter (289 DSD samples). The output sample rate is 88.2kHz which is 32 times slower than the DSD raw data rate. So 289 samples at DSD rate is equivalent to 9.032 output samples, hence the 9 sample delay before the click.


![https://dsf2flac.googlecode.com/svn/wiki/clicks/example1.png](https://dsf2flac.googlecode.com/svn/wiki/clicks/example1.png)

With an 575 tap FIR filter you MUST have 575 samples read into a buffer before you can calculate your first output sample. Thereafter you can just keep pushing more samples in the buffer and dropping one out the other end.  Remembering that we get a 289 sample delay in absolute time when applying the filter, this means that the first output sample you can calculate from the input data is the 289th (in DSD samples). So, are we happy to loose 288 DSD samples from the start of the data? Perhaps we are, after all that is only 0.1 millisecond.

However, for arguments sake, let's say we are not happy to loose the first 0.1 millisecond. So what do we do? Basically we make up some extra data and put it into the buffer before we start to filter. Then we can put a real sample in the front and calculate the first output sample using 574 made up samples and the first sample from our data. Great, that works!

This is the method used in the example above which gives the click.

What made up sample data should we put into the buffer? A pretty sensible choice is to fill the buffer with zeros. The reason that this is sensible is that we are effectively making up part of the audio signal and if we use zeros then the made up part has no energy so it will not cause any irregularity in the output.

You can probably guess the rest now. The issue with DSD is that there is no way to represent zero. The DSD signal is always switching between 0 and 1. There is always energy in the signal.

Given that we can't fill the buffer with zeros, the obvious choice is to use a sequence of samples which represent silence. In DSD speak these are called idle tones. For convenience DSD is often represented and stored 8 samples at a time in a byte of data. Here are a few examples of 8 sample idle tones (also given in hex):

  1. 0xAA: 10101010
  1. 0x55: 01010101
  1. 0xCA: 11001010
  1. 0x69: 01101001

I could go on, there are loads of them! Note that each contains four 1's and four 0's. In DSD all of these are silent. However, all of them have a lot of energy in the signal. The energy is at high frequencies out of the audio band. However, if you suddenly change from one silent DSD signal to another silent DSD signal then at the transition then energy will not be confined to the inaudible part of the frequency spectrum. It is exactly the same effect that occurs if you cut short a pcm audio signal - you will hear a click. The thing that makes DSD strange is that you can't hear the signal before the click (because it is above the audio bandwidth).

To demonstrate this I made a special DSD file with 8000 samples using the idle tone 0x69 and then another 8000 using the idle tone 0xCA. I then ran this DSD through dsf2pcm and in Audacity I got the waveform below. Note that the signal is zero before and after the click. This DSD signal would never occur in reailty, the noise shapers in the DSD creation process are designed to avoid these types of problem.

![https://dsf2flac.googlecode.com/svn/wiki/clicks/example2.png](https://dsf2flac.googlecode.com/svn/wiki/clicks/example2.png)

# Conclusions #

The bottom line is that you can't just splice together two DSD signals and not expect to get a click. Note that although I have only discussed the problem at the start of the data the same occurs at the end.


In terms of dsd -> pcm conversion there are a few options:

1) Truncate the start and end of the file to remove the clicks

0.1ms is not much to loose and as a lot of tracks have some silence at the start and end this will probably be good enough for most things.

2) Convert the whole disc at once

SACD discs are continuous DSD streams, if you convert the whole disc at once then you only have to worry about the clicks at the start and the end of the disc where silence is pretty likely. For discs where track boundaries occur in the middle of the music this might be the best option as truncation is likely to result in clicks just due to the discontinuity between tracks.

3) Apply a short fade and fade out.

A simple solution which might work for both situations above.


That is all, hopefully interesting reading.
Jack.


# dsf2flac #

[Revision 37](https://code.google.com/p/dsf2flac/source/detail?r=37) applies method 1 above.
If you have a DSDIFF edit master than you should be able to do disc at once ( method 2 above) although please note that I've not tested edit masters yet plus you'll end up with one large flac file with all the track in it.
I'll have a look into method 3 because that seems a better default than method 1.
Long term I'll get edit masters working and splitting tracks. Plus I'll try to allow the user to specify a preceding and following track from the command line and use the data from these to fill the buffer.




