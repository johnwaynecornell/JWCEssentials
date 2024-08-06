#include <cstring>

#include "JWCEssentials/JWCEssentials.h"

namespace JWCEssentials {

    simple_kv feffect_entries[] = {
        {"reset", "0"},
        {"bold", "1"},
        {"italic", "3"},
        {"underline", "4"},
        {"blink", "5"},
        {"reverse", "7"},
        {"crossed", "9"},
        {"font0", "10"},
        {"font1", "11"},
        {"font2", "12"},
        {"font3", "13"},
        {"font4", "14"},
        {"font5", "15"},
        {"font6", "16"},
        {"font7", "17"},
        {"font8", "18"},
        {"font9", "19"},
        {"bold_off", "22"},
        {"italic_off", "23"},
        {"underline_off", "24"},
        {"blink_off", "25"},
        {"reverse_off", "27"},
        {"crossed_off", "29"},
        {"reveal", "28"},
        {"fg_black", "30"},
        {"fg_red", "31"},
        {"fg_green", "32"},
        {"fg_yellow", "33"},
        {"fg_blue", "34"},
        {"fg_magenta", "35"},
        {"fg_cyan", "36"},
        {"fg_white", "37"},
        {"fg_set", "38"},
        {"fg_default", "39"},
        {"bg_black", "40"},
        {"bg_red", "41"},
        {"bg_green", "42"},
        {"bg_yellow", "43"},
        {"bg_blue", "44"},
        {"bg_magenta", "45"},
        {"bg_cyan", "46"},
        {"bg_white", "47"},
        {"bg_set", "48"},
        {"bg_default", "49"},
        {"framed", "51"},
        {"encircled", "52"},
        {"overlined", "53"},
        {"framed_off", "54"},
        {"encircled_off", "54"},
        {"overlined_off", "55"},
        {nullptr, nullptr}
    };

    utf8_string_struct_array feffect_list() {
        int count = 0;
        while (feffect_entries[count].key) count++;

        utf8_string_struct_array R;
        R.Alloc(count);

        for (int i = 0; i < count; i++) {
            R[i] = feffect_entries[i].key;
        }

        return R;
    }

    utf8_string_struct feffect_code(utf8_string_struct name) {
        for (int i = 0; feffect_entries[i].key != nullptr; i++) {
            if (strcmp(feffect_entries[i].key, name) == 0)
                return feffect_entries[i].value;
        }
        return nullptr;
    }

    utf8_string_struct feffect_name(utf8_string_struct code) {
        for (int i = 0; feffect_entries[i].key != nullptr; i++) {
            if (strcmp(feffect_entries[i].value, code) == 0)
                return feffect_entries[i].key;
        }
        return nullptr;
    }

    string_cursor::string_cursor(const std::string& string)
        : str(string), length(string.length()), position(0) {}

    size_t string_cursor::remain() const {
        return length - position;
    }

    bool string_cursor::end() const {
        return position >= length;
    }

    void string_cursor::whitespace_advance() {
        while (!end() && c() <= ' ') advance();
    }

    char& string_cursor::c() {
        return str[position];
    }

    void string_cursor::advance(size_t count) {
        position += count;
    }

    feffect_processor::feffect_processor() : cursor("") {}

