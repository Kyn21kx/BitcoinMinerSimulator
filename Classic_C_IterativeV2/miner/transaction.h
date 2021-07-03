#ifndef TRANSACTION
#define TRANSACTION
#include "list.h"

typedef struct Transaction {
	uint id;
	uint amount;
	uint fee;
	char payer[32];
	char payee[32];
}
Transaction;

Transaction* new_transaction(uint id, uint amount, uint fee, char payer[32], char payee[32]);

void print_added_tx(Transaction* tx);

#endif 
