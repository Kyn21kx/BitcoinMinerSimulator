#include "Definitions.h"

int main(int argc, char** argv) {
	set_up();
	check_events();
	free_list_nodes(tPool);
	free_list_nodes(blocks);
}

void set_up()
{
	tPool = initialize_list();
	blocks = initialize_list();
	blocks->ordered = false;
	validNonce = false;
}

void check_events()
{
	unsigned int numberOfTransactions = 0;
	
	char buffer[100];

	// fgets function is safer than scanf, and although makes for some issues with
	// getting data from a non formatted string, it is quite manageable
	while (fgets(buffer, 100, stdin) != NULL)
	{
		char selection[5];
		strncpy(selection, buffer, 4);
		
		// Null terminate the string
		*(selection + 4) = 0;
		
		// Added one space to make it consistent across the various events
		if (strcmp(selection, "BLK ") == 0)
		{
			block = block_from_str(buffer, &numberOfTransactions);
			
			NEXTLINE:
			// The rest of the info is in the next line, so we change the buffer
			fgets(buffer, 100, stdin);
			
			// If there are no transactions, there must be another line detailing the nonce and the signature of the block
			if (numberOfTransactions < 1)
			{
				// Hex conversion from https://stackoverflow.com/a/10156436
				char* subStr = strtok(buffer, " ");
				
				unsigned int nonce = (unsigned int)strtol(subStr, NULL, 16);
				
				subStr = strtok(NULL, " ");
				
				unsigned int sign = (unsigned int)strtol(subStr, NULL, 16);
				
				block->signature = sign;
				block->nonce = nonce;
				
				add_to_list(blocks, block, 0);
			}
			else
			{
				// Get the integer value of the buffer
				char* id = strtok(buffer, " ");
				unsigned int numberId = atol(id);

				Node* targetNode = find_in_list(tPool, numberId);

				// If the node is null, we just skip the rest of the function (except for the decrease part)
				if (targetNode == NULL)
				{
					goto DECREASE;
				}
				Trans* target = targetNode->data;
				
				printf("Removing transaction: ");
				printf("%u %s %s %u %u\n", target->id, target->payer, target->payee, target->amount, target->fee);
				
				add_to_list(block->transactions, target, 0);
				delete_from_list(tPool, targetNode);
				DECREASE:
				numberOfTransactions--;
				goto NEXTLINE;
			}
		}

		else if (strcmp(selection, "TRX ") == 0)
		{
			Trans* t = transaction_from_str(buffer);
			execute_transaction(t);
		}

		// The EPOCH event is 5 characters long, but we just need the first 4 to be sure :)
		else if (strcmp(selection, "EPOC") == 0)
		{
			/*
			* TODO: the priority will be gathered from the Node itself, not the function
			*/
			Node* iteration = tPool->head;
			int prev = -1;
			while (iteration->data != NULL)
			{
				// Skip to the next level of priority once we have sorted the head
				if (iteration->priority == prev || iteration->priority >= 9)
				{
					iteration = iteration->next;
					continue;
				}
				prev = iteration->priority;
				iteration->priority++;
				Trans* target = (Trans*)iteration->data;
				printf("Aging transaction (%d):", iteration->priority);
				printf("%u %s %s %u %u\n", target->id, target->payer, target->payee, target->amount, target->fee);
				iteration = iteration->next;
			}
			sort_list(tPool);
		}
		
		else if (strcmp(selection, "END ") == 0 || strcmp(selection, "END\n") == 0)
		{
			return;
		}
		
		else if (strcmp(selection, "MINE") == 0)
		{
			prevBlock = blocks->tail->data;
			block = prevBlock == NULL ? initialize_block(1, 0, 0) : initialize_block(prevBlock->id + 1, prevBlock->id, prevBlock->signature);
			
			unsigned int nt = 0;
			if (tPool->length < 1)
			{
				goto SIGNATURE;
			}
			int available = 232;
			Node* it = tPool->head;
			Trans* itTrans = it->data;
			int itTransSize = get_trans_size(itTrans);

			/*
			* TODO: If the current size exceeds the available space, go look for another one down in the pool
			*/

			while (it->data != NULL)
			{
				if (available <= 0)
				{
					break;
				}
				int diff = available - itTransSize;
				if (diff > 0)
				{
					add_to_list(block->transactions, itTrans, 0);
					delete_from_list(tPool, it);
					available = diff;
					nt++;
				}
				
				it = it->next;
				itTrans = it->data;
				if (itTrans == NULL)
				{
					break;
				}
				itTransSize = get_trans_size(itTrans);
			}
			SIGNATURE:
			influence_signature(block, nt);

			printf("Block mined: ");
			printf("%u %u 0x%8.8x %u\n", block->id, block->prevId, block->prevSign, nt);
			
			it = block->transactions->head;
			Trans* data = it->data;
			
			while (data != NULL)
			{
				calculate_signature(block, data);
				printf("%u %s %s %u %u\n", data->id, data->payer, data->payee, data->amount, data->fee);
				it = it->next;
				data = it->data;
			}
			block->nonce = 0;

			// Get the amount of threads needed
			// The threads will be at the offset MINE + SPACE (5)
			char strThreads[5] = { 0 };
			strcpy(strThreads, buffer + 5);
			int threads = atoi(strThreads);

			calculate_nonce(block, threads);
			add_to_list(blocks, block, 0);
			printf("0x%8.8x 0x%8.8x\n", block->nonce, block->signature);
		}
	}
}

