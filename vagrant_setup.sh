#!/bin/bash

VAGRANT_HOME='/home/vagrant'
VAGRANT_SETUP_FILE="$VAGRANT_HOME/.asl_vagrant_setup_complete"
VAGRANT_S3_FILE_URI="https://s3-us-west-2.amazonaws.com/asl-firmware/vagrant_setup"
ASL_PROFILE="/etc/profile.d/asl.sh"

set -e
[ -n "$DEBUG" ] && set -x

if [ -e "$VAGRANT_SETUP_FILE" ]; then
    echo "Setup already complete" >&2
    exit 0
fi

# exec this script as a vagrant user if run as root.
if [ "$EUID" = 0 ]; then
    echo "Dropping privileges to vagrant user" >&2
    exec su vagrant -c "$0"
fi

get_vagrant_file() {
    for name in $@; do
        echo "Fetching $name" >&2
	     curl -s "$VAGRANT_S3_FILE_URI/$name"
    done
}

# Install the needed packages
sudo apt-get update
sudo apt-get -y install \
	  emacs24-nox \
	  clang \
	  gcc \
	  libc++6 \
	  libc++-dev \
	  libcppunit-1.13-0 \
	  libcppunit-dev \
	  python-pip \
	  git \
	  gcc-arm-none-eabi \
	  python-crcmod \
	  minicom \
	  gzip \
	  zip \

# Install Toolchains for MK1 and MK2
unset asl_bins
unset asl_bin_join
for f in gcc-arm-none-eabi-4_7-2013q1.tgz gnuarm-4.0.2.tgz; do
    get_vagrant_file $f | tar xz
    f_name="${f%.tgz}"
    asl_bins="${asl_bins}${asl_bin_join}$VAGRANT_HOME/${f_name}/bin"
    asl_bin_join=':'
done

# Install ihexpy first as asl-f4-loader
cd
echo "Installing ihexpy..." >&2
[ -d ihexpy ] && rm -rf ihexpy
git clone https://github.com/Jeff-Ciesielski/ihexpy.git ihexpy
cd ihexpy
sudo python setup.py install

# Install the ASL bootloader needed for MK2
cd
echo "Installing asl_f4_loader..." >&2
get_vagrant_file 'asl_f4_loader-0.0.6.tgz' | tar xz
cd asl_f4_loader-0.0.6
sudo python setup.py install

# Setup our path in system profile
tmp_file="$(mktemp)"
cat >"$tmp_file" <<EOF
# ASL script to setup our bin path.
export PATH="${asl_bins}:\$PATH"

EOF
sudo mv "$tmp_file" "$ASL_PROFILE"
sudo chmod 755 "$ASL_PROFILE"

# Do this to avoid redundant work
touch "$VAGRANT_SETUP_FILE"
