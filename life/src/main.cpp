#include "Field.h"
#include "Generator.h"
#include "Stepper.h"
#include "Visualizer.h"

#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage:\n"
                  << "  life generate OUTPUT_FILE WIDTH HEIGHT PROBABILITY\n"
                  << "  life step INPUT_FILE NUM_THREADS [OUTPUT_FILE]\n"
                  << "  life visualize INPUT_FILE NUM_THREADS\n";
        return 1;
    }

    try
    {
        std::string command = argv[1];

        if (command == "generate")
        {
            if (argc != 6)
            {
                std::cerr << "Usage: life generate OUTPUT_FILE WIDTH HEIGHT "
                             "PROBABILITY\n";
                return 1;
            }
            std::string outputFile = argv[2];
            int width = std::stoi(argv[3]);
            int height = std::stoi(argv[4]);
            double probability = std::stod(argv[5]);

            Field field = Generator::generate(width, height, probability);
            field.save(outputFile);
        }
        else if (command == "step")
        {
            if (argc < 4 || argc > 5)
            {
                std::cerr << "Usage: life step INPUT_FILE NUM_THREADS "
                             "[OUTPUT_FILE]\n";
                return 1;
            }
            std::string inputFile = argv[2];
            int numThreads = std::stoi(argv[3]);
            std::string outputFile = (argc == 5) ? argv[4] : inputFile;

            Field field = Field::load(inputFile);
            auto [nextField, ms] = Stepper::step(field, numThreads);
            nextField.save(outputFile);

            std::cout << ms << " ms\n";
        }
        else if (command == "visualize")
        {
            if (argc != 4)
            {
                std::cerr << "Usage: life visualize INPUT_FILE NUM_THREADS\n";
                return 1;
            }
            std::string inputFile = argv[2];
            int numThreads = std::stoi(argv[3]);

            Field field = Field::load(inputFile);
            Visualizer::run(std::move(field), numThreads);
        }
        else
        {
            std::cerr << "Unknown command: " << command << "\n";
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
