# names for execs and object libraries
REFLECTOR = urfd
TRANSCODER = tcd
INICHECK = inicheck
DBUTIL = dbutil

# this is where the binary will be installed
BINDIR = /usr/local/bin

# besides making an executable that gdb can use,
# this will also provide some additional log messsage
debug = false

# Passed to the -j option of make for compiling on a multi-threaded processor
JOBS = 1

# To disable DHT support, set DHT to false
DHT = true
