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
#include <fcntl.h>

#include <string.h>
#include "MQTTClient.h"
#define ADDRESS     "tcp://localhost:1883" //Host Adress
#define CLIENTID    "Keyboard2"
#define TOPIC       "Election" //Used for reading subsribed messages
#define TOPIC2       "EC" //Used for publishing messages so Election officer can read

#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

#define MSG_SIZE 40 // message size

#define CHAR_DEV "/dev/Chardevice" // "/dev/YourDevName"

bool ishost=false; // Whether or not is the host
 MQTTClient client;
 char boardip[MSG_SIZE]="#";//Puts hashtag at beginning of string
 char host[NI_MAXHOST];
 int randnum;
char host[NI_MAXHOST];

volatile MQTTClient_deliveryToken deliveredtoken;

bool startsWith( char *pre,  char *str) // Checks if string starts with another string(pre)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
//printf("In msg arrived\n");

MQTTClient_message pubmsg = MQTTClient_message_initializer;
 
MQTTClient_deliveryToken token;
    int i;
    char* payloadptr;
    int rc;
   
   
 
    payloadptr = message->payload;
   
    char buffer1[MSG_SIZE];
    strcpy(buffer1, payloadptr);
   
   
   
   
   printf("Received a datagram. It says: %s\n", buffer1); //Used to test if properly sending through socket
 
   if(*payloadptr == '@'){
  int cdev_id, dummy;
size_t bufsize = 50;
puts(topicName);
if(ishost==true && startsWith("Election", topicName)== true){
printf("Sending Note from Master to other Boards\n");
pubmsg.payload = (void*)payloadptr;
pubmsg.payloadlen = strlen(payloadptr);
pubmsg.qos = QOS;
pubmsg.retained = 0;
MQTTClient_publishMessage(client, TOPIC2, &pubmsg, &token);
printf("Waiting for up to %d seconds for publication of %s\n"
"on topic %s for client with ClientID: %s\n",
(int)(TIMEOUT/1000), pubmsg.payload, TOPIC2, CLIENTID);
   
printf("Message with delivery token %d delivered\n", token);
   
payloadptr++;
 
 
// Open the Character Device for writing
if((cdev_id = open(CHAR_DEV, O_WRONLY)) == -1) {
printf("Cannot open device %s\n", CHAR_DEV);
exit(1);
}
dummy = write(cdev_id, payloadptr, sizeof(payloadptr));
if(dummy != sizeof(payloadptr)) {
printf("Write failed, leaving...\n");
}
}
if ( ishost==false){
payloadptr++;
 
 
// Open the Character Device for writing
if((cdev_id = open(CHAR_DEV, O_WRONLY)) == -1) {
printf("Cannot open device %s\n", CHAR_DEV);
exit(1);
}
dummy = write(cdev_id, payloadptr, sizeof(payloadptr));
if(dummy != sizeof(payloadptr)) {
printf("Write failed, leaving...\n");
}
}
 


close(cdev_id);
   
}

if(startsWith("WHOIS", payloadptr)== true){
 
if (ishost==true){
//char tmp1[MSG_SIZE];
char*tmp1= (char *)malloc(40 *sizeof(char));
strcpy(tmp1, "Matt");
strcat(tmp1, " on Board ");
strcat(tmp1, host); //appends ip
char temp3[MSG_SIZE];
strcat(tmp1, " is the master");
puts(tmp1);
strcpy(temp3, tmp1);

pubmsg.payload = (void*)temp3;
pubmsg.payloadlen = strlen(temp3);
pubmsg.qos = QOS;
pubmsg.retained = 0;
MQTTClient_publishMessage(client, TOPIC2, &pubmsg, &token);
printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), pubmsg.payload, TOPIC2, CLIENTID);
rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
printf("Message with delivery token %d delivered\n", token);
free(tmp1);
}
}
   
     
else if(startsWith("VOTE",payloadptr)== true){
//printf("In vote\n");
randnum= rand() % 10 + 1; //rand num
char*tempbuffer= (char *)malloc(2*sizeof(char));

sprintf(tempbuffer, "%d ", randnum); //converts integer to string form

char *tempmessage=(char *)malloc(40*sizeof(char)); //Seg faults

//char tempmessage[MSG_SIZE];
strcpy(tempmessage, boardip);
strcat(tempmessage, tempbuffer);
puts(tempmessage);
 
pubmsg.payload = (void*)tempmessage;
pubmsg.payloadlen = strlen(tempmessage);
// pubmsg.payload = PAYLOAD;
//pubmsg.payloadlen = strlen(PAYLOAD);
   
pubmsg.qos = QOS;
pubmsg.retained = 0;
   
int error=5;
//MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
error=MQTTClient_publishMessage(client, TOPIC2, &pubmsg, &token);
printf("Returns %d\n" ,error);
   
printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), pubmsg.payload, TOPIC2, CLIENTID);
           
//rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
printf("Message with delivery token %d delivered\n", token);
 
 
free(tempbuffer);
free(tempmessage);
printf("Leaving vote\n");
      }


else if(startsWith("#128.206", payloadptr)==true){ //Vote message recieved from another board
printf("In startsWithcmp\n");
int temp;
char tempchar;
tempchar = *(payloadptr + 15);
temp= tempchar - '0';
printf("randnum is %d\n", randnum);
printf("temp is %d\n", temp);
if(randnum>temp){
ishost=true; //Temporarily becomes master, if no vote is higher, it will stay master
printf("In funciton\n");
}
else if(randnum<temp){
ishost=false;
}
else {
puts(payloadptr);
puts(boardip);
if(memcmp((const void*)payloadptr, (const void*)boardip, 14) >= 0){
ishost=true;
}
else{
ishost=false;
}
}
 
}



//printf("Hey\n");

    MQTTClient_freeMessage(&message);
   // printf("Hi\n");
    MQTTClient_free(topicName);
    //printf("hard\n");
    return 1;
}
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}



void error(const char *msg)
{
    perror(msg);
    exit(0);
}




int main(int argc, char* argv[])
{
char*pre="128.206"; //Compare with this string, string must start with these numbers
bool value= false; // Checks if string starts with or not
struct ifaddrs *ifaddr;
int family, s;


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

strcat(boardip, host);
strcat(boardip, " "); //Adds space at end

puts(boardip); //tests if boardip is correct


time_t t; //random number
srand((unsigned) time(&t)); //initialization

char random; //Convert to char
char*cmp="#128.206";
char*name="Matthew"; //Board name of user


   
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
    MQTTClient_create(&client, ADDRESS, CLIENTID, //Creates client using client ID
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered); //Sets callback function for background msgarrvd function
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
 
    MQTTClient_subscribe(client, TOPIC, QOS);
    MQTTClient_subscribe(client, TOPIC2, QOS); //Subscribe to both topics
      printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
             printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC2, CLIENTID, QOS);
   
   //printf("Before\n");
   
   // Infinite loop to stop "main()" thread from closing all the call backs.
   do
    {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
   
   printf("After\n");
   
   
   
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}