    utf8_string_struct feffect_processor::process(utf8_string_struct command, utf8_string_struct escape) {
        pstack.push_back({});

        if (escape == nullptr) escape = "\033";

        cursor = {command.c_str};

        this->escape = escape;

        fg_stack.push_back("fg_default");
        bg_stack.push_back("bg_default");

        while (!pstack.empty()) {
            feffect_stack_data cur = pstack.back();
            pstack.pop_back();

            bool last_was_identifier = false;
            bool expect_identifier = false;

            while (!cursor.end()) {
                cursor.whitespace_advance();

                if (cursor.c() == '_' || alpha(cursor.c())) {
                    std::string identifier;
                    while (!cursor.end() && id_char(cursor.c())) {
                        identifier += cursor.c();
                        cursor.advance();
                    }

                    last_was_identifier = true;

                    if (!expect_identifier) {
                        if (!cur.cycle_command_accum(this)) return nullptr;
                    } else {
                        expect_identifier = false;
                    }

                    if (identifier.empty()) {
                        std::cerr << "developer error" << std::endl;
                        return nullptr;
                    }

                    cur.command_accum.push_back(identifier);

                } else {
                    if (expect_identifier) {
                        std::cerr << "expected identifier after '.'" << std::endl;
                        return nullptr;
                    }

                    if (cursor.c() == '.') {
                        if (!last_was_identifier) {
                            std::cerr << "expected identifier before '.'" << std::endl;
                            return nullptr;
                        }

                        expect_identifier = true;
                        cursor.advance();
                    } else if (cursor.c() == '\'' || cursor.c() == '"') {
                        char end_quote = cursor.c();
                        cursor.advance();

                        std::string text;
                        bool escape = false;
                        bool found_end_quote = false;

                        while (!cursor.end()) {
                            if (!escape) {
                                if (cursor.c() == end_quote) {
                                    if (!cur.cycle_command_accum(this)) return nullptr;
                                    result_write(text);
                                    cursor.advance();
                                    found_end_quote = true;
                                    break;
                                }
                                if (cursor.c() == '\\') {
                                    escape = true;
                                    cursor.advance();
                                    continue;
                                }
                            }

                            escape = false;
                            text += cursor.c();
                            cursor.advance();
                        }

                        if (!found_end_quote) {
                            std::cerr << "unterminated quotation" << std::endl;
                            return nullptr;
                        }

                    } else if (cursor.c() == '(') {
                        cursor.advance();
                        feffect_stack_data d = {};
                        for (const auto& t : cur.command_accum) {
                            d.command_close.push_back(t);
                        }

                        if (!cur.cycle_command_accum(this)) return nullptr;
                        d.is_paren = true;

                        pstack.push_back(cur);
                        pstack.push_back(d);
                        goto __continue;

                    } else if (cursor.c() == ')') {
                        if (!cur.is_paren) {
                            std::cerr << "unexpected close paren" << std::endl;
                            return nullptr;
                        }

                        if (!cur.cycle_command_accum(this)) return nullptr;

                        cur.cycle_command_close(this);
                        /*
                        if (!pstack.empty()) {
                            //pstack.back().result_accum += cur.result_accum;
                            cur.command_close_transfer(this, pstack.back());
                        } else {
                            cur.cycle_command_close(this);
                        }*/
                        cursor.advance();

                        last_was_identifier = false;
                        break;
                    } else {
                        std::cerr << "unexpected character: " << cursor.c() << std::endl;
                        return nullptr;
                    }

                    last_was_identifier = false;
                }
            }

            __continue:

            if (pstack.empty()) {
                cur.cycle_command_accum(this);
                escape_flush();
                return result_accum.c_str();
            }
        }

        return nullptr;
    }

    void feffect_processor::escape_flush() {
        if (escape_accum.empty()) return;
        result_accum += escape;
        result_accum += "[";

        bool first = true;
        for (int i = escape_accum.size() - 1; i >= 0; --i) {
            std::string close = escape_accum[i];

            utf8_string_struct code = feffect_code(escape_accum[i].c_str());;

            if (code) {
                if (!first) result_accum += ";";
                result_accum += code;
                first = false;
            }  else {
                std::cerr << "code not found" << std::endl;
                throw std::runtime_error("developer error");
            }
        }

        result_accum += "m";
        escape_accum.clear();
    }
    void feffect_processor::result_write(std::string string) {
        escape_flush();
        result_accum += string;
    }

    void feffect_processor::escape_put(std::string string) {
        utf8_string_struct code = feffect_code(string.c_str());;

        if (!code) {
            std::cerr << "code not found" << std::endl;
            throw std::runtime_error("developer error");
        }
        escape_accum.push_back(string);
    }

