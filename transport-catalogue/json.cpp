#include "json.h"

using namespace std;

namespace json {

    Node LoadNode(istream& is);

    inline bool IsSpace(char c) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            return true;
        }

        return false;
    }

    Node LoadArray(istream& is) {
        Array result;

        while (true) {
            char c;

            while (true) {
                if (is >> c) {
                    if (IsSpace(c)) {
                        continue;
                    }
                    else if (c == ']') {
                        return result;
                    }
                    else {
                        is.putback(c);
                        break;
                    }
                }
                throw ParsingError("LoadArray: unexpected EOF before element"s);
            }

            Node node = LoadNode(is);

            result.push_back(node);

            bool end = false;

            //wait for ',' of ']'
            while (true) {
                if (is >> c) {
                    if (IsSpace(c)) {
                        continue;
                    }
                    else if (c == ']') {
                        end = true;
                        break;
                    }
                    else if (c == ',') {
                        break;
                    }
                    else {
                        throw ParsingError("LoadArray: not ',' of ']' after node"s);
                    }
                }
                throw ParsingError("LoadArray: unexpected EOF after node"s);
            }

            if (end) {
                break;
            }
        }

        return Node(move(result));
    }

    Node LoadNumber(istream& is) {
        string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &is] {
            parsed_num += static_cast<char>(is.get());
            if (!is) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&is, read_char] {
            if (!isdigit(is.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (isdigit(is.peek())) {
                read_char();
            }
        };

        if (is.peek() == '-') {
            read_char();
        }

        // Парсим целую часть числа
        if (is.peek() == '0') {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        }
        else {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (is.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = is.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = is.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                // Сначала пробуем преобразовать строку в int
                try {
                    return Node{ stoi(parsed_num) };
                }
                catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return Node{ stod(parsed_num) };
        }
        catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    string LoadString(istream& is) {
        string res;

        auto it = istreambuf_iterator<char>(is);
        auto end = istreambuf_iterator<char>();

        while (true) {
            if (it == end) {
                // Поток закончился до того, как встретили закрывающую кавычку?
                throw ParsingError("String parsing error");
            }

            const char ch = *it;

            if (ch == '"') {
                // Встретили закрывающую кавычку
                ++it;
                break;
            }

            else if (ch == '\\') {
                // Встретили начало escape-последовательности
                ++it;
                if (it == end) {
                    // Поток завершился сразу после символа обратной косой черты
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char) {
                case 'n':
                    res.push_back('\n');
                    break;
                case 't':
                    res.push_back('\t');
                    break;
                case 'r':
                    res.push_back('\r');
                    break;
                case '"':
                    res.push_back('"');
                    break;
                case '\\':
                    res.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r') {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            }
            else {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                res.push_back(ch);
            }
            ++it;
        }

        return (res);
    }

    Node LoadDict(istream& is) {
        Dict result;
        while (true) {
            char c;

            while (true) {
                if (is >> c) {
                    if (IsSpace(c)) {
                        continue;
                    }
                    else if (c == '"') {
                        break;
                    }
                    else if (c == '}') {
                        return result;
                    }
                    else {
                        throw ParsingError("LoadDict: not '\"' before key"s);
                    }
                }
                throw ParsingError("LoadDict: unexpected EOF before key"s);
            }

            auto key = LoadString(is);


            //wait for ':'
            while (true) {
                if (is >> c) {
                    if (IsSpace(c)) {
                        continue;
                    }
                    else if (c == ':') {
                        break;
                    }
                    else {
                        throw ParsingError("LoadDict: not ':' after key"s);
                    }
                }
                throw ParsingError("LoadDict: unexpected EOF after key"s);
            }

            result.insert({ move(key), LoadNode(is) });

            bool end = false;
            //wait for ',' of '}'
            while (true) {
                if (is >> c) {
                    if (IsSpace(c)) {
                        continue;
                    }
                    else if (c == '}') {
                        end = true;
                        break;
                    }
                    else if (c == ',') {
                        break;
                    }
                    else {
                        throw ParsingError("LoadDict: not ',' of '}' after element"s);
                    }
                }
                throw ParsingError("LoadDict: unexpected EOF after element"s);
            }

            if (end) {
                break;
            }
        }

        return Node(move(result));
    }

    Node LoadNull(istream& is) {
        Node result;
        char c[4]{};

        for (size_t i = 0; i < 4; ++i) {
            if (!(is >> c[i])) {
                throw ParsingError("null unexpected EOF"s);
            }
        }

        if (c[0] != 'n' || c[1] != 'u' || c[2] != 'l' || c[3] != 'l') {
            throw ParsingError("null parsing: n is not begin of null"s);
        }

        return result;
    }

    Node LoadBool(istream& is) {
        string s = " "s;
        is >> s[0];
        const size_t len = s[0] == 'f' ? 5 : 4;
        s.resize(len);

        for (size_t i = 1; i < len; ++i) {
            if (!(is >> s[i])) {
                throw ParsingError("null unexpected EOF"s);
            }
        }

        if (s == "true"s) {
            return Node{ true };
        }
        else if (s != "false"s) {
            throw ParsingError("Bool parsing error"s);
        }

        return Node{ false };
    }

    Node LoadNode(istream& is) {
        Node res;

        bool end = false;
        char c;
        while (is >> c) {
            switch (c) {
            case ' ':
            case '\t':
            case '\n':
                continue;
            case '[':
                res = LoadArray(is);
                end = true;
                break;
            case ']':
                throw ParsingError("unexpected ']'"s);
                break;
            case '{':
                res = LoadDict(is);
                end = true;
                break;
            case '}':
                throw ParsingError("unexpected '}'"s);
                break;
            case '"':
                res = Node{ LoadString(is) };
                end = true;
                break;
            case 'n':
                is.putback(c);
                res = LoadNull(is);
                end = true;
                break;
            case 't':
            case 'f':
                is.putback(c);
                res = LoadBool(is);
                end = true;
                break;
            default:
                if (isdigit(c) || c == '-') {
                    is.putback(c);
                    res = LoadNumber(is);
                    end = true;
                }
                else {
                    throw ParsingError("Unknown character: "s + c);
                }
            }

            if (end) {
                break;
            }
        }

        return res;
    }

    const Array& Node::AsArray() const {
        if (holds_alternative<Array>(value_)) {
            return get<Array>(value_);
        }

        throw std::logic_error("AsArray: value is not Array");
    }

    const Dict& Node::AsMap() const {
        if (holds_alternative<Dict>(value_)) {
            return get<Dict>(value_);
        }

        throw std::logic_error("AsMap: value is not Map");
    }

    bool Node::AsBool() const {
        if (holds_alternative<bool>(value_)) {
            return get<bool>(value_);
        }

        throw std::logic_error("AsBool: value is not Bool");
        return false;
    }

    int Node::AsInt() const {
        if (holds_alternative<int>(value_)) {
            return get<int>(value_);
        }

        throw std::logic_error("AsInt: value is not Int (425) "s);
    }

    double Node::AsDouble() const {
        if (holds_alternative<int>(value_)) {
            return static_cast<double>(get<int>(value_));
        }

        if (holds_alternative<double>(value_)) {
            return get<double>(value_);
        }

        throw std::logic_error("AsDouble: value is not int or double"s);
    }

    const string& Node::AsString() const {
        if (holds_alternative<string>(value_)) {
            return get<string>(value_);
        }

        throw std::logic_error("AsString: value is not String"s);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintNode(const Node& node, ostream& os) {
        if (node.IsNull()) {
            os << "null"s;
            return;
        }

        if (node.IsBool()) {
            if (node.AsBool()) {
                os << "true"s;
            }
            else {
                os << "false"s;
            }
            return;
        }

        if (node.IsInt()) {
            os << node.AsInt();
            return;
        }

        if (node.IsPureDouble()) {
            os << node.AsDouble();
            return;
        }

        if (node.IsString()) {
            istringstream is(node.AsString());
            os << '"';

            auto it = istreambuf_iterator<char>(is);
            auto end = istreambuf_iterator<char>();

            while (it != end) {
                switch (*it) {
                case '"':
                    os << '\\' << '"';
                    break;
                case '\n':
                    os << '\\' << 'n';
                    break;
                case '\r':
                    os << '\\' << 'r';
                    break;
                case '\\':
                    os << '\\' << '\\';
                    break;
                default:
                    os << *it;
                }
                ++it;
            }
            os << '"';
            return;
        }

        if (node.IsArray()) {
            os << '[';
            bool not_first = true;
            for (const auto& elem : node.AsArray()) {
                if (not_first) {
                    not_first = false;
                }
                else {
                    os << ',';
                }
                PrintNode(elem, os);
            }
            os << ']';
        }

        if (node.IsMap()) {
            os << '{';
            bool not_first = true;
            for (const auto& [key, val] : node.AsMap()) {
                if (not_first) {
                    not_first = false;
                }
                else {
                    os << ',';
                }
                os << '"';
                os << key;
                os << '"';
                os << ':';
                PrintNode(val, os);
            }
            os << '}';
        }
    }

    void Print(const Document& doc, ostream& os) {
        PrintNode(doc.GetRoot(), os);
    }

    bool Node::IsNull()  const {
        return holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsArray()  const {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap()  const {
        return holds_alternative<Dict>(value_);
    }

    bool Node::IsBool()  const {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsInt()  const {
        return holds_alternative<int>(value_);
    }

    bool Node::IsDouble()  const {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsPureDouble()  const {
        return holds_alternative<double>(value_);
    }

    bool Node::IsString() const {
        return holds_alternative<string>(value_);
    }

    const Value& Node::GetValue() const {
        return value_;
    }

} // namespace json

