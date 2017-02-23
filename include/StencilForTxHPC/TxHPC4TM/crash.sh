#!/bin/bash
#
# This is a script file for applications built using the TxHPC framwework,
# for shared access to Gen-Z-style byte addressable shared memory across nodes
# (as in The Machine).
# See: http://http://genzconsortium.org/
#
#    [HPE copyright notice]
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#    As an exception, the copyright holders of this Library grant you permission to
#   (i) compile an #Application with the Library, and
#    (ii) distribute the Application containing code generated by the Library and    
#    added to the Application during this compilation process under terms of your
#    choice, provided you also meet the terms and conditions of the Application license.

./r6_emu 1048576 8 32 2 16 1 1 1024 0 > results/log_before_crash &
for i in `seq 1 10`;
do
	j=0
done

kill `pidof r6_emu`
./r6_emu 1048576 8 32 2 16 1 1 1024 1 > results/log_after_crash

