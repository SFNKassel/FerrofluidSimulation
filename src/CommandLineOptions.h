#ifndef _COMMANDLINEOPTIONS_H
#define _COMMANDLINEOPTIONS_H

#include <argp.h>
#include <string>

#include <iostream>

#include "config.h"

class CommandLineOptions {
public:
    CommandLineOptions(int argc, char ** argv) {
        argp_parse(&argp, argc, argv, 0, 0, &args);
    }

    std::string jarfile() { return args.jarfile; }
    std::string output() { return args.output; }

private:
    // documentation of the program
    std::string doc = "Ferrofluid -- Ferrofluid simulations\v";

    // documentation of non option arguments (there are none)
    char args_doc[255] = "";

    // supported options (no custom options for now)
    struct argp_option options[3] = {
        {"jarfile", 'j', "PATH", 0, "jarfile path"},
        {"output", 'o', "PATH", 0, "output path"},
        {0}};

    // struct for holding the argument info
    struct arguments {
        std::string jarfile = "BinToHRF.jar";
        std::string output = "";
    };

    // parsing of a single argument
    static error_t parse_opt(int key, char * arg, struct argp_state * state) {

        // get the struct for saving the arguments
        arguments * arguments = (struct arguments *)state->input;

        switch(key) {
        case 'j':
            arguments->jarfile = arg;
            break;
        case 'o':
            arguments->output = arg;
            break;
        case ARGP_KEY_ARG: /* non option argument */
            // check if more than all needed arguments are passed (and abort if
            // so)
            argp_usage(state);

            break;
        /*
                case ARGP_KEY_END:  end of arguments
                                   // check for too few non option arguments
                    break;
                    */

        default:
            return ARGP_ERR_UNKNOWN;
        }
        return 0;
    }

    // argp parser definiton
    struct argp argp = {options, parse_opt, args_doc, doc.c_str()};

    arguments args;
};

#endif
