
/*  First try of implementation of a gdb wrapper. 
 *
 *  See gdb documentation, section D for more information on the
 *  remote serial protocol. To make it short, a packet looks like the following:
 *
 *  $packet-data#checksum  or  $sequence-id:packet-data#checksum.
 * 
 *  where the checksum is the sum of all the characters modulo 256.
 */


#include              "gdbwrapper.h"
#include              "gdbwrapper-internals.h"


/**
 * This function parses a string *strtoparse* starting at character
 * *begin* and ending at character *end*. The new parsed string is
 * saved in *strret*. If *begin* is not found in *strtoparse* then
 * *strret* is NULL. If *end* is not found in *strtoparse*, then
 * *strret* has the value from *begin* to the end of *strtoparse*. No
 * verification on strret is done.
 *
 * @param strtoparse: String to parse.
 * @param strret    : String to return without *begin* and *end*.
 * @param begin     : String where to start parsing. If NULL,
 *                    we start from the beginning.
 * @param end       : String where to end parsing. If NULL,
 *                    we copy the string from *begin* to then
 *                    end of *strtoparse* (ie a NULL char is found).
 *
 * @return         : A pointer on *end* + 1 or NULL if we've reached the end.
 */
static char          *gdbwrap_extract_from_packet(const char *strtoparse,
						  char *strret,
						  const char *begin,
						  const char *end,
                                                  unsigned maxsize)
{
  const char         *loccharbegin;
  char               *loccharend;
  char               *retvalue;
  ptrdiff            strsize;
  
  assert(strtoparse != NULL);

  printf(" maxsize is: %#x\n ", maxsize);
  
  if (begin != NULL)
     {
        loccharbegin = strstr(strtoparse, begin);

        /* If begin is found, we point on the next char. */
        if (loccharbegin != NULL)
           loccharbegin+= strlen(begin);
     } else
        loccharbegin = strtoparse;
  
  if (loccharbegin != NULL)
     {
        if (end != NULL)
           {
              loccharend = strstr(loccharbegin, end);

              if (loccharend == NULL)
                 loccharend = strstr(loccharbegin, GDBWRAP_END_PACKET);
              
              if (loccharend != NULL)
                 {
                    strsize = loccharend - loccharbegin;
                    
                    if (strsize > maxsize)
                       strsize = maxsize;
                    printf(" strsize is: %#lx\n ", strsize);
                    fflush(stdout);
                    strncpy(strret, loccharbegin, strsize);
                    /* Note that if we didn't copy anything, then we put
                       GDBWRAP_NULL_CHAR at strret[0]. */
                    strret[strsize] = GDBWRAP_NULL_CHAR;
                    retvalue = loccharend + 1;
                 }
              else
                 retvalue = NULL;
           } else
           {
              strncpy(strret, loccharbegin, maxsize);
              /* *end* is null, we don't want to point on *end* + 1. */
              retvalue = NULL;
           }
     }
  else
     retvalue = NULL;
  
  return retvalue;
}


static la32          gdbwrap_little_endian(la32 addr)
{
   la32              addrlittle = 0;
   unsigned          i;

   for (i = 0; addr > 0; i++)
      {
         addrlittle += (LOBYTE(addr) << (8 * (sizeof(addr) - 1 - i)));
         addr >>= 8;
      }

   return addrlittle;
}


static unsigned      gdbwrap_atoh(const char * str, const unsigned size)
{
  unsigned           i;
  unsigned           hex;

  printf(" We received in %s - %s:\n ", __PRETTY_FUNCTION__, str);
  for (i = 0, hex = 0; i < size; i++)
    if (str[i] >= 'a' && str[i] <= 'f')
      hex += (str[i] - 0x57) << 4 * (size - i - 1);
    else if (str[i] >= '0' && str[i] <= '9')
      hex += (str[i] - 0x30) << 4 * (size - i - 1);
    else
      assert(0); /* will be replaced by false */

  printf(" We return: %#x\n ", hex);
  fflush(stdout);
  return hex;
}


