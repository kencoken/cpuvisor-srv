CPUVISOR-SRV Backend Service for Visor
======================================

Author: Ken Chatfield, University of Oxford (ken@robots.ox.ac.uk)

Copyright 2014-2015, all rights reserved.

Release: v0.2.1 (February 2015)
License: MIT (see LICENSE.md)

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

The following preprocessing steps are required before running the service:

 1. Edit `./config.prototxt` with Caffe model files and dataset base paths
 2. Edit `./dsetpaths.txt` and `./negpaths.txt` with the paths to all dataset and
    negative training images (by default the paths in `dsetpaths.txt` contain all
    images from the PASCAL VOC 2007 dataset)
 3. Run `./cpuvisor_preproc` to precompute all features

For convenience, a script is provided to complete steps 1-2 for a sample dataset
(PASCAL VOC2007), including the downloading of dataset images and Caffe model
files. This can be run as follows:

    $ python download_data.py

Now the cpuvisor service can be launched from the `/bin` directory:

    $ ./cpuvisor_service

Connecting to the Service
-------------------------

Ensure that all demo script dependencies have been installed as described in the section
below. Following this, a simple example of how to use the service from code can be run
by first starting the CPUVISOR Service as described above, and then issuing:

    $ python test_client.py

A more complete demo including a web frontend is provided in the `webservice/` subdirectory.
Refer to the README file there for further details.

Dependencies
------------

#### 1. CPUVISOR Service Dependencies

The following C++ libraries are required:

 + [Caffe](https://github.com/kencoken/caffe) – use the `dev` branch of the
   forked version of the repo `kencoken/caffe`
 + [cppnetlib](https://github.com/kencoken/cpp-netlib) – use the `0.11-devel`
   branch of the forked version of the repo `kencoken/cpp-netlib`
 + Boost v1.55.0+
 + Liblinear
 + OpenCV
 + Google Logging (GLOG)
 + Google Flags (GFLAGS) v2.1+
 + Google Protobuf
 + ZeroMQ

#### 2. Demo Script Dependencies

All dependencies for the Python demo scripts can be installed by issuing the following
command from the root directory:

    $ pip install -r requirements.txt

The demo scripts also require the `imsearch-tools` submodule to have been initialized:

    $ git submodule init
    $ git submodule update

Utilities
---------

The following utilities are provided in the `utils/` subdirectory:

  * `cpuvisor_service_forever.sh` – launches the CPUVISOR service, restarting
      automatically if it ends unexpectedly, and places a log of all errors and crashes
      in the `logs/` subdirectory
  * `generate_imagelist.py` – scans a directory tree for images and saves their relative
      paths to a text file – useful for generating dataset path indexes for use with
      CPUVISOR

Incremental Indexing
--------------------

It is possible to add images to the test dataset index in a live manner (without stopping
CPUVISOR serving requests). To do this, first ensure the CPUVISOR service is running:

    $ ./cpuvisor_service

And then use the `cpuvisor_add_dset_images` to add new dataset images on the fly:

    $ ./cpuvisor_add_dset_images --paths=/PATH/TO/IMAGE/LIST.txt

Where the `paths` parameter specifies the location of a text file which contains the paths
of the dataset images to add to the index. Note that these paths should be relative to
the dataset root directory (specified in `config.prototxt` as
*preproc_config->dataset_im_base_path*) and should be contained within this directory.

Note also that whilst it is possible to continue using the CPUVISOR service whilst images
are added to the index, the processing of queries is likely to be slower until the process
has completed.

Version History
---------------

- **v0.1** – *October 2014* – Initial release
- **v0.2** – *January 2015* – Added webserver demo
- **v0.2.1** – *February 2015* – Added incremental indexing
