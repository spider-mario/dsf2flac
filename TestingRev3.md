## Stage 1. Create a test signal ##

I created a 176.4kHz 32bit wav file in Matlab with a 1kHz tone at -3dBFs. I added 1/(2^24) peak amplitude of tpdf dither (i.e. equiv to the significand precision).

The fft spectrum of 1 sec of the data is shown below. Note that this results in a 1Hz bin spacing so the 1kHz tone falls into a single bin. This was computed in Matlab from the wav file. It looks pretty good, no harmonics are visible and the dither/quantization noise is hovering at around -190dB.

![https://dsf2flac.googlecode.com/svn/wiki/testingRev3/wavTestTone.png](https://dsf2flac.googlecode.com/svn/wiki/testingRev3/wavTestTone.png)

## Stage 2. Convert to DSD64 (DSF) ##

The only tool I have to convert to DSD is Audiogate. This was the reason for choosing 32bit 176.4kHz wav as Audiogate cannot read anything higher.

Prior to writing dsf2pcm in cpp, I first wrote a dsf reader in Matlab before them discovering that was going to be way too slow for useful conversion. However, it has turned out to be pretty useful for analysing dsd data and designing decimation filters. I read the converted dsd data back into Matlab and computed the fft spectrum of the data (all done at 64\*44.1kHz and with double precision).

The 1bit quantization noise is pretty clear and you can see that it starts rising pretty severely just above 20kHz. The noise floor below 20kHz is worse than the 32bit tone. More concerning are the three peaks that have appeared (200Hz, 3000Hz and 5000Hz). These seem to be distortion products of the 1000Hz test tone (possibly with the exception of the 200Hz tone). Why do these appear... I don't know. Is this an error in how the Audiogate conversion process works? Is this because the 1bit data is not fully dithered? Let me know if you have an idea of what is causing these.

![https://dsf2flac.googlecode.com/svn/wiki/testingRev3/dsdTestTone.png](https://dsf2flac.googlecode.com/svn/wiki/testingRev3/dsdTestTone.png)

## Stage 3. Convert to PCM with dsf2pcm ##

I've now used dsf2pcm ([revision 4](https://code.google.com/p/dsf2flac/source/detail?r=4)) to convert back to pcm from the dsd64 dsf test file. I repeated the conversion with a few different settings just to see the difference.

### 88200Hz 24bit 6dB gain dither on ###

command: "dsf2flac -i 1kHztone\_dsd64.dsf -[r88200](https://code.google.com/p/dsf2flac/source/detail?r=88200) -b24 -s6"

Noise floor is very slightly higher than dsd. The harmonic products seen in the dsd signal are present here too at the same levels. Everything seems to be working pretty well.

![https://dsf2flac.googlecode.com/svn/wiki/testingRev3/converted88200Hz24bit.png](https://dsf2flac.googlecode.com/svn/wiki/testingRev3/converted88200Hz24bit.png)

### 176400Hz 24bit 6dB gain dither on ###

command: "dsf2flac -i 1kHztone\_dsd64.dsf -[r176400](https://code.google.com/p/dsf2flac/source/detail?r=176400) -b24 -s6"

This one is a bit of a surprise, as it is clearly a bit worse than the result with 88200Hz sampling rate. I suspect the problem is that the anti-aliasing filter used for 176500 does not have quite enough stop band rejection to prevent the dsd ultrasonic noise from leaching back into the audio spectrum. I'll update this filter in the future and see if things improve!

Incidentally this does highlight that there may be significant differences between dsd->pcm converters depending on the filters they use. I chose to use filters with very good stop band rejection (around -180dB) but I've seen quite a few bits of code using filters with much less (approx -140dB from memory).

![https://dsf2flac.googlecode.com/svn/wiki/testingRev3/converted176400Hz24bit.png](https://dsf2flac.googlecode.com/svn/wiki/testingRev3/converted176400Hz24bit.png)

### 88200Hz 24bit 6dB gain dither off ###

command: "dsf2flac -i 1kHztone\_dsd64.dsf -[r88200](https://code.google.com/p/dsf2flac/source/detail?r=88200) -b24 -s6 -n"

Does the dither have any effect at 24bit or is the DSD noise greater than the 1/(2^23) amplitude tpdf dither? Well given the result below looks just like the version with dither (above) then it seems like the DSD noise is greater.

![https://dsf2flac.googlecode.com/svn/wiki/testingRev3/converted88200Hz24bitnd.png](https://dsf2flac.googlecode.com/svn/wiki/testingRev3/converted88200Hz24bitnd.png)

## Conclusions ##

The conversion seems to work well. With the 88200Hz filter there seems to be virtually no loss in the noise floor compared to dsd. The result for 176400Hz indicates that there may be a slight issue with the filter which needs to be looked at. I'll generate a new filter and see if that improves the situation.

If I have the chance I'll do a similar test with multitones to see what happens to the intermodulation products.