void execute_transaction(Trans* t)
{
	int priority = get_priority(t);
	add_to_list(tPool, t, priority);
	printf("Adding transaction: ");
	printf("%u %s %s %u %u\n", t->id, t->payer, t->payee, t->amount, t->fee);
}

Node* find_in_list(List* l, unsigned int id)
{
	Node* iteration = l->head;
	while (iteration->data != NULL)
	{
		Trans* trPtr = iteration->data;
		if (trPtr->id == id)
		{
			return iteration;
		}
		iteration = iteration->next;
	}
	return NULL;
}

Trans* transaction_from_str(char* str)
{
	// Eliminate the prefix by tokenizing the string once
	char* subStr = strtok(str, " ");
	
	subStr = strtok(NULL, " ");
	unsigned int id = atol(subStr);

	char* p1 = strtok(NULL, " ");
	char* p2 = strtok(NULL, " ");
	
	subStr = strtok(NULL, " ");
	unsigned int amount = atol(subStr);
	
	subStr = strtok(NULL, " ");
	unsigned int fee = atol(subStr);

	Trans* t = initialize_transaction(id, p1, p2, amount, fee);
	return t;
}

Block* block_from_str(char* str, unsigned int* outCounter) {
	char* subStr = strtok(str, " ");

	subStr = strtok(NULL, " ");
	unsigned int id = atol(subStr);
	
	subStr = strtok(NULL, " ");
	unsigned int prevId = atol(subStr);
	
	subStr = strtok(NULL, " ");
	// Hex conversion from https://stackoverflow.com/a/10156436
	unsigned int prevSign = (unsigned int)strtol(subStr, NULL, 16);
	
	subStr = strtok(NULL, " ");
	*outCounter = atol(subStr);

	Block* b = initialize_block(id, prevId, prevSign);
	return b;
}

void influence_signature(Block* b, unsigned int transactions)
{
	b->signature = siggen_new();
	b->signature = siggen_int(b->signature, b->id);
	b->signature = siggen_int(b->signature, b->prevId);
	b->signature = siggen_int(b->signature, b->prevSign);
	b->signature = siggen_int(b->signature, transactions);
}

void calculate_signature(Block* b, Trans* t)
{
	b->signature = siggen_int(b->signature, t->id);
	b->signature = siggen_string(b->signature, t->payer);
	b->signature = siggen_string(b->signature, t->payee);
	b->signature = siggen_int(b->signature, t->amount);
	b->signature = siggen_int(b->signature, t->fee);
}

unsigned int get_trans_size(Trans* t)
{
	if (t == NULL)
	{
		return -1;
	}
	// The number of bytes the integers take + the number of bytes the strings take
	return strlen(t->payer) + strlen(t->payee) + 14;
}

void calculate_nonce(Block* b, int threads)
{
	validNonce = false;
	pthread_t* children = malloc(sizeof(pthread_t) * threads);
	Args** arguments = malloc(sizeof(Args) * threads);
	Results** results = malloc(sizeof(Results) * threads);

	// Semaphore initialization
	sem_init(&lock, 0, 1);

	for (int i = 0; i < threads; i++) 
	{
		arguments[i] = malloc(sizeof(Args));
		arguments[i]->b = *b;
		arguments[i]->id = i;
		arguments[i]->totalThreads = threads;
		pthread_create(&children[i], NULL, thread_check_nonce, arguments[i]);
	}

	for (int i = 0; i < threads; i++)
	{
		pthread_join(children[i], (void**)&results[i]);
		if (results[i] != NULL && (results[i]->nonce < b->nonce || b->nonce == 0) && results[i]->valid) 
		{
			b->nonce = results[i]->nonce;
			b->signature = results[i]->signature;
		}
	}

	for (int i = 0; i < threads; i++)
	{
		if (results[i] != NULL)
		{
			free(results[i]);
			results[i] = NULL;
		}
		if (arguments[i] != NULL)
		{
			free(arguments[i]);
			arguments[i] = NULL;
		}
	}

	free(children);
	free(arguments);
	free(results);

	//Destroy semaphore
	sem_destroy(&lock);
}

int get_priority(Trans* t)
{
	int size = get_trans_size(t);
	int p = (int)floor((float)t->fee / (float)size);
	clamp(&p, 9);
	return p;
}

void clamp(int* value, int maxValue)
{
	if (*value > maxValue)
	{
		*value = maxValue;
	}
}

void* thread_check_nonce(void* args)
{
	Args* m_args = (Args*)args;
	Block b = m_args->b;
	int id = m_args->id;
	int threads = m_args->totalThreads;
	int iteration = 0;
	unsigned int nonce = 0;
	unsigned int blockSig = b.signature;
	// f(i) = iN + t
	while (true)
	{
		nonce = (iteration * threads) + id;
		unsigned int sig = siggen_int(blockSig, nonce);

		// Critical sections
		printf("Thread %d checking nonce 0x%8.8x\n", id, nonce);
		if (sig <= UPPER_BOUNDS)
		{
			sem_wait(&lock);
			Results* res = malloc(sizeof(Results));
			res->signature = sig;
			res->nonce = nonce;
			res->valid = true;
			sem_post(&lock);
			return res;
		}
		iteration++;
	}
	return NULL;
}
