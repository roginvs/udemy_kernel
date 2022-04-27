#ifndef IO_H
#define IO_H

/*   Reads a byte from a port */
unsigned char insb(unsigned short port);
/*   Reads a word from a port */
unsigned short insw(unsigned short port);

/*   Write a byte from a port */
void outb(unsigned short port, unsigned char data);

/*   Write a word from a port */
void outw(unsigned short port, unsigned short data);

#endif