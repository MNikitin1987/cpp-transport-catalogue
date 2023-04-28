#include "json.h"

namespace json {
    using namespace std;

    class Context;
    class DictContext;
    class DictKeyContext;
    class DictValueContext;
    class ArrayContext;

    class Builder {
    public:
        Builder() {
            nodes_.push_back(&root_);
        }
        Builder& Value(Node node);
        Node Build();
        DictContext StartDict();
        Builder& EndDict();
        DictKeyContext Key(string key);
        ArrayContext StartArray();
        Builder& EndArray();

    private:
        Node root_;
        vector<Node*> nodes_;
    };


    class Context {
    public:
        Context(Builder& builder)
            : builder_(builder) {
        }
    protected:
        Builder& builder_;
    };

    class DictContext : public Context {
    public:
        DictContext(Builder& builder)
            : Context(builder) {
        }
        DictKeyContext Key(string key);
        Builder& EndDict();
    };

    class DictKeyContext : public Context {
    public:
        DictKeyContext(Builder& builder)
            : Context(builder) {
        }
        DictValueContext Value(Node value);
        ArrayContext StartArray();
        DictContext StartDict();

    };

    class DictValueContext : public Context {
    public:
        DictValueContext(Builder& builder)
            : Context(builder) {
        }
        DictKeyContext Key(string key);
        Builder& EndDict();
    };
    class ArrayContext : public Context {
    public:
        ArrayContext(Builder& builder)
            : Context(builder) {
        }
        Builder& EndArray();
        DictContext StartDict();
        ArrayContext Value(Node node);
        ArrayContext StartArray();
    };
}