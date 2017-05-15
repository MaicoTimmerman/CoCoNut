#pragma once

void generate_binary_serialization_util(Config *config, FILE *fp);
void generate_binary_serialization_util_header(Config *config, FILE *fp);
void generate_binary_serialization_node(Config *config, FILE *fp, Node *n);
void generate_binary_serialization_nodeset(Config *config, FILE *fp,
                                           Nodeset *n);
