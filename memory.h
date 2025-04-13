#ifndef MEMORY_H
#define MEMORY_H


#include "types.h"
#include <stddef.h>

typedef struct Node Node;
typedef struct AddrNode AddrNode;

#ifdef HLS
#include "../memgr.hpp"
#include <hls_stream.h>
typedef hls::stream<mread_req_t> mread_req_stream_t;
typedef hls::stream<mread_resp_t> mread_resp_stream_t;
typedef hls::stream<mwrite_t> mwrite_stream_t;
#else
typedef void mread_req_stream_t;
typedef void mread_resp_stream_t;
typedef void mwrite_stream_t;
#endif

//! @brief Read a node from memory without grabbing any locks
Node mem_read(bptr_t address, mread_req_stream_t *req_stream, mread_resp_stream_t *resp_stream);

//! @brief Read a node from memory, locking it in memory
Node mem_read_lock(bptr_t address, mread_req_stream_t *req_stream, mread_resp_stream_t *resp_stream);

//! @brief Write a node to memory and unlock it
void mem_write_unlock(AddrNode *node, mwrite_stream_t *req_stream);

//! @brief Unlock a node in memory
//!
//! Used when a node has been read and locked, but should be unlocked without
//! modification. For example, if an error has been detected before a
//! modification operation completes.
void mem_unlock(bptr_t address, mwrite_stream_t *req_stream);

//! @brief Reset memory to a slate of blank nodes
//!
//! All data is 1s except for locks
void mem_reset_all();

extern bptr_t ptr_to_addr(void *ptr);


#endif
