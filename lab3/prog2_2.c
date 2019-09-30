
#include <stdio.h>
#include <stdlib.h> /* for malloc, free, srand, rand */
#include <string.h>
#include <stdbool.h>

#define BUFFER_MAX_SIZE 50
#define WINDOW_MAX_SIZE 8

/*******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };

struct event;

void starttimer(int AorB, float increment);
void stop_buffertimer(int AorB);
void tolayer3(int AorB, struct pkt packet);
void tolayer5(int AorB, char datasent[20]);

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* Here I define some function prototypes to avoid warnings */
/* in my compiler. Also I declared these functions void and */
/* changed the implementation to match */
void init();
void generate_next_arrival();
void insertevent(struct event *p);

int get_checksum(struct pkt packet);
struct pkt create_packet(int AorB, struct msg message);

float A_increment = 200.0;
int A_seqnum;
int A_expected_seqnum;
int A_sent_first;
int A_last_seqnum_received;
struct pkt A_buffer[BUFFER_MAX_SIZE];
int A_front;
int A_rear;
int A_itemCount;
struct pkt A_window[WINDOW_MAX_SIZE];
int A_window_front;
int A_window_rear;
int A_window_itemCount;

float B_increment = 200.0;
int B_seqnum;
int B_expected_seqnum;
int B_sent_first;
int B_last_seqnum_received;
struct pkt B_buffer[BUFFER_MAX_SIZE];
int B_front;
int B_rear;
int B_itemCount;
struct pkt B_window[WINDOW_MAX_SIZE];
int B_window_front;
int B_window_rear;
int B_window_itemCount;

/* FUNCOES REFERENTES A WINDOW */

struct pkt top_window(AorB)
  int AorB;
{
  // printf("Start top_window\n");

  if (AorB == 0) {
    // printf("End top_window\n");
    return A_window[A_window_front];

  } else if (AorB == 1) {
    // printf("End top_window\n");
    return B_window[B_window_front];
  }
}

bool isEmpty_window(AorB)
  int AorB;
{
  // printf("Start isEmpty_window\n");

  if (AorB == 0) {
    // printf("End isEmpty_window\n");
    return A_window_itemCount == 0;

  } else if (AorB == 1) {
    // printf("End isEmpty_window\n");
    return B_window_itemCount == 0;
  }
}

bool isFull_window(AorB)
  int AorB;
{
  // printf("Start isFull_window\n");

  if (AorB == 0) {
    // printf("End isFull_window\n");
    return A_window_itemCount == WINDOW_MAX_SIZE;

  } else if (AorB == 1) {
    // printf("End isFull_window\n");
    return B_window_itemCount == WINDOW_MAX_SIZE;
  }
}

int size_window(AorB)
  int AorB;
{
  // printf("Start size_window\n");

  if (AorB == 0) {
    // printf("End size_window\n");
    return A_window_itemCount;

  } else if (AorB == 1) {
    // printf("End size_window\n");
    return B_window_itemCount;
  }
}

void push_window(AorB, packet)
  int AorB;
  struct pkt packet;
{
  // printf("Start push_window\n");

  if (!isFull_window(AorB)) {

    if (AorB == 0) {

      if(A_window_rear == WINDOW_MAX_SIZE-1) {
        A_window_rear = -1;            
      }       
      
      A_window_rear++;

      A_window[A_window_rear].seqnum = packet.seqnum;
      A_window[A_window_rear].acknum = packet.acknum;
      A_window[A_window_rear].checksum = packet.checksum;
      for (int i = 0; i < 20; i++) {
        A_window[A_window_rear].payload[i] = packet.payload[i];
      }
      A_window_itemCount++;

    } else if (AorB == 1) {

      if(B_window_rear == WINDOW_MAX_SIZE-1) {
        B_window_rear = -1;            
      }       
      
      B_window_rear++;

      B_window[B_window_rear].seqnum = packet.seqnum;
      B_window[B_window_rear].acknum = packet.acknum;
      B_window[B_window_rear].checksum = packet.checksum;
      for (int i = 0; i < 20; i++) {
        B_window[B_window_rear].payload[i] = packet.payload[i];
      }
      B_window_itemCount++;
    }
  }

