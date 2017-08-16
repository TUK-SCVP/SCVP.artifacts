#!/usr/bin/sh
#Install systemc on a ubuntu like linux
wget http://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.1a.tar.gz
tar xfv systemc-2.3.1a.tar.gz
cd systemc-2.3.1a
./configure --prefix=/opt/systemc/
make -j 4
sudo make install
