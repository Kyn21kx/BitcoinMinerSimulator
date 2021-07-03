#include "transaction.h"
#include "list.h"
#include "block.h"
#include <math.h>

#define HEX 16
#define MAX_BLOCK_SIZE 232

#pragma region Global variables
List* mempool[10];
List* blockchain;
Block* currentBlock;
Block* previousBlock;
uint blockT;
bool finalBlockCommand = false;
#pragma endregion

#pragma region Function definitions
void process_commands();
void send_to_mempool(const char* rawLine);
void buffer_block(const char* rawLine);
void append_tx_to_block(const char* rawLine);
void process_final_block_command(const char* rawLine);
void mine_block(uint threads);
void salt_signature(Block* block, Transaction* tx);
uint block_id_getter(void* block);
uint tx_id_getter(void* tx);
Transaction* process_tx_encoding(const char* encoding);
Block* process_block_encoding(const char* encoding);
Block* get_last_block(List* list);
int calculate_priority(Transaction* transaction);
List* split_str(const char* str, char delimitter);
char* get_substr_delimitter(const char* str, char delimitter, uint offset);
char* get_substr(const char* str, uint offset);
uint hex2int(char* hex);
void age_transactions();
#pragma endregion

int main(int argc, char** argv) {
	//Initialization
	for (uint i = 0; i < 10; i++) {
		mempool[i] = new_list();
	}
	blockchain = new_list();
	currentBlock = NULL;
	previousBlock = NULL;
	blockT = 0;
	process_commands();

	for (uint i = 0; i < 10; i++) {
		list_free(mempool[i]);
	}

	list_free(blockchain);
	return 0;
}

void process_commands() {
	char line[200];
	while (fgets(line, 200, stdin) != NULL) {
		
		if (finalBlockCommand) {
			process_final_block_command(line);
			finalBlockCommand = false;
			continue;
		}

		if (blockT < 1) {
			if (strstr(line, "TRX") != NULL) {
				send_to_mempool(line);
			}
			else if (strstr(line, "BLK") != NULL) {
				//This will ignore this section in the next iteration because we need more information to add the block to the blockchain
				buffer_block(line);
			}
			else if (strstr(line, "MINE") != NULL) {
				char* nThreadsRaw = get_substr(line, 5);
				uint t = atol(nThreadsRaw);
				mine_block(t);
				free(nThreadsRaw);
			}
			else if (strstr(line, "EPOCH") != NULL) {
				age_transactions();
			}
			else if (strstr(line, "END") != NULL) {
				return;
			}
			continue;
		}
		append_tx_to_block(line);
	}
}

void send_to_mempool(const char* rawLine) {
	//The line starts with TRX and a space
	char* line = get_substr(rawLine, 4);
	Transaction* toAdd = process_tx_encoding(line);
	
	//Get the transaction's priority
	int priority = calculate_priority(toAdd);

	//Add it to the correct list
	list_add(mempool[priority], toAdd);
	printf("Adding transaction: ");
	print_added_tx(toAdd);
	free(line);
}

void buffer_block(const char* rawLine) {
	char* line = get_substr(rawLine, 4);
	currentBlock = process_block_encoding(line);
	blockT = currentBlock->t;
	finalBlockCommand = blockT == 0;
	free(line);
}

void append_tx_to_block(const char* rawLine) {
	printf("Removing transaction: ");
	char* strId = get_substr_delimitter(rawLine, ' ', 0);
	uint id = atol(strId);
	List* container = NULL;
	int priorityIndex = -1;
	for (uint i = 0; i < 10; i++) {
		container = list_find(mempool[i], &tx_id_getter, id);
		//If the container is not null, we have found the transaction
		if (container != NULL) {
			priorityIndex = i;
			break;
		}
	}
	Transaction* tx = (Transaction*)container->element;
	//Copy to the list of transactions of the current block
	list_add(currentBlock->transactions, tx);
	//Take care of the heap allocated memory
	print_added_tx(tx);
	list_delete(&(mempool[priorityIndex]), container);
	free(strId);
	//Reduce the variable that keeps track of how many transactions there are
	blockT--;
	finalBlockCommand = blockT == 0;
}

