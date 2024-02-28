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
	cd reflector && make all && cd ../transcoder && make all

.PHONY : clean
clean :
	cd transcoder && make clean && cd ../reflector && make clean

.PHONY : install
install :
	# Transcoder Install...
	cp -f transcoder/$(TRANSCODER) $(BINDIR)
	cp -f $(TRANSCODER).service /etc/systemd/system/
	systemctl enable $(TRANSCODER)
	systemctl daemon-reload
	systemctl start $(TRANSCODER)
	# Reflector Install...
	cp -f reflector/$(REFLECTOR) $(BINDIR)
	cp -f $(REFLECTOR).service /etc/systemd/system/
	systemctl enable $(REFLECTOR)
	systemctl daemon-reload
	systemctl start $(REFLECTOR)

.PHONY : uninstall
uninstall :
	# Uninstall Reflector...
	systemctl stop $(REFLECTOR)
	systemctl disable $(REFLECTOR)
	$(RM) /etc/systemd/system/$(REFLECTOR).service
	systemctl daemon-reload
	$(RM) $(BINDIR)/$(REFLECTOR)
	# Uninstall Transcoder...
	systemctl stop $(TRANSCODER)
	systemctl disable $(TRANSCODER)
	$(RM) /etc/systemd/system/$(TRANSCODER).service
	$(RM) $(BINDIR)/$(TRANSCODER)
	systemctl daemon-reload
