#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <variant>

using namespace std;

namespace json {

    class Node;

    // Сохраните объявления Dict и Array без изменения
    using Dict = map<string, Node>;
    using Array = vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = variant<nullptr_t, Array, Dict, bool, int, double, string>;

    class Node {
    
    // комментарий: "если унаследуете класс от вашего std::variant со всеми типами и подключите использование using varint::variant  для доступа конструкторов класса variant как сказано в теории, то вам не нужно будет определять конструкторы и отпадет необходимость организовывать переменную NodeType node_;, так как в этом случае объект класса становится сам переменной, а для получения значения достаточно разыменовывать данный объект"
  // ответ: спасибо за комментарий, постараюсь учесть в будущем
        
        
    public:

        Node() = default;
        Node(nullptr_t null) : value_(null) {}
        Node(Array value) : value_(move(value)) { }
        Node(Dict value) : value_(move(value)) { }
        Node(bool value) : value_(value) { }
        Node(int value) : value_(value) { }
        Node(double value) : value_(value) { }
        Node(string value) : value_(move(value)) { }

        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const string& AsString() const;

        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;

        const Value& GetValue() const;

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(istream& is);

    void Print(const Document& doc, ostream& os);

    inline bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    void PrintNode(const Node& node, ostream& os);

}   // namespace json