#ifndef FEFFECT_H
#define FEFFECT_H

namespace JWCEssentials {
    struct simple_kv {
        const char* key;
        const char* value;
    };

    extern simple_kv feffect_entries[];

    _EXPORT_ utf8_string_struct_array feffect_list();
    _EXPORT_ utf8_string_struct feffect_code(utf8_string_struct name);
    _EXPORT_ utf8_string_struct feffect_name(utf8_string_struct code);
    _EXPORT_ utf8_string_struct feffect(utf8_string_struct command, utf8_string_struct escape);

    struct basic_token {
        utf8_string_struct type;
        utf8_string_struct value;
    };

    struct string_cursor {
        std::string str;
        size_t length;
        size_t position;

        string_cursor(const std::string& string);
        size_t remain() const;
        bool end() const;
        void whitespace_advance();
        char& c();
        void advance(size_t count = 1);
    };

    class feffect_processor {
    public:
        struct feffect_stack_data {
            std::string escape_accum;
            std::string result_accum;
            bool is_paren = false;
            std::vector<std::string> command_accum;
            std::vector<std::string> command_close;

            void cycle_command_close(feffect_processor *processor);
            bool cycle_command_accum(feffect_processor *processor);

            void command_close_transfer(feffect_processor *processor, feffect_stack_data &target);

        };

        std::vector<std::string> fg_stack;
        std::vector<std::string> bg_stack;

        feffect_processor();
        utf8_string_struct process(utf8_string_struct command, utf8_string_struct escape);

    private:
        std::string transit(std::string command, bool direction);

        utf8_string_struct escape = "\\033";

        std::vector<feffect_stack_data> pstack;
        string_cursor cursor;

        bool lower(char c) const;
        bool upper(char c) const;
        bool alpha(char c) const;
        bool numer(char c) const;
        bool id_char(char c) const;
    };

} // namespace JWCEssentials

#endif // FEFFECT_H
