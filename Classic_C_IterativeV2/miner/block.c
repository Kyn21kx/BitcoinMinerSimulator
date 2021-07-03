#include "block.h"

Block* new_block(uint id, uint previousId, uint previousSig, uint t) {
	Block* result = (Block*)malloc(sizeof(Block));
	result->id = id;
	result->previousId = previousId;
	result->previousSig = previousSig;
	result->t = t;
	result->transactions = new_list();
	return result;
}

void set_initial_signature(Block* target) {
	target->signature = siggen_new();
	target->signature = siggen_int(target->signature, target->id);
	target->signature = siggen_int(target->signature, target->previousId);
	target->signature = siggen_int(target->signature, target->previousSig);
	target->signature = siggen_int(target->signature, target->t);
}

void set_final_signature(Block* block, uint threads) {
	//Initialize the threads
	pthread_t* execThreads = malloc(sizeof(pthread_t) * threads);
	//Initialize the arguments as a signature, id, numberThreads
	Arguments* args = malloc(sizeof(Arguments) * threads);
	//This variable will store all the correct nonces found
	CorrectNoncePair** correctPairs = malloc(sizeof(CorrectNoncePair*) * threads);

	uint initialSignature = block->signature;

	for (uint i = 0; i < threads; i++) {
		args[i].signature = initialSignature;
		args[i].id = i;
		args[i].numberThreads = threads;
		pthread_create(&execThreads[i], NULL, single_thread_nonce_check, &args[i]);
	}
	//Separate for to join the threads
	for (uint i = 0; i < threads; i++) {
		pthread_join(execThreads[i], (void**)&correctPairs[i]);
		//If the returning value from the function stored in the array is not null, then the nonce was found
		if (correctPairs[i] != NULL) {
			if (correctPairs[i]->nonce >= block->nonce && block->nonce != 0) {
				continue;
			}
			//The pair's nonce is less than the current block's nonce, so we can replace it
			block->nonce = correctPairs[i]->nonce;
			block->signature = correctPairs[i]->signature;
		}
	}

}

void* single_thread_nonce_check(void* arguments) {
	//Allocate here so that we don't have to waste time allocating the result when we find the nonce
	CorrectNoncePair* pair = malloc(sizeof(CorrectNoncePair));
	//Dereference pointer that contains arguments
	Arguments args = *(Arguments*)(arguments);
	uint factor = 0;
	uint result = 0;
	uint potentialNonce = 0;
	do {
		potentialNonce = (factor * args.numberThreads) + args.id;
		printf("Thread %d checking nonce 0x%8.8x\n", args.id, potentialNonce);
		result = siggen_int(args.signature, potentialNonce);
		factor++;
	} while (result > MAX_ZERO_BYTED);
	//block->nonce = initialNonce > 0 ? initialNonce - 1 : initialNonce;
	pair->nonce = potentialNonce;
	pair->signature = result;
	return pair;
}
