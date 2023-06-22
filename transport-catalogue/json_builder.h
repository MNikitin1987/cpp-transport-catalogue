#include "json.h"

namespace json {

    class DictContext;
    class DictKeyContext;
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
        DictKeyContext Key(std::string key);
        Builder& StartArray();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_;
    };

    class DictContext {
    public:
        DictContext(Builder& builder)
            : builder_(builder) {
        }
        DictKeyContext Key(std::string key);
        Builder& EndDict();
    protected:
        Builder& builder_;
    };

    class DictKeyContext {
    public:
        DictKeyContext(Builder& builder)
            : builder_(builder) {
        }
        Builder& Value(Node value);
        Builder& StartArray();
        DictContext StartDict();
    protected:
        Builder& builder_;
    };
    
    class ArrayContext {
    public:
        ArrayContext(Builder& builder)
            : builder_(builder) {
        }

        Builder& EndArray();
        DictContext StartDict();
        ArrayContext Value(Node node);
        ArrayContext StartArray();
    protected:
        Builder& builder_;
    };
}