#ifndef TRANSACTION
#define TRANSACTION
#include "siggen.h"
#include "List.h"

typedef struct Trans
{
	unsigned int id;
	char payer[32];
	char payee[32];
	unsigned int amount;
	unsigned int fee;
	unsigned int signature;
} Trans;

typedef struct Block 
{
	List* transactions;
	unsigned int id;
	unsigned int prevId;
	unsigned int prevSign;
	unsigned int nonce;
	unsigned int signature;
} Block;

Trans* initialize_transaction(unsigned int id, char* payer, char* payee, unsigned int amount, unsigned int fee);

Block* initialize_block(unsigned int id, unsigned int prevId, unsigned int prevSign);

#endif 
