#include <memory>

#include "xeus/xkernel.hpp"
#include "xeus/xkernel_configuration.hpp"

#include "custom_interpreter.hpp"

int main(int argc, char* argv[])
{
    // Load configuration file
    std::string file_name = (argc == 1) ? "connection.json" : argv[2];
    xeus::xconfiguration config = xeus::load_configuration(file_name);

    // Create interpreter instance
    using interpreter_ptr = std::unique_ptr<custom::custom_interpreter>;
    custom::custom_interpreter* c_interpreter = new custom::custom_interpreter();
    interpreter_ptr interpreter = interpreter_ptr(c_interpreter);

    // Configure Gemoc language
    c_interpreter->set_language(atoi(argv[3]));

    // Create kernel instance and start it
    xeus::xkernel kernel(config, xeus::get_user_name(), std::move(interpreter));
    kernel.start();

    return 0;
}
