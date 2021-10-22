#ifndef CUSTOM_INTERPRETER
#define CUSTOM_INTERPRETER

#include "xeus/xinterpreter.hpp"

#include "nlohmann/json.hpp"

using xeus::xinterpreter;

namespace nl = nlohmann;

namespace custom
{
    class custom_interpreter : public xinterpreter
    {

    public:

        custom_interpreter() = default;
        virtual ~custom_interpreter() = default;
	void set_language(int gemoc_port);

    private:
        int gemoc_port;

        void configure_impl() override;

        void send_gemoc_request(std::string endpoint,
			        const std::string& message,
				struct string* response_body,
	                        long* response_code);

        nl::json execute_request_impl(int execution_counter,
                                      const std::string& code,
                                      bool silent,
                                      bool store_history,
                                      nl::json user_expressions,
                                      bool allow_stdin) override;

        nl::json complete_request_impl(const std::string& code,
                                       int cursor_pos) override;

        nl::json inspect_request_impl(const std::string& code,
                                      int cursor_pos,
                                      int detail_level) override;

        nl::json is_complete_request_impl(const std::string& code) override;

        nl::json kernel_info_request_impl() override;

        void shutdown_request_impl() override;
    };
}

#endif