static uint8       gdbwrap_calc_checksum(const char * str, gdbwrap_desc *desc)
{
  unsigned            i;
  uint8               sum;
  char                *result;
  
  
  result = gdbwrap_extract_from_packet(str, desc->packet, GDBWRAP_BEGIN_PACKET,
                                       GDBWRAP_END_PACKET, desc->max_packet_size);
  /* If result == NULL, it's not a packet. */
  if (result == NULL)
     gdbwrap_extract_from_packet(str, desc->packet, NULL, NULL,
                                 desc->max_packet_size);

  fflush(stdout);
  for (i = 0, sum = 0; i < strlen(desc->packet); i++)
     sum += desc->packet[i];

  return  sum;
}

#if 0
static uint8         gdbwrap_atoh_checksum(const char * str)
{
  char               *checksum_start = strstr(str, GDBWRAP_END_PACKET) + 1;

  return gdbwrap_atoh(checksum_start, strlen(checksum_start));
}
#endif

static void          gdbwrap_make_message(const char * query, gdbwrap_desc *desc)
{
   uint8              checksum       = gdbwrap_calc_checksum(query, desc);
  unsigned           max_query_size = (desc->max_packet_size -
                                       strlen(GDBWRAP_BEGIN_PACKET)
                                       - strlen(GDBWRAP_END_PACKET)
                                       - sizeof(checksum));
  if (strlen(query) < max_query_size)
     sprintf(desc->packet, "%s%s%s%x", GDBWRAP_BEGIN_PACKET, query, GDBWRAP_END_PACKET,
             checksum);
  else
    assert(0);
}


/**
 * Populate the gdb registers with the values received in the
 * packet. A packet has the following form:
 *
 * $n:r;[n:r;]#checksum
 *
 * where n can be a number (the register), or "thread" and r is the
 * value of the thread/register.
 *
 * @param packet: the packet to parse.
 * @param reg   : the structure in which we want to write the registers.
 */
static void          gdbwrap_populate_reg(const char *packet,
                                          gdbwrap_desc *desc)
{
   const char        *nextpacket = packet;
   char              *nextupacket;
   char              packetsemicolon[100];
   char              packetcolon[100];

   printf("packet received in %s: %s", __PRETTY_FUNCTION__,  packet);
   fflush(stdout);
   if (strstr(packet, "*") != NULL)
      {
         printf("Sorry, we don't support run-length encoding for now. ");
         return;
      }

   /*useless since static data + not what we want.


    */
   
   memset(&desc->reg32, 0, sizeof(gdbwrap_gdbreg32));
   while ((nextpacket = gdbwrap_extract_from_packet(nextpacket,
                                                    packetsemicolon,
                                                    NULL,
                                                    GDBWRAP_SEP_SEMICOLON,
                                                    sizeof(packetsemicolon)))
          != NULL)
      {
         nextupacket = gdbwrap_extract_from_packet(packetsemicolon, packetcolon,
                                                   NULL, GDBWRAP_SEP_COLON,
                                                   sizeof(packetcolon));
         /* Depending on the host, we must convert to little
            endian. Since the registers are represented as a 1 byte
            value, strlen(packetcolon) exactly equals 2.*/
         if (strlen(packetcolon) == 2)
            {
               uint8 regnumber = gdbwrap_atoh(packetcolon, 2);
               la32  regvalue  = gdbwrap_little_endian(gdbwrap_atoh(nextupacket, strlen(nextupacket)));

               *((uint32)desc->reg32 + regnumber) =  regvalue;
            }
      }
   
   
}


static void          gdbwrap_ack(const int fd)
{
  send(fd, GDBWRAP_COR_CHECKSUM, strlen(GDBWRAP_COR_CHECKSUM), 0x0);
}


