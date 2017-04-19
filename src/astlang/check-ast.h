#pragma once

#include "htable.h"

struct Info {
    htable_t *enum_name;
    htable_t *enum_prefix;
    htable_t *node_name;
    htable_t *nodeset_name;
    htable_t *traversal_name;
    htable_t *phase_name;
};

int check_config(struct Config *config);
