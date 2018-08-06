/*
 * The MIT License
 *
 * @copyright Copyright (c) 2017-2018 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// clang-format is disabled on the first enum so that we can manually indent it
// properly.
// clang-format off
#ifdef TILEDB_QUERY_TYPE_ENUM
    /** Read query */
    TILEDB_QUERY_TYPE_ENUM(READ) = 0,
    /** Write query */
    TILEDB_QUERY_TYPE_ENUM(WRITE) = 1,
#endif
// clang-format on

#ifdef TILEDB_OBJECT_TYPE_ENUM
    /** Invalid object */
    TILEDB_OBJECT_TYPE_ENUM(INVALID) = 0,
    /** Group object */
    TILEDB_OBJECT_TYPE_ENUM(GROUP) = 1,
    /** Array object */
    TILEDB_OBJECT_TYPE_ENUM(ARRAY) = 2,
    /** Key-value object */
    TILEDB_OBJECT_TYPE_ENUM(KEY_VALUE) = 3,
#endif

#ifdef TILEDB_FILESYSTEM_ENUM
    /** HDFS filesystem */
    TILEDB_FILESYSTEM_ENUM(HDFS) = 0,
    /** S3 filesystem */
    TILEDB_FILESYSTEM_ENUM(S3) = 1,
#endif

#ifdef TILEDB_DATATYPE_ENUM
    /** 32-bit signed integer */
    TILEDB_DATATYPE_ENUM(INT32) = 0,
    /** 64-bit signed integer */
    TILEDB_DATATYPE_ENUM(INT64) = 1,
    /** 32-bit floating point value */
    TILEDB_DATATYPE_ENUM(FLOAT32) = 2,
    /** 64-bit floating point value */
    TILEDB_DATATYPE_ENUM(FLOAT64) = 3,
    /** Character */
    TILEDB_DATATYPE_ENUM(CHAR) = 4,
    /** 8-bit signed integer */
    TILEDB_DATATYPE_ENUM(INT8) = 5,
    /** 8-bit unsigned integer */
    TILEDB_DATATYPE_ENUM(UINT8) = 6,
    /** 16-bit signed integer */
    TILEDB_DATATYPE_ENUM(INT16) = 7,
    /** 16-bit unsigned integer */
    TILEDB_DATATYPE_ENUM(UINT16) = 8,
    /** 32-bit unsigned integer */
    TILEDB_DATATYPE_ENUM(UINT32) = 9,
    /** 64-bit unsigned integer */
    TILEDB_DATATYPE_ENUM(UINT64) = 10,
    /** ASCII string */
    TILEDB_DATATYPE_ENUM(STRING_ASCII) = 11,
    /** UTF-8 string */
    TILEDB_DATATYPE_ENUM(STRING_UTF8) = 12,
    /** UTF-16 string */
    TILEDB_DATATYPE_ENUM(STRING_UTF16) = 13,
    /** UTF-32 string */
    TILEDB_DATATYPE_ENUM(STRING_UTF32) = 14,
    /** UCS2 string */
    TILEDB_DATATYPE_ENUM(STRING_UCS2) = 15,
    /** UCS4 string */
    TILEDB_DATATYPE_ENUM(STRING_UCS4) = 16,
    /** This can be any datatype. Must store (type tag, value) pairs. */
    TILEDB_DATATYPE_ENUM(ANY) = 17,
#endif

#ifdef TILEDB_ARRAY_TYPE_ENUM
    /** Dense array */
    TILEDB_ARRAY_TYPE_ENUM(DENSE) = 0,
    /** Sparse array */
    TILEDB_ARRAY_TYPE_ENUM(SPARSE) = 1,
#endif

#ifdef TILEDB_LAYOUT_ENUM
    /** Row-major layout */
    TILEDB_LAYOUT_ENUM(ROW_MAJOR) = 0,
    /** Column-major layout */
    TILEDB_LAYOUT_ENUM(COL_MAJOR) = 1,
    /** Global-order layout */
    TILEDB_LAYOUT_ENUM(GLOBAL_ORDER) = 2,
    /** Unordered layout */
    TILEDB_LAYOUT_ENUM(UNORDERED) = 3,
#endif

#ifdef TILEDB_COMPRESSOR_ENUM
    /** No compressor */
    TILEDB_COMPRESSOR_ENUM(NO_COMPRESSION) = 0,
    /** Gzip compressor */
    TILEDB_COMPRESSOR_ENUM(GZIP) = 1,
    /** Zstandard compressor */
    TILEDB_COMPRESSOR_ENUM(ZSTD) = 2,
    /** LZ4 compressor */
    TILEDB_COMPRESSOR_ENUM(LZ4) = 3,
    /** Run-length encoding compressor */
    TILEDB_COMPRESSOR_ENUM(RLE) = 4,
    /** Bzip2 compressor */
    TILEDB_COMPRESSOR_ENUM(BZIP2) = 5,
    /** Double-delta compressor */
    TILEDB_COMPRESSOR_ENUM(DOUBLE_DELTA) = 6,