static char          *gdbwrap_get_packet(gdbwrap_desc *desc)
{
  int                rval;
  char               checksum[3];

  ASSERT(desc != NULL);
  
  rval = recv(desc->fd, desc->packet, 1, 0);
  assert(desc->packet[0] == '+');
  rval = recv(desc->fd, desc->packet, desc->max_packet_size, 0);
  desc->packet[rval] = GDBWRAP_NULL_CHAR;
  printf("Received in %s: %s\n", __PRETTY_FUNCTION__, desc->packet);
  fflush(stdout);
  gdbwrap_extract_from_packet(desc->packet, checksum, GDBWRAP_END_PACKET, NULL,
                              sizeof(checksum));
  printf(" the checksum is: %s\n ", checksum);
  fflush(stdout);
  
  if (rval > 0 &&
      gdbwrap_atoh(checksum, strlen(checksum)) ==
      gdbwrap_calc_checksum(desc->packet, desc))
     gdbwrap_ack(desc->fd);

  return desc->packet;
}


/* We can still send a pointer with a small buffer */
static char          *gdbwrap_send_data(const char *query, gdbwrap_desc *desc)
{
  unsigned           rval = 0;

  ASSERT(desc != NULL && query != NULL);

  gdbwrap_make_message(query, desc);
  send(desc->fd, desc->packet, strlen(desc->packet), 0);
  gdbwrap_get_packet(desc);
  printf("Query: %s - rval: %#x - buf: %s  \n", query, rval, desc->packet);
  fflush(stdout);

  return desc->packet;
}


/**
 * Initialize a connection with the gdb server and allocate memory for
 * packets.
 *
 * @param fd: file descriptor of the socket.   
 */
gdbwrap_desc           *gdbwrap_hello(int fd)
{
  static gdbwrap_desc  desc;
  char                 *received;
  char                 *result;

  desc.max_packet_size = 600;
  desc.fd = fd;
  desc.packet = malloc(desc.max_packet_size * (sizeof(char) + 1));
  ASSERT(fd && desc.packet != NULL);

  received = gdbwrap_send_data(GDBWRAP_QSUPPORTED, &desc);
  LOG(" CAAAAAAAAAAAAAARA;BA");
  result   = gdbwrap_extract_from_packet(received, desc.packet, "PacketSize=",
                                         GDBWRAP_SEP_SEMICOLON,
                                         desc.max_packet_size);
  LOG("WE ARE HERE MAN\n");
  /* If we receive the info, we update gdbwrap_max_packet_size. */
  if (result != NULL)
     {
        printf("buf is %s\n ", desc.packet);
        fflush(stdout);
        desc.max_packet_size = gdbwrap_atoh(desc.packet, strlen(desc.packet));
        desc.packet = realloc(desc.packet, desc.max_packet_size * (sizeof(char) + 1));
     }
  LOG("WE FINISHED THAT CRAP\n");
  return &desc;
}


/**
 * Send a disconnecting command to the server and free the packet..
 */
void                   gdbwrap_bye(gdbwrap_desc *desc)
{
  gdbwrap_send_data(GDBWRAP_DISCONNECT, desc);
  free(desc->packet);
  printf("\nThx for using gdbwrap :)");
}


gdbwrap_gdbreg32       *gdbwrap_reason_halted(gdbwrap_desc *desc)
{
   char                *received;
   
   received = gdbwrap_send_data(GDBWRAP_WHY_HALTED, desc);

   if (received != NULL)
      {
         printf("At least we are here: %s\n", __PRETTY_FUNCTION__);
         /* Let's populate the gdb registers. We make the (true ?) supposition that */

         gdbwrap_populate_reg(received, desc->reg32);
         /* NOT GOOD SHOULD NOT BE RETURNED DIRECTLY TO USER. */


         
         return &desc->reg32;

         


         /* kasjkdjaksl dlksajd lksajlsajl */
      }
   else
      return NULL;
}


void                   gdbwrap_test(gdbwrap_desc *desc)
{
   const char *received = "T05thread:00000001;05:00000000;04:00000000;08:f0ff0f00;";
   gdbwrap_gdbreg32 t;

   gdbwrap_populate_reg(received, &t);
}

char                   *gdbwrap_own_command(const char *command, gdbwrap_desc *desc)
{
   return gdbwrap_send_data(command, desc);
}
