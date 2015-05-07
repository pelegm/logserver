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
// Command line arguments
#include <tclap/CmdLine.h>


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


void log_(char const * msg) {
    logfile << msg << "\n";
    logfile.flush();
}


int main(int argc, const char * argv[]) {
    // ----- Command line arguments via TCLAP ----- //
    // The constructors arguments are message, delimiter and version
    TCLAP::CmdLine cmd("LOG SERVER", ' ', "0.1"); 

    // The -V argument is for "print version and exit"
    // ValueArg signature:
    // - const std::string &flag
    // - const std::string &name
    // - const std::string &desc
    // - bool req
    // - T value
    // - const std::string &typeDesc
    // - CmdLineInterface &parser

    // The -p/--port argument is for the listening port
    TCLAP::ValueArg<std::string> portArg("p", "port", "port", false, "5555",
            "####");

    // The -o/--output argument is for the location of the logfile
    TCLAP::ValueArg<std::string> outputArg("o", "output", "output", true, "",
            "xxx.log");

    // Add arguments
    // We do it here rather in the constructors, in order to control the order
    // in the help text, which is the reverse of the following
    cmd.add(outputArg);
    cmd.add(portArg);

    // Parse command line arguments, and fetch the rest from Redis
    cmd.parse(argc, argv);
    std::string port = portArg.getValue();
    std::string output = outputArg.getValue();

    socket.bind(("tcp://*:" + port).c_str());

    // Append to an existing log file
    logfile.open(output.c_str(), std::fstream::out | std::fstream::app);

    // Log start
    log_("Start logging...");

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

