#include "tcp_receiver.hh"
#include <iostream>
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.


using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) { 
    const TCPHeader & header = seg.header();
    size_t segment_len = seg.payload().copy().size();

    // if syn is not set, no segment should be reveived
    if (!isSYNSet && !header.syn) return false;
    // other SYN should be ignored
    if (isSYNSet && header.syn) return false;

    // SYN segment, with no data
    if (!isSYNSet && header.syn) {
        isSYNSet = true;
        isn = header.seqno.raw_value();
        ack = isn;
    }

    // if FIN is set, no segment should be received
    //if (isFINSet) return false;

    // FIN segment with no data
    if (!isFINSet && header.fin) {
        isFINSet = true;
    }

    // empty segment
    if (!header.syn && !header.fin && segment_len == 0) return true;

    uint64_t segment_start_seqno = unwrap(header.seqno, WrappingInt32{isn}, checkpoint);
    uint64_t segment_end_seqno   = segment_start_seqno + segment_len - 1;
    // if the segment_len == 0
    if (segment_end_seqno < segment_start_seqno) ++segment_end_seqno;

    uint64_t window_start_seqno = unwrap(WrappingInt32{ack}, WrappingInt32{isn}, checkpoint);
    uint64_t window_end_seqno   = window_start_seqno + window_size() - 1;
    // if window size == 0
    if (window_end_seqno < window_start_seqno) ++window_end_seqno;
        
    // the segement not inside the window range
    bool isInside = true;
    if ((segment_end_seqno < window_start_seqno) || (segment_start_seqno > window_end_seqno)) isInside = false;
    if (isInside) {
        size_t writeLen = min(segment_len, window_start_seqno + window_size() - segment_start_seqno);
       _reassembler.push_substring(seg.payload().copy().substr(0, writeLen), segment_start_seqno - 1, isFINSet);
    }
    
    if (isFINSet && _reassembler.unassembled_bytes() == 0) _reassembler.stream_out().end_input();

    checkpoint = _reassembler.stream_out().bytes_written();
    ack        = wrap(_reassembler.stream_out().bytes_written() + 1, WrappingInt32{isn}).raw_value();

    if (isFINSet && _reassembler.unassembled_bytes() == 0) ++ack;
    
    return isInside;
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{
	if (isSYNSet) return WrappingInt32{ack};

	return std::nullopt;
}

size_t TCPReceiver::window_size() const 
{
    return _capacity - _reassembler.stream_out().buffer_size();
}
