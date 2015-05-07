//
// Super simple log server, based on ZeroMQ, written in C++
// Hello World server in C++
// Binds REP socket to tcp://*:5555
//

// std::map
#include <map>
// std::string
#include <string>
// std::cout
#include <iostream>
// std::ofstream
#include <fstream>
// zmq::context_t zmq::socket_t zmq::message_t
#include <zmq.hpp>
// Json::Reader Json::Value
#include "json/json.h"
// Command line arguments
#include <tclap/CmdLine.h>
// Interrupt signal handling
#include <signal.h>


zmq::context_t context(1);
zmq::socket_t socket(context, ZMQ_REP);
zmq::message_t req;
zmq::message_t rep(2);

Json::Reader reader;
Json::Value msg_;

std::ofstream logfile;

// Interrupt signal handler
static int s_interrupted = 0;
static void s_signal_handler (int signal_value)
{
    s_interrupted = 1;
}


std::string recv() {
    socket.recv(&req);
    return std::string(static_cast<char*>(req.data()), req.size());
}


static void s_catch_signals (void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
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
        logfile << logLevels[msg_["level"].asInt()] << " :: "
            << msg_["dt"].asString()
            << " :: " << msg_["pid"].asString() << " :: "
            << msg_["thrNum"] << " :: "
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
    log_(">>> Start logging <<<");
    std::cout << "Listening on port " << port << "\n";

    s_catch_signals();
    while (true) {
        try {
            std::string req = recv();
            reply();
            log_(req);
        }
        catch(zmq::error_t& e) {
            logfile << "E :: interrupt received, proceeding..." << "\n";
        }
        if (s_interrupted) {
            logfile << "E :: interrupt received, killing server..." <<
                "\n" << ">>> Stop logging <<<" << "\n";
            std::cout << "interrupt received, killing server..." << "\n";
            break;
        }
    }

    logfile.close();
    return 0;
}

