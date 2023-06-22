#include "json_builder.h"

#include <utility>

using namespace json;

Node Builder::Build() {
    if (!nodes_.empty()) {
        throw std::logic_error("Build: stack not empty");
    }
    return root_;
}

Builder& Builder::Value(Node node) {
    if (nodes_.empty()) {
        throw std::logic_error("Value: stack empty");
    }
    if (!(nodes_.back()->IsNull() || nodes_.back()->IsArray())) {
        throw std::logic_error("Value: after something not ended");
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
        throw std::logic_error("StartDict: stack empty");
    }

    if (!(nodes_.back()->IsNull() || nodes_.back()->IsArray())) {
        throw std::logic_error("StartDict: after something not ended");
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

DictKeyContext Builder::Key(std::string key) {
    if (nodes_.empty()) {
        throw std::logic_error("Key: stack empty");
    }

    if (!nodes_.back()->IsDict()) {
        throw std::logic_error("Key: not after Dict");
    }

    nodes_.push_back(&const_cast<json::Dict&>(nodes_.back()->AsDict())[key]);

    DictKeyContext res(*this);
    return res;
}

Builder& Builder::EndDict() {
    if (nodes_.empty()) {
        throw std::logic_error("EndDict: stack empty");
    }

    if (!nodes_.back()->IsDict()) {
         throw std::logic_error("EndDict: not after Dict");
    }

    nodes_.pop_back();
    return *this;
}

Builder& DictContext::EndDict() {
    return builder_.EndDict();
}

DictContext DictKeyContext::StartDict() {
    return builder_.StartDict();
}

Builder& Builder::EndArray() {
    if (nodes_.empty()) {
        throw std::logic_error("EndArray: stack empty");
    }
    
    if(!nodes_.back()->IsArray()) {
        throw std::logic_error("EndArray: not after Array");
    }
    nodes_.pop_back();
    return *this;
}

DictKeyContext DictContext::Key(std::string key) {
    return builder_.Key(move(key));
}

Builder& Builder::StartArray() {
    if (nodes_.empty()) {
        throw std::logic_error("StartArray: stack empty");
    }

    if(!(nodes_.back()->IsNull() || nodes_.back()->IsArray())) {
        throw std::logic_error("StartArray: after something not ended");
    }

    if (nodes_.back()->IsArray()) {
        const_cast<Array&>(nodes_.back()->AsArray()).push_back(Array{});
        nodes_.push_back(&const_cast<Array&>(nodes_.back()->AsArray()).back());
    }
    else {
        *nodes_.back() = Array{};
    }

    return *this;
}

Builder& DictKeyContext::Value(Node node) {
    return builder_.Value(move(node));
}

Builder& DictKeyContext::StartArray() {
    return builder_.StartArray();
}