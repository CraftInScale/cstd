#include "NetAddress.h"

#include "../../error.h"
#include "../../math.h"
#include "../../mem.h"
#include "../../string/StringConversion.h"

#include <string.h>

void init_ipaddr(IpAddr& ip, sockaddr* in, socklen_t in_size)
{
	if (in_size == sizeof(sockaddr_in))
	{
		sockaddr_in* in4 = (sockaddr_in*)in;

		memcpy(ip.bytes, &in4->sin_addr, 4);
		ip.version = ADDR_V4;
	}
	else if (in_size == sizeof(sockaddr_in6))
	{
		sockaddr_in6* in6 = (sockaddr_in6*)in;

		memcpy(ip.bytes, &in6->sin6_addr, 16);
		ip.version = ADDR_V6;
	}
	else
	{
		bzero(ip.bytes, 16);
		ip.version = ADDR_V6;
	}
}

void init_ipaddr_v4(IpAddr& ip, uint8_t* bytes)
{
	memcpy(ip.bytes, bytes, 4);
	ip.version = ADDR_V4;
}

void init_ipaddr_v6(IpAddr& ip, uint8_t* bytes)
{
	memcpy(ip.bytes, bytes, 16);
	ip.version = ADDR_V6;
}

void init_netaddr(NetAddress& address, sockaddr* in, socklen_t in_size)
{
	init_ipaddr(address.ip, in, in_size);

	if (in_size == sizeof(sockaddr_in))
	{
		sockaddr_in* in4 = (sockaddr_in*)in;

		address.port = ntohs(in4->sin_port);
	}
	else if (in_size == sizeof(sockaddr_in6))
	{
		sockaddr_in6* in6 = (sockaddr_in6*)in;

		address.port = ntohs(in6->sin6_port);
	}
	else
	{
		address.port = 0;
	}
}

void init_netaddr(NetAddress& address, IpAddr& ip, uint16_t port)
{
	address.ip = ip;
	address.port = port;
}

int netaddr_to_string(NetAddress& address, String& append_to)
{
	IpAddr& ip = address.ip;

	switch (address.ip.version)
	{
	case ADDR_V4:
		number_to_string(ip.bytes[0], append_to);
		string_concat(append_to, ".", 1, 1);
		number_to_string(ip.bytes[1], append_to);
		string_concat(append_to, ".", 1, 1);
		number_to_string(ip.bytes[2], append_to);
		string_concat(append_to, ".", 1, 1);
		number_to_string(ip.bytes[3], append_to);
		break;
	case ADDR_V6:
	{
		size_t mask = 0xF;
		size_t shift = 0;
		size_t i = 0;
		while (i < 16)
		{
			number_to_hex((ip.bytes[i] & 0xF0) >> 4, append_to);
			number_to_hex((ip.bytes[i] & 0xF), append_to);

			if ((i+1) % 4 == 0 && i < 15)
			{
				string_concat(append_to, ":", 1, 1);
			}

			i += 1;
		}
	}
		break;
	}

	string_concat(append_to, ":", 1, 1);
	number_to_string(address.port, append_to);

	return E_OK;
}

