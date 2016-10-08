
#include "mini-ntpclient.h"

#include "rmfsystem.h"
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/timex.h>
#include "read_file.h"
#include <unistd.h>


/***************************************************************************
Function: send_packet

Description: to send the udp packet to NTP server 

Calls: NULL

Called By: query_server

Table Accessed: NULL

Table Updated: NULL

INput:
	sd: the socket descriptor

Output: NULL

Return: NULL

Others: NULL
***************************************************************************/ 
static void send_packet(int sd)
{
	uint32_t data[12];
	struct timeval now;

	memset(data, 0, sizeof(data));
	data[0] = htonl (
		( LI << 30 ) | ( VN << 27 ) | ( MODE << 24 ) |
		( STRATUM << 16) | ( POLL << 8 ) | ( PREC & 0xff ) );
	data[1] = htonl(1<<16);  /* Root Delay (seconds) */
	data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */
	gettimeofday(&now,NULL);
	data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
	data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
	send(sd,data,48,0);
}



/***************************************************************************
Function: set_time

Description: to set the system clock according to time received from ntp server

Calls: NULL

Called By: query_server

Table Accessed: NULL
		
Table Updated: NULL

INput: 
	srv : NTP server
	addr: NTP server ip address
	data: the time the NTP server give
Output:

Return:
	if success return 0
	else return -1

Others: NULL
***************************************************************************/
static int set_time(char *srv, struct in_addr addr, uint32_t *data)
{
	struct timeval tv;

	tv.tv_sec  = ntohl(((uint32_t *)data)[10]) - JAN_1970;
	tv.tv_usec = USEC(ntohl(((uint32_t *)data)[11]));
	if (settimeofday(&tv, NULL) < 0) {
		perror("settimeofday");
		return -1;	/* Ouch, this should not happen :-( */
	}

	syslog(LOG_DAEMON | LOG_INFO, "Time set from %s [%s].", srv, inet_ntoa(addr));

	return 0;		/* All good, time set! */
}



/***************************************************************************
Function:query_server

Description: send NTP request to NTPserver and receive the response, then set the time

Calls: send_packet set_time

Called By: ntp

Table Accessed: NULL

Table Updated: NULL

INput:  
	srv: the ntp server address

Output: NULL

Return: 
	if success return 0
	else return -1

Others: NULL
***************************************************************************/
static int query_server(char *srv)
{
	int sd, rc;
	struct pollfd pfd;
	struct hostent *he;
	struct sockaddr_in sa;

	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sd == -1) {
		perror("socket error");
		return -1;	/* Fatal error, cannot even create a socket? */
	}

//	he = gethostbyaddr(srv,4,AF_INET);
//	he = gethostbyname(srv);
//	if (!he) {
//		perror("gethostbyname");
//		close(sd);

//		return -1;	/* Failure in name resolution. */
//	}

//	memset(&sa, 0, sizeof(sa));
//	memcpy(&sa.sin_addr, he->h_addr_list[0], sizeof(sa.sin_addr));
	printf("address=%s\n",srv);
	fflush(stdout);
	sa.sin_port = htons(NTP_PORT);
	sa.sin_addr.s_addr=inet_addr(srv);
	sa.sin_family = AF_INET;

	syslog(LOG_DAEMON | LOG_DEBUG, "Connecting to %s [%s] ...", srv, inet_ntoa(sa.sin_addr));
	if (connect(sd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		perror("connect");
		close(sd);

		return -1;      /* Cannot connect to server, try next. */
	}

	/* Send NTP query to server ... */
	send_packet(sd);

	/* Wait for reply from server ... */
	pfd.fd = sd;
	pfd.events = POLLIN;
	rc = poll(&pfd, 1, 10000);
	if (rc == 1) {
		int len;
		uint32_t packet[12];

		//syslog(LOG_DAEMON | LOG_DEBUG, "Received packet from server ...");
		len = recv(sd, packet, sizeof(packet), 0);
		if (len == sizeof(packet)) {
			close(sd);

			/* Looks good, try setting time on host ... */
			if (set_time(srv, sa.sin_addr, packet))
				return -1; /* Fatal error */

			return 0;          /* All done! :) */
		}
	} else if (rc == 0) {
		syslog(LOG_DAEMON | LOG_DEBUG, "Timed out waiting for %s [%s].",
		       srv, inet_ntoa(sa.sin_addr));
	}

	close(sd);

	return -1;			   /* No luck, try next server. */
}



/***************************************************************************
Function: ntp

Description: the interface to set system clock

Calls: query_server

Called By: init

Table Accessed: NULL

Table Updated: NULL

INput: NULL

Output: NULL

Return: 
	if sueccess return 0
	else return -1

Others: NULL
***************************************************************************/
/* Connects to each server listed on the command line and sets the time */
int ntp()
{
	int i;
	char value[30];
	memset(value,'\0',sizeof(value));
	read_file("NTP","ntpaddress",value);
	if(strlen(value)!=0) {
		int rc = query_server(value);

		/* Done, time set! */
		if (0 == rc){
			printf("set time ok!\n");
			system("date");
			return 0;
		}
		/* Fatal error, exit now. */
		if (-1 == rc)
			return -1;
	}
	return 1;
}

/**
 * Local Variables:
 *  compile-command: "make mini-ntpclient"
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
