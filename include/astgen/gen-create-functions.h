#pragma once

void generate_create_node_header(struct Config *c, FILE *fp, struct Node *n);
void generate_create_node_definitions(struct Config *c, FILE *fp,
                                      struct Node *n);
void generate_create_nodeset_header(struct Config *c, FILE *fp,
                                    struct Nodeset *n);

void generate_create_nodeset_definitions(struct Config *c, FILE *fp,
                                         struct Nodeset *n);
void generate_create_header(struct Config *config, FILE *fp);
