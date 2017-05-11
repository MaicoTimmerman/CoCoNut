Binary serialization format
===========================

.. highlight:: c

.. code-block:: c

    AstBinFile {
        // Magic of the filetype
        u4          magic;

        // Magic of the AST specification from which the file was generated
        u4          ast_magic;

        u2          string_pool_count;
        sp_entry    string_pool[string_pool_count];
        u2          enum_pool_count;
        ep_entry    enum_pool[enum_pool_count];

        u4          node_count;
        node        nodes[node_count];

        // TODO: add checksum?
    }

.. code-block:: c

    sp_entry {
        u2 length;

        // DISCUSS: ascii or utf-8
        u1 bytes[length];
    }

.. code-block:: c

    ep_entry {
        // Index in string pool representing the name
        u2 name_index;

        // Index in string pool representing the prefix
        u2 prefix_index;

        u2 values_count;

        // Indices in string pool representing the values without the prefix
        u2 values[values_count];
    }

.. code-block:: c

    node {
        // Type of node. Index in string pool
        u2 type_index;

        u2 child_count;
        child children[child_count];
        u2 attribute_count;
        attr attributes[attribute_count];
    }

.. code-block:: c

    child {
        // Name of the child. Index in string pool
        u2 name_index;

        // Index in nodes array of the node representing the child
        u4 node_index;
    }

.. code-block:: c

    attr {
        u1 type;

        // Name of the child. Index in string pool
        u2 name_index;

        // One of te AT_*_data structs
        u1 data[]
    }


Where ``type`` is one of:

+-----------------+--------+
| Type            | Value  |
+=================+========+
| ``AT_int``      |   1    |
+-----------------+--------+
| ``AT_uint``     |   2    |
+-----------------+--------+
| ``AT_int8``     |   3    |
+-----------------+--------+
| ``AT_int16``    |   4    |
+-----------------+--------+
| ``AT_int32``    |   5    |
+-----------------+--------+
| ``AT_int64``    |   6    |
+-----------------+--------+
| ``AT_uint8``    |   7    |
+-----------------+--------+
| ``AT_uint16``   |   8    |
+-----------------+--------+
| ``AT_uint32``   |   9    |
+-----------------+--------+
| ``AT_uint64``   |   10   |
+-----------------+--------+
| ``AT_float``    |   11   |
+-----------------+--------+
| ``AT_double``   |   12   |
+-----------------+--------+
| ``AT_bool``     |   13   |
+-----------------+--------+
| ``AT_string``   |   14   |
+-----------------+--------+
| ``AT_link``     |   15   |
+-----------------+--------+
| ``AT_enum``     |   16   |
+-----------------+--------+

The format of data[] is dependent on the value of type


.. code-block:: c

    // DISCUSS: serialization of int and uint depend on host architecture

    AT_int_data {
       sN value;
    }

where ``N = sizeof(int)``

.. code-block:: c

    AT_uint_data {
       sN value;
    }

where ``N = sizeof(unsigned int)``

.. code-block:: c

    AT_int8_data {
       s1 value;
    }

.. code-block:: c

    AT_int16_data {
       s2 value;
    }

.. code-block:: c

    AT_int32_data {
       s4 value;
    }

.. code-block:: c

    AT_int64_data {
       s8 value;
    }

.. code-block:: c

    AT_uint8_data {
       u1 value;
    }

.. code-block:: c

    AT_uint16_data {
       u2 value;
    }

.. code-block:: c

    AT_uint32_data {
       u4 value;
    }

.. code-block:: c

    AT_uint64_data {
       u8 value;
    }

.. code-block:: c

    AT_float_data {
        // Represents IEEE 754 32-bit float
        u4 value;
    }

.. code-block:: c

    AT_double_data {
        // Represents IEEE 754 64-bit float
        u8 value;
    }

.. code-block:: c

    AT_bool_data {
        u1 value;
    }

.. code-block:: c

    AT_string_data {
        u2 value_index;
    }

.. code-block:: c

    AT_enum {
        // Index in enum pool
        u2 enum_type_index;

        // Index in values array of enum
        u2 value_index;
    }

.. code-block:: c

    AT_link {
        // Index in nodes array
        u4 node_index;
    }
