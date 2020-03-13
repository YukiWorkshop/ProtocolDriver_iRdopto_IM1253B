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

	const uint8_t test_data0[] = {0x01, 0xaa, 0x01, 0x03, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe7, 0x00, 0x00, 0x00, 0x00, 0x58, 0xd1};

	const uint8_t test_data1[] = {0x01, 0xaa, 0x01, 0x10, 0x00, 0x4b, 0x00, 0x02, 0x31, 0xde};

	i.generate_request_measurements();
	i.parse_response_data((void *) test_data0, sizeof(test_data0));

	i.generate_request_clear();
	i.parse_response_data((void *) test_data1, sizeof(test_data1));


	return 0;
}