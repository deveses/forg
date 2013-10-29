#include "forg_pch.h"

#include "core/BitArray.h"

namespace forg { namespace core {

BitArray::BitArray(const BitArray& bits)
:	m_array(bits.m_array),
	m_length(bits.m_array.size())
{

}

//BitArray::BitArray(const array<int>& values)
//: m_array(values.get(), values.get_Count()), m_length(values.get_Count() << 5)
//{
//}
//
//BitArray::BitArray(const array<byte>& values)
//: m_array((values.get_Count()+3)/4), m_length(values.get_Count()<<3)
//{
//	int i = 0;
//	int c = 0;
//	while ((values.get_Count() - c) >= 4)
//	{
//		m_array[i++] = (values[c]) | ((values[c+1]) << 8) | values[c+2] << 16 | values[c+3] << 24;
//		c += 4;
//	}
//
//	switch(values.get_Count() - c) {
//	case 3:
//		m_array[i] |= values[c+2]<<16;
//	case 2:
//		m_array[i] |= values[c+1]<<8;
//	case 1:
//		m_array[i] |= values[c];
//		break;
//	}
//}
//
//BitArray::BitArray(const array<bool>& values)
//: m_array(values.get_Count()), m_length(values.get_Count())
//{
//	for (uint i = 0; i < values.get_Count(); i++)
//	{
//		if (values[i])
//		{
//			m_array[i/0x20] |=  (1 << (i % 0x20));
//		}
//	}
//}

BitArray::BitArray(uint length)
    : m_array((length+0x1f)>>5) // 0x1f = 31
    , m_length(length)
{
	m_array.clear();
}

BitArray::BitArray(uint length, bool defaultValue)
    : m_array((length+0x1f)>>5)
    , m_length(length)
{
	//m_array.clear();
	SetAll(defaultValue);
}

BitArray::~BitArray(void)
{
}

BitArray& BitArray::operator = (const BitArray& bits)
{
    m_array = bits.m_array;

	m_length = bits.m_length;

	return *this;
}

bool BitArray::operator[](uint index) const
{
	return (m_array[index>>5] & 1<<(index%0x20))!=0;
}

BitArray BitArray::operator &(const BitArray& arg) const
{
	//int len1 = m_array.Size();
	//int len2 = arg.m_array.Size();
	//int len = len1 > len2 ? len2 : len1;

	//for (int i = 0; i < len; i++)
	//{
	//	m_array[i] &= arg.m_array[i];
	//}

	//return *this;

	uint len1 = get_Count();
	uint len2 = arg.get_Count();

	BitArray r(len1 > len2 ? *this : arg);

	r &= (len1 > len2 ? arg : *this);

	return r;
}

BitArray& BitArray::operator &= (const BitArray& arg)
{
	uint len1 = get_Count();
	uint len2 = arg.get_Count();
	uint len = len1 > len2 ? len2 : len1;

	for (uint i = 0; i < len; i+=0x20)
	{
		uint idx = i>>5;
		uint cnt = len - i;
		m_array[idx] &= arg.m_array[idx] & (cnt>=0x20 ? 0xffffffff : 0xffffffff >> (0x20 - cnt));
	}

	return *this;
}

BitArray BitArray::operator |(const BitArray& arg) const
{
	uint len1 = get_Count();
	uint len2 = arg.get_Count();

	BitArray r(len1 > len2 ? *this : arg);

	r |= (len1 > len2 ? arg : *this);

	return r;
}

BitArray& BitArray::operator |= (const BitArray& arg)
{
	uint len1 = get_Count();
	uint len2 = arg.get_Count();
	uint len = len1 > len2 ? len2 : len1;

	for (uint i = 0; i < len; i+=0x20)
	{
		uint idx = i>>5;
		uint cnt = len - i;
		m_array[idx] |= arg.m_array[idx] & (cnt>=0x20 ? 0xffffffff : 0xffffffff >> (0x20 - cnt));
	}

	return *this;
}

BitArray BitArray::operator ~() const
{
	BitArray result(m_length, false);
	uint len = m_array.size();

	for (uint i = 0; i < len; i++)
	{
		result.m_array[i] = ~m_array[i];
	}

	return result;
}

BitArray BitArray::operator ^(const BitArray& arg) const
{
	uint len1 = get_Count();
	uint len2 = arg.get_Count();

	BitArray r(len1 > len2 ? *this : arg);

	r ^= (len1 > len2 ? arg : *this);

	return r;
}

BitArray& BitArray::operator ^= (const BitArray& arg)
{
	uint len1 = get_Count();
	uint len2 = arg.get_Count();
	uint len = len1 > len2 ? len2 : len1;

	for (uint i = 0; i < len; i+=0x20)
	{
		uint idx = i>>5;
		uint cnt = len - i;
		m_array[idx] ^= arg.m_array[idx] & (cnt>=0x20 ? 0xffffffff : 0xffffffff >> (0x20 - cnt));
	}

	return *this;
}

BitArray BitArray::And(const BitArray& value) const
{
	return (*this & value);
}

BitArray BitArray::Or(const BitArray& value) const
{
	return (*this | value);
}

BitArray BitArray::Xor(const BitArray& value) const
{
	return (*this ^ value);
}

BitArray BitArray::Not() const
{
	return (~ (*this));
}


inline bool BitArray::Get(uint index) const
{
	return (m_array[index>>5] & 1<<(index%0x20))!=0;
}

void BitArray::Set(uint index, bool value)
{
	if (index >= m_length)
		set_Length(index + 1);

	if (value)
	{
		m_array[index>>5] |= (1 << (index % 0x20));
	}
	else
	{
		m_array[index>>5] &= ~(1 << (index % 0x20));
	}

}

//void BitArray::Set(int index, int value)
//{
//
//}

void BitArray::SetAll(bool value)
{
	int v = value ? -1 : 0;
	uint len = m_array.size();
	for (uint i = 0; i < len; i++)
	{
		m_array[i] = v;
	}
}

inline uint BitArray::get_Count() const
{
	return m_length;
}

inline uint BitArray::get_Length() const
{
	return m_length;
}

void BitArray::set_Length(uint value)
{
	uint nsize = (value+0x1f)>>5;

	if (value > m_length && m_array.size()<nsize)
	{
		m_array.resize(nsize);
	}

	m_length = value;
}

BitArray::string BitArray::ToString()
{
    uint len = get_Count();
    BitArray::string str(len, '0');

	for (uint i=0; i<len; i++)
	{
		str[i] = Get(len - i - 1) ? '1' : '0';
	}

	return str;
}

bool operator == (const BitArray& b1, const BitArray& b2)
{
	uint len1 = b1.m_array.size();
	uint len2 = b2.m_array.size();
	bool equals = true;

	BitArray tmp = b1 | b2;

	if (len1 > len2)
	{
		for (uint i=0; equals && i<len1; i++)
		{
			equals = (tmp.m_array[i] == b1.m_array[i]);
		}
	}
	{
		for (uint i=0; equals && i<len2; i++)
		{
			equals = (tmp.m_array[i] == b2.m_array[i]);
		}
	}

	return equals;
}

bool operator != (const BitArray& b1, const BitArray& b2)
{
	return !(b1 == b2);
}

}}

