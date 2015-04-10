//
// Super simple log server, based on ZeroMQ, written in C++
// Hello World client in C++
// Connects REQ socket to tcp://localhost:5555
//
// This is an example logging client
//

// std::string
#include <string>
// zmq::context_t zmq::socket_t zmq::message_t
#include <zmq.hpp>
// [linux] getpid()
#include <unistd.h>
// Json::FastWriter Json::Value
// boost::gregorian::date boost::gregorian::to_iso_string
#include <boost/date_time/gregorian/gregorian.hpp>
// boost::posix_time::ptime boost::posix_time::microsec_clock
// boost::posix_time::time_duration boost::posix_time::to_simple_string
#include <boost/date_time/posix_time/posix_time.hpp>

#include <thread>

#include "json/json.h"


int pid = getpid();


std::string formattedNow() {
    // Get current time from the clock, using microseconds resolution
    const boost::posix_time::ptime now = 
        boost::posix_time::microsec_clock::local_time();

    // Extract date and time
    const boost::gregorian::date date = now.date();
    const boost::posix_time::time_duration time = now.time_of_day();

    // Make strings
    // YYYYMMDD
    const std::string dateStr = boost::gregorian::to_iso_string(date);
    // hh::mm::ss.fffffff
    const std::string timeStr = boost::posix_time::to_simple_string(time);
    return dateStr + "-" + timeStr;
}


// TODO: write a general log function which generalizes "info", "warning", etc.
void info(std::string text) {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect("tcp://localhost:5555");
    Json::Value msg_;
    msg_["level"] = 1;
    msg_["dt"] = formattedNow();
    msg_["pid"] = pid;
    msg_["text"] = text;
    Json::FastWriter fastWriter;
    std::string msg = fastWriter.write(msg_);

    int n = msg.length();
    zmq::message_t request(n);
    memcpy((void *) request.data(), msg.c_str(), n);
    socket.send(request);

    // Get the reply
    zmq::message_t reply;
    socket.recv(&reply);
    std::string rep = std::string(static_cast<char*>(reply.data()),
            reply.size());

    // TODO: should we close the socket..?
}


int main ()
{
    //  Do 100 log requests, waiting each time for a response
    for (int request_nbr = 0; request_nbr != 100; request_nbr++) {
        std::thread t(&info, "bla bla");
        t.detach();
    }
    return 0;
}