void process_final_block_command(const char* rawLine) {
	//Get the first substr (0x ignore)
	char* partPtr = get_substr_delimitter(rawLine, ' ', 2);
	uint parsedNonce = strtol(partPtr, NULL, HEX);
	uint off = strlen(partPtr);
	free(partPtr);
	//2 (0x) + strlen + 1 (space) + 2(0x)
	partPtr = get_substr(rawLine, off + 2 + 1);
	uint parsedSig = strtol(partPtr, NULL, HEX);
	currentBlock->signature = parsedSig;
	currentBlock->nonce = parsedNonce;
	list_add(blockchain, currentBlock);
	free(partPtr);
}

void mine_block(uint threads) {
	//Create a new block with the signature based on the previous one
	previousBlock = get_last_block(blockchain);
	if (previousBlock == NULL) {
		currentBlock = new_block(1, 0, 0, 0);
	}
	else {
		currentBlock = new_block(previousBlock->id + 1, previousBlock->id, previousBlock->signature, 0);
	}
	List* current;
	Transaction* currTransaction;
	int max = MAX_BLOCK_SIZE;
	uint s1Length, s2Length;
	int elementSize;

	for (int i = 9; i >= 0; i--) {
		if (!list_is_empty(mempool[i])) {
			current = mempool[i];
			currTransaction = (Transaction*)current->element;
			
			while (currTransaction != NULL) {
				List* next = current->next;
				
				s1Length = strlen(currTransaction->payee);
				s2Length = strlen(currTransaction->payer);
				elementSize = (4 * 3) + 2 + s1Length + s2Length;
				int subResult = max - elementSize;

				if (subResult > 0) {
					//There's space available, so we can add, delete, and do our stuff
					//Add the current transaction to the linked list
					list_add(currentBlock->transactions, currTransaction);
					list_delete(&mempool[i], current);
					max = subResult;
				}
				current = next;
				currTransaction = (Transaction*)current->element;
			}
		}
	}
	//To avoid input errors
	currentBlock->t = currentBlock->transactions->count;
	set_initial_signature(currentBlock);
		
	printf("Block mined: ");
	printf("%u %u 0x%8.8x %u\n", currentBlock->id, currentBlock->previousId, currentBlock->previousSig, currentBlock->t);
		
	current = currentBlock->transactions;
	currTransaction = (Transaction*)current->element;
	while (currTransaction != NULL) {
		salt_signature(currentBlock, currTransaction);
		print_added_tx(currTransaction);
		current = current->next;
		currTransaction = (Transaction*)current->element;
	}
	set_final_signature(currentBlock, threads);
	list_add(blockchain, currentBlock);
	printf("0x%8.8x 0x%8.8x\n", currentBlock->nonce, currentBlock->signature);
}

void salt_signature(Block* block, Transaction* tx) {
	block->signature = siggen_int(block->signature, tx->id);
	block->signature = siggen_string(block->signature, tx->payer);
	block->signature = siggen_string(block->signature, tx->payee);
	block->signature = siggen_int(block->signature, tx->amount);
	block->signature = siggen_int(block->signature, tx->fee);
}

uint block_id_getter(void* block) {
	Block* ptr = (Block*)block;
	return ptr->id;
}

uint tx_id_getter(void* tx) {
	Transaction* ptr = (Transaction*)tx;
	return ptr->id;
}

Block* get_last_block(List* list) {
	List* current = list;
	List* prev = current;
	while (current->element != NULL) {
		prev = current;
		current = current->next;
	}
	return (Block*)prev->element;
}

