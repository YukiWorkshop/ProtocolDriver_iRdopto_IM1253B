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

#pragma once

#include <vector>
#include <functional>

#include <cstdlib>
#include <cstring>
#include <cinttypes>


namespace YukiWorkshop::ProtocolDriver::iRdopto {

	class IM1253B {
	private:
		std::vector<uint8_t> read_buffer;
		size_t cursor = 0;

		uint8_t device_id_;
		uint8_t command_;

		bool is_valid_data();

	public:
		struct Values {
			float voltage;
			float current;
			float power;
			float energy;
			float pf;
			float co2;
			float temp;
			float freq;
		};

		std::function<void(const Values&)> callback_measurements;
		std::function<void()> callback_cleared;

		IM1253B() = default;
		explicit IM1253B(uint8_t __device_id) {
			set_device_id(__device_id);
		}

		void set_device_id(uint8_t __device_id) {
			device_id_ = __device_id;
		}

		const std::vector<uint8_t>& generate_request_measurements();
		const std::vector<uint8_t>& generate_request_clear();

		void parse_response_data(void *__data, uint32_t __len);
	};
}