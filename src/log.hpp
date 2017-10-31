#pragma once

/*! \def VariableData(x)
 * 
 * \brief A convenience macro for printing debugging information
 * 
 * Can be used to easily print pretty debug information. For example
 * 
 * `int val = 32; Log << VariableData(val) << Message();`
 * 
 * will print
 * 
 * `val = 32`
 */

#define VariableData(x) "" #x " = " << x << " "
#include <iostream>
#include <fstream>
#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#include "hash.hpp"


struct Message
{
};

struct Trace
{
    Hash type;
    Trace(Hash h) : type(h) {}
};

/*! \brief The common application logger
 * 
 * Used for logging from common application flow to failure states and errors.
 * Logger uses stream operators for logging. The level of logging is selected
 * by either Message or Trace specifiers. Message is for common standard
 * output while Trace fits more specific situations.
 * 
 * 
 * The Message specifier is used to output general application results:
 * 
 * `Log << (42 * 21) << Message();`
 * 
 * The Trace outputs more specific messages, with a logging group specifier
 * 
 * `Log << "Feature not supported" << Trace(CHash("Warning"));`
 * 
 * Message specifier always outputs to the log, while Trace requires the the
 * specific trace is enabled in the Logger via enableTrace or enableAllTraces
 * methods.
 */
class Logger
{
    std::ofstream infolog;
    std::function<void(const std::string&)> lineOutput;
    std::stringstream lineStream;

    std::unordered_map<Hash, std::string> enabledTraces;
    bool allTracesEnabled = false;
public:

    /*! \brief Enables or disables all Trace log outputs.
     * 
     * The traces enabled via this function will be prefixed with the Hash
     * number of the Trace.
     * 
     * \param enable if true enable all traces, otherwise disable all traces
     */
    void enableAllTraces(bool enable = true)
    {
        allTracesEnabled = enable;
    }

    /*! \brief Enables a specific Trace with a prefix
     *
     * \param h the hash number of the Trace
     * \param name the prefix to use for the Trace
     */
    void enableTrace(Hash h, const std::string& name)
    {
        enabledTraces[h] = name;
    }

    //! Sets a log line output handler
    void setOutput(std::function<void(const std::string&)> lp)
    {
        lineOutput = lp;
    }

    Logger()
    {
        infolog.open("info.log");
    }

    Logger& operator << (Message t)
    {
        std::string outp;
        outp = lineStream.str();
        lineStream.str("");
        lineStream.clear();

        std::cout << outp << std::endl;
        infolog << outp << std::endl;

        if (lineOutput)
            lineOutput(outp);

        return *this;
    }

    Logger& operator << (Trace t)
    {

        std::string outp;
        outp = lineStream.str();
        lineStream.str("");
        lineStream.clear();



        auto it = enabledTraces.find(t.type);

        if (it != enabledTraces.end())
        {
            outp = it->second + outp;
            std::cout << outp << std::endl;
            infolog << outp <<  std::endl;

            if (lineOutput)
                lineOutput(outp);

        }
        else if (allTracesEnabled)
        {
            std::stringstream ss;
            ss << "Trace " << t.type << ": " << outp;
            outp = ss.str();
            std::cout << outp << std::endl;
            infolog << outp << std::endl;

            if (lineOutput)
                lineOutput(outp);
        }
        return *this;
    }


    template <typename T>
    Logger& operator << (const T& t)
    {
        lineStream << t;
        return *this;
    }
};


class ErrorLogger
{
    std::ofstream errlog;
public:
    ErrorLogger()
    {
        errlog.open("error.log",std::ofstream::app);
        errlog << "-------------------" << std::endl;
        std::time_t t = std::time(nullptr);
        struct std::tm *l = std::localtime(&t);
        errlog << l->tm_mday << " " << l->tm_mon+1 << " " << l->tm_year+1900 << " - " << std::setw(2) << l->tm_hour << ":" << std::setw(2) << l->tm_min << std::endl;
    }
    ErrorLogger& operator << (Message t)
    {
        std::cerr << std::endl;
        errlog << std::endl;
        return *this;
    }
    template <typename T>
    ErrorLogger& operator << (const T& t)
    {
        std::cerr << t;
        errlog << t;
        return *this;
    }
};


extern Logger Log;
extern ErrorLogger LogError;
