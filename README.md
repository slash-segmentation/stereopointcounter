[cmake]: http://www.cmake.org/
[itk]: http://www.itk.org/
[segmentation]: https://en.wikipedia.org/wiki/Image_segmentation
[csv]: https://en.wikipedia.org/wiki/Comma-separated_values
[png]: https://en.wikipedia.org/wiki/Portable_Network_Graphics
[stereology]: https://en.wikipedia.org/wiki/Stereology
# stereopointcounter

Performs automated point counting [stereology] on greyscale probability maps.

![example](images/cartoon.png)

A probability map is simply an image with pixel values of 0-255 where the higher the value the more likely a given pixel belongs to whatever feature being [segmented][segmentation].


Requirements
============

* [Cmake][cmake] >=2.8
* C++ >= (On linux g++ 4.4.7)
* [ITK][itk] >= 4.8 

To Build
========

    # download stereopointcounter
    wget https://github.com/slash-segmentation/stereopointcounter/archive/master.zip
    unzip master.zip
    mkdir stereo_build
    cd stereo_build
    cmake ../stereopointcounter-master
    make

Usage
=====
    stereopointcounter
    usage: stereopointcounter [options]

    Performs automated stereology point counting on probability map images passed in
    via --images path. 

    This tool looks for *.png files and assumes they are 8-bit greyscale images all
    with the same size.

    Output is to standard out and format is comma separated variables in the
    following format:

                    Image,GridSize,GridSizePixel,Positive,Total
                    /../foo.png,12x8,120x80,10,,67
                    ...
                    ...
                    Seconds,GrandTotalPositive,GrandTotal
                    123,29342,234292

    Options:
     --help, -h        Print usage and exit.
     --version, -v     Print version and exit.
     --images, -m      Can be set to a single greyscale 8-bit image or directory of
                       8-bit greyscale *.png images
     --gridx,          Grid size in X.  A value of say 4 means to generate
                       4vertical lines evenly spaced across the image.
     --gridy,          Grid size in Y.  A value of say 8 means to generate
                       8horizontal lines evenly spaced across the image.
     --threshold, -t   Threshold to for pixel intensity that denotes a given pixel
                       intersection is a positive hit (0 - 255)
     --saveimages, -s  If set to <dir>, writes out images as RGB with grid
                       overlayed in red and green circles denoting intersections
                       with matches to a file with format of
                       grid(--gridx)x(--gridy)_pixel(pixelw)x(pixelh)_thresh(-t).(o
                       rigname)

Example usage
=============

![ProbabilityMap](images/probmap.png)

Here is the output image from **stereopointcounter** with **--saveimages** flag turned
on:

    stereopointcounter --images probmap.png --gridx 52 --gridy 50 --threshold 200 --saveimages .

**Output to standard out**

This is the output from running the above command.  **--images** can be given a single
**[png][png]** file or a directory containing a set of **[png]** files.

    Image,GridSize,GridSizePixel,Positive,Total
    probmap.png,52x50,23x16,111,2548

    Seconds,GrandTotalPositive,GrandTotal
    0.22234,111,2548

The above output is in [csv] format and breaks down number of intersections where
the pixel value at the intersection meets or exceeds the threshold set by **--threshold**
flag.  At the very end of the output is a summary denoting totals for all **[png]** images
analyzed.

**Image written to directory specified by --saveimages**

Each intersection is examined and those intersections above the **threshold** which
in the example case is **200** is circled in green and counted in the stats
displayed above.

![ProbabilityMapResult](images/grid52x50_pixel23x16_thresh200.probmap.png)

    
Copyright
=========

Copyright 2015 The Regents of the University of California All Rights Reserved

Permission to copy, modify and distribute any part of this stereopointcounter for educational, research and non-profit purposes, without fee, and without a written agreement is hereby granted, provided that the above copyright notice, this paragraph and the following three paragraphs appear in all copies.

Those desiring to incorporate this stereopointcounter into commercial products or use for commercial purposes should contact the Technology Transfer Office, University of California, San Diego, 9500 Gilman Drive, Mail Code 0910, La Jolla, CA 92093-0910, Ph: (858) 534-5815, FAX: (858) 534-7345, E-MAIL:invent@ucsd.edu.

IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS stereopointcounter, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE stereopointcounter PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.  THE UNIVERSITY OF CALIFORNIA MAKES NO REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE OF THE stereopointcounter WILL NOT INFRINGE ANY PATENT, TRADEMARK OR OTHER RIGHTS.
