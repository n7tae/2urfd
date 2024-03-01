# Universal, Another Multi-protocol Digital Voice Reflector

The URF Multi-protocol Gateway Reflector Server, ***urfd***, is part of the software system for a Digital Voice Network. The sources are published under GPL Licenses.

## Introduction

This will build a new kind of digital voice reflector. A *urfd* supports DStar protocols (DCS, DExtra) DMR protocols (MMDVMHost), M17, YSF, and P25 (Phase 1, using IMBE). A key part of this is the hybrid transcoder, *tcd*, is in this repository. You can't interlink urfd with xlxd. This reflector can be built without a transcoder, but clients will only hear other clients using the same codec. Please note that currently, urfd only supports the tcd transcoder when run locally. As a local device, urfd and tcd uses UNIX DGRAM sockets for inter-process communications. These kernel-base sockets are significantly faster than conventional UDP/IP sockets. It should be noted that tcd supports DVSI-3003 nad DVSI-3000 devices, which it uses for AMBE vocoding.

This build support *dual-stack* operation, so the server on which it's running, must have both an IPv4 and IPv6 routeable address if you are going to configure a dual-stack reflector.

Only systemd-based operating systems are supported. Debian or Ubuntu is recommended. If you want to install this on a non-systemd based OS, you are on your own. Finally, this repository is designed so that you don't have to modify any file in the repository when you build your system. Any file you need to modify to properly configure your reflector will be a file you copy from you locally cloned repo. This makes it easier to update the source code when this repository is updated. Follow the instructions below to build your transcoding URF reflector.

## Usage

The packages which are described in this document are designed to install server software which is used for the D-Star network infrastructure. It requires a 24/7 internet connection which can support up to three transcoded modules and up to 23 more untranscoded modules to connect repeaters and hot-spot dongles.

- The server can build a reflector that support IPv4, and also IPv6 (dual stack).
- The public IP addresses should have a DNS record which must be published in the common host files.

## Installation

Below are instructions to build a URF reflector. If you are planning on an URF reflector without a transcoder, you can help your users by naming modules with names that suggest which protocol is welcome. You name modules in the config.inc.php file mentioned below.

The transcoder is in a separate repository, but you can install and monitor the transcoder and reflector from a script, *radmin* in this repository. You *should* look over the README.md file in the tcd repository to understand the transcoder.

### After a clean installation of Debian make sure to run update and upgrade

```bash
sudo apt update
sudo apt upgrade
```

### Required packages (some of these may already be installed)

```bash
sudo apt install git apache2 php build-essential nlohmann-json3-dev libcurl4-gnutls-dev
```

### Ham-DHT support (optional, but highly recommended)

**Ham-DHT**, a DHT network for hams, is implemented using a distributed hash table provided by OpenDHT.

