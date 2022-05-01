#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

int hit_count = 0, miss_count = 0, eviction_count = 0;
int s, E, b; /* 2^s: set count;  E: line count in a set */
int verbose_flag = 0, help_flag = 0;
char *trace_file_name;
FILE *trace_file;

struct Line {
    int valid_bit;
    unsigned int tag;
    struct Line *next;
};

struct Set{
    int E;  // associativity number of cache line in the set
    struct Line *head_line;
    struct Set *next;
    struct Line *tail_line;
};

struct Cache{
    int S;  // number of set in the cache
    struct Set *head_set;
};

/* init struct, allocate memory */
void init_cache(struct Cache *cache, int s, int E);
void init_set(struct Set *set, int E);
void init_line(struct Set *set, unsigned int tag);

/*  address handle  */
int get_block_offset(unsigned int address, int b);
int get_line_tag(unsigned int address, int b, int s);
int get_set_index(unsigned int address, int b, int s);

/* operations/steps when accessing the cache  */
struct Set* get_set_by_index(struct Cache* cache, int setIndex);
int get_set_size(struct Set* set);
struct Line* find_line_in_set(struct Set *set, int tag, short *evicted);

/* LRU operations */
void move_line_to_tail(struct Set *set, struct Line* targetLine);
void remove_head_line(struct Set *set, int tag);

/* main access function */
void access_cache(struct Cache* cache, int addr);

// for test
int get_set_num(struct Cache* cache){
    int size = 0;
    struct Set* cur = cache->head_set;
    while(cur != NULL){
        size++;
        cur = cur->next;
    }
    return size;
}

int main(int argc, char *argv[]){
   int opt;
   char op;
   unsigned addr;
   int size;

	/* parse flag commands */
	while(-1 != (opt = getopt(argc, argv, "vhs:E:b:t:"))) {
		switch(opt) {
			case 'v':
				verbose_flag = 1;
				break;
			case 'h':
				help_flag = 1;
				break;
			case 's':
				s = atoi(optarg);
				if(s < 0 && s > 64 ){
					printf("Error: s value must be in [0, word_length].\n");
					exit(-1);
				}
				break;
			case 'E':
				E = atoi(optarg);
				if(E <= 0){
					printf("Error: E value must be larger than 0.\n");
					exit(-1);
				}
				break;
			case 'b':
				b = atoi(optarg);
				if(b < 0 || b > 64){
					printf("Error: b value must be in [0, word_length].\n");
					exit(-1);
				}
				break;
			case 't':
				trace_file_name = optarg;
				break;
			default:
				break;
		}
	}	   


	    
    struct Cache *cache = (struct Cache*) malloc(sizeof(struct Cache));
    init_cache(cache, s, E);


    trace_file = fopen(trace_file_name, "r"); 
    if (!trace_file){
		fprintf(stderr, "Error: Trace file cannot be opened.\n");
		return -1;
	} 
	while(fscanf(trace_file, "%c %x, %d", &op, &addr, &size) > 0) {
		if (verbose_flag) printf("%c %x, %d ", op, addr, size);
		switch(op){
			case 'L':
				access_cache(cache, addr);
				break;
			case 'S':
				access_cache(cache, addr);
				break;
			case 'M':
				access_cache(cache, addr);
				access_cache(cache, addr);
				break;
		}
		if (verbose_flag) printf("\n");
	}

    fclose(trace_file);
    
    printSummary(hit_count, miss_count, eviction_count);
    
    return 0;
}


/**
 1. find set by index
 2. traverse all the lines in the set
 3. LRU strategy
 */
void access_cache(struct Cache* cache, int addr){
    int setIndex = get_set_index(addr, b, s);
    struct Set *targetSet = get_set_by_index(cache, setIndex);

    int matchTag = get_line_tag(addr, b, s);
    short evicted = 0;
    struct Line *targetLine = find_line_in_set(targetSet, matchTag, &evicted);

    // 1. has empty line space and no match => miss_count, and bring in
    // 2. no empty line space and no match => eviction_count, and pick a victim (LRU)
    // 3. tag matches, hit_count, and move to tail
    if(targetLine != NULL){
        hit_count++;
        targetLine->tag = matchTag;
        targetLine->valid_bit = 1;
        move_line_to_tail(targetSet, targetLine);  // LRU - move to tail

    }else if(targetLine == NULL && !evicted){
        miss_count++;
        // need to bring in a new line
        init_line(targetSet, matchTag);
    }else if(evicted){
        miss_count++;
        eviction_count++;
        // LRU - evict head_line (not really evicted and freed here, just move it to the tail and update the tag)
        remove_head_line(targetSet, matchTag);
    }
}

