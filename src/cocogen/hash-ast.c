#include "cocogen/hash-ast.h"
#include "cocogen/print.h"
#include "cocogen/str-ast.h"

#include "lib/memory.h"
#include "lib/errors.h"

#include <mhash.h>

#define hash(a, b)                                                            \
    do {                                                                      \
        mhash(td, a, strlen(a) * sizeof(b));                                  \
    } while (0)
#define hashc(a, b)                                                           \
    do {                                                                      \
        mhash(config_td, a, strlen(a) * sizeof(b));                           \
    } while (0)

static MHASH td;
static MHASH config_td;
static unsigned char hash[17]; /* enough size for MD5 */

static void set_hash(NodeCommonInfo *info, bool config) {
    info->hash = (char *)mem_alloc(
        ((mhash_get_block_size(MHASH_MD5) * 2) + 1) * sizeof(char));
    info->hash[(mhash_get_block_size(MHASH_MD5) * 2)] = '\0';

    // Save the hash.
    for (int i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
        sprintf(info->hash + (i * 2), "%.2x", hash[i]);
    }
}

static void hash_node(Node *n) {

    td = mhash_init(MHASH_MD5);
    if (td == MHASH_FAILED) {
        print_error(n->id, "Error generating hashes.");
        exit(HASH_ERROR);
    }

    hash(n->id, char);
    hash(n->root ? "y" : "n", char);
    for (int i = 0; i < array_size(n->children); ++i) {
        Child *child = array_get(n->children, i);
        hash(child->id, char);
        hash(child->type, char);
        hash(child->construct ? "y" : "n", char);

        // Node children can change to different type without name changes.
        hash(child->node == NULL ? "y" : "n", char);
        hash(child->nodeset == NULL ? "y" : "n", char);
    }

    for (int i = 0; i < array_size(n->attrs); ++i) {
        Attr *attr = array_get(n->attrs, i);
        hash(attr->id, char);
        hash(str_attr_type(attr), char);
        hash(attr->construct ? "y" : "n", char);
        if (attr->default_value) {
            AttrValue *val = attr->default_value;

            switch (val->type) {
            case AV_string:
                hash(val->value.string_value, char);
                break;
            case AV_int:
                mhash(td, &val->value.int_value, sizeof(int64_t));
                break;
            case AV_uint:
                mhash(td, &val->value.uint_value, sizeof(uint64_t));
                break;
            case AV_float:
                mhash(td, &val->value.float_value, sizeof(float));
                break;
            case AV_double:
                mhash(td, &val->value.double_value, sizeof(double));
                break;
            case AV_bool:
                hash(val->value.bool_value ? "y" : "n", char);
                break;
            case AV_id:
                hash(val->value.string_value, char);
                break;
            }
        }
    }
    mhash_deinit(td, hash);
    set_hash(n->common_info, false);
}

static void hash_nodeset(Nodeset *nodeset) {
    td = mhash_init(MHASH_MD5);
    if (td == MHASH_FAILED) {
        print_error(nodeset->id, "Error generating hashes.");
        exit(HASH_ERROR);
    }

    hash(nodeset->id, char);

    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        Node *node = array_get(nodeset->nodes, i);
        hash(node->id, char);
    }

    mhash_deinit(td, hash);
    set_hash(nodeset->common_info, false);
}

static void hash_traversal(Traversal *trav) {
    td = mhash_init(MHASH_MD5);
    if (td == MHASH_FAILED) {
        print_error(trav->id, "Error generating hashes.");
        exit(HASH_ERROR);
    }

    hash(trav->id, char);
    if (trav->func)
        hash(trav->func ? "y" : "n", char);

    for (int i = 0; i < array_size(trav->nodes); ++i) {
        char *node = array_get(trav->nodes, i);
        hash(node, char);
    }

    mhash_deinit(td, hash);
    set_hash(trav->common_info, false);
}

static void hash_phase(Phase *phase) {
    td = mhash_init(MHASH_MD5);
    if (td == MHASH_FAILED) {
        print_error(phase->id, "Error generating hashes.");
        exit(HASH_ERROR);
    }

    hash(phase->id, char);
    hash(phase->cycle ? "y" : "n", char);
    hash(phase->root ? "y" : "n", char);

    for (int i = 0; i < array_size(phase->passes); ++i) {
        char *pass = array_get(phase->passes, i);
        hash(pass, char);
    }
    for (int i = 0; i < array_size(phase->subphases); ++i) {
        char *subphase = array_get(phase->subphases, i);
        hash(subphase, char);
    }

    mhash_deinit(td, hash);
    set_hash(phase->common_info, false);
}

static void hash_pass(Pass *pass) {
    td = mhash_init(MHASH_MD5);
    if (td == MHASH_FAILED) {
        print_error(pass->id, "Error generating hashes.");
        exit(HASH_ERROR);
    }

    hash(pass->id, char);
    if (pass->func)
        hash(pass->func, char);

    mhash_deinit(td, hash);
    set_hash(pass->common_info, false);
}

static void hash_enum(Enum *e) {
    td = mhash_init(MHASH_MD5);
    if (td == MHASH_FAILED) {
        print_error(e->id, "Error generating hashes.");
        exit(HASH_ERROR);
    }

    hash(e->id, char);
    hash(e->prefix, char);

    for (int i = 0; i < array_size(e->values); ++i) {
        char *value = array_get(e->values, i);
        hash(value, char);
    }

    mhash_deinit(td, hash);
    set_hash(e->common_info, false);
}

void hash_config(Config *c) {
    Node *node;
    Nodeset *nodeset;
    Traversal *trav;
    Phase *phase;
    Pass *pass;
    Enum *e;

    config_td = mhash_init(MHASH_MD5);
    if (config_td == MHASH_FAILED) {
        fprintf(stderr, "Cannot make config hash.\n");
        exit(HASH_ERROR);
    }

    for (int i = 0; i < array_size(c->nodes); ++i) {
        node = array_get(c->nodes, i);
        hash_node(node);
        hashc(node->common_info->hash, char);
    }
    for (int i = 0; i < array_size(c->nodesets); ++i) {
        nodeset = array_get(c->nodesets, i);
        hash_nodeset(nodeset);
        hashc(nodeset->common_info->hash, char);
    }
    for (int i = 0; i < array_size(c->traversals); ++i) {
        trav = array_get(c->traversals, i);
        hash_traversal(trav);
        hashc(trav->common_info->hash, char);
    }
    for (int i = 0; i < array_size(c->phases); ++i) {
        phase = array_get(c->phases, i);
        hash_phase(phase);
        hashc(phase->common_info->hash, char);
    }
    for (int i = 0; i < array_size(c->passes); ++i) {
        pass = array_get(c->passes, i);
        hash_pass(pass);
        hashc(pass->common_info->hash, char);
    }
    for (int i = 0; i < array_size(c->enums); ++i) {
        e = array_get(c->enums, i);
        hash_enum(e);
        hashc(e->common_info->hash, char);
    }

    mhash_deinit(config_td, hash);
    set_hash(c->common_info, true);

    printf("Config hash is : %s\n", c->common_info->hash);
}
