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

#include "IM1253B.hpp"

using namespace YukiWorkshop::ProtocolDriver::iRdopto;

static uint16_t calccrc(uint8_t crcbuf, uint16_t crc) {
	uint8_t i;
	uint8_t chk;
	crc=crc ^ crcbuf;
	for(i=0;i<8;i++) {
		chk = (uint8_t)(crc & 1);
		crc = crc>>1;
		crc = crc&0x7fff;
		if (chk == 1)
			crc = crc ^ 0xa001;
		crc = crc & 0xffff;
	}
	return crc;
}
static uint16_t chkcrc(uint8_t *buf, uint32_t len) {
	uint8_t hi, lo;
	uint16_t crc = 0xFFFF;
	for (uint32_t i = 0; i < len; i++) {
		crc = calccrc(*buf, crc);
		buf++;
	}
	hi = (uint8_t) (crc % 256);
	lo = (uint8_t) (crc / 256);
	crc = (((uint16_t) (hi)) << 8) | lo;
	return crc;
}

static const std::vector<uint8_t> cmd_request_meas = {0x01, 0x03, 0x00, 0x48, 0x00, 0x08, 0xc4, 0x1a};
static const std::vector<uint8_t> cmd_request_clear = {0x01, 0x10, 0x00, 0x4b, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0xb6, 0x2c};

const std::vector<uint8_t> &IM1253B::generate_request_measurements() {
	command_ = 0x03;
	return cmd_request_meas;
}

const std::vector<uint8_t> &IM1253B::generate_request_clear() {
	command_ = 0x10;
	return cmd_request_clear;
}

bool IM1253B::is_valid_data() {
	return cursor >= 2 && read_buffer[0] == device_id_ && read_buffer[1] == command_;
}



void IM1253B::parse_response_data(void *__data, uint32_t __len) {
	read_buffer.insert(read_buffer.end(), (uint8_t *) __data, ((uint8_t *) __data) + __len);
	cursor += __len;

	while (!is_valid_data()) {
		if (read_buffer.size() > 2) {
			std::vector<decltype(read_buffer)::value_type>(read_buffer.begin() + 1,
								       read_buffer.end()).swap(read_buffer);
			cursor -= 1;
		} else if (read_buffer[0] == device_id_ && read_buffer[1] == command_) return;
		else break;
	}

	while (cursor > 2) {
		switch (command_) {
			case 0x03: {
				if (cursor < 3) return;
				uint8_t msg_length = read_buffer[2];
				uint8_t full_msg_length = msg_length + 5;
				if (cursor < full_msg_length) return;

				uint8_t check_length = msg_length + 3;

				uint16_t crc = *((uint16_t *)(read_buffer.data()+check_length));

				if (crc == htobe16(chkcrc(read_buffer.data(), check_length))) {
					auto d = read_buffer.data();

					Values v{};

					uint32_t vbuf;

					if (msg_length >= 4) {
						memcpy(&vbuf, d + 3, 4);
						vbuf = htobe32(vbuf);
						v.voltage = 0.0001f * vbuf;
					}

					if (msg_length >= 8) {
						memcpy(&vbuf, d + 7, 4);
						vbuf = htobe32(vbuf);
						v.current = 0.0001f * vbuf;
					}

					if (msg_length >= 12) {
						memcpy(&vbuf, d + 11, 4);
						vbuf = htobe32(vbuf);
						v.power = 0.0001f * vbuf;
					}

					if (msg_length >= 16) {
						memcpy(&vbuf, d + 15, 4);
						vbuf = htobe32(vbuf);
						v.energy = 0.0001f * vbuf;
					}

					if (msg_length >= 20) {
						memcpy(&vbuf, d + 19, 4);
						vbuf = htobe32(vbuf);
						v.pf = 0.001f * vbuf;
					}

					if (msg_length >= 24) {
						memcpy(&vbuf, d + 23, 4);
						vbuf = htobe32(vbuf);
						v.co2 = 0.0001f * vbuf;
					}

					if (msg_length >= 28) {
						memcpy(&vbuf, d + 27, 4);
						vbuf = htobe32(vbuf);
						v.temp = 0.01f * vbuf;
					}

					if (msg_length >= 32) {
						memcpy(&vbuf, d + 31, 4);
						vbuf = htobe32(vbuf);
						v.freq = 0.01f * vbuf;
					}

					if (callback_measurements) callback_measurements(v);
				}

				std::vector<decltype(read_buffer)::value_type>(read_buffer.begin() + full_msg_length,
									       read_buffer.end()).swap(read_buffer);

				cursor -= full_msg_length;

				break;
			}
			case 0x10: {
				if (cursor < 8) return;
				uint16_t crc = *((uint16_t *)(read_buffer.data()+6));

				if (crc == htobe16(chkcrc(read_buffer.data(), 6))) {
					if (callback_cleared) callback_cleared();
				}

				std::vector<decltype(read_buffer)::value_type>(read_buffer.begin() + 8,
									       read_buffer.end()).swap(read_buffer);

				cursor -= 8;

				break;
			}
			default: {
				std::vector<decltype(read_buffer)::value_type>(read_buffer.begin() + 2,
									       read_buffer.end()).swap(read_buffer);
				cursor -= 2;
			}
		}
	}

}

