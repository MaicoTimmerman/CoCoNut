#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "framework/serialization-txt-ast.h"
#include "lib/imap.h"
#include "lib/print.h"



int _serialization_txt_check_file(AST_TXT_File *file) {

    int errors = 0;

    imap_t *nodemap = imap_init(64);

    AST_TXT_Node *root_node = NULL;

    for (int i = 0; i < array_size(file->nodes); i++) {
        AST_TXT_Node *n = array_get(file->nodes, i);

        if (n->is_root) {
            if (root_node == NULL)
                root_node = n;
            else {
                print_error(n, "Duplicate declaration of root node");
                print_note(root_node, "Previously declared here");
                errors++;
            }
        }

        AST_TXT_Node *n_id = imap_retrieve(nodemap, (void *)n->id);
        if (n_id == NULL) {
            imap_insert(nodemap, (void *)n->id, n);
        } else {
            print_error(n, "Duplicate declaration of node with id %d", n->id);
            print_note(n_id, "Previously declared here");
            errors++;
        }
    }

    imap_t *used_nodes = imap_init(64);

    for (int i = 0; i < array_size(file->nodes); i++) {
        AST_TXT_Node *n = array_get(file->nodes, i);

        for (int j = 0; j < array_size(n->children); j++) {
            AST_TXT_Child *c = array_get(n->children, j);

            if (imap_retrieve(nodemap, (void *)c->id) != NULL) {
                imap_insert(used_nodes, (void *)c->id, c);
            } else {
                print_error(n, "Reference to non-existing node with id %" PRIu64,
                            c->id);
                errors++;
            }
        }
    }

    for (int i = 0; i < array_size(file->nodes); i++) {
        AST_TXT_Node *n = array_get(file->nodes, i);
        if (n != root_node &&
            imap_retrieve(used_nodes, (void *)n->id) == NULL) {
            print_warning(n, "Node unused in AST");
        }
    }

    file->rootnode = root_node;
    file->node_id_map = nodemap;
    file->used_nodes = used_nodes;

    return errors;
}
