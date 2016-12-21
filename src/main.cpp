#include "FerroSim.h"
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

    std::string out   = "";
    std::string jfile = "";
    if(output) { out = args::get(output); }

    if(jarfile) { jfile = args::get(jarfile); }

    if(out.size() > 0) {
        if(out[out.size() - 1] != '/') { out = out + "/"; }
    }

    /*
     * Simulation s(80, 500, 100, 0.05, 1, 0, 0, 72.75, 0.0001, 0.01, 0.001,
     *              Vec3(0, 0, 0), 500, out, jfile);
     */

    string timestamp = to_string(time(0) + 1);
    sim f(1000, 10000, 100, [&](vector<Vec3> & positions) {
        FILE * file = fopen((out + timestamp + ".sim").c_str(), "ab");
        fwrite(positions.data(), sizeof(Vec3), positions.size(), file);
        fclose(file);
    });
    f.write_metadata(out + timestamp + ".sim.metadata", timestamp);
    f.simulate();

    cout << (out + timestamp + ".sim").c_str() << endl;
    system((std::string("java -jar ") + jfile + " " + out).c_str());
}