#endif

#ifdef TILEDB_FILTER_TYPE_ENUM
    /** No-op filter */
    TILEDB_FILTER_TYPE_ENUM(FILTER_NONE) = 0,
    /** Gzip compressor */
    TILEDB_FILTER_TYPE_ENUM(FILTER_GZIP) = 1,
    /** Zstandard compressor */
    TILEDB_FILTER_TYPE_ENUM(FILTER_ZSTD) = 2,
    /** LZ4 compressor */
    TILEDB_FILTER_TYPE_ENUM(FILTER_LZ4) = 3,
    /** Run-length encoding compressor */
    TILEDB_FILTER_TYPE_ENUM(FILTER_RLE) = 4,
    /** Bzip2 compressor */
    TILEDB_FILTER_TYPE_ENUM(FILTER_BZIP2) = 5,
    /** Double-delta compressor */
    TILEDB_FILTER_TYPE_ENUM(FILTER_DOUBLE_DELTA) = 6,
    /** Bit width reduction filter. */
    TILEDB_FILTER_TYPE_ENUM(FILTER_BIT_WIDTH_REDUCTION) = 7,
    /** Bitshuffle filter. */
    TILEDB_FILTER_TYPE_ENUM(FILTER_BITSHUFFLE) = 8,
    /** Byteshuffle filter. */
    TILEDB_FILTER_TYPE_ENUM(FILTER_BYTESHUFFLE) = 9,
    /** Positive-delta encoding filter. */
    TILEDB_FILTER_TYPE_ENUM(FILTER_POSITIVE_DELTA) = 10,
#endif

#ifdef TILEDB_FILTER_OPTION_ENUM
    /** Compression level. Type: `int32_t`. */
    TILEDB_FILTER_OPTION_ENUM(COMPRESSION_LEVEL) = 0,
    /** Max window length for bit width reduction. Type: `uint32_t`. */
    TILEDB_FILTER_OPTION_ENUM(BIT_WIDTH_MAX_WINDOW) = 1,
    /** Max window length for positive-delta encoding. Type: `uint32_t`. */
    TILEDB_FILTER_OPTION_ENUM(POSITIVE_DELTA_MAX_WINDOW) = 2,
#endif

#ifdef TILEDB_ENCRYPTION_TYPE_ENUM
    /** No encryption. */
    TILEDB_ENCRYPTION_TYPE_ENUM(NO_ENCRYPTION) = 0,
    /** AES-256-GCM encryption. */
    TILEDB_ENCRYPTION_TYPE_ENUM(AES_256_GCM) = 1,
#endif

#ifdef TILEDB_QUERY_STATUS_ENUM
    /** Query failed */
    TILEDB_QUERY_STATUS_ENUM(FAILED) = 0,
    /** Query completed (all data has been read) */
    TILEDB_QUERY_STATUS_ENUM(COMPLETED) = 1,
    /** Query is in progress */
    TILEDB_QUERY_STATUS_ENUM(INPROGRESS) = 2,
    /** Query completed (but not all data has been read) */
    TILEDB_QUERY_STATUS_ENUM(INCOMPLETE) = 3,
    /** Query not initialized.  */
    TILEDB_QUERY_STATUS_ENUM(UNINITIALIZED) = 4,
#endif

#ifdef TILEDB_SERIALIZATION_TYPE_ENUM
    /** Serialize to json */
    TILEDB_SERIALIZATION_TYPE_ENUM(JSON),
    /** Serialize to capnp */
    TILEDB_SERIALIZATION_TYPE_ENUM(CAPNP),
#endif

#ifdef TILEDB_WALK_ORDER_ENUM
    /** Pre-order traversal */
    TILEDB_WALK_ORDER_ENUM(PREORDER) = 0,
    /** Post-order traversal */
    TILEDB_WALK_ORDER_ENUM(POSTORDER) = 1,
#endif

/** TileDB VFS mode */
#ifdef TILEDB_VFS_MODE_ENUM
    /** Read mode */
    TILEDB_VFS_MODE_ENUM(VFS_READ) = 0,
    /** Write mode */
    TILEDB_VFS_MODE_ENUM(VFS_WRITE) = 1,
    /** Append mode */
    TILEDB_VFS_MODE_ENUM(VFS_APPEND) = 2,
#endif