  // printf("End push_window\n");
}

struct pkt pop_window(AorB)
  int AorB;
{
  // printf("Start pop_window\n");
  struct pkt packet;

  if (AorB == 0) {
    A_window_front++; // TODO: isso vai depois do for abaixo, nao?

    packet.seqnum = A_window[A_window_front].seqnum;
    packet.acknum = A_window[A_window_front].acknum;
    packet.checksum = A_window[A_window_front].checksum;
    for (int i = 0; i < 20; i++) {
      packet.payload[i] = A_window[A_window_front].payload[i];
    }

    if(A_window_front == WINDOW_MAX_SIZE) {
      A_window_front = 0;
    }
	
    A_window_itemCount--;

  } else if (AorB == 1) {
    B_window_front++; // TODO: isso vai depois do for abaixo, nao?

    packet.seqnum = B_window[B_window_front].seqnum;
    packet.acknum = B_window[B_window_front].acknum;
    packet.checksum = B_window[B_window_front].checksum;
    for (int i = 0; i < 20; i++) {
      packet.payload[i] = B_window[B_window_front].payload[i];
    }

    if(B_window_front == WINDOW_MAX_SIZE) {
      B_window_front = 0;
    }
	
    B_window_itemCount--;
  }

  // printf("End pop_window\n");
  return packet;  
}

void print_window(AorB)
  int AorB;
{
  printf("Start print_window\n");

  struct pkt packet;
  
  if (AorB == 0) {

    if (!isEmpty_window(0)) {
      printf("Window A nao vazia, tamanho %d\n", size_window(0));

      for (int i = 0; i < size_window(0); i++) {
        packet.seqnum = A_window[(A_front+i) % WINDOW_MAX_SIZE].seqnum;
        packet.acknum = A_window[(A_front+i) % WINDOW_MAX_SIZE].acknum;
        packet.checksum = A_window[(A_front+i) % WINDOW_MAX_SIZE].checksum;
        printf("Mensagem %d: ", packet.seqnum);
        
        for (int j = 0; j < 20; j++) {
          printf("%c", packet.payload[j]);
        }
        printf("\n");
      }

    } else {
      printf("Window A vazia\n");
    }

  } else if (AorB == 1) {

    if (!isEmpty_window(1)) {
      printf("Window B nao vazia, tamanho %d\n", size_window(1));

      for (int i = 0; i < size_window(1); i++) {
        packet.seqnum = B_window[(B_front+i) % WINDOW_MAX_SIZE].seqnum;
        packet.acknum = B_window[(B_front+i) % WINDOW_MAX_SIZE].acknum;
        packet.checksum = B_window[(B_front+i) % WINDOW_MAX_SIZE].checksum;
        printf("Mensagem %d: ", packet.seqnum);
        
        for (int j = 0; j < 20; j++) {
          printf("%c", packet.payload[j]);
        }
        printf("\n");
      }

    } else {
      printf("Window B vazia\n");
    }
  }

  printf("End print_window\n");
}

void send_window(AorB) // chamado no timerinterrupt
  int AorB;
{
  printf("Start send_window\n");

  if (AorB == 0) {

    for (int i = 0; i < size_window(0); i++) {
      printf("Enviando pacote %d: \n", A_window[(A_front+i) % WINDOW_MAX_SIZE].seqnum);
      for (int i = 0; i < 20; i++) {
        printf("%c", A_window[(A_front+i) % WINDOW_MAX_SIZE].payload[i]);
      }
      printf("\n");

      tolayer3(0, A_window[(A_front+i) % WINDOW_MAX_SIZE]);
    }

  } else if (AorB == 1) {

    for (int i = 0; i < size_window(1); i++) {
      printf("Enviando pacote %d: \n", B_window[(B_front+i) % WINDOW_MAX_SIZE].seqnum);
      for (int i = 0; i < 20; i++) {
        printf("%c", B_window[(B_front+i) % WINDOW_MAX_SIZE].payload[i]);
      }
      printf("\n");

      tolayer3(1, B_window[(B_front+i) % WINDOW_MAX_SIZE]);
    }
  }

  printf("End send_window\n");
}

