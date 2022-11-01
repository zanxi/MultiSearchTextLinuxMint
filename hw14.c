
#include <stdio.h> // for printf
#include <stdlib.h> // for exit()
#include <pthread.h>
#include "function14.h"

int main(void)
{
	pthread_t thread;
	int count;
	int err;
	client_threads=CLIENT_THREADS; /* #defined to 4 */

	// Asks the user for a request
	printf("\t---------------------------------------------------------\n");
	printf("\t|What would you like to do with the 'Books' textfile?\t|\n\t|\tRead ('1') the textfile\t\t\t\t|\n\t|\tSearch ('2') a book entry\t\t\t|\n\t|\tInsert ('3') a book entry\t\t\t|\n\t|\tReplace ('4') a book entry\t\t\t|\n\t|\tDelete ('5') a book entry\t\t\t|\n\t|\tQuit ('6') this program\t\t\t\t|\n");
	printf("\t---------------------------------------------------------\n");

	for (count=0; count<client_threads; count++)
	{
		//printf("Creating client #%d...\n", count);
		AskUser();
		err=pthread_create(&thread, NULL, client_routine, (void *)count);
		//printf("After creating client #%d...\n\n", count);
	}

	printf("All clients done.\n");

return 0;
}