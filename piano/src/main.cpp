#include "Piano.h"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        Piano piano;
        piano.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";

        return 1;
    }

    return 0;
}