int init_netaddr(NetAddress& address, String& ip, uint16_t port)
{
	// parse ip string
	StringIndexofResult match_result;
	int error = string_indexof(ip, ":", 1, match_result);
	if (error != E_OK)
		return error;

	if (!match_result.found)
	{
		// IPv4
		uint8_t octets[4];
		int octet_fill_count = 0;
		char num[3];
		int num_char_count = 0;
		
		StringIter iter;
		init_stringiter_forward(iter, &ip);

		uint32_t ch;
		while (true)
		{
			error = stringiter_next(iter, ch);
			if (error != E_OK)
			{
				if (error == E_NULL)
					break;
			}

			if (ch == '.')
			{
				if (num_char_count == 0)
					return E_PARSE_FAIL;

				if (octet_fill_count == 4)
					return E_PARSE_FAIL;

				// convert array of chars to number
				uint16_t octet = 0;

				int i = 0;
				while (i < num_char_count-1)
				{
					char ch1 = num[i];
					octet += pow10(num_char_count - i - 1) * (ch1 - '0');
					i += 1;
				}

				octet += num[num_char_count - 1] - '0';

				if (octet > 255)
					return E_PARSE_FAIL;

				octets[octet_fill_count] = octet;
				octet_fill_count += 1;

				num_char_count = 0;
			}
			else
			{
				if (num_char_count < 3)
				{
					if (ch >= '0' && ch <= '9')
					{
						// exclude numbers beginning with 0
						if (ch == '0' && num[0] == '0' && num_char_count == 1)
						{
							return E_PARSE_FAIL;
						}

						num[num_char_count] = (char)ch;
						num_char_count += 1;
					}
					else return E_PARSE_FAIL;
				}
				else return E_PARSE_FAIL;
			}
		}

		if (num_char_count > 0)
		{
			uint8_t octet = 0;

			int i = 0;
			while (i < num_char_count-1)
			{
				char ch1 = num[i];
				octet += ((num_char_count - i - 1) * 10) * (ch1 - '0');
				i += 1;
			}

			octet += num[num_char_count-1] - '0';

			octets[octet_fill_count] = octet;
			octet_fill_count += 1;
		}

		memcpy(address.ip.bytes, octets, 4);
		address.ip.version = ADDR_V4;
	}
	else
	{
		// IPv6
		uint16_t segments[8] = {0};
		int segment_count = 0;

		char num[4];
		int num_count = 0;

		bool zero = false;
		int zero_at = 0;

		bool zero_init = false;

		StringIter iter;
		init_stringiter_forward(iter, &ip);

		uint32_t ch;
		while (true)
		{
			error = stringiter_next(iter, ch);
			if (error != E_OK && error != E_NULL)
			{
				return error;
			}

			if (ch != ':' && error != E_NULL)
			{
				if (num_count == 4)
					return E_PARSE_FAIL;

				num[num_count] = ch;
				num_count += 1;
			}
			else
			{
				if (segment_count == 8)
					return E_PARSE_FAIL;

				if (num_count > 0)
				{
					// convert num[] to uint16_t number

					uint32_t number = 0;

					int i = 0;
					uint8_t digit;
					while (i < num_count)
					{
						char ch1 = num[i];

						if (ch1 >= '0' && ch1 <= '9')
						{
							digit = ch1 - '0';
						}
						else if (ch1 >= 'A' && ch1 <= 'F')
						{
							digit = 10 + (ch1 - 'A');
						}
						else if (ch1 >= 'a' && ch1 <= 'f')
						{
							digit = 10 + (ch1 - 'a');
						}
						else return E_PARSE_FAIL;

						number += pow16(num_count - 1 - i) * digit;
						i += 1;
					}

					if (number > 65535)
						return E_PARSE_FAIL;

					segments[segment_count] = (uint16_t)number;
					segment_count += 1;

					num_count = 0;
				}
				else
				{
					if (error == E_NULL)
						break;

					if (zero)
						return E_PARSE_FAIL;

					if (zero_init || segment_count > 0)
					{
						zero = true;
						zero_at = segment_count;
					}

					zero_init = segment_count == 0;
				}
			}

			if (error == E_NULL)
				break;
		}

		if (zero)
		{
			int zero_segments_count = 8 - segment_count;

			memcpybackward(
				((uint8_t*)segments) + zero_at * 2 + zero_segments_count * 2,
				((uint8_t*)segments) + zero_at * 2,
				2 * (8 - zero_at - zero_segments_count)
			);

			int i = 0;
			while (i < zero_segments_count)
			{
				segments[i + zero_at] = 0;
				i += 1;
			}
		}

		memcpy(address.ip.bytes, segments, 16);
		address.ip.version = ADDR_V6;

		/*uint16_t segments[8];
		int segments_count = 0;

		uint8_t num[4];
		int num_count = 0;

		bool zero = false;
		int zero_start = -1;
		//int zero_end = -1;
		int zero_colon_count = 0;
		bool zero_colon_reset = false;

		StringIter iter;
		init_stringiter_forward(iter, &ip);

		uint32_t ch;
		while (true)
		{
			error = stringiter_next(iter, ch);
			if (error != E_OK && error != E_NULL)
			{
				return error;
			}

			if (ch == ':' || error == E_NULL)
			{
				if (num_count > 0)
				{
					if (zero_colon_count > 0 && !zero_colon_reset)
					{
						zero_colon_reset = true;
					}

					if (segments_count < (8 - zero))
					{
						int i = 0;
						uint32_t number;
						while (i < num_count-1)
						{
							char ch1 = num[i];

							// convert hex char to number
							uint8_t hex_value = 0;
							if (ch1 >= '0' && ch1 <= '9')
							{
								hex_value = ch1 - '0';
							}
							else if (ch1 >= 'A' && ch1 <= 'F')
							{
								hex_value = ch1 - 'A';
							}
							else if (ch1 >= 'a' && ch1 <= 'f')
							{
								hex_value = ch1 - 'a';
							}
							else return E_PARSE_FAIL;

							number += ((num_count - i - 1) * 16) * hex_value;

							i += 1;
						}

						char ch1 = num[num_count-1];
						uint8_t hex_value = 0;
						if (ch1 >= '0' && ch1 <= '9')
						{
							hex_value = ch1 - '0';
						}
						else if (ch1 >= 'A' && ch1 <= 'F')
						{
							hex_value = ch1 - 'A';
						}
						else if (ch1 >= 'a' && ch1 <= 'f')
						{
							hex_value = ch1 - 'a';
						}
						else return E_PARSE_FAIL;

						number += ((num_count - i - 1) * 16) * hex_value;

						if (number > 65535)
							return E_PARSE_FAIL;

						segments[segments_count] = number;
						segments_count += 1;
					}
					else return E_PARSE_FAIL;
				}
				else
				{
					if (zero && zero_colon_reset)
						return E_PARSE_FAIL;

					if (!zero)
					{
						zero = true;
						zero_start = segments_count;
					}
					
					if(zero_colon_count > 2)
						return E_PARSE_FAIL;// 2nd zero

					zero_colon_count += 1;
				}
			}
			else
			{
				if (num_count < 4)
				{
					if ((ch >= '0' && ch <= '9') ||
						(ch >= 'A' && ch <= 'F') ||
						(ch >= 'a' && ch <= 'f')
					   )
					{
						num[num_count] = (char)ch;
						num_count += 1;
					}
					else return E_PARSE_FAIL;
				}
				else return E_PARSE_FAIL;
			}

			if (error == E_NULL)
				break;
		}

		uint16_t segments2[8];
		int segments2_count = 0;

		int i = 0;
		int dst_i = 0;
		while (i < segments_count)
		{
			if (zero)
			{
				if (zero_start == i)
				{
					int zero_count = 8 - segments_count;
					int j = 0;
					while (j < zero_count)
					{
						segments2[dst_i] = 0;
						dst_i += 1;
						j += 1;
					}
				}
			}
			i += 1;
		}

		memcpy(address.ip.bytes, segments2, 16);
		address.ip.version = ADDR_V6;*/
	}

	address.port = port;

	return E_OK;
}
