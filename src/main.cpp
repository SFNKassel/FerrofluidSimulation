#include "args.h"
#include "simulation.h"

int main(int argc, char ** argv) {
    args::ArgumentParser parser("Ferrofluid simulation", "");
    args::HelpFlag       help(parser, "help", "Display this help menu",
                        {'h', "help"});
    args::ValueFlag<std::string> output(parser, "output", "output path", {'o'});
    args::ValueFlag<std::string> jarfile(parser, "jarfile", "jarfile path",
                                         {'j'});
    try {
        parser.ParseCLI(argc, argv);
    } catch(args::Help) {
        std::cout << parser;
        return 0;
    } catch(args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    std::string out;
    std::string jfile;
    if(!output) {
        out = "";
    } else {
        out = args::get(output);
    }

    if(!jarfile) {
        jfile = "";
    } else {
        jfile = args::get(jarfile);
    }

    Simulation s(80, 500, 100, 0.05, 1, 0, 0, 72.75, 0.0001, 0.01, 0.001,
                 Vec3(0, 0, 0), 500, out, jfile);
}