int get_set_size(struct Set* set){
    int line_count = 0;
    struct Line *cur = set->head_line;
    while (cur != NULL) {
        line_count++;
        cur = cur->next;
    }
    return line_count;
}

struct Set* get_set_by_index(struct Cache* cache, int setIndex){
    struct Set *curSet = cache->head_set;
    for(int i = 0; i != setIndex; i++){
        curSet = curSet->next;
    }
    return curSet;
}

/**
 try to find a matching line in the set
 */
struct Line* find_line_in_set(struct Set *set, int tag, short *evicted){
    struct Line *curLine = set->head_line;
    struct Line* targetLine = NULL;

    int curLineNum = get_set_size(set);
    if(curLineNum == 0){
        return targetLine;
    }
    int i;
    for(i = 1; i <= curLineNum; i++){
        if(tag == curLine->tag){
            return curLine;
        }
        curLine = curLine->next;
    }
    if(targetLine == NULL && curLineNum == (set->E) ){ // traverse all the lines but no tag matched, and set is full
        *evicted = (*evicted) ^ 1;
    }
    return targetLine;
}

// LRU - move hit line to tail
void move_line_to_tail(struct Set *set, struct Line* targetLine){
    struct Line* head = set->head_line;
    struct Line* tail = set->tail_line;
    struct Line* cur = head;

    if(targetLine == tail) return;

    if(targetLine == head){
        struct Line* cur = head;
        set->head_line = head->next;
        cur->next = NULL;
        tail->next = cur;
        set->tail_line = cur;
    }else{
        // line in the middle
        while(cur->next != targetLine){
            cur = cur->next;
        }
        // cur => targetLine.prev
        cur->next = targetLine->next;
        targetLine->next = NULL;
        tail->next = targetLine;
        set->tail_line = targetLine;
    }
}
/**
* not really removed and freed, just moved to tail and updated line tag
*/
void remove_head_line(struct Set *set, int tag){
    struct Line* removed = set->head_line;
    removed->tag = tag;
    removed->valid_bit=1;
    move_line_to_tail(set, removed);
}


/**
 * allocate memory for a certain number of sets in the cache
 *
 * cache: reference to the cache
 * s: the number of Set in the cache is 2^s
 * E: the number of Line in a Set is E
 */
void init_cache(struct Cache *cache, int s, int E){
    int setNumber = 1 << s;
    cache->S = setNumber;
    struct Set* headSet = (struct Set*) malloc(sizeof(struct Set));
    init_set(headSet, E);
    cache->head_set = headSet;

    struct Set* currSet = headSet;
    for(int i = 1; i < setNumber; i++){
        struct Set* nextSet = (struct Set*) malloc(sizeof(struct Set));
        init_set(nextSet, E);
        currSet->next = nextSet;
        currSet = nextSet;
    }
}

/**
* init an empty set
* E: the max number of lines in the set
*/
void init_set(struct Set *set, int E){
    int lineNumber = E;
    set->E = lineNumber;
    set->head_line = NULL;
    set->next = NULL;
    set->tail_line = NULL;
}

/**
 1. empty set, need to create the first line after miss
 2. no matched tag among existing lines, need to create a new line after miss
 */
void init_line(struct Set *set, unsigned int tag){
    struct Line* newLine = (struct Line*) malloc(sizeof(struct Line));
    newLine->valid_bit = 1;
    newLine->tag = tag;
    newLine->next=NULL;

    struct Line* head = set->head_line;
    if(head == NULL){
        set->head_line = newLine;
        set->tail_line = newLine;
    }else{
        set->tail_line->next = newLine;
        set->tail_line = newLine;
    }
}

int get_block_offset(unsigned int address, int b){
    unsigned int blockMask = (1 << b) - 1;
    return address & blockMask;
}

int get_line_tag(unsigned int address, int b, int s){
    unsigned int tagMask = (1 << (32- b - s)) - 1;
    address >>= (b + s);
    return address & tagMask;
}

int get_set_index(unsigned int address, int b, int s){
    unsigned int setIndexMask = (1 << s) - 1;
    address >>= b;
    return address & setIndexMask;
}
