#ifndef BLOCK_H
#define BLOCK_H
#include "siggen.h"
#include "list.h"
#include <pthread.h>

#define MAX_ZERO_BYTED 0x00FFFFFF

typedef struct Block {
	uint id;
	uint previousId;
	uint previousSig;
	uint t;
	uint nonce;
	uint signature;
	List* transactions;
}
Block;

typedef struct Arguments{
	uint signature;
	uint id;
	uint numberThreads;
} Arguments;

typedef struct CorrectNoncePair {
	uint signature;
	uint nonce;
} CorrectNoncePair;

Block* new_block(uint id, uint previousId, uint previousSig, uint t);

void set_initial_signature(Block* target);

void set_final_signature(Block* block, uint threads);

void* single_thread_nonce_check(void* arguments);

#endif // !BLOCK_H
