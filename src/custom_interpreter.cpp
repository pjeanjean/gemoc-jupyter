#include <iostream>
#include <curl/curl.h>
#include "custom_interpreter.hpp"

namespace custom
{
    CURL *curl;

    void custom_interpreter::set_language(int gemoc_port) {
	this->gemoc_port = gemoc_port;
        curl_global_init(CURL_GLOBAL_ALL);
    }

    struct string {
        char *ptr;
        size_t len;
    };

    void init_string(struct string *s) {
        s->len = 0;
        s->ptr = (char*) malloc(s->len+1);
        s->ptr[0] = '\0';
    }

    size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
        size_t new_len = s->len + size*nmemb;
        s->ptr = (char*) realloc(s->ptr, new_len+1);
        memcpy(s->ptr+s->len, ptr, size*nmemb);
        s->ptr[new_len] = '\0';
        s->len = new_len;
        return size*nmemb;
    }

    size_t readfunc(void *ptr, size_t size, size_t nmemb, void *userdata) {
	std::string* str = (std::string*) userdata;
	size_t read_size;
	if (str->size() < size*nmemb) {
            memcpy(ptr, str->c_str(), str->size());
	    read_size = str->size();
	} else {
            memcpy(ptr, str->c_str(), size*nmemb);
	    read_size = size*nmemb;
	}
        return read_size;
    }

    void custom_interpreter::send_gemoc_request(std::string endpoint,
		                                const std::string& message,
						struct string* response_body,
						long* response_code) {
	curl = curl_easy_init();

	char url[30];
	sprintf(url, "127.0.0.1:%d/%s", this->gemoc_port, endpoint.c_str());
	curl_easy_setopt(curl, CURLOPT_URL, url);

	curl_easy_setopt(curl, CURLOPT_PUT, 1L);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_READDATA, &message);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, readfunc);
	curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) message.size());

        init_string(response_body);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_body);
	
	curl_easy_perform(curl);

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, response_code);
	
	curl_easy_cleanup(curl);
    }

    nl::json custom_interpreter::execute_request_impl(int execution_counter, // Typically the cell number
                                                      const std::string& code, // Code to execute
                                                      bool /*silent*/,
                                                      bool /*store_history*/,
                                                      nl::json /*user_expressions*/,
                                                      bool /*allow_stdin*/)
    {
	struct string response_body;
        long response_code;
	
	this->send_gemoc_request("interpret", code, &response_body, &response_code);
	
	if (response_code == 400) {
            publish_execution_error("Error", std::string(response_body.ptr), {});
	} else {
            nl::json pub_data;
	    //*
	    std::string str_data = std::string(response_body.ptr);
	    std::string mimetype = str_data.substr(0, str_data.find("\n"));
	    pub_data[mimetype] = str_data.substr(str_data.find("\n") + 1);
	    //*/ pub_data["text/plain"] = std::string(response_body.ptr);
            publish_execution_result(execution_counter, std::move(pub_data), nl::json());
	}

        nl::json result;
        result["status"] = "ok";
        return result;
    }

    void custom_interpreter::configure_impl()
    {
        // Perform some operations
    }

    std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    nl::json custom_interpreter::complete_request_impl(const std::string& code,
                                                       int cursor_pos)
    {
	struct string response_body;
        long response_code;

        std::string message = std::to_string(cursor_pos) + "|||" + code;
	
	this->send_gemoc_request("complete", message, &response_body, &response_code);

        std::string received = std::string(response_body.ptr);

        nl::json result;

	if (received.size() == 0) {
            result["status"] = "ok";
            result["matches"] = {};
            result["cursor_start"] = cursor_pos;
            result["cursor_end"] = cursor_pos;
        } else {
	    std::vector<std::string> lines = split(received, '\n');
	    result["matches"][lines.size()/3];
	    for (long unsigned int i = 0; i < lines.size() / 3; i++) {
	        result["matches"][i] = lines.at(i * 3);
	        if (i == 0) {
                    result["cursor_start"] = std::stoi(lines.at(i+1));
                    result["cursor_end"] = cursor_pos;
		}
	    }
            result["status"] = "ok";
	}

        return result;
    }

    nl::json custom_interpreter::inspect_request_impl(const std::string& code,
                                                      int /*cursor_pos*/,
                                                      int /*detail_level*/)
    {
        nl::json result;

        if (code.compare("print") == 0)
        {
            result["found"] = true;
            result["text/plain"] = "Print objects to the text stream file, [...]";
        }
        else
        {
            result["found"] = false;
        }

        result["status"] = "ok";
        return result;
    }

    nl::json custom_interpreter::is_complete_request_impl(const std::string& /*code*/)
    {
        nl::json result;

        // if (is_complete(code))
        // {
            result["status"] = "complete";
        // }
        // else
        // {
        //    result["status"] = "incomplete";
        //    result["indent"] = 4;
        //}

        return result;
    }

    nl::json custom_interpreter::kernel_info_request_impl()
    {
        nl::json result;
        result["implementation"] = "GEMOC Kernel";
        result["implementation_version"] = "0.1.0";
        result["language_info"]["name"] = "gemoc";
        result["language_info"]["mimetype"] = "text/x-gemoc";
        result["language_info"]["file_extension"] = ".xmi";
        //result["language_info"]["name"] = "python";
        //result["language_info"]["version"] = "3.7";
        //result["language_info"]["mimetype"] = "text/x-python";
        //result["language_info"]["file_extension"] = ".py";
        return result;
    }

    void custom_interpreter::shutdown_request_impl() {
	std::string message;
	long response_code;
	struct string response_body;
	this->send_gemoc_request("reset", message, &response_body, &response_code);
    }

}
