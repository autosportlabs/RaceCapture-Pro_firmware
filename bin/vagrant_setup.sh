#!/bin/bash

VAGRANT_HOME='/home/vagrant'
ASL_PROFILE="/etc/profile.d/asl.sh"

setup_profile_file()
{
    # Setup our path in system profile
    tmp_file="$(mktemp)"
    dest="/etc/profile.d/${1}"
    cat >"$tmp_file"
    sudo mv "$tmp_file" "$dest"
    sudo chmod 755 "$dest"
}

set -e
[ -n "$DEBUG" ] && set -x

# exec this script as a vagrant user if run as root.
if [ "$EUID" = 0 ]; then
    echo "Dropping privileges to vagrant user" >&2
    exec su vagrant -c "$0"
fi

# Install the needed packages
sudo apt-get -y update
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
     python-crcmod \
     minicom \
     gzip \
     zip \


# Install Toolchain for MK2
echo "Installing MK2 toolchain (takes a while)..." >&2
cd
rm -rf gcc-arm-none-eabi-4_9-2015q3
curl -sL https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2 | tar -xj

cat <<EOF | setup_profile_file "asl_mk2_tc.sh"
# ASL script to setup our bin path for MK2
export PATH="${VAGRANT_HOME}/gcc-arm-none-eabi-4_9-2015q3/bin:\$PATH"

EOF


# Install ihexpy first as asl-f4-loader
echo "Installing ihexpy..." >&2
cd
rm -rf ihexpy
[ -d ihexpy ] && rm -rf ihexpy
git clone https://github.com/Jeff-Ciesielski/ihexpy.git ihexpy
cd ihexpy
sudo python setup.py install


# Install the ASL bootloader needed for MK2
echo "Installing asl_f4_loader..." >&2
cd
rm -rf asl_f4_loader
git clone https://github.com/autosportlabs/ASL_F4_bootloader.git asl_f4_loader
cd asl_f4_loader
sudo python setup.py install

cd
exit 0
