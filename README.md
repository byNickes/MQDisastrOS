# Message Queue bloccante in DisastrOS

L'applicazione aggiunge a DisastrOS una coda di messaggi bloccante in stile mailbox, limitata al massimo numero allocabile di messaggi per ogni mailbox.
## How?
Ciò è stato fatto con l'idea che una Message Queue è di fatto una risorsa, perciò, si può estendere direttamente la struct Resource ottenendo la seguente struttura:

```C
typedef struct MessageQueue{
  Resource resource; //MQ extends resource struct
  ListHead messages; //list of messages
  ListHead waiting_to_read;
  ListHead waiting_to_write;
  int available; //written messages
}MessageQueue;
```
In questo modo è stato possibile utilizzare le syscall già presenti per l'allocazione e deallocazione delle risorse con l'unica aggiunta del seguente array:
```C
static Resource* (*resource_alloc_func[MAX_NUM_TYPE_RESOURCES])();
```
il quale contiene i puntatori alla funzione di allocazione per ogni risorsa implementata nel sistema operativo. La selezione della funzione avverrà attraverso il campo type presente nella struct Resource.
Stesso ragionamento è stato fatto con la funzione per liberare memoria occupata dalla Message Queue ed è quindi stato allocato il seguente array:

```C
static int (*resource_free_func[MAX_NUM_TYPE_RESOURCES])(Resource*);
```
che verrà utilizzato nella system call di deallocazione del tipo Resource.
In una Message Queue teniamo anche traccia dei processi che sono stati messi in attesa per leggere, perchè hanno trovato una coda vuota, e per scrivere, perchè hanno trovato una coda piena. Questi vengono messi in waiting nella system call di read e write della Message Queue mentre vengono risvegliati da chi scrive il primo messaggio o da chi legge l'ultimo, a seconda di in quale coda si trovino.

La struct dei messaggi è, invece, la seguente:
```C
typedef struct Message{
  ListItem list;
  int pid_sender;
  int length;
  char message[MAX_MESSAGE_LENGTH];
}Message;
```
Le funzioni implementate per la Message Queue sono, quindi:
```C
void MessageQueue_init(); //Inizializza la Message Queue rendendo possibile l'utilizzo nel sistema operativo

Resource* MessageQueue_alloc(); //Alloca una nuova Message Queue, chiamata attraverso la funzione Resource_alloc() di Resource

int MessageQueue_free(Resource* r); //Dealloca la Message Queue r, viene chiamata dalla funzione Resource_free() di Resource

Message* MessageQueue_getFirstMessage(MessageQueue* mq); //Fornisce il primo messaggio della Message Queue mq

void print_MQ(MessageQueue* mq); //Stampa la Message Queue mq
```
Infine, sono state installate le system call per richiedere la lettura con i relativi wrapper:
```C
void internal_MessageQueue_read();
int disastrOS_readMessageQueue(int fd, char* buf_des, int buf_length); //Wrapper della read

void internal_MessageQueue_write();
int disastrOS_writeMessageQueue(int fd, char* message, int m_length); //Wrapper della write
```

## How to run?
E' stato predisposto un programma di test dove vengono instanziate un numero di Message Queue e processi figli settando le relative costanti:
```C
#define MQ 4 //Numero di Message Queue da aprire
#define CHILDREN 8 //Numero di figli da spawnare
```
Il programma di test deve essere compilato con `make` ed avviato con `./disastrOS_test`.

Le code vengono allocate dal padre che poi spawnerà i figli a coppie (uno lettore ed uno scrittore) passandogli come argomento l'ID della risorsa legata a quella Message Queue. I processi con PID dispari saranno lettori ed i processi con PID pari saranno scrittori i quali scriveranno 128 messaggi (numero massimo di messaggi per ogni Message Queue) sulla coda.
I figli apriranno, una volta passati in esecuzione, la Message Queue. Ogni MQ cicli di spawn dei figli si ricomincia da capo ad assegnare la Message Queue ai processi, si potranno quindi avere più di 2 processi che leggono e/o scrivono su una stessa Message Queue.

Ciò che ci si aspetta è che il numero di messaggi scritti e numero di messaggi letti coincidano.

**Nicolò Baroncini**