OpenDHT is available [here](https://github./com/savoirfairelinux/opendht.git). Building and installing instructions are in the [OpenDHT Wiki](https://github.com/savoirfairelinux/opendht/wiki/Build-the-library). Pascal support and proxy-server support (RESTinio) is not required for urfd and so can be considered optional. With this in mind, this should work on Debian/Ubuntu-based systems:

```bash
# Install OpenDHT dependencies
sudo apt install libncurses5-dev libreadline-dev nettle-dev libgnutls28-dev libargon2-0-dev libmsgpack-dev  libssl-dev libfmt-dev libjsoncpp-dev libhttp-parser-dev libasio-dev cmake pkg-config libcppunit-dev

# clone the repo
git clone https://github.com/savoirfairelinux/opendht.git

# build and install
cd opendht
mkdir build && cd build
cmake -DOPENDHT_PYTHON=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

Please note that there is no easy way to uninstall OpenDHT once it's been installed.

### Download and build the repository and enter the build directory.

```bash
git clone https://github.com/n7tae/2urfd.git
cd urfd
```

Except for the OpenDHT library, this repository contains everything needed to build both *urfd* and *tcd*!

### Create and edit your configuration files

Create your working copies of the compile-time and run-time configuration files. Some of the these working files you can move most anywhere, more on that below.

```bash
cp ../config/* .
```

This will create eight files:
1. The `urfd.mk` file contains compile-time options for *urfd*. If you change the `BINDIR`, you'll need to update how the two service files start *urfd* and *tcd*. After you've edited this file build everything by typing `make`.
2. The `urfd.ini` file contains the run-time options for *urfd* and will be discussed below.
3. The `urfd.blacklist` file defines callsigns that are blocked from linking or transmitting.
4. The `urfd.whitelist` file defines callsigns that are allowed to link and transmit. Both of these files support the asterisk as a wild-card. The supplied blacklist and whitelist file are empty, which will allow any callsign to link and transmit, blocking no one. Both files support a limited wildcard feature.
5. The `urfd.interlink` file defines URF linking.
6. The `urfd.service` file is a systemd file that will start and stop *urfd*. Importantly, it contains the only reference to where the *urfd* ini file is located. Be sure to set a fully qualified path to your `urfd.ini` file on the `ExecStart` line.
7. The `tcd.ini` file contains run-time options for *tcd*. Make sure the `Transcoded` line is identical for both `tcd.ini` and `urfd.ini`.
8. The `tcd.service` file is a systemd file that will start and stop *tcd*. Importantly, it contains the only reference to where the *tcd* ini file is located. Be sure to set a fully qualified path to your `tcd.ini` file on the `ExecStart` line.

You can actually put the blacklist, whitelist, interlink and the two ini files anyplace and even rename them because ini file locations are only referenced in the two service files and the blacklist, whitelist and interlink files are only referenced in the `urfd.ini` file. The service files and the mk files need to remain in your `urfd` directory.

### Configuring your reflector

Use your favorite text editor to set your run-time configuration in your copy of `urfd.ini` and `tcd.ini`.

There are only a few things that need to be specified. Most important are, the reflector callsign and the IP addresses for the IPv4 and IPv6 listen ports and and identifying the transcoded modules. Dual-stack operation is enabled by specifying both an IPv4 and IPv6 address. IPv4-only single stack can be specified by leaving the IPv6 address undefined. Note that some protocols don't yet support IPv6.

You can configure any modules, from **A** to **Z**. Up to three modules can be transcoded if you have the necessary hardware.

There are three databases needed by *urfd*:
1. The *DMR ID* database maps a DMR ID to a callsign and *vis versa*.
3. The *YSF Tx/Rx* database maps a callsign to a transmit/receive RF frequencies.
These databases can come from a URL or a file, or both. If you specify "both", then the file will be read after the URL. Using "both" is what you want if you need to supply some custom values for your setup, but still want the latest values from the web.

The files section specifies specific locations of important runtime configurations. The DHTSavePath is important for allowing *urfd* to quickly and reliably connect to the **Ham-DHT** network. The first time you boot up *urfd* it will make its initial connection to the network through any single, already operating node that you specify got the **\[Names\] Bootstrap** item. As urfd operates over time, it will establish connections to several other operating nodes that are "close" (as defined by some criteria). This is a fundamental characteristic of a DHT network. When you shutdown *urfd*, it will save the current connection state that your reflector has developed using the file path you specified in DHTSavePath. The next time you boot up *urfd*, if this file exists, it will be read and used to quickly connect your reflector to the network.

### Helper apps

There are two, very useful helper applications, *inicheck* and *dbutil*. Both apps will show you a usage message if you execute them without any arguments.

The *inicheck* app will use the exact same code that urfd uses to validate your `urfd.ini` file. Do `reflector/inicheck -q urefd.ini` to check your infile for errors. If you see any messages containing `ERROR`, that means that *urfd* won't start. You'll have to fix the errors described in the message(s). If you only see messages containing `WARNING`, *urfd* will start, but it may not perform as expected. You will have to decide if the warning should be fixed. If you don't see any messages, it means that *urfd* and *tcd* will start without any configuration problems. The **one exception** is that bad things will happen if you don't have identical **Transcoded** lines in the two ini files.

The *dbutil* app can be used for several tasks relating to the three databases that *urfd* uses. The usage is: `./dbutil DATABASE SOURCE ACTION INIFILE`, where:
- DATABASE is "dmr" or "ysf"
- SOURCE is "html" or "file"
- ACTION is "parse" or "errors"
- INIFILE is the path to the infile that defines the location of the http and file sources for these three databases.
One at a time, *dbutil* can work with either of the two DATABASEs. It can read either the http or the file SOURCE. It can either show you the data entries that are syntactically correct or incorrect (ACTION). Using the "parse" ACTION, you can create a file that can be subsequently read by urfd:

```bash
reflector/dbutil dmr html parse urfd.ini > /home/user/urfd/dmrid.dat
```

This can save some time during startup. You can then offload the task of periodically updating these to files with other Linux tools, like crontab.

### Installing your system

After you have written your configuration files, you can install your system:

```bash
sudo make install
```

and uninstall your system:

```bash
sudo make uninstall
```

There is also an interactive script you can launch with `./radmin` you can use to install and uninstall your system, restarting the reflector or transcoder process, or viewing the reflector or transcoder log in real time.


### Copy dashboard to /var/www

Since URF is a superset of XLX, we can still take advantage of the existing XLX infrastructure. In fact, the xml file generated by urfd reports itself as an XLX reflector. This will change at some point in time.

```bash
sudo cp -r ~/urfd/dashboard /var/www/urf     # or whatever your html server uses
```

Please note that your www root directory might be some place else. There is one file that needs configuration. Edit the copied files, not the ones from the repository:

- **pgs/config.inc.php** - At a minimum set your email address, country and comment.

**DO NOT** enable the "calling home" feature unless you are sure that you will not be infringing on an existing XLX or XRF reflector with the same callsign suffix. If you don't understand what this means, don't set `$CallingHome['Active']` to true!

## Firewall settings

URF Server requires the following ports to be open and forwarded properly for in- and outgoing network traffic:

```text
TCP port    80         (http) optional TCP port 443 (https)
UDP port 10017         (URF interlinking)
UDP port 17000         (M17 protocol)
UPD port 20001         (DPlus protocol)
UDP port 30001         (DExtra protocol)
UDP port 30051         (DCS protocol)
UDP port 40000         (DSD Protocol)
UDP port 41000         (P25 port)
UDP port 42000         (YSF protocol)
UDP port 62030         (MMDVM protocol)
```

## YSF Master Server

Pay attention, the URF Server acts as an YSF Master, which provides 26 wires-x rooms.
It has nothing to do with the regular YSFReflector network, hence you don’t need to register your URF at ysfreflector.de!

A new dashboard is on the to-do list!

## Copyright

- Copyright © 2016 Jean-Luc Deltombe LX3JL and Luc Engelmann LX1IQ
- Copyright © 2022 Doug McLain AD8DP
- Copyright © 2024 Thomas A. Early N7TAE
