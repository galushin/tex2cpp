#include "tex2cpp.h"

#include <iostream>
#include <fstream>

int main()
{
    std::ifstream file("source.txt");

    std::string src;
    std::getline(file, src);

    std::cout << "Source:\n" << src << "\n";
    std::cout << "------------------\n";

    std::cout << tex2cpp(src) << "\n";

    return 0;
}
