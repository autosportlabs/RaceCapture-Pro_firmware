#!/bin/bash

VAGRANT_S3_FILE_URI="https://s3-us-west-2.amazonaws.com/asl-firmware/vagrant_setup"

get_vagrant_file() {
    for name in $@; do
	curl "$VAGRANT_S3_FILE_URI/$name"
    done
}

set -e
[ -n "$DEBUG" ] && set -x

# Install ihexpy first as asl-f4-loader
git clone https://github.com/Jeff-Ciesielski/ihexpy.git ihexpy
cd ihexpy
sudo python setup.py install
cd ..
# Install the ASL bootloader needed for MK2

get_vagrant_file 'asl_f4_loader-0.0.6.tgz' | tar xz
cd asl_f4_loader-0.0.6
sudo python setup.py install
cd ..
