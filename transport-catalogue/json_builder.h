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

    class DictContext {
    public:
        DictContext(Builder& builder)
            : builder_(builder) {
        }
        DictKeyContext Key(string key);
        Builder& EndDict();
    protected:
        Builder& builder_;
    };

    class DictKeyContext {
    public:
        DictKeyContext(Builder& builder)
            : builder_(builder) {
        }
        DictValueContext Value(Node value);
        ArrayContext StartArray();
        DictContext StartDict();
    protected:
        Builder& builder_;
    };

    class DictValueContext {
    public:
        DictValueContext(Builder& builder)
            : builder_(builder) {
        }
        DictKeyContext Key(string key);
        Builder& EndDict();
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