/* FUNCOES REFERENTES AO BUFFER */

struct pkt top_buffer(AorB)
  int AorB;
{
  // printf("Start top_buffer\n");

  if (AorB == 0) {
    // printf("End top_buffer\n");
    return A_buffer[A_front];

  } else if (AorB == 1) {
    // printf("End top_buffer\n");
    return B_buffer[B_front];
  }
}

bool isEmpty_buffer(AorB)
  int AorB;
{
  // printf("Start isEmpty_buffer\n");
  if (AorB == 0) {
    // printf("End isEmpty_buffer\n");
    return A_itemCount == 0;

  } else if (AorB == 1) {
    // printf("End isEmpty_buffer\n");
    return B_itemCount == 0;
  }
}

bool isFull_buffer(AorB)
  int AorB;
{
  // printf("Start isFull_buffer\n");

  if (AorB == 0) {
    // printf("End isFull_buffer\n");
    return A_itemCount == BUFFER_MAX_SIZE;

  } else if (AorB == 1) {
    // printf("End isFull_buffer\n");
    return B_itemCount == BUFFER_MAX_SIZE;
  }
}

int size_buffer(AorB)
  int AorB;
{
  // printf("Start size_buffer\n");

  if (AorB == 0) {
    // printf("End size_buffer\n");
    return A_itemCount;

  } else if (AorB == 1) {
    // printf("End size_buffer\n");
    return B_itemCount;
  }
}  

void push_buffer(AorB, packet)
  int AorB;
  struct pkt packet;
{
  // printf("Start push_buffer\n");

  if (!isFull_buffer(AorB)) {

    if (AorB == 0) {

      if(A_rear == BUFFER_MAX_SIZE-1) {
        A_rear = -1;            
      }       
      
      A_rear++;
      A_buffer[A_rear].seqnum = packet.seqnum;
      A_buffer[A_rear].acknum = packet.acknum;
      A_buffer[A_rear].checksum = packet.checksum;
      for (int i = 0; i < 20; i++) {
        A_buffer[A_rear].payload[i] = packet.payload[i];
      }
      A_itemCount++;

    } else if (AorB == 1) {

      if(B_rear == BUFFER_MAX_SIZE-1) {
        B_rear = -1;            
      }       
      
      B_rear++;
      B_buffer[B_rear].seqnum = packet.seqnum;
      B_buffer[B_rear].acknum = packet.acknum;
      B_buffer[B_rear].checksum = packet.checksum;
      for (int i = 0; i < 20; i++) {
        B_buffer[B_rear].payload[i] = packet.payload[i];
      }
      B_itemCount++;
    }
       
  } else {
    printf("Buffer cheio! exit(0)");
    // printf("End push_buffer\n");
    exit(0);
  }

  // printf("End push_buffer\n");
}

struct pkt pop_buffer(AorB)
  int AorB;
{
  // printf("Start pop_buffer\n");

  struct pkt packet;

  if (AorB == 0) {
    A_front++;
    packet.seqnum = A_buffer[A_front].seqnum;
    packet.acknum = A_buffer[A_front].acknum;
    packet.checksum = A_buffer[A_front].checksum;
    for (int i = 0; i < 20; i++) {
        packet.payload[i] = A_buffer[A_front].payload[i];
    }

    if(A_front == BUFFER_MAX_SIZE) {
      A_front = 0;
    }
	
    A_itemCount--;

  } else if (AorB == 1) {
    B_front++;
    packet.seqnum = B_buffer[B_front].seqnum;
    packet.acknum = B_buffer[B_front].acknum;
    packet.checksum = B_buffer[B_front].checksum;
    for (int i = 0; i < 20; i++) {
        packet.payload[i] = B_buffer[B_front].payload[i];
    }

    if(B_front == BUFFER_MAX_SIZE) {
      B_front = 0;
    }
	
    B_itemCount--;
  }

  // printf("End pop_buffer\n");
  return packet;  
}

