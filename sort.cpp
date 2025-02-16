#include <iostream>
#include <climits>
#include "ArgumentParser.h"

int main(int argc, char* argv[]) {
    std::cout << "Hello world";

    ArgumentParser args = ArgumentParser(
        argc, argv, 
        { ArgumentSpec::forString("output-path", { "op" }, "Path to output file. Default is ./output.txt", "./output.txt"),
          ArgumentSpec::forInt("size", { "s" }, "Number of random numbers to generate. Default is 100", 100),
          ArgumentSpec::forInt("min-value", { "m" }, "Minimal value for each random element. Default is zero", 0),
          ArgumentSpec::forInt("max-value", { "M" }, "Maximal value for each random element. Default is 32-bit INT_MAX", INT_MAX),
          ArgumentSpec::forBool("verbose", { "v" }, "Enable verbose mode") });

    std::cout << "path=" << args.get_string("output-path") << " s=" << args.get_int("size");
    std::cout << " m=" << args.get_int("min-value") << " M=" << args.get_int("max-value");
    std::cout << " v=" << args.get_bool("verbose") << endl;

}
