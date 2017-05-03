#pragma once

#define out(...) fprintf(fp, __VA_ARGS__)

// TODO fix casing/ prefix options.
#define NS_FMT "NS_%s_%s"
#define NS_ENUMTYPE_FMT "NS_%s_enum"

#define NT_FMT "NT_%s"
#define TRAV_FMT "TRAV_%s"

// TODO prefix definitions.
#define CREATE_PREFIX
#define CREATE_FMT CREATE_PREFIX "_%s"

#define TRAVERSAL_HANDLER_FMT "%s_%s"
