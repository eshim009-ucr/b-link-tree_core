#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "memory.h"

//! @brief Search a tree for a key
//! @param[in]  root   The root of the tree to search
//! @param[in]  key    The key to search for
//! @return Struct containing requested data on success and an error code
bstatusval_t search(bptr_t root, bkey_t key, mread_req_stream_t *mem_read_reqs, mread_resp_stream_t *mem_read_resps);

#endif
