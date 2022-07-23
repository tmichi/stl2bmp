#!/bin/bash
# Usage
# ./bmp2png.sh image_dir
if [ $# -ne 1 ];
then
        echo "Usage : %0 image_dir"
else
        for file in $(\find $1 -name '*.bmp' | sort); do
                magick convert ${file} ${file%.bmp}.png
                echo "\r ${file} --> ${file%.bmp}.png \c"
        done
fi
echo "Done."