    utf8_string_struct feffect(utf8_string_struct command, utf8_string_struct escape) {
        feffect_processor fp;
        return fp.process(command, escape);
    }

    std::string feffect_processor::transit(std::string command, bool direction) {
        bool inverted = false;

        if (command.size() >= 8 && command.substr(command.length() - 8) == "_off._off")
            command = command.substr(0, command.length()-8);

        if (command.size() >= 4 && command.substr(command.length() - 4) == "_off") inverted = true;

        char type = '_';

        if (command.size() >= 3 && command.substr(0, 3) == "fg_") type = 'f';
        else if (command.size() >= 3 && command.substr(0, 3) == "bg_") type = 'b';

        if (inverted ^ direction) {
            if (type == 'f') fg_stack.push_back(command);
            if (type == 'b') bg_stack.push_back(command);

            //if (inverted) return command.substr(0, command.length() - 4);

            return command;
        } else {
            bool popped = false;
            if (type == 'f') {
                fg_stack.pop_back();
                command = fg_stack.back();
                popped = true;
            } else if (type == 'b') {
                bg_stack.pop_back();
                command = bg_stack.back();
                popped = true;
            }

            if (!popped) {
                if (command.size() >= 4 && command.substr(command.size() - 4) != "_off") {
                    //command += "_off";
                } else {
                    command = command.substr(0, command.size() - 4);
                }
            }

            //if (inverted) return command.substr(0, command.length() - 4);

            //if (inverted) return command.substr(0, command.length() - 4);
            return command;
        }
    }


    bool feffect_processor::feffect_stack_data::cycle_command_accum(feffect_processor* processor) {
        if (command_accum.empty()) return true;

        for (size_t i = 0; i < command_accum.size(); ++i) {
            if (command_accum[i].empty()) {
                std::cerr << "developer error" << std::endl;
                return false;
            }

            std::string final = processor->transit(command_accum[i], 1);
            processor->escape_put(final);
        }

        command_accum.clear();

        return true;
    }

    void feffect_processor::feffect_stack_data::cycle_command_close(feffect_processor* processor) {
        if (command_close.empty()) return;

        for (int i = command_close.size() - 1; i >= 0; --i) {
            std::string close = command_close[i];

            std::string final = processor->transit(close, false);;
            processor->escape_put(final);
        }

        command_close.clear();
    }

    void feffect_processor::feffect_stack_data::command_close_transfer(feffect_processor* processor, feffect_processor::feffect_stack_data& target) {
        if (command_close.empty()) return;

        for (int i = command_close.size() - 1; i >= 0; --i) {
            std::string command = command_close[i];

            if (command.empty()) {
                std::cerr << "developer error" << std::endl;
                throw std::runtime_error("developer error");
            }

            std::string close = command_close[i];

            std::string cmd = close;

            if (cmd.size() <= 4 || cmd.substr(cmd.size() - 4) != "_off") {
                cmd += "_off";
            } else {
                cmd = cmd.substr(0, cmd.size() - 4);
            }

            if (cmd.empty()) {
                std::cerr << "developer trace" << std::endl;
                __asm("int3");
            }

            if (!cmd.empty()) target.command_accum.push_back(cmd);
        }
        command_close.clear();
    }

    bool feffect_processor::lower(char c) const {
        return c >= 'a' && c <= 'z';
    }

    bool feffect_processor::upper(char c) const {
        return c >= 'A' && c <= 'Z';
    }

    bool feffect_processor::alpha(char c) const {
        return lower(c) || upper(c);
    }

    bool feffect_processor::numer(char c) const {
        return c >= '0' && c <= '9';
    }

    bool feffect_processor::id_char(char c) const {
        return c == '_' || alpha(c) || numer(c);
    }

} // namespace JWCEssentials
