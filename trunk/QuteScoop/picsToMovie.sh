#!/bin/sh
echo Using mencoder, you can put pics together to a video stream. Here are some standard options:
echo mencoder mf://*.png -mf type=png:w=1280:h=720:fps=8 -ovc lavc -lavcopts vcodec=mpeg4 -oac copy -o foo.avi
