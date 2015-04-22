#!/usr/bin/env bash

apt-get update
apt-get -y upgrade

# get ourselves some basic toys
apt-get -y install git-core build-essential openssl libssl-dev 

# install some tools for the vagrant user only by executing
# a script AS the vagrant user
cp /vagrant/install-tools.sh /home/vagrant
chown vagrant /home/vagrant/install-tools.sh
chgrp vagrant /home/vagrant/install-tools.sh
chmod ug+x /home/vagrant/install-tools.sh
su -c "/home/vagrant/install-tools.sh" vagrant

