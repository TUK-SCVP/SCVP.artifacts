#!/usr/bin/env sh
#Install systemc on a ubuntu like linux
wget http://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
tar xfv systemc-2.3.3.tar.gz
cd systemc-2.3.3
./configure --prefix=/opt/systemc/
make -j4
sudo make install
