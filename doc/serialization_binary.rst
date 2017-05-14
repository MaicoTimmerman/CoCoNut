Binary serialization format
===========================

.. highlight:: c

.. code-block:: c

    AstBinFile {
        // Magic of the filetype.
        u4          magic;

        // Bit array of boolean flags.
        u2          flags;

        // MD5 Hash of the specification of the root node of the (sub)tree
        // from which the file was generated
        u16         hash;

        u4          string_pool_count;
        sp_entry    string_pool[string_pool_count];

        u2          enum_pool_count;
        ep_entry    enum_pool[enum_pool_count];

        u4          node_count;
        node        nodes[node_count];

        // TODO: add checksum?
    }


The first entry in the ``nodes`` array is the root node of the (sub)tree.

The ``flags`` array is ordered from left to right.
The flags have the following meaning:

+--------+-------------------------------------------+
| Index  | Value                                     |
+========+===========================================+
| 0      | | **Little-endianness**                   |
|        | | When set, all integers are encoded      |
|        |   in little-endian format.                |
|        | | Otherwise, big-endian format is used.   |
+--------+-------------------------------------------+
| 1      | | Reserved                                |
+--------+-------------------------------------------+
| 2      | | Reserved                                |
+--------+-------------------------------------------+
| 3      | | Reserved                                |
+--------+-------------------------------------------+
| 4      | | Reserved                                |
+--------+-------------------------------------------+
| 5      | | Reserved                                |
+--------+-------------------------------------------+
| 6      | | Reserved                                |
+--------+-------------------------------------------+
| 7      | | Reserved                                |
+--------+-------------------------------------------+
| 8      | | Reserved                                |
+--------+-------------------------------------------+
| 9      | | Reserved                                |
+--------+-------------------------------------------+
| 10     | | Reserved                                |
+--------+-------------------------------------------+
| 11     | | Reserved                                |
+--------+-------------------------------------------+
| 12     | | Reserved                                |
+--------+-------------------------------------------+
| 13     | | Reserved                                |
+--------+-------------------------------------------+
| 14     | | Reserved                                |
+--------+-------------------------------------------+
| 15     | | Reserved                                |
+--------+-------------------------------------------+


.. code-block:: c

    sp_entry {
        u2 length;

        // utf-8 encoded string
        u1 bytes[length];
    }

.. code-block:: c

    ep_entry {
        // Index in string pool representing the name
        u4 name_index;

        // Index in string pool representing the prefix
        u4 prefix_index;

        u2 values_count;

        // Indices in string pool representing the values without the prefix
        u4 values[values_count];
    }

.. code-block:: c

    node {
        // Type of node. Index in string pool
        u4 type_index;

        u2 child_count;
        child children[child_count];
        u2 attribute_count;
        attr attributes[attribute_count];
    }

.. code-block:: c

    child {
        // Name of the child. Index in string pool
        u4 name_index;

        // Index in nodes array of the node representing the child
        u4 node_index;
    }

.. code-block:: c

    attr {
        // Name of the attribute. Index in string pool
        u4 name_index;

        u1 type;

        // One of te AT_*_data structs
        u1 data[]
    }


Where ``type`` is one of:

+-----------------+--------+
| Type            | Value  |
+=================+========+
| ``AT_int``      |   0    |
+-----------------+--------+
| ``AT_uint``     |   1    |
+-----------------+--------+
| ``AT_int8``     |   2    |
+-----------------+--------+
| ``AT_int16``    |   3    |
+-----------------+--------+
| ``AT_int32``    |   4    |
+-----------------+--------+
| ``AT_int64``    |   5    |
+-----------------+--------+
| ``AT_uint8``    |   6    |
+-----------------+--------+
| ``AT_uint16``   |   7    |
+-----------------+--------+
| ``AT_uint32``   |   8    |
+-----------------+--------+
| ``AT_uint64``   |   9    |
+-----------------+--------+
| ``AT_float``    |   10   |
+-----------------+--------+
| ``AT_double``   |   11   |
+-----------------+--------+
| ``AT_bool``     |   12   |
+-----------------+--------+
| ``AT_string``   |   13   |
+-----------------+--------+
| ``AT_link``     |   14   |
+-----------------+--------+
| ``AT_enum``     |   15   |
+-----------------+--------+

The format of data[] is dependent on the value of type


.. code-block:: c

    AT_int_data {
       s8 value;
    }

.. code-block:: c

    AT_uint_data {
       s8 value;
    }

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
        u4 value_index;
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
