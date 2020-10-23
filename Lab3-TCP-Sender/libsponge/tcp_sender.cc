#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _timer()
    , _retransmission_timeout(retx_timeout)
    , _unacked_segments() {}

uint64_t TCPSender::bytes_in_flight() const { return max(_next_seqno - _abs_ackno, 0ul); }

void TCPSender::fill_window() 
{
	if (!_is_syn_sent) {
        TCPSegment tcp_segment;
		tcp_segment.header().seqno = next_seqno();
		tcp_segment.header().syn   = true;

        _segments_out.push(tcp_segment);
        ++_next_seqno;
        _unacked_segments.push(tcp_segment);
        _is_syn_sent = true;
        if (!_timer.is_turn_on()) {
            _timer.turn_on(_retransmission_timeout);
            _timer.set_last_expire_time(_current_time);
        }
    }

    while (_stream.buffer_size()                                    // there is data to be sent
           && (bytes_in_flight() < _window_size || !_window_size)) { // zero window probing
        size_t len_to_read =
            min(TCPConfig::MAX_PAYLOAD_SIZE,
                min(_stream.buffer_size(),
                    max(static_cast<size_t>(1), static_cast<size_t>(_window_size)) - bytes_in_flight()));
        if (!len_to_read) break;

        TCPSegment tcp_segment;
        tcp_segment.payload() = Buffer(_stream.read(len_to_read));
        auto & header = tcp_segment.header();
        // fin occupies sequence space, so check whether it fits in
        if (!_is_fin_sent && bytes_in_flight() < _window_size && _stream.eof()) {
            header.fin = true;
            _is_fin_sent = true;
        }
        header.seqno = next_seqno();
        _next_seqno += tcp_segment.length_in_sequence_space();
        _segments_out.push(tcp_segment);
        _unacked_segments.push(tcp_segment);

        if (!_timer.is_turn_on()) {
            _timer.turn_on(_retransmission_timeout);
            _timer.set_last_expire_time(_current_time);
        }

        if (!_window_size) break;
    }

    if (!_is_fin_sent && _stream.eof() 
    	&& (bytes_in_flight() < _window_size || !bytes_in_flight())) {
        TCPSegment tcp_segment;
		tcp_segment.header().seqno = next_seqno();
		tcp_segment.header().fin   = true;

        _segments_out.push(tcp_segment);
        _next_seqno++;
        _unacked_segments.push(tcp_segment);
        _is_fin_sent = true;
        if (!_timer.is_turn_on()) {
            _timer.turn_on(_retransmission_timeout);
            _timer.set_last_expire_time(_current_time);
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{ 
	auto abs_ackno = unwrap(ackno, _isn, _abs_ackno);
    // invalid ackno
    if (abs_ackno > _next_seqno) return false;
    // has been acked
    if (abs_ackno < _abs_ackno) return true;

    _consecutive_retrans = 0;
    _retransmission_timeout = _initial_retransmission_timeout;
    _timer.set_last_expire_time(_current_time);
    _timer.set_timeout(_retransmission_timeout);
    _abs_ackno = abs_ackno;
    _window_size = window_size;

    auto first_unacked_seno = !_unacked_segments.empty() ? 
    	unwrap(_unacked_segments.front().header().seqno, _isn, _abs_ackno) : 0;

    while(!_unacked_segments.empty() && first_unacked_seno < abs_ackno) {
        _unacked_segments.pop();
        if (!_unacked_segments.empty())
            first_unacked_seno = unwrap(_unacked_segments.front().header().seqno, _isn, _abs_ackno);
        else break;
    }

    fill_window();
    if (!_unacked_segments.size()) _timer.turn_off();

    return true;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{
	_current_time += ms_since_last_tick;
    if (_timer.is_turn_on() && _timer.is_expire(_current_time) 
    	&& _unacked_segments.size()) {
        _segments_out.push(_unacked_segments.front());
        if (_window_size) {
            ++_consecutive_retrans;
            _retransmission_timeout *= 2;
        }
        _timer.set_last_expire_time(_current_time);
        _timer.set_timeout(_retransmission_timeout);
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retrans; }

void TCPSender::send_empty_segment() 
{
	TCPSegment tcp_segment;
    auto & header = tcp_segment.header();
    header.seqno = next_seqno();
    _segments_out.push(tcp_segment);
}
