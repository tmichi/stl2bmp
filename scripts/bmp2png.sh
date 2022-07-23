#!/bin/bash
# Usage
# ./bmp2png.sh image_dir
for file in $(\find $1 -name '*.bmp' | sort); do
        convert ${file} ${file%.bmp}.png
        echo "\r ${file} --> ${file%.bmp}.png\c"
done
echo " Done."

