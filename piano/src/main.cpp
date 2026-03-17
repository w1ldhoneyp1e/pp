#include "PianoController.h"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        PianoController controller;
        controller.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";

        return 1;
    }

    return 0;
}