struct pkt create_packet(AorB, message)
  int AorB; 
  struct msg message;
{
  printf("Start create_packet\n");

  struct pkt packet;

  if (AorB == 0) {
    packet.seqnum = ++A_seqnum;
  } else if (AorB == 1) {
    packet.seqnum = ++B_seqnum;
  }

  packet.acknum = 0;
  
  for (int i = 0; i < 20; i++) {
    packet.payload[i] = message.data[i];
  }
  
  packet.checksum = get_checksum(packet);

  printf("End create_packet\n");
  return packet;
}

int get_checksum(packet)
  struct pkt packet;
{
  printf("Start checksum\n");

  int result = packet.acknum + packet.seqnum;

  for (int i = 0; i < 20; i++) {
    result += (int)packet.payload[i];
  }

  printf("End checksum\n");
  return result;
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  printf("Start A_init\n");

  A_increment = 200.0;
  A_seqnum = 0;
  A_expected_seqnum = 1;
  A_sent_first = 0;
  A_last_seqnum_received = 0;

  A_front = 0;
  A_rear = -1;
  A_itemCount = 0;

  A_window_front = 0;
  A_window_rear = -1;
  A_window_itemCount = 0;

  printf("End A_init\n");
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  printf("Start B_init\n");

  B_increment = 200.0;
  B_seqnum = 0;
  B_expected_seqnum = 1;
  B_sent_first = 0;
  B_last_seqnum_received = 0;

  B_front = 0;
  B_rear = -1;
  B_itemCount = 0;

  B_window_front = 0;
  B_window_rear = -1;
  B_window_itemCount = 0;

  printf("End B_init\n");
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
  printf("Start A_input\n");

  int checksum = get_checksum(packet);
  if (packet.checksum == checksum) { // veio nao corrompido

    printf("Pacote %d nao corrompido: ", packet.seqnum);
    for (int i = 0; i < 20; i++) {
      printf("%c", packet.payload[i]);
    }
    printf("\n");
  
    if (packet.acknum == 0) { // normal message
      printf("Mensagem normal\n");

      A_sent_first = 0;

      if (packet.seqnum == A_expected_seqnum) {
        printf("Seqnum %d veio como esperado\n", packet.seqnum);

        tolayer5(0, packet.payload);

        struct pkt ack;
        ack.seqnum = packet.seqnum;
        ack.acknum = packet.seqnum;
        for (int i = 0; i < 20; i++) {
          ack.payload[i] = '-';
        }
        ack.checksum = get_checksum(ack);

        printf("Enviando ACK: %d\n", ack.acknum);
        tolayer3(0, ack);

      } else {
        printf("Seqnum %d nao veio como esperado %d -> ignora e reenvia antigo ACK %d\n", 
          packet.seqnum, A_expected_seqnum, A_expected_seqnum - 1);
        
        struct pkt ack;
        ack.seqnum = packet.seqnum;
        ack.acknum = A_expected_seqnum - 1;
        for (int i = 0; i < 20; i++) {
          ack.payload[i] = '-';
        }
        ack.checksum = get_checksum(ack);

        printf("Enviando ACK: %d\n", ack.acknum);
        tolayer3(0, ack);
      }

    } else if (packet.acknum > 0) { // ack message
      printf("Recebido ACK %d\n", packet.acknum);
      A_sent_first = 0;

      pop_window(0);
      if (!isEmpty_buffer(0)) {
        struct pkt packet = pop_buffer(0);
        push_window(1, packet);
      }

      printf("PAREI O TIMER DO A\n");
      stop_buffertimer(0);

      if (!isEmpty_window(0)) {
        printf("COMECEI O TIMER DO A\n");
        starttimer(0, A_increment);
      }

    } else if (packet.acknum < 0) { // nack message
      printf("Recebido NACK %d\n", packet.acknum);
    }

  } else { // veio corrompido
    printf("Pacote %d corrompido -> ignora\n", packet.seqnum);
  }

  printf("End A_input\n");
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
  printf("Start B_input\n");

  int checksum = get_checksum(packet);
  if (packet.checksum == checksum) { // veio nao corrompido

    printf("Pacote %d nao corrompido: ", packet.seqnum);
    for (int i = 0; i < 20; i++) {
      printf("%c", packet.payload[i]);
    }
    printf("\n");
  
    if (packet.acknum == 0){ // normal message

      printf("Mensagem normal %d\n");

      B_sent_first = 0;

      if (packet.seqnum == B_expected_seqnum) {
        printf("Seqnum %d veio como esperado\n", packet.seqnum);
        struct pkt ack;
        ack.seqnum = packet.seqnum;
        ack.acknum = packet.seqnum;
        for (int i = 0; i < 20; i++) {
          ack.payload[i] = '-';
        }
        ack.checksum = get_checksum(ack);

        printf("Enviando ACK: %d\n", ack.acknum);
        tolayer3(1, ack);
        tolayer5(1, packet.payload);

      } else {
        printf("Seqnum %d nao veio como esperado %d -> ignora e reenvia antigo ACK %d\n", 
          packet.seqnum, B_expected_seqnum, B_expected_seqnum - 1);
        
        struct pkt ack;
        ack.seqnum = packet.seqnum;
        ack.acknum = B_expected_seqnum - 1;
        for (int i = 0; i < 20; i++) {
          ack.payload[i] = '-';
        }
        ack.checksum = get_checksum(ack);

        printf("Enviando ACK: %d\n", ack.acknum);
        tolayer3(1, ack);
      }

    } else if (packet.acknum > 0) { // ack message
      printf("Recebido ACK %d\n", packet.acknum);
      B_sent_first = 0;

      pop_window(1);
      if (!isEmpty_buffer(1)) {
        struct pkt packet = pop_buffer(1);
        push_window(1, packet);
      }

      printf("PAREI O TIMER DO B\n");
      stop_buffertimer(1); 

      if (!isEmpty_window(1)) {
        printf("COMECEI O TIMER DO B\n");
        starttimer(1, B_increment);
      }

    } else if (packet.acknum < 0) { // nack message
      printf("Recebido NACK %d: ", packet.acknum);
    }

  } else { // veio corrompido
    printf("Pacote %d corrompido -> ignora\n", packet.seqnum);
  }

  printf("End B_input\n");
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
  printf("Start A_output\n");

  if (!isFull_window(0)) {
    A_sent_first = 1;

    push_window(0, message);

    struct pkt packet;
    packet = create_packet(0, message);
    
    if (size_window(0) == 1) {
      printf("COMECEI O TIMER DO A PARA PACOTE: %d\n", packet.seqnum);
      starttimer(0, A_increment);
    }

    tolayer3(0, packet);

  } else {
    printf("WINDOW CHEIA: ADICIONANDO AO BUFFER\n");
    push_buffer(0, message);
  }

  print_window(0);
  printf("End A_output\n");
}

void B_output(message)  /* need be completed only for extra credit */
  struct msg message;
{
  printf("Start B_output\n");

  if (!isFull_window(1)) {
    B_sent_first = 1;

    push_window(1, message);

    struct pkt packet;
    packet = create_packet(0, message);

    if (size_window(1) == 1) {
      printf("COMECEI O TIMER DO B PARA PACOTE: %d\n", packet.seqnum);
      starttimer(1, B_increment);
    }
    
    tolayer3(1, packet);

  } else {
    printf("WINDOW CHEIA: ADICIONANDO AO BUFFER\n");
    push_buffer(1, message);
  }

  print_window(1);
  printf("End B_output\n");
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  printf("Start A_timerinterrupt\n");

  printf("Comecei timer do A para seqnum %d\n", A_seqnum+1);
  starttimer(0, A_increment);
  send_window(0);

  printf("End A_timerinterrupt\n");
}  

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called when B's timer goes off */
void B_timerinterrupt()
{
  printf("Start B_timerinterrupt\n");

  printf("COMECEI O TIMER DO B\n");
  starttimer(1, B_increment);
  send_window(1);

  printf("End B_timerinterrupt\n");
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stop_bufferping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop_buffer */
float time = (float)0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

void main()
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;

   int i,j;
   /* char c; // Unreferenced local variable removed */

   init();
   A_init();
   B_init();

   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nEVENT time: %f,",eventptr->evtime);
           printf("  type: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", timerinterrupt  ");
             else if (eventptr->evtype==1)
               printf(", fromlayer5 ");
             else
	     printf(", fromlayer3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim==nsimmax)
	  break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i=0; i<20; i++)
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: data given to student: ");
                 for (i=0; i<20; i++)
                  printf("%c", msg2give.data[i]);
               printf("\n");
	     }
            nsim++;
            if (eventptr->eventity == A)
               A_output(msg2give);
             else
               B_output(msg2give);
            }
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* deliver packet by calling */
   	       A_input(pkt2give);            /* appropriate entity */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
             }
        free(eventptr);
        }

terminate:
   printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
}



void init()                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();


   printf("-----  Stop_buffer and Wait Network Simulator Version 1.1 -------- \n\n");
   printf("Enter the number of messages to simulate: ");
   nsimmax = 5;
   printf("\nnsimmax = %d\n", nsimmax);
  //  scanf("%d",&nsimmax);
   printf("Enter  packet loss probability [enter 0.0 for no loss]:");
   lossprob = 0.3;
   printf("\nlossprob = %f\n", lossprob);
  //  scanf("%f",&lossprob);
   printf("Enter packet corruption probability [0.0 for no corruption]:");
   corruptprob = 0.3;
   printf("\ncorruptprob = %f\n", corruptprob);
  //  scanf("%f",&corruptprob);
   printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
   lambda = 1000;
   printf("\nlambda = %f\n", lambda);
  //  scanf("%f",&lambda);
   printf("Enter TRACE:");
   TRACE = 2;
   printf("\nTRACE = %d\n", TRACE);
  //  scanf("%d",&TRACE);

   srand(9999);              /* init random number generator */
   sum = (float)0.0;         /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/(float)1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" );
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(0);
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   time=(float)0.0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
  double mmm = RAND_MAX;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */
  x = (float)(rand()/mmm);            /* x should be uniform in [0,1] */
  return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
   /* char *malloc(); // malloc redefinition removed */
   /* float ttime; // Unreferenced local variable removed */
   /* int tempint; // Unreferenced local variable removed */

   if (TRACE>2)
       printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  (float)(time + x);
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
}


void insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime);
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q;
        if (q==NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* front of list */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

void printevlist()
{
  struct event *q;
  /* int i; // Unreferenced local variable removed */
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stop_buffertimer(AorB)
int AorB;  /* A or B is trying to stop_buffer timer */
{
 struct event *q;/* ,*qold; // Unreferenced local variable removed */

 if (TRACE>2)
    printf("          STOP_buffertop_buffer TIMER: stop_bufferping timer at %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop_buffer timer */
float increment;
{

 struct event *q;
 struct event *evptr;
 /* char *malloc(); // malloc redefinition removed */

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }

/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  (float)(time + increment);
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
}


/************************** TOLAYER3 ***************/
void tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop_buffer timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 /* char *malloc(); // malloc redefinition removed */
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)
	printf("          TOLAYER3: packet being lost\n");
      return;
    }

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) )
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();



 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)
	printf("          TOLAYER3: packet being corrupted\n");
    }

  if (TRACE>2)
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
}

void tolayer5(AorB,datasent)
  int AorB;
  char datasent[20];
{
  int i;
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)
        printf("%c",datasent[i]);
     printf("\n");
   }
  
}
