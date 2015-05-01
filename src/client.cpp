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


void log_(int level, std::string text) {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect("tcp://localhost:5555");
    Json::Value msg_;
    msg_["level"] = level;
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

void debug(std::string text) {
    log_(1, text);
}
void verbose(std::string text) {
    log_(2, text);
}
void info(std::string text) {
    log_(3, text);
}
void warning(std::string text) {
    log_(4, text);
}
void error(std::string text) {
    log_(5, text);
}
void critical(std::string text) {
    log_(6, text);
}


int main ()
{
    //  Do 100 log requests, waiting each time for a response
    for (int request_nbr = 0; request_nbr != 100; request_nbr++) {
        std::thread t1(&debug, "debug");
        t1.detach();
        std::thread t2(&verbose, "verbose");
        t2.detach();
        std::thread t3(&info, "info");
        t3.detach();
        std::thread t4(&warning, "warning");
        t4.detach();
        std::thread t5(&error, "error");
        t5.detach();
        std::thread t6(&critical, "critical");
        t6.detach();
    }
    return 0;
}

