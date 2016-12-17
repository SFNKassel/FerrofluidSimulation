#include "CommandLineOptions.h"
#include "simulation.h"

int main(int argc, char ** argv) {
    CommandLineOptions ops(argc, argv);
    Simulation s(80, 500, 100, 0.05, 1, 0, 0, 72.75, 0.0001, 0.01, 0.001, Vec3(0, 0, 0),
         500, ops.output(), ops.jarfile());
}