Transaction* process_tx_encoding(const char* encoding) {
	List* commands = split_str(encoding, ' ');
	List* head = commands;
	//Transaction encoding is:
	//TID Payer Payee Amount Fee
	uint tId = atol((char*)commands->element);
	commands = commands->next;
	char* payer = (char*)commands->element;
	commands = commands->next;
	char* payee = (char*)commands->element;
	commands = commands->next;
	uint amount = atol((char*)commands->element);
	commands = commands->next;
	uint fee = atol((char*)commands->element);
	Transaction* result = new_transaction(tId, amount, fee, payer, payee);
	list_free(head);
	free(payer);
	free(payee);
	return result;
}

int test_mempool_length() {
	int sum = 0;
	for (int i = 0; i < 10; i++) {
		sum += mempool[i]->count;
	}
	return sum;
}

Block* process_block_encoding(const char* encoding) {
	char* commands = get_substr_delimitter(encoding, ' ', 0);
	uint off = strlen(commands) + 1;
	//Block encoding is:
	//Id previousId prevSig t
	uint id = atol(commands);
	
	free(commands);
	commands = get_substr_delimitter(encoding, ' ', off);
	off += strlen(commands) + 1;
	
	uint previousId = atol(commands);
	
	free(commands);
	commands = get_substr_delimitter(encoding, ' ', off);
	off += strlen(commands) + 1;
	
	char* hexSubStr = get_substr(commands, 2);
	uint previousSig = hex2int(hexSubStr);
	commands = get_substr(encoding, off);
	uint t = atol(commands);
	Block* result = new_block(id, previousId, previousSig, t);
	free(hexSubStr);
	free(commands);
	return result;
}

int calculate_priority(Transaction* transaction) {
	uint s1Length = strlen(transaction->payee);
	uint s2Length = strlen(transaction->payer);
	int encodingSize = (4 * 3) + 2 + s1Length + s2Length;
	int priority = floor((double)transaction->fee / (double)encodingSize);
	//Keep the priority at a maximum of 9
	if (priority > 9) {
		priority = 9;
	}
	return priority;
}

List* split_str(const char* str, char delimitter) {
	List* result = new_list();
	char* currentSub = get_substr_delimitter(str, delimitter, 0);
	uint currentOffset = 0;
	while (currentSub != NULL) {
		list_add(result, currentSub);
		currentOffset += strlen(currentSub) + 1;
		currentSub = get_substr_delimitter(str, delimitter, currentOffset);
	}
	if (currentOffset > 0) {
		currentSub = get_substr(str, currentOffset);
		list_add(result, currentSub);
	}
	return result;
}

char* get_substr_delimitter(const char* str, char delimitter, uint offset) {
	uint length = strlen(str);
	char* temp = malloc(sizeof(char) * (length + 1));
	strcpy(temp, str + offset);
	for (uint i = 0; i < strlen(temp); i++) {
		if (temp[i] == delimitter) {
			char* r = malloc(sizeof(char) * (i + 1));
			strncpy(r, temp, i);
			r[i] = 0;
			free(temp);
			return r;
		}
	}
	return NULL;
}

char* get_substr(const char* str, uint offset) {
	uint length = strlen(str) - offset + 1;
	char* res = malloc(sizeof(char) * length);
	strncpy(res, str + offset, length);
	return res;
}

//Function provided by stack overflow's radhoo
//https://stackoverflow.com/questions/10324/convert-a-hexadecimal-string-to-an-integer-efficiently-in-c
uint hex2int(char* hex) {
	uint val = 0;
	while (*hex) {
		// get current character then increment
		char byte = *hex++;
		// transform hex character to the 4bit equivalent number, using the ascii table indexes
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
		// shift 4 to make space for new digit, and add the 4 bits of the new digit 
		val = (val << 4) | (byte & 0xF);
	}
	return val;
}

void age_transactions() {
	List* movedTransactions = new_list();
	//Get the head for each
	for (int i = 8; i >= 0; i--) {
		if (!list_is_empty(mempool[i])) {
			//Move head to the tail of the next level
			Transaction* t = (Transaction*)mempool[i]->element;
			list_delete(&(mempool[i]), mempool[i]);
			printf("Aging transaction (%d):", i + 1);
			print_added_tx(t);
			list_add(mempool[i + 1], t);
		}
	}
}
