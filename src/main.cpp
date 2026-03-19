#include "app/application.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        bytedeck::Application app;
        return app.run(argc, argv);
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        return 1;
    }
}
