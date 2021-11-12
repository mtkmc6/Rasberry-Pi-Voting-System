#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <time.h>
 

#define MSG_SIZE 40 // message size

void error(const char *msg)
{
    perror(msg);
    exit(0);
}



bool startsWith( char *pre,  char *str) // Checks if string starts with another string(pre)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

int main(int argc, char *argv[])
{
char*pre="128.206"; //Compare with this string, string must start with these numbers
bool value= false; // Checks if string starts with or not
struct ifaddrs *ifaddr;
int family, s;
char host[NI_MAXHOST];

if (getifaddrs(&ifaddr) == -1) {
          perror("getifaddrs");
          exit(EXIT_FAILURE);
           }

           /* Walk through linked list, maintaining head pointer so we
              can free list later. */

for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
 ifa = ifa->ifa_next) {
               if (ifa->ifa_addr == NULL)
                   continue;

               family = ifa->ifa_addr->sa_family;

               /* Display interface name and family (including symbolic
                  form of the latter for the common families). */

               printf("%-8s %s (%d)\n",
                      ifa->ifa_name,
                      (family == AF_PACKET) ? "AF_PACKET" :
                      (family == AF_INET) ? "AF_INET" :
                      (family == AF_INET6) ? "AF_INET6" : "???",
                      family);

               /* For an AF_INET* interface address, display the address. */

               if (family == AF_INET || family == AF_INET6) {
                   s = getnameinfo(ifa->ifa_addr,
                           (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                 sizeof(struct sockaddr_in6),
                           host, NI_MAXHOST,
                           NULL, 0, NI_NUMERICHOST);
                   if (s != 0) {
                       printf("getnameinfo() failed: %s\n", gai_strerror(s));
                       exit(EXIT_FAILURE);
                   }

                   printf("\t\taddress: <%s>\n", host);
                   if(startsWith(pre, host)== true){
break;// Breaks out of for loop, and host is the correct ip.
}
                   
                   

               } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
                   struct rtnl_link_stats *stats = ifa->ifa_data;

                   printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                          "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                          stats->tx_packets, stats->rx_packets,
                          stats->tx_bytes, stats->rx_bytes);
               }
           }

           freeifaddrs(ifaddr);
         //  exit(EXIT_SUCCESS);
       
printf("The board ip is %s\n", host);
char boardip[MSG_SIZE]="#";//Puts hashtag at beginning of string
strcat(boardip, host);
strcat(boardip, " "); //Adds space at end
bool ishost=false; // Whether or not is the host
puts(boardip); //tests if boardip is correct

int sock, length, n;
int boolval = 1; // for a socket option
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in addr;
char buffer[MSG_SIZE]; // to store received messages or messages to be sent.
char internalCopy[MSG_SIZE]; //Keeps copy of string containing ip and vote

   if (argc < 2)
   {
printf("usage: %s port\n", argv[0]);
      exit(0);
   }

   sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
   if (sock < 0)
error("Opening socket");

   length = sizeof(server); // length of structure
   bzero(&server,length); // sets all values to zero. memset() could be used
   server.sin_family = AF_INET; // symbol constant for Internet domain
   server.sin_addr.s_addr = INADDR_ANY; // IP address of the machine on which
// the server is running
   server.sin_port = htons(atoi(argv[1])); // port number

   // binds the socket to the address of the host and the port number
   if (bind(sock, (struct sockaddr *)&server, length) < 0)
       error("binding");

   // change socket permissions to allow broadcast
   if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
    {
    printf("error setting socket options\n");
    exit(-1);
    }

   fromlen = sizeof(struct sockaddr_in); // size of structure

time_t t; //random number
srand((unsigned) time(&t)); //initialization
int randnum;
char random; //Convert to char
char*cmp="#128.206";
char*name="Matthew"; //Board name of user

   while (1)
   {
		//printf("in while\n");
		// bzero: to "clean up" the buffer. The messages aren't always the same length...
		bzero(buffer,MSG_SIZE); // sets all values to zero. memset() could be used

		// receive from a client
		n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &fromlen);
		//printf("Received a datagram. It says: %s\n", buffer); //Used to test if properly sending through socket
		if (n < 0)
		error("recvfrom");
     
		if(startsWith("WHOIS", buffer)== true){
 
				if (ishost==true){
					//char tmp1[MSG_SIZE];
					char*tmp1= (char *)malloc(40*sizeof(char));
					strcpy(tmp1, "Matthew");
					strcat(tmp1, " on Board ");
					strcat(tmp1, host); //appends ip
					char temp3[MSG_SIZE];
					strcat(tmp1, " is the master");
					puts(tmp1);
					strcpy(temp3, tmp1);

					addr.sin_addr.s_addr = inet_addr("128.206.19.255");  
					n = sendto(sock, temp3, 32, 0,
					(struct sockaddr *)&addr, fromlen);
					if (n  < 0)
					error("sendto");
					free(tmp1);
					}
		}
     
	else if(startsWith("VOTE", buffer)== true){
 
			randnum= rand() % 10 + 1; //rand num
			char*tempbuffer;
			sprintf(tempbuffer, "%d", randnum); //converts integer to string form
			char *tempmessage=(char *)malloc(40*sizeof(char)); //Had to change to malloc to avoid Seg faults
			//char tempmessage[MSG_SIZE];
			strcpy(tempmessage, boardip);
			strcat(tempmessage, tempbuffer);
			addr.sin_addr.s_addr = inet_addr("128.206.19.255");  
			n = sendto(sock, tempmessage, 32, 0,
			(struct sockaddr *)&addr, fromlen);
			if (n  < 0)
			error("sendto");  
			free(tempmessage);
     
		}
		
	else if(startsWith("#128.206", buffer)==true){ //Vote message recieved from another board
		printf("In startsWithcmp\n");
		int temp;
		char tempchar;
		tempchar = buffer[15];
		temp= tempchar - '0';
		printf("randnum is %d\n", randnum);
		printf("temp is %d\n", temp);
		
		if(randnum>temp){
			ishost=true; //Temporarily becomes master, if no vote is higher, it will stay master
			}
		else if(randnum<temp){
			ishost=false;
			}
		else {
			puts(buffer);
			puts(boardip);
 
			if(memcmp((const void*)buffer, (const void*)boardip, 14) >= 0){
				ishost=true;
				}
			else{
				ishost=false;
				}
			}
 
		}


	}

   return 0;
 }
