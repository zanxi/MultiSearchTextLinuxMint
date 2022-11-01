
#define READ_REQUEST 1
#define SEARCH_REQUEST 2
#define INSERT_REQUEST 3
#define REPLACE_REQUEST 4
#define DELETE_REQUEST 5
#define QUIT_REQUEST 6

#define CLIENT_THREADS 4

FILE *fd;
char user_book[128];
int user_request;
int line_num;

int client_threads; /* client has 4 threads */
pthread_mutex_t client_mutex;
pthread_cond_t clients_done;

int Book_Search(char *s);
void *server_routine(void *arg);
void server_request(int operation, int sync, const char *prompt, char *string);
void *client_routine(void *arg);
void Ask_for_Book(void);
void flush();
void AskUser(void);