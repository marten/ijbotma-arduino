.PHONY: all
all: sounddata.h

markonaboat.wav: markonaboat.m4a
	ffmpeg -i $< $@

markonaboat_downsampled.wav: markonaboat.wav
	sox $< -c 1 -b 8 -r 8000 $@

# _shortened created from _downsampled by trimming in Audacity.
sounddata.h: captain_obvious.wav
	python2 audio2huff.py --sndfile=$< --hdrfile=$@ --bits=8
