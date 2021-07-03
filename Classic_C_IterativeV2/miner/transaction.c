#include "transaction.h"
#include <time.h>

Transaction* new_transaction(uint id, uint amount, uint fee, char payer[32], char payee[32]) {
	Transaction* result = malloc(sizeof(Transaction));
	//Copy the integers
	result->id = id;
	result->amount = amount;
	result->fee = fee;
	//Copy the strings
	strcpy(result->payer, payer);
	strcpy(result->payee, payee);
	return result;
}

void print_added_tx(Transaction* tx) {
	printf("%u %s %s %u %u\n", tx->id, tx->payer, tx->payee, tx->amount, tx->fee);
}