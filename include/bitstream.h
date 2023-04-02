/* bitstream---bitstream access utility class
 *
 * This is a part of Westbook's Application Utility Library
 * Copyright (C) 2003 Westbook
 * All Rights Reserved.
 */
#ifndef __WB_Util_bitstream__
#define __WB_Util_bitstream__
#pragma once

namespace WB {
namespace Util {

static char ver_id_bitstream[]="WB/Util/bitstream $Revision: 1.1 $ Copyright (C) 2003 Westbook";

typedef unsigned char BYTE;

template<typename _Ty> class bitstream
{
	BYTE* ptr_;
	size_t size_;
	size_t c_bytes_;
	size_t c_bit_offs_;
	size_t buf_type_bits;
public:
	typedef _Ty value_type;
	typedef value_type bits_value;
	bitstream(BYTE* d, size_t s)
		: ptr_(d), size_(s), c_bytes_(0), c_bit_offs_(0), buf_type_bits(8) {};
	bool eos() const { return (c_bytes_ >= size_); }
	size_t byte_size() const { return size_; }
	size_t bit_size() const { return size_ * buf_type_bits; }
	bits_value nextbits(size_t need_bits, size_t* out_bits) const
	{
		return read_cur_bits(need_bits, out_bits);
	}
	bits_value readbits(size_t need, size_t* readed = NULL)
	{
		size_t r;
		bits_value data = 0;
		if (readed == NULL) { readed = &r; }
		data = read_cur_bits(need, readed);
		seek_bits(*readed);
		return data;
	}
#if 0
	size_t readbits(bits_value* data, size_t need)
	{
		size_t readed;
		*data = read_cur_bits(need, &readed);
		seek_bits(readed);
		return readed;
	}
#endif
	bits_value readbits(unsigned pos, size_t n, size_t* readed = NULL)
	{
		if (bit_pos() != pos) {
			seek_to_bits(pos);
		}
		return readbits(n, readed);
	}
	size_t writebits(size_t size, const bits_value& data)
	{
		size_t writed;
		write_cur_bits(size, data, &writed);
		seek_bits(writed);
		return data;
	}
	size_t writebits(unsigned pos, size_t n, const bits_value& data)
	{
		if (bit_pos() != pos) {
			seek_to_bits(pos);
		}
		return writebits(n, data);
	}
	unsigned byte_pos() const
	{
		return static_cast<unsigned>(c_bytes_);
	}
	unsigned bit_pos() const
	{
		return static_cast<unsigned>(c_bytes_ * buf_type_bits + c_bit_offs_);
	}
	void seek_to_bytes(size_t n)
	{
		c_bytes_ = n;
		c_bit_offs_ = 0;
		fix_pointer();
	}
	void seek_to_bits(size_t n)
	{
		c_bytes_ = (n / 8);
		c_bit_offs_ = (n % 8);
		fix_pointer();
	}
	void seek_bytealigned(size_t n = 0)
	{
		c_bytes_ += n;
		c_bit_offs_ = 0;
		fix_pointer();
	}
	void seek_bytes(size_t n) { return seek_bytealigned(n); }
	void seek_bits(size_t n)
	{
		c_bytes_ += (n / 8);
		c_bit_offs_ += (n % 8);
		fix_pointer();
	}
protected:
	BYTE make_mask(unsigned start, size_t size) const
	{
		BYTE b;
		if (size <= 0) { return 0; }
		if ((start + size) > buf_type_bits) {
			size = buf_type_bits - start;
		}
		b = 1;
		size_t i;
		for (i = 1; i < size; ++i) {
			b = b << 1;
			b |= 1;
		}
		i += start;
		for (; i < buf_type_bits; ++i) {
			b = b << 1;
		}
		return b;
	}
	void fix_pointer()
	{
		c_bytes_ += (c_bit_offs_ / 8);
		c_bit_offs_ = (c_bit_offs_ % 8);
		if (c_bytes_ >= size_) {
			c_bytes_ = size_;
			c_bit_offs_ = 0;
		}
	}
	bits_value read_cur_bits(size_t need_bits, size_t* out_bits) const
	{
		size_t assigned = 0;
		bits_value data = 0;
		size_t remain = need_bits;
		size_t bit_offs = c_bit_offs_;
		size_t pos = c_bytes_;
		BYTE* p = ptr_ + c_bytes_;
		while (remain > 0 && pos < size_) {
			BYTE b = *p;
			size_t c_remain = buf_type_bits - bit_offs;
			size_t c_need = remain;
			if (c_need > c_remain) { c_need = c_remain; }
			b &= make_mask(static_cast<unsigned>(bit_offs), c_need);
			b = b >> static_cast<BYTE>(buf_type_bits - c_need - bit_offs);
			data = data << static_cast<BYTE>(c_need);
			data |= b;
			assigned += c_need;
			remain = need_bits - assigned;
			++p;
			++pos;
			bit_offs = 0;
		}
		if (out_bits != NULL) { *out_bits = assigned; }
		return data;
	}
	size_t write_cur_bits(size_t write_bits, const bits_value& data, size_t* writed)
	{
		size_t assigned = 0;
		size_t remain = write_bits;
		size_t bit_offs = c_bit_offs_;
		size_t pos = c_bytes_;
		BYTE* p = ptr_ + c_bytes_;
		while (remain > 0 && pos < size_) {
			size_t c_remain = buf_type_bits - bit_offs;
			size_t c_write = remain;
			if (c_write > c_remain) { c_write = c_remain; }
			BYTE mask = make_mask(static_cast<unsigned>(bit_offs), c_write);
			BYTE r_mask = ~mask;
			*p = *p & r_mask;
			bits_value val = data >> (remain - c_write);
			val = val << static_cast<bits_value>(buf_type_bits - c_write - bit_offs);
			BYTE b = static_cast<BYTE>(val & mask);
			*p |= b;
			assigned += c_write;
			remain = write_bits - assigned;
			++p;
			++pos;
			bit_offs = 0;
		}
		if (writed != NULL) { *writed = assigned; }
		return assigned;
	}
};

}	// namespace WB::Util
}	// namespace WB

#endif /* __WB_Util_bitstream__ */
