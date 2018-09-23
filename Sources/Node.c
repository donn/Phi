#include "Node.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Node* makeNode(enum NodeType type, void* target, struct Node* left, struct Node* right) {
    struct Node* node = malloc(sizeof(struct Node));
    node->left = left;
    node->right = right;
    node->type = type;

    struct Identifier* identifier = (struct Identifier*)target;
    struct Operator* op = (struct Operator*)target;
    struct Int* integer = (struct Int*)target;
    switch(type) {
        case NODE_TYPE_IDENTIFIER:
            node->content.identifier.string = malloc(strlen(identifier->string));
            strcpy(node->content.identifier.string, identifier->string);
            break;
        case NODE_TYPE_OPERATOR:
            node->content.op.string = malloc(strlen(op->string));
            strcpy(node->content.op.string, op->string);
            break;
        case NODE_TYPE_INT:
            node->content.integer.width = integer->width;
            node->content.integer.string = malloc(strlen(integer->string));
            strcpy(node->content.integer.string, integer->string);
            break;
        default:
            printf("????");
            free(node);
            return NULL;
    }
    return node;
}

struct Node* traverse(struct Node* node) {
    if (node == NULL) {
        return node;
    }
    traverse(node->left);
    switch(node->type) {
        case NODE_TYPE_IDENTIFIER:
            printf("%s", node->content.op.string);
            break;
        case NODE_TYPE_OPERATOR:
            printf("%s", node->content.identifier.string);
            break;
        case NODE_TYPE_INT:
            printf("N%i %sN\n", node->content.integer.width, node->content.integer.string);
            break;
        default:
            ;
    }
    traverse(node->right);
    return node;
}