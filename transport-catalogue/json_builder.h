#include "json.h"

namespace json {

    class Context;
    class DictContext;
    class DictKeyContext;

    class Builder {
    public:
        Builder() {
            nodes_.push_back(&root_);
        }
        Builder& Value(Node node);
        Node Build();
        DictContext StartDict();
        Builder& EndDict();
        DictKeyContext Key(std::string key);
        Builder& StartArray();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_;
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
        DictKeyContext Key(std::string key);
        Builder& EndDict();
    };

    class DictKeyContext : public Context {
    public:
        DictKeyContext(Builder& builder)
            : Context(builder) {
        }
        Builder& Value(Node value);
        Builder& StartArray();
        DictContext StartDict();

    };
}