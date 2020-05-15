#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <netdb.h>
#include "../libcs50/hashtable.h"
#include "../libcs50/set.h"
#include "../libcs50/bag.h"
#include "../libcs50/webpage.h"
#include "../libcs50/memory.h"
#include "../common/pagedir.h"

int crawl(char *seed, int max_depth, int curr_depth, char* cleandirectory);
void pagefetcher(webpage_t *page, int seconds_to_sleep);
void pagesaver(webpage_t *page, char *dir, int id);
void crawler_bag_insert(hashtable_t *table, char *URL, bag_t *bag, webpage_t *newpage);
void pagescanner(char *URL, webpage_t *page, int *pos, bag_t* bag, hashtable_t *table);
void bag_to_string(FILE *fp, void*item);
int check_creations(bag_t *bag, hashtable_t *table);
void itemdeletehashtable(void *item);
void itemdeletebag(void *item);

int main(int argc, char **argv){

	int curr_depth = 0;
	char *directory = malloc(sizeof(char*));
	directory = strcpy(directory, argv[2]);
	char *cleandirectory = malloc(sizeof(char*));
	cleandirectory = strcpy(cleandirectory, argv[2]);	


	
	// this is for non-gdb
	char *seed = argv[1]; 
	int max_depth = strtol(argv[3], NULL, 0);

	
	bool seedcheck = IsInternalURL(seed);
	// the block of code below is to check if user inputs are valid
	if(seed == NULL || max_depth < 0 || seedcheck == false || strcmp(directory, "") == 0 ){

		printf("whoops, one of the inputs was incorrect.\n");
		return 1;
	}

	int pos_last_letter = strlen(directory) - 1;
	char last_letter = directory[pos_last_letter];

	if(last_letter != '/'){
		strcat(cleandirectory, "/");
		strcat(directory, "/");
	}

	if(checkdirectory(directory) != true){
		return 2;
	}

	crawl(seed, max_depth, curr_depth, cleandirectory);
	free(directory);
	free(cleandirectory);

	return 0;
}


// loop and explore webpages until the list is exaughested
int crawl(char *seed, int max_depth, int curr_depth, char* cleandirectory){
	printf("now running crawl...\n");
	int seconds_to_sleep = 1; 
	int id = 1;

	//creating a new hashtable data structure to hold visited URLs & bag structure to hold URLs to visit 
	int table_slots = 11;
	hashtable_t *table = hashtable_new(table_slots);
	bag_t *bag = bag_new();
	if(check_creations(bag, table) != 0){
		return 1;
	}
	// insert seed into bag and hashtable
	webpage_t *seedsite = webpage_new(seed, 0, NULL);
	bag_insert(bag, seedsite);
	hashtable_insert(table, seed, " ");
	webpage_t *extractedpage = malloc(sizeof(webpage_t*));
	char *URL = malloc(sizeof(char*));
	int *pos = malloc(sizeof(int*));

	// while i can get a webpage item
	while((extractedpage = bag_extract(bag)) != NULL){	

		// get the HTML data from the webpage and put into the website variable
		pagefetcher(extractedpage, seconds_to_sleep);

		printf("we are currently at the %d depth for web searching\n", webpage_getDepth(extractedpage));

		// for this item, if the depth is less than the maximum, we will add its URLs into the bag
		if(webpage_getDepth(extractedpage)<= max_depth){
			// save this page into a document
			pagesaver(extractedpage, cleandirectory, id);
			id++;
			*pos = 0;
			URL = webpage_getNextURL(extractedpage, pos);
			pagescanner(URL, extractedpage, pos, bag, table);
		}
		if(webpage_getDepth(extractedpage) > max_depth){
			printf("we breached the max depth and will stop creating new files\n");
		}

		
	}

	printf("we are ending the crawl program now...\n\n\n");
	webpage_delete(extractedpage);
	hashtable_delete(table, itemdeletehashtable);
	bag_delete(bag, itemdeletebag);
	free(pos);
	free(URL);
	free(seedsite);

	return 0;
}

// takes the URL of the webpage_t variable and gets the HTML from the page. Will put the HTML into the variable's html pointer spot. 
void pagefetcher(webpage_t *page, int seconds_to_sleep){
	// wait to not call pages too fast
	sleep(seconds_to_sleep);
	webpage_fetch(page);
}

//will create a document within the given directory and will label with specific ID
void pagesaver(webpage_t *page, char *dir, int id){
	char *filenumber = malloc(sizeof(char*));
	sprintf(filenumber, "%d", id);
	char *tempdirectory = malloc(sizeof(char*));
	strcpy(tempdirectory, dir);
	strcat(tempdirectory, filenumber);
	char *temp_html = webpage_getHTML(page);
	if(temp_html != NULL){
		FILE *file_ptr = fopen(tempdirectory, "w");
		char *url = webpage_getURL(page); int depth = webpage_getDepth(page);
		fprintf(file_ptr, "%s\n%d\n", url, depth);
		fprintf(file_ptr, "%s", temp_html);
		fclose(file_ptr);
	}
	
	free(filenumber);
	free(tempdirectory);
}

//this function checks if we've seen this item before. If not, we insert into the bag
void crawler_bag_insert(hashtable_t *table, char *URL, bag_t *bag, webpage_t *newpage){
	if(hashtable_insert(table, URL, " ") == true){
		bag_insert(bag, newpage);
	} else if(hashtable_insert(table, URL, " ") == false){
		printf("we've already seen this URL\n");
	}
}

// basically takes a URL and will put it in if possible. Will increment the URL pos 
void pagescanner(char *URL, webpage_t *page, int *pos, bag_t* bag, hashtable_t *table){
	while ( URL != NULL && (URL[0] != '\0')) {
	
		printf("we just got a new URL from the page. it is: %s \n", URL);
		sleep(1);
		// normalize the URL & check if it worked correctly
		bool rip = NormalizeURL(URL);
		if(rip == true){
			printf("now checking to see if it is an internal URL...\n");
			// if it is internal URL
			bool internalcheck = IsInternalURL(URL);
		if( internalcheck == true){
			printf("this is an intenral URL\n");
			webpage_t *newpage = webpage_new(URL, webpage_getDepth(page) + 1, NULL);
			if(newpage != NULL){
				crawler_bag_insert(table, URL, bag, newpage);
				URL = webpage_getNextURL(page, pos);
			} else if (newpage == NULL){
				printf("the webpage variable we created with the internal URL was invalid\n");
			}

		} else if(internalcheck == false){
			printf("we just hit a URL that is outside of the dartmouth playground\n");
			URL = webpage_getNextURL(page, pos);
		}

		} else if (rip == false){
			printf("we could not normalize the URL\n");
			URL = webpage_getNextURL(page, pos);
		}
	}
}

void bag_to_string(FILE *fp, void *item){
	char *printURL = webpage_getURL(item);
	int printdepth = webpage_getDepth(item);
	printf("URL: %s \n Depth: %d\n", printURL, printdepth);
}

int check_creations(bag_t *bag, hashtable_t *table){
	if(bag != NULL && table != NULL){
		return 0;
	}
	else if (bag == NULL && table != NULL){
		printf("the bag was created unsuccessfully but the hashtable was\n");
		return 1;
	}
	else if(bag != NULL && table == NULL){
		printf("the bag was created successfully but the hashtable was not \n");
		return 2;
	}
	return 0;
}

void itemdeletehashtable(void *item){
	if(item != NULL){
		item = NULL;
	}
}

void itemdeletebag(void *item){
	webpage_delete(item);
}
