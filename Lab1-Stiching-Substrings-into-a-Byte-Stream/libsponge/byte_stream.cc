#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity) 
	: buffer{}, qCapacity{capacity}, readBytesNum{0}, writeBytesNum{0}, isEnd{false} {}

size_t ByteStream::write(const string & data) {
	size_t len = data.length();
	if (len > qCapacity - buffer.size()) len = qCapacity - buffer.size();
	
    writeBytesNum += len;

    for (size_t i = 0; i < len; ++i) buffer.emplace_back(data[i]);

    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const 
{
	size_t length = len;
	if (length > buffer.size()) length = buffer.size();

    return string().assign(buffer.begin(), buffer.begin() + length);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{ 
	size_t length = len;
	if (length > buffer.size()) length = buffer.size();

    readBytesNum += length;

    while (length--) buffer.pop_front();
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
	string res = ByteStream::peek_output(len);
	ByteStream::pop_output(len);

	return res;
}

void ByteStream::end_input() { isEnd = true; }

bool ByteStream::input_ended() const { return isEnd; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.empty(); }

bool ByteStream::eof() const { return ByteStream::buffer_empty() && ByteStream::input_ended(); }

size_t ByteStream::bytes_written() const { return writeBytesNum; }

size_t ByteStream::bytes_read() const { return readBytesNum; }

size_t ByteStream::remaining_capacity() const { return qCapacity - buffer.size(); }
