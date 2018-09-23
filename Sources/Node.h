#ifndef _node_h
#define _node_h

enum NodeType {
    NODE_TYPE_INT = 1024,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_OPERATOR
};

struct Int {
    short width;
    char* string;
};
#define NODE_INT_DEFAULT_WIDTH 32

struct Identifier {
    char* string;
};

struct Operator {
    char* string;
};

struct Node {
    enum NodeType type;
    union {
        struct Int integer;
        struct Operator operator;
        struct Identifier identifier;
    } content;
    struct Node* left;
    struct Node* right;
};

struct Node* makeNode(enum NodeType type, void* target, struct Node* left, struct Node* right);
int traverse(struct Node* head);
#endif
