//
// Super simple log server, based on ZeroMQ, written in C++
// Hello World server in C++
// Binds REP socket to tcp://*:5555
//

// std::map
#include <map>
// std::string
#include <string>
// std::ofstream
#include <fstream>
// zmq::context_t zmq::socket_t zmq::message_t
#include <zmq.hpp>
// Json::Reader Json::Value
#include "json/json.h"


zmq::context_t context(1);
zmq::socket_t socket(context, ZMQ_REP);
zmq::message_t req;
zmq::message_t rep(2);

Json::Reader reader;
Json::Value msg_;

std::ofstream logfile;


std::string recv() {
    socket.recv(&req);
    return std::string(static_cast<char*>(req.data()), req.size());
}


void reply() {
    memcpy ((void *) rep.data(), "OK", 2);
    socket.send(rep);
}


// Log levels
std::map<int, char> logLevels {
    {1, 'D'}, {2, 'V'}, {3, 'I'}, {4, 'W'}, {5, 'E'}, {6, 'C'}
};


void log_(std::string json) {
    bool parsingSuccessful = reader.parse(json, msg_);
    if (parsingSuccessful) {
        logfile << logLevels[msg_["level"].asInt()] << "::"
            << msg_["dt"].asString()
            << "::" << msg_["pid"].asString() << "::"
            << msg_["thrNum"] << "::"
            << msg_["text"].asString() << "\n";
    } else {
        logfile << "-----";
    }

    logfile.flush();
}


int main() {
    socket.bind("tcp://*:5555");

    // TODO: make this a command line argument
    logfile.open("test.log");

    // TODO: handle ctrl-c properly
    // http://zguide.zeromq.org/php:chapter2#Handling-Interrupt-Signals
    while (true) {
        std::string req = recv();
        reply();
        log_(req);
    }

    logfile.close();
    return 0;
}

