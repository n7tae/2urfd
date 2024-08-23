# Copyright (c) 2021 by Thomas A. Early N7TAE
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# locations for the executibles and other files are set here
# NOTE: IF YOU CHANGE THESE, YOU WILL NEED TO UPDATE THE service.* FILES AND
# if you change these locations, make sure the sgs.service file is updated!
# you will also break hard coded paths in the dashboard file, index.php.

include urfd.mk

.PHONY : all
all :
	cd reflector && make all -j$(JOBS) && cd ../transcoder && make all -j$(JOBS)

.PHONY : clean
clean :
	cd transcoder && make clean && cd ../reflector && make clean

.PHONY : install
install :
	# Transcoder Install...
	cp -f transcoder/tcd $(BINDIR)
	cp -f tcd.service /etc/systemd/system/
	systemctl enable tcd
	systemctl daemon-reload
	systemctl start tcd
	# Reflector Install...
	cp -f reflector/urfd $(BINDIR)
	cp -f urfd.service /etc/systemd/system/
	systemctl enable urfd
	systemctl daemon-reload
	systemctl start urfd

.PHONY : uninstall
uninstall :
	# Uninstall Reflector...
	systemctl stop urfd
	systemctl disable urfd
	$(RM) /etc/systemd/system/urfd.service
	systemctl daemon-reload
	$(RM) $(BINDIR)/urfd
	# Uninstall Transcoder...
	systemctl stop tcd
	systemctl disable tcd
	$(RM) /etc/systemd/system/tcd.service
	$(RM) $(BINDIR)/tcd
	systemctl daemon-reload
