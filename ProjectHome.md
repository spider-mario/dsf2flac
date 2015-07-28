A file conversion tool for translating dsf and dff dsd audio files into
flac pcm audio files.

**Please note that google are phasing out the downloads tab in googlecode, please use the link on the left for the latest downloads.**

  * Now also reads and splits dsdff edited masters into separate tracks - click free!

  * Now also reads compressed dsdff files

  * Now also reads dsdff files

  * Optional flac-dop mode (packs unconverted raw DSD into flac files in DoP format)

I originally wrote this because I could not quite find any tool which
did exactly what I wanted.

I program quite a lot in java and I'm pretty good at matlab. Although I've
very occasionally written things in C and CPP, this is really my first attempt
at a full CPP program. I'm sure there are errors, hopefully none to horrible.

I've tried to write things in a pretty versatile/clear way. It should be possible
to expand the functionality to add other file types quite easily.

If you want to try different convesion filters then it should be dead simple:
just look in filters.cpp.