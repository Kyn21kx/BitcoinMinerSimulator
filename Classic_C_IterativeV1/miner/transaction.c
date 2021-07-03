#include "transaction.h"
#include "List.h"

Trans* initialize_transaction(unsigned int id, char* payer, char* payee, unsigned int amount, unsigned int fee)
{
	Trans* transaction = malloc(sizeof(Trans));
	
	strncpy(transaction->payer, payer, 32);
	strncpy(transaction->payee, payee, 32);
	
	transaction->id = id;
	transaction->amount = amount;
	transaction->fee = fee;
	transaction->signature = 0;
	
	return transaction;
}


Block* initialize_block(unsigned int id, unsigned int prevId, unsigned int prevSign) {
	Block* b = malloc(sizeof(Block));

	b->id = id;
	b->prevId = prevId;
	b->prevSign = prevSign;
	b->signature = 0;

	b->transactions = initialize_list();
	return b;
}
