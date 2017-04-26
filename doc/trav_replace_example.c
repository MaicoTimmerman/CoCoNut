/* --------------------- Generated --------------------  */
static enum NodeType node_replacement_type;
static void *node_replacement = NULL;

// WHEN LEFT IS A NODE
void trav_BinOp_Left(struct BinOp *node, struct Info *info) {
    node_replacement = NULL;
    USER_TRAV_monop(node->Left, info);
    if (node_replacement != NULL) {
        if (node_replacement_type != BinOp) {
            error("Replacement node for %s->%s is not of type %s.");
        } else {
            node->Left = node_replacement;
        }
    }
}

// WHEN LEFT IS A NODESET
void trav_BinOp_Left(struct BinOp *node, struct Info *info) {
    node_replacement = NULL;
    switch (node->Left->type) {
    case NS_Expr_MonOp:
        USER_TRAV_monop(node->Left->value.val_MonOp, info);
        return;
    case NS_Expr_BinOp:
        USER_TRAV_binop(node->Left->value.val_BinOp, info);
        break;
    }

    if (node_replacement != NULL) {
        switch (node_replacement_type) {
        case NODE_BinOp:
            node->Left->type = NS_Expr_BinOp;
            node->Left->value.val_binop = node_replacement;
            break;
        case NODE_MonOp:
            node->Left->type = NS_Expr_MonOp;
            node->Left->value.val_monop = node_replacement;
            break;
        default:
            error("Node %s is not in nodeset Expr");
        }
    }
}

void replace_binop(struct BinOp *node) {
    node_replacement_type = NODE_BinOp;
    node_replacement = node;
}

void replace_binop(struct BinOp *node) {
    switch (current_node) {
    case NODE_expr_monop:
        ... break;
    case NODE_monop:
        ... break;
    }
}

/* --------------------- User function --------------------  */
void user_trav_monop(MonOp *node, struct Info *info) {
    // ...

    BinOp newnode = ...;
    return replace_BinOp(newnode);

    //...

    return;
}

void user_trav_binop(BinOp *node, struct Info *info) {
    // ...

    trav_BinOp_Left(node, info);
    travBinOpLeft() trav_binop_left()

        TRVbinopLeft
}
