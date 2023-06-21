#include "json_builder.h"

#include <utility>

using namespace json;

Node Builder::Build() {
    if (!nodes_.empty()) {
        throw logic_error("Build: stack not empty"s);
    }
    return root_;
}

Builder& Builder::Value(Node node) {
    if (nodes_.empty()) {
        throw std::logic_error("Value: stack empty"s);
    }
    if (!(nodes_.back()->IsNull() || nodes_.back()->IsArray())) {
        throw std::logic_error("Value: after something not ended"s);
    }
    if (nodes_.back()->IsArray()) {
        const_cast<Array&>(nodes_.back()->AsArray()).push_back(node);
    }
    else {
        *nodes_.back() = node;
        nodes_.pop_back();
    }
    return *this;
}

DictContext Builder::StartDict() {

    if (nodes_.empty()) {
        throw logic_error("StartDict: stack empty"s);
    }

    if (!(nodes_.back()->IsNull() || nodes_.back()->IsArray())) {
        throw logic_error("StartDict: after something not ended"s);
    }

    if (nodes_.back()->IsArray()) {
        const_cast<Array&>(nodes_.back()->AsArray()).push_back(Dict{});
        nodes_.push_back(&const_cast<Array&>(nodes_.back()->AsArray()).back());
    }
    else {
        *nodes_.back() = Dict{};
    }

    DictContext res(*this);
    return res;
}

DictKeyContext Builder::Key(string key) {
    if (nodes_.empty()) {
        throw logic_error("Key: stack empty"s);
    }

    if (!nodes_.back()->IsDict()) {
        throw logic_error("Key: not after Dict"s);
    }

    nodes_.push_back(&const_cast<json::Dict&>(nodes_.back()->AsDict())[key]);

    DictKeyContext res(*this);
    return res;
}

Builder& Builder::EndDict() {
    if (nodes_.empty()) {
        throw logic_error("EndDict: stack empty"s);
    }

    if (!nodes_.back()->IsDict()) {
         throw logic_error("EndDict: not after Dict"s);
    }

    nodes_.pop_back();
    return *this;
}

Builder& DictContext::EndDict() {
    return builder_.EndDict();
}

ArrayContext Builder::StartArray() {
    if (nodes_.empty()) {
        throw logic_error("StartArray: stack empty"s);
    }

    if(!(nodes_.back()->IsNull() || nodes_.back()->IsArray())) {
        throw logic_error("StartArray: after something not ended"s);
    }

    if (nodes_.back()->IsArray()) {
        const_cast<Array&>(nodes_.back()->AsArray()).push_back(Array{});
        nodes_.push_back(&const_cast<Array&>(nodes_.back()->AsArray()).back());
    }
    else {
        *nodes_.back() = Array{};
    }

    ArrayContext res(*this);
    return res;
}

Builder& Builder::EndArray() {
    if (nodes_.empty()) {
        throw logic_error("EndArray: stack empty"s);
    }

    if(!nodes_.back()->IsArray()) {
        throw logic_error("EndArray: not after Array"s);
    }
    nodes_.pop_back();
    return *this;
}

DictKeyContext DictValueContext::Key(string key) {
    return builder_.Key(key);
}

Builder& DictValueContext::EndDict() {
    return builder_.EndDict();
}

DictKeyContext DictContext::Key(string key) {
    return builder_.Key(move(key));
}

DictValueContext DictKeyContext::Value(Node node) {
    return builder_.Value(move(node));
}

ArrayContext DictKeyContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayContext::EndArray() {
    return builder_.EndArray();
}

ArrayContext ArrayContext::Value(Node node) {
    return builder_.Value(move(node));
}

ArrayContext ArrayContext::StartArray() {
    return builder_.StartArray();
}

DictContext DictKeyContext::StartDict() {
    return builder_.StartDict();
}

DictContext ArrayContext::StartDict() {
    return builder_.StartDict();
}