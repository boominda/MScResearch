#include "contiki-net.h"
#include <stdio.h>

static struct psock ps;
static char buffer[100];

struct conn_state{
	enum {send1, send2, send3, send4, send5, send6, send7} state;
};


PROCESS(example_psock_client_process, "Example protosocket client");

/*---------------------------------------------------------------------------*/
  static int
handle_connection(struct psock *p)
{
  PSOCK_BEGIN(p);
  struct conn_state *s;
  s->state = send1;

  PSOCK_SEND_STR(p, "Connection Request");

  while(1) {
    if(s->state == send1)
    {
    	memset(buffer,0,sizeof(buffer));
    	PSOCK_READTO(p, '\n');
	printf("Got: %s", buffer);
	printf("Server is Authenticated \n");
	s->state = send2;
   
    }
    if(s->state == send2)
    {	
	PSOCK_SEND_STR(p,"certificate\n");
        s->state = send3;
    }
    if(s->state == send3)
    {
    	memset(buffer,0,sizeof(buffer));
    	PSOCK_READTO(p, '\n');
    	printf("Got: %s", buffer);
        printf("I received servers shared key \n"); 
        s->state = send4;
    }
    if(s->state == send4)
    {
	printf("I accepted servers shared key \n");
	s->state = send5;
	PSOCK_SEND_STR(p,"shared key accepted\n");
    }
    if(s->state == send5)
    {
    	memset(buffer,0,sizeof(buffer));
	PSOCK_READTO(p, '\n');
 	printf("Got : %s", buffer);
	printf("Secure Connection Established\n");
	PSOCK_SEND_STR(p,"Connected\n");
	printf("Connected Sent\n");
    }
    
}

  PSOCK_END(p);
}
/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(&example_psock_client_process);
PROCESS_THREAD(example_psock_client_process, ev, data)
{
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  uip_ipaddr(&addr, 172,16,1,0);
  tcp_connect(&addr, UIP_HTONS(5555), NULL);

  printf("Connecting...\n");
  PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

  if(uip_aborted() || uip_timedout() || uip_closed()) {
    printf("Could not establish connection...\n");
  } else if(uip_connected()) {
    printf("Connected\n");

    PSOCK_INIT(&ps, buffer, sizeof(buffer));

    do {
      handle_connection(&ps);
      PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    } while(!(uip_closed() || uip_aborted() || uip_timedout()));

    printf("Connection closed.\n");
  }
  PROCESS_END();
}

