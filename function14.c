
#include <stdio.h> // for printf, sprintf, fputs
#include <string.h> // for strlen
#include <stdlib.h> // for malloc and free()
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "function14.h"

/* a per-request struct, one per client thread */
typedef struct request_tag
{
	struct request_tag *next; /* pointer to next request */
	int operation; /* one of read/write/quit request */
	int synchronous; /* whether request is synchronous */
	int done_flag; /* predicate this request done */
	pthread_cond_t done;/* wait for this thread to finish */
	char prompt[32]; /* prompt server message to client */
	char text[28]; /* read/write text */
} request_t;

/* a server struct, organized as a client request queue */
typedef struct server_tag
{
	request_t *first; /* first request in the queue */
	request_t *last; /* last request in the queue */
	int running; /* predicate this server running */
	pthread_mutex_t mutex; /* lock around server data */
	pthread_cond_t request; /* wait for a request */
} server_t;

server_t server={NULL, NULL, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

int Book_Search(char *s)
{
	//printf("\n\t\t\tIn Book_Search()...\n");
	
	//OPENING THE FILE 'Books'
	fd = fopen("Books.txt", "r");
	
	//CHECKING IF THE USER'S BOOK ENTRY IS ALREADY IN THE FILE
	char *buf[1024];
	line_num = 0;
	int found = 0;
	do
	{
		if (strstr(buf, s)!=NULL)
		{
			//printf("\t\tBook found on line %d as %s", line_num, buf);
			//if(feof(fd))
			//	printf("\n");
			printf("\t\tBOOK FOUND on line %d.\n", line_num);
			found++;
		}
		line_num++;
	} while (fgets(buf, 1024, fd)!=NULL);
	
	//CLOSING THE FILE
	fclose(fd);

	//printf("\t\t\tExiting Book_Search()...\n");
return found;
}

void *server_routine(void *arg)
{
	//printf("\nIn server_routine()...\n");

	static pthread_mutex_t prompt_mutex=PTHREAD_MUTEX_INITIALIZER;
	request_t *request;
	int operation;
	int err;

	while (1)
	{
		// Lock the server mutex, to protect server-modifying data
		err=pthread_mutex_lock(&server.mutex);

		while (server.first==NULL)
			err=pthread_cond_wait(&server.request, &server.mutex); // wait for request
		
		// Dequeue 1st request in the queue
		request=server.first;

		// Advance the head of the queue
		server.first=request->next;

		if (server.first==NULL)
			server.last=NULL; // if queue is empty, reset tail to NULL
		
		// Unlock the server mutex
		err=pthread_mutex_unlock(&server.mutex);

		// Read what the requested op was: read/insert/replace/delete/quit
		operation=request->operation;

		switch (operation)
		{
			case QUIT_REQUEST:
				exit(0);
			case READ_REQUEST:
				printf("\tIN READ_REQUEST...\n");
				
				//OPENING THE FILE 'Books'
				fd = fopen("Books.txt", "r");
			    
			    //Reading the contents in Books
			    printf("The contents of Books.txt is:\n---------------------------------\n");
			    char s;
				while ((s = fgetc(fd)) != EOF) 
				{
					printf("%c", s);
				}

				//CLOSING THE FILE
				fclose(fd);
				printf("\n---------------------------------\n");
				printf("\tEXITING READ_REQUEST...\n");
				break;
			case SEARCH_REQUEST:
				printf("\tIN SEARCH_REQUEST...\n");
				int isthebookhere = Book_Search(&request->text);
				if (isthebookhere==0)
					printf("\t\tBOOK '%s' NOT FOUND.\n", &request->text);
				printf("\tEXITING SEARCH_REQUEST...\n");
				break;
			case INSERT_REQUEST:
				printf("\tIN INSERT_REQUEST...\n");
				int value = Book_Search(&request->text);
				if(value == 0)
				{
					printf("\tCouldn't find the book!\n\tNow inserting it at the end of the file.\n");
					
					//OPENING THE FILE 'Books'
					fd = fopen("Books.txt", "a+");

					// Storing the contents in the end of the file
					fprintf(fd, "\n%s", &request->text);

					//CLOSING THE FILE
					fclose(fd);
				}
				else
					printf("\tNO NEED TO INSERT! Book is already in the file.\n");
				printf("\tEXITING INSERT_REQUEST...\n");
				break;
			case REPLACE_REQUEST:
				printf("\tIN REPLACE_REQUEST...\n\tGoing to replace %s\n", &request->text);
				char buf[1024];
				int count = 0;
				char *filename = "Books.txt";
				char *temporary = "temp.txt";

				//OPENING TWO FILES
				FILE *fd2;
				fd = fopen(filename, "r");
				fd2 = fopen(temporary, "w"); //creates a new file if it doesn't exist

				// Copying the contents to the temporary file
				while (!feof(fd))
				{
					strcpy(buf, "\0");
					fgets(buf, 1024, fd);
					
					count ++;
					if(count!=line_num)
						fprintf(fd2, "%s", buf);
					else
						fprintf(fd2, "%s\n", &request->text);
				}

				//CLOSING THE FILES
				fclose(fd);
				fclose(fd2);

				//GETTING RID OF THE OLD Books.txt and RENAMING THE TEMP TO Books.txt
				remove(filename);
				rename(temporary, filename);
				printf("\tEXITING REPLACE_REQUEST...\n");
				break;
			case DELETE_REQUEST:
				printf("\tIN DELETE_REQUEST...\n\t\tGoing to delete line %d.\n", line_num);
				char b[1024];
				int c = 0;
				char *file = "Books.txt";
				char *temp = "tmp.txt";

				//OPENING TWO FILES
				FILE *fd3;
				fd = fopen(file, "r");
				fd3 = fopen(temp, "w"); //creates a new file if it doesn't exist

				// Copying the contents to the temporary file
				while (!feof(fd))
				{
					strcpy(b, "\0");
					fgets(b, 1024, fd);
					
					c ++;
					if(c!=line_num)
						fprintf(fd3, "%s", b);
					else
						fprintf(fd3, "");
				}

				//CLOSING THE FILES
				fclose(fd);
				fclose(fd3);

				//GETTING RID OF THE OLD Books.txt and RENAMING THE TEMP TO Books.txt
				remove(file);
				rename(temp, file);
				printf("\tEXITING DELETE_REQUEST...\n");
				break;
			default:
				break;
		}


		/* 0=non-synchronous, 1=synchronous */
		if (request->synchronous==1)
		{
			err=pthread_mutex_lock(&server.mutex);
			request->done_flag=1; // set the done flag to 1, will be the predicate to client to synchronize with other
			pthread_cond_signal(&request->done); // signal that the request is done processing
			err=pthread_mutex_unlock(&server.mutex);
		}
		else
			free(request);
	}

	//printf("Exiting server_routine()...\n");
	
return NULL;
}

void server_request(int operation, int sync, const char *prompt, char *string)
{
	//printf("\nIn server_request()...\n");

	request_t *request;
	int err;

	// Lock the server data
	err=pthread_mutex_lock(&server.mutex);

	// Create the server thread
	if (!server.running)
	{
		pthread_t thread;
		pthread_attr_t detached_attr; // server thread is created as a detached thread
		err=pthread_attr_init(&detached_attr);
		err=pthread_attr_setdetachstate(&detached_attr, PTHREAD_CREATE_DETACHED);
		server.running=1;
		err=pthread_create(&thread, &detached_attr, server_routine, NULL);
		pthread_attr_destroy(&detached_attr);
	}
	// Create the request
	request=(request_t *)malloc(sizeof(request_t));
	request->next=NULL; // this is tail
	request->operation=operation; // these are going to be passed-in arguments
	request->synchronous=sync;
	if (sync)
	{
		request->done_flag=0; // set the done predicate to 0
		err=pthread_cond_init(&request->done, NULL); // initialize the pthread_cond_t variable "done"
	}

	// All requests get a prompt
	if (prompt!=NULL)
		strncpy(request->prompt, prompt, 32);
	else
		request->prompt[0]='\0';
	
	// Only the requests: search, insert, replace, and delete get to write
	if (operation==SEARCH_REQUEST && string!=NULL)
		strncpy(request->text, string, 128);
	else if (operation==INSERT_REQUEST && string!=NULL)
		strncpy(request->text, string, 128);
	else if (operation==REPLACE_REQUEST && string!=NULL)
		strncpy(request->text, string, 128);
	//else if (operation==DELETE_REQUEST && string!=NULL)
	//	strncpy(request->text, string, 128);
	else
		request->text[0]='\0';

	// Enqueue the request
	if (server.first==NULL) // if queue is empty
	{
		server.first=request; // head=tail=request
		server.last=request;
	}
	else // if queue is non-empty
	{
		(server.last)->next=request; // make next of current tail point to request
		server.last=request; // make tail equal to request
	}

	// Tell server that there is a request made
	err=pthread_cond_signal(&server.request);

	//If request was synchronous, wait for a reply
	if (sync)
	{
		while (!request->done_flag)
			err=pthread_cond_wait(&request->done, &server.mutex);
		if (operation==READ_REQUEST)
			strcpy(string, request->text);
		err=pthread_cond_destroy(&request->done);
		free(request);
	}

	// Unlock the server data
	err=pthread_mutex_unlock(&server.mutex);

	//printf("\nExiting server_request()...\n");
}

void *client_routine(void *arg)
{
	int client_number=(int )arg;
	int err;
	char prompt[32];

	sprintf(prompt, "Client %d>", client_number);
	server_request(user_request, 0, prompt, user_book);

	//printf("Exiting client_routine()...\n");
return (void *)0;
}

void Ask_for_Book(void)
{
	flush();

	// Asks the user for the book's name and author
	printf("\n\tEnter a name of a book and the its author; each seperated by '_'.\n");
	printf("\tNote:\tThe title and author's name has to be seperated by a space\n");
	printf("\t\t\tExample: 'The_Hunger_Games Suzanne_Collins'\n");
	printf("\t\tIT'S CASE-SENSITIVE.\n\t>> ");
	//fgets(user_book, sizeof(user_book), stdin);
	int i = 0;
	char c;
	while(c != '\n') {
		c = getchar();
		user_book[i] = c;
		i++;
	} user_book[i-1] = '\0';
	printf("\n\t\tYou wrote the following book: %s\n", user_book);
	printf("\t____________________________________________________________________\n");
}

void flush()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void AskUser(void)
{
	// Asks the user for a request
	scanf("%d", &user_request);
	printf("\t\t\tYou said you would like to ");
	
	if (user_request == 1)
		printf("READ.\n");

	else if (user_request == 2)
	{
		printf("SEARCH.\n");
		printf("\t____________________________________________________________________\n");
		Ask_for_Book();
	}

	else if (user_request == 3)
	{
		printf("INSERT.\n");
		printf("\t____________________________________________________________________\n");
		Ask_for_Book();
	}

	else if (user_request == 4)
	{
		printf("REPLACE.\n");
		printf("\tWhat line in the text file would you like to replace? ");
		scanf("%d", &line_num);
		printf("\n");
		printf("\t____________________________________________________________________\n");
		Ask_for_Book();
	}
	
	else if (user_request == 5)
	{
		printf("DELETE.\n");
		printf("\tWhat line in the text file would you like to remove? ");
		scanf("%d", &line_num);
		printf("\n");
		//printf("\t____________________________________________________________________\n");
		//Ask_for_Book();
	}

	else if (user_request == 6)
		printf("QUIT.\n");

	else
	{
		printf("do nothing.\nNOW EXITING THE PROGRAM...\n");
		exit(0);
	}
}