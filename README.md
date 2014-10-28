CPUVISOR-SRV backend service for Visor (CPU version)
====================================================

Author: Ken Chatfield, University of Oxford (ken@robots.ox.ac.uk)

Copyright 2014, all rights reserved.

Release: v0.1 (October 2014)

Installation Instructions
-------------------------
The CMAKE package is used for installation. Ensure you have all dependencies
installed from the section below, and then:

    $ mkdir build
    $ cd build
    $ cmake ../
    $ make
    $ make install

The resultant binaries will be installed into the `bin/` subdirectory of the
root folder.

Usage
-----

 1. Edit `./config.prototxt` with Caffe model files and dataset base paths
 2. Edit `./dsetpaths.txt` and `./negpaths.txt` with the paths to all dataset and
    negative training images (by default the paths in `dsetpaths.txt` contain all
    images from the PASCAL VOC 2007 dataset)
 3. Run `./cpuvisor_preproc` to precompute all features

Now the backend service can be run using:

    $ ./cpuvisor_service

An example of how to use the service is provided by the `./test_client.py`
script. To use it, the `imsearch-tools` submodule is required. Get it by issuing:

    $ git submodule init
    $ git submodule update

Once the backend service is up and running, you can then test it using:

    $ python test_client.py

Dependencies
------------
The following packages are required:

 + [Caffe](https://github.com/kencoken/caffe) -- use the `dev` branch of the
   forked version of the repo `kencoken/caffe`
 + [cppnetlib](https://github.com/kencoken/cpp-netlib) -- use the `0.11-devel`
   branch of the forked version of the repo `kencoken/cpp-netlib`
 + Boost 1.55.0+
 + Liblinear
 + OpenCV
 + Google Logging (GLOG)
 + Google Flags (GFLAGS)
 + Google Protobuf (C++ and Python libraries)
 + ZeroMQ (C++ and Python libraries)
