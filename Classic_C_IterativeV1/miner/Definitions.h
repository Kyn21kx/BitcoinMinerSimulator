#include "transaction.h"
#include "List.h"
#include <stdbool.h>
#include <semaphore.h>

#ifdef WIN32
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_WARNINGS
#define HAVE_STRUCT_TIMESPEC
#endif
#include <pthread.h>


#define UPPER_BOUNDS 16777215

typedef struct Args
{
	int id;
	int totalThreads;
	Block b;
} Args;

typedef struct Results
{
	unsigned int signature;
	unsigned int nonce;
	bool valid;
} Results;

void check_events();

void set_up();

void execute_transaction(Trans* t);

Trans* transaction_from_str(char* str);

Node* find_in_list(List* l, unsigned int id);

Block* block_from_str(char* str, unsigned int* outCounter);

void influence_signature(Block* b, unsigned int transactions);

void calculate_signature(Block* b, Trans* t);

unsigned int get_trans_size(Trans* t);

void calculate_nonce(Block* b, int threads);

int get_priority(Trans* t);

void clamp(int* value, int maxValue);

void* thread_check_nonce(void* args);

List* tPool, *blocks;
Block* block, * prevBlock;
sem_t lock;
bool validNonce;