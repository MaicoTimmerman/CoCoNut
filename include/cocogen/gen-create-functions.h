#pragma once

void generate_create_node_header(Config *c, FILE *fp, Node *n);
void generate_create_node_definitions(Config *c, FILE *fp, Node *n);
void generate_create_nodeset_header(Config *c, FILE *fp, Nodeset *n);

void generate_create_nodeset_definitions(Config *c, FILE *fp, Nodeset *n);
void generate_create_header(Config *config, FILE *fp);
