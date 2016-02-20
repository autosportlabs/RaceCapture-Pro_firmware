# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure(2) do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://atlas.hashicorp.com/search.
  config.vm.box = 'ubuntu/14.04-amd64'
  config.vm.box_url = "https://cloud-images.ubuntu.com/vagrant/trusty/current/trusty-server-cloudimg-amd64-vagrant-disk1.box"

  # Provider-specific configuration so you can fine-tune various
  # backing providers for Vagrant. These expose provider-specific options.
  config.vm.provider :virtualbox do |vb|
    # Box name info
    vb.name = 'ASL Firmware Builder'

    host = RbConfig::CONFIG['host_os']
    mem = 512
    if host =~ /darwin/
      cpus = `sysctl -n hw.ncpu`.to_i
    elsif host =~ /linux/
      cpus = `nproc`.to_i
    else # sorry Windows folks, I can't help you
      cpus = 2
    end

    # Needed for multi-core support on Vagrant machines.
    vb.customize ["modifyvm", :id, "--ioapic", "on"]
    vb.customize ["modifyvm", :id, "--memory", mem]
    vb.customize ["modifyvm", :id, "--cpus", cpus]

    # Enable USB.  Needed for programming RCP units
    vb.customize ["modifyvm", :id, "--usb", "on"]
    # Disable EHCI becasue it requires Enterprise version.
    vb.customize ["modifyvm", :id, "--usbehci", "off"]

    # Attempt to auto attach RCP USB devices to VM
    vb.customize ["usbfilter", "add", "0", "--target", :id,
                  "--name", "Capture ASL Devices", "--action", "hold",
                  "--active", "no", "--vendorid", "16d0"]
  end

  # Enable provisioning with a shell script. Additional provisioners such as
  # Puppet, Chef, Ansible, Salt, and Docker are also available. Please see the
  # documentation for more information about their specific syntax and use.
  config.vm.provision "shell", inline: <<-SHELL
     /vagrant/bin/vagrant_setup.sh
  SHELL
end
