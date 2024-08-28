#include <cstring>
#include <utility>

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

        {"fg_bright_black", "90"},
        {"fg_bright_red", "91"},
        {"fg_bright_green", "92"},
        {"fg_bright_yellow", "93"},
        {"fg_bright_blue", "94"},
        {"fg_bright_magenta", "95"},
        {"fg_bright_cyan", "96"},
        {"fg_bright_white", "97"},

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

        {"bg_bright_black", "100"},
        {"bg_bright_red", "101"},
        {"bg_bright_green", "102"},
        {"bg_bright_yellow", "103"},
        {"bg_bright_blue", "104"},
        {"bg_bright_magenta", "105"},
        {"bg_bright_cyan", "106"},
        {"bg_bright_white", "107"},

        {"overline", "53"},
        {"overline_off", "55"},
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

    feffect_processor::feffect_processor() : cursor("") {
        std::vector<std::string> binaries = {"bold","italic", "underline","blink",
            "reverse", "crossed", "overline"};

        stacks.Alloc(2+binaries.size());

        stacks[0] = {"fg"};
        stacks[0].stack.push_back("fg_default");
        stacks[0].stack.push_back("fg_default");

        stacks[1] = { "bg" };
        stacks[1].stack.push_back("bg_default");
        stacks[1].stack.push_back("bg_default");

        for (int i=0; i < binaries.size(); i++) {
            stacks[2+i] = { binaries[i] };
            stacks[2+i].stack.push_back(((std::string) binaries[i]));
            stacks[2+i].stack.push_back(((std::string) binaries[i])+"_off");
        }
    }

    bool feffect_processor::has_stack(std::string identifier) {
        for (int i=0; i < stacks.length; i++) {
            if (stacks[i].identifier == identifier) return true;
        }
        return false;
    }

    std::vector<std::string> & feffect_processor::get_stack(std::string identifier) {
        for (int i=0; i < stacks.length; i++) {
            if (stacks[i].identifier == identifier) return stacks[i].stack;
        }
        throw std::runtime_error("stack not found");
    }

    /*
    struct code_stack{
        std::string identifier;
        std::vector<std::string> stack;
        std::vector<int> water;
    };*/

    void feffect_processor::enter()
    {
        for (int i=0; i < stacks.length; i++) {
            stacks[i].water.push_back(stacks[i].stack.size());
        }
    }

    void feffect_processor::exit()
    {
        for (int i=0; i < stacks.length; i++) {
            while (stacks[i].stack.size() > stacks[i].water.back()) stacks[i].stack.pop_back();
            stacks[i].water.pop_back();
        }

    }


    utf8_string_struct feffect_processor::process(utf8_string_struct command, utf8_string_struct escape) {
        pstack.push_back({});

        if (escape == nullptr) escape = "\033";

        cursor = {command.c_str};

        this->escape = escape;

        while (!pstack.empty()) {
            feffect_stack_data cur = pstack.back();
            pstack.pop_back();

            bool last_was_identifier = false;
            bool expect_identifier = false;

            while (!cursor.end()) {
                cursor.whitespace_advance();

                if (cursor.c() == '_' || cursor.c() == '~' || alpha(cursor.c())) {
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
                        enter();
                        goto __continue;

                    } else if (cursor.c() == ')') {
                        if (!cur.is_paren) {
                            std::cerr << "unexpected close paren" << std::endl;
                            return nullptr;
                        }

                        if (!cur.cycle_command_accum(this)) return nullptr;

                        cur.cycle_command_close(this);
                        cursor.advance();

                        last_was_identifier = false;
                        exit();
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
        for (int i = 0; i < escape_accum.size(); i++) {
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
            std::cerr << "code not found for: " <<string << std::endl;
            throw std::runtime_error("developer error");
        }
        escape_accum.push_back(string);
    }

    utf8_string_struct feffect(utf8_string_struct command, utf8_string_struct escape) {
        feffect_processor fp;
        return fp.process(std::move(command), std::move(escape));
    }

    std::string feffect_processor::transit(std::string command, bool direction) {
        int inverted = 0;

        while (command.size() >= 1 && command.substr(0, 1) == "~") {
            inverted++;
            command = command.substr(1);
        }

        std::string stack_id = "";

        if ((command.size() >= 3 && command.substr(0, 3) == "fg_") || command == "fg") stack_id = "fg";
        else if ((command.size() >= 3 && command.substr(0, 3) == "bg_") || command=="bg") stack_id = "bg";
        else {
            utf8_string_struct trunc_command = command.c_str();
            for (int i=0; i < trunc_command.length; i++) {
                if (trunc_command[i] == '_') {
                    trunc_command[i] = 0;
                    break;
                }
            }

            if (has_stack(trunc_command.c_str)) stack_id = trunc_command.c_str;
        }


        if (inverted) {
            std::vector<std::string> &stack = get_stack(stack_id);

            if (inverted % 2) {
                std::string reg = stack.back();
                stack.pop_back();

                std::string reg2 = stack.back();
                stack.pop_back();

                stack.push_back(reg);
                stack.push_back(reg2);
            }
            return stack.back();
        }

        //if (inverted ^ direction) {
        if (direction) {
            if (stack_id != "") {
                std::vector<std::string> &stack = get_stack(stack_id);
                stack.push_back(command);

                return stack.back();
            }
            return command;
        } else {

            if (stack_id != "") {
                std::vector<std::string> &stack = get_stack(stack_id);

                stack.pop_back();

                return stack.back();
            }

            if (!((stack_id == "fg") || (stack_id == "bg"))) {
                if (command.size() >= 4 && command.substr(command.size() - 4) != "_off") {
                    command += "_off";
                } else {
                    command = command.substr(0, command.size() - 4);
                }
            }

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
        return c == '_' || c == '~'|| alpha(c) || numer(c);
    }

} // namespace JWCEssentials
