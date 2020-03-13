/*
    This file is part of ProtocolDriver_iRdopto_IM1253B.
    Copyright (C) 2020 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fcntl.h>
#include <cassert>
#include <unistd.h>
#include <sys/poll.h>

#include "ProtocolDriver/iRdopto/IM1253B.hpp"

using namespace YukiWorkshop::ProtocolDriver::iRdopto;

int main() {

	IM1253B i(0x1);

	i.callback_measurements = [](const IM1253B::Values &v) {
		std::cout << "Voltage: " << v.voltage << " V\n";
		std::cout << "Current: " << v.current << " A\n";
		std::cout << "Power: " << v.power << " W\n";
		std::cout << "Energy: " << v.energy << " kWh\n";
		std::cout << "Power factor: " << v.pf << "\n";
		std::cout << "CO2 production: " << v.co2 << " KG\n";
		std::cout << "Temperature: " << v.temp << " °C\n";
		std::cout << "Frequency: " << v.freq << " Hz\n";
	};

	i.callback_cleared = [](){
		std::cout << "Data cleared.\n";
	};


	system("stty -F /dev/ttyUSB1 4800 raw");
	system("stty -F /dev/ttyUSB1");

	int fd = open("/dev/ttyUSB1", O_RDWR);

	assert(fd > 0);

	struct pollfd fds = {
		.fd = fd,
		.events = POLLIN
	};

	char buf[64];

	while (1) {

		auto r = i.generate_request_measurements();
		write(fd, r.data(), r.size());

		size_t read_size = 0;

		while (1) {
			int rc_poll = poll(&fds, 1, 2000);

			if (!rc_poll)
				break;
			else if (rc_poll > 0) {
				ssize_t rc = read(fd, buf + read_size, 64);
				if (rc > 0)
					read_size += rc;
				else if (!rc)
					break;
				else
					abort();
			}
		}

		std::cout << "Read " << read_size << " bytes.\n";

		i.parse_response_data(buf, read_size);

	}

	return 0;
}