/////////////////////////////////////////////////////////////////////////////
//
//  Asynchronous Class for C++
//  Copyright (C) 1991 by Jui-Lin Hung and SPD!
//
//  You may freely use or incorporate these routines into your own programs
//  without royalty to me, as I believe this is beneficial to programmers.
//  However, I would like to request that if you distribute the source code,
//  you would include this header in the source file and not remove it.
//  Thank you, and I hope these routines are useful.
//
//  April, 1991 - Version 1.1
//
/////////////////////////////////////////////////////////////////////////////


#include <dos.h>
#include <stdarg.h>
#include <stdio.h>

#include "asynch.h"     // Asynch class definitions


/////////////////////////////////////////////////////////////////////////////


_ainfo_t _ainfo;                // Global asynchronous info variable


/////////////////////////////////////////////////////////////////////////////


void far interrupt (*OldVect)(...);

void far interrupt asynch_irq(...)
// Asynchronous port interrupt handler.  This is the new interrupt which
// we will swap with the old one to handle all asynchronous i/o
// This interrupt handler was apparently ripped off of Rob Raper's
// buffered NEWCOM routines for the WWIV bulletin board software.
// Original credit goes to him.
{
    static int temp;

    enable();                               // enable interrupts
    for (;;)
    {
        temp = inportb(_ainfo.base+IIR);    // why interrupt was called
        if (temp & 0x01)                    // Nothing else to do
        {
            outportb(ICR,EOI);              // reset interrupt
            return;                         // return to program
        }
        switch(temp)
        {
            case 0x00:  // modem status changed
                inportb(_ainfo.base+MSR);   // read in useless char
                break;
            case 0x02:  // Request To Send char
                if (_ainfo.outhead != _ainfo.outtail)  // there's a char to send
                {
                    // send the character
                    outportb(_ainfo.base+TXR,_ainfo.outbuf[_ainfo.outhead++]);
                    // if at end of buffer, reset pointer
                    if (_ainfo.outhead == OBUF_LEN) _ainfo.outhead=0;
                }
                break;
            case 0x04:  // character ready to be read in
                // read character into inbuffer
                _ainfo.inbuf[_ainfo.inhead++] = inportb(_ainfo.base+RXR);
                if (_ainfo.inhead == IBUF_LEN) // if at end of buffer
                    _ainfo.inhead=0;           // reset pointer
                break;
            case 0x06:  // line status has changed
                inportb(_ainfo.base+LSR);     // read in useless char
                break;
            default:
                break;
        }
    }
}


void Asynch::asynchInit()
// Initializes variables, saves old interrupt vectors and sets the new
// interrupts.  Initializes the asynchronous port too.
{
    OldVect = getvect(_ainfo.irq+MCI);      // Save old interrupt vector
    setvect(_ainfo.irq+MCI,asynch_irq);     // Set up serial int handler
    outportb(_ainfo.base+LCR, 0x03);        // Turn DTR and RTS on
    disable();                              // disable ints during init
    int temp = inportb(_ainfo.base+LSR);    // read serial port line status
    temp = inportb(_ainfo.base+RXR);        // read char
    temp = inportb(IMR);                    // get interrupt settings
    temp = temp & ((IER<<_ainfo.irq)^0xff); // turn on serial interrupt
    outportb(IMR, temp);                    // send out new int settings
    outportb(_ainfo.base+IER, IER);         // turn on all IRQ events
    outportb(_ainfo.base+MCR, inportb(_ainfo.base+MCR) | 0x0a);
    enable();                               // re-enable all interrupts

    _ainfo.inhead=_ainfo.intail=0;          // reset in buffer pointers
    _ainfo.outhead=_ainfo.outtail=0;        // reset out buffer pointers
    _ainfo.flow = 0;                        // Default flow control is off
    _ainfo.nohangup = 0;                    // hangup when done
}


Asynch::Asynch(unsigned char p)
// Class constructor - set base address and irq number.
{
    switch(p)   // set up correct base address and irq for this port
    {
        case COM1:  // serial port 1
            _ainfo.base = 0x03f8;   // base address for port 1
            _ainfo.irq  = 4;        // interrupt number for port 1
            break;
        case COM2:  // serial port 2
            _ainfo.base = 0x02f8;   // base address for port 2
            _ainfo.irq  = 3;        // interrupt number for port 2
            break;
        case COM3:  // serial port 3
            _ainfo.base = 0x03e8;   // base address for port 3
            _ainfo.irq  = 4;        // interrupt number for port 3
            break;
        case COM4:  // serial port 4
            _ainfo.base = 0x02e8;   // base address for port 4
            _ainfo.irq  = 3;        // interrupt number for port 4
            break;
        default:  // defaults to com2, cuz that's what I have!
            _ainfo.base = 0x02f8;   // base address for port 2
            _ainfo.irq  = 3;        // interrupt number for port 2
            break;
    }

    _ainfo.baud = 2400;             // Default baud rate is 2400

    asynchInit();                   // call interrupt initialization
    setBaud(_ainfo.baud);           // set to default baud rate
}


Asynch::Asynch(unsigned char p,unsigned int b)
// Class constructor - set base address and irq number.
// This particular constructor also sets the baud rate to the
// specified speed
{
    switch(p)   // set up correct base address and irq for this port
    {
        case COM1:  // serial port 1
            _ainfo.base = 0x03f8;   // base address for port 1
            _ainfo.irq  = 4;        // interrupt number for port 1
            break;
        case COM2:  // serial port 2
            _ainfo.base = 0x02f8;   // base address for port 2
            _ainfo.irq  = 3;        // interrupt number for port 2
            break;
        case COM3:   // serial port 3
            _ainfo.base = 0x03e8;   // base address for port 3
            _ainfo.irq  = 4;        // interrupt number for port 3
            break;
        case COM4:  // serial port 4
            _ainfo.base = 0x02e8;   // base address for port 4
            _ainfo.irq  = 3;        // interrupt number for port 4
            break;
        default:    // defaults to com2
            _ainfo.base = 0x02f8;   // base address for port 2
            _ainfo.irq  = 3;        // interrupt number for port 2
            break;
    }

    _ainfo.baud = b;                // Default baud rate is 2400

    asynchInit();                   // call interrupt initialization
    setBaud(_ainfo.baud);           // set to default baud rate
}



Asynch::~Asynch()
// Class destructor - de-initializes the asynchronous port, and restores
// the old interrupts
{
    disable();                          // disable interrupts
    int temp = inportb(IMR);
    temp = temp | (IER << _ainfo.irq);
    outportb(IMR, temp | CTS);
    outportb(_ainfo.base+IIR, 0x00);
    outportb(_ainfo.base+MCR, IER);
    enable();                           // enable interrupts
    setvect(_ainfo.irq+MCI,OldVect);    // reinstate old vector
}


void Asynch::setBaud(unsigned int b)
// Sets the baud rate to the specified speed
{
    if (b > 49 && (long)b < 57601L)
    {
        float rate = 115200.0 / ((float)b);
        _ainfo.baud = (unsigned int)rate;

        outportb(_ainfo.base+LCR, inportb(_ainfo.base+LCR) | 0x80);
        outportb(_ainfo.base, (_ainfo.baud & 0x00ff));
        outportb(_ainfo.base+IER, ((_ainfo.baud >> MCI) & 0x00ff));
        outportb(_ainfo.base+LCR, inportb(_ainfo.base+LCR) & 0x7f);
    }
}


int Asynch::dtr()
// Returns 1 if the DTR is high, or if DTR remains low, returns a 0
{
    int addr;

    switch(_ainfo.port)
    {
        case COM1: addr = 0x03fe; break;
        case COM2: addr = 0x02fe; break;
        case COM3: addr = 0x03ee; break;
        case COM4: addr = 0x02ee; break;
    }
    if (inportb(addr) & 128)
        return(1);
    delay(500);
    return(inportb(addr) & 128);
}


void Asynch::setDtr()
// This function basically sets the DTR to high.
{
    outportb(_ainfo.base+MCR, inportb(_ainfo.base+MCR) & 0x00fe);
}


void Asynch::dropDtr()
// This function basically sets the DTR to low (i.e. hangs up the phone)
{
    int addr;

    if (!_ainfo.nohangup)
    {
        switch(_ainfo.port)
        {
            case COM1: addr = 0x03fc; break;
            case COM2: addr = 0x02fc; break;
            case COM3: addr = 0x03ec; break;
            case COM4: addr = 0x02ec; break;
        }
        outportb(addr, inportb(addr) & 0xfe);
        delay(500);
    }
}    


/////////////////////////////////////////////////////////////////////////////


Asynch &Asynch::operator<<(char ch)
// Inserts the character to be outputted into the output buffer, checking
// for an open slot in the output buffer array.  If there is, insert
// the character, or if there isn't, wait until a slot opens up.
{
    if (ch)                         // If this is a valid char
    {
        enable();                   // turn on irqs to ensure data output

        // check buffer, and if full, wait for an available opening
        while((_ainfo.outhead-1==_ainfo.outtail) ||
              (_ainfo.outtail==OBUF_LEN-1 && _ainfo.outhead==0))
            ;
        disable();  // make sure nothing happens while changing buffer
        _ainfo.outbuf[_ainfo.outtail++]=ch; // insert character into buffer;
        if (_ainfo.outtail == OBUF_LEN)     // if at end of out buffer
            _ainfo.outtail = 0;             // reset pointer
        enable();                           // re-enable interrupts
        outportb(_ainfo.base+DTR,0x0f);
    }
    return(*this);
}


Asynch &Asynch::operator<<(char *str)
// Outputs a string to the serial port
{
    while (*str)
    {
        if (*str=='\n')
            (*this) << '\r', (*this) << '\n';
        else
            (*this) << (*str);
        str++;
    }
    return(*this);
}


Asynch &Asynch::operator>>(char &ch)
// Returns either the character to be received from modem if there is one
// waiting in the buffer, or returns a 0 if there is no character waiting.
{
    if (_ainfo.inhead != _ainfo.intail)     // there is a character
    {
        disable();                          // disable irqs while getting char
        ch = _ainfo.inbuf[_ainfo.intail++]; // get character from buffer
        if (_ainfo.intail == IBUF_LEN)      // if at end of in buffer
            _ainfo.intail=0;                // reset pointer
        enable();                           // re-enable interrupt
        return(*this);                      // return the char
    }
    ch = 0;
    return(*this);                          // return nothing
}


/////////////////////////////////////////////////////////////////////////////


// C type functions for asynchronous i/o

char Asynch::ainkey()
// Returns 0 if no character waiting, or the character itself if there
// is a character waiting in the buffer
{
    if (_ainfo.inhead != _ainfo.intail)     // there is a character
    {
        disable();                          // disable irqs while getting char

        // get character from buffer
        unsigned char ch = _ainfo.inbuf[_ainfo.intail++];

        if (_ainfo.intail == IBUF_LEN)      // if at end of in buffer
            _ainfo.intail=0;                // reset pointer
        enable();                           // re-enable interrupt
        return(ch);                         // return the char
    }
    return(0);                              // return nothing
}


void Asynch::aputch(char ch)
// Inserts the character to be outputted into output buffer.  If there is
// an open slot in the output buffer array, insert character there, or
// wait for a slot to open.
// This output character function apparently ripped off of Rob Raper's
// buffered NEWCOM routines for the WWIV bulletin board software.
// Original credit goes to him.
{
    if (ch)                         // If this is a valid char
    {
        enable();                   // turn on irqs to ensure data output

        // check buffer, and if full, wait for an available opening
        while((_ainfo.outhead-1==_ainfo.outtail) ||
              (_ainfo.outtail==OBUF_LEN-1 && _ainfo.outhead==0))
            ;
        disable();  // make sure nothing happens while changing buffer
        _ainfo.outbuf[_ainfo.outtail++]=ch; // insert character into buffer;
        if (_ainfo.outtail == OBUF_LEN)     // if at end of out buffer
            _ainfo.outtail = 0;             // reset pointer
        enable();                           // re-enable interrupts
        outportb(_ainfo.base+DTR,0x0f);
    }
}


void Asynch::aputs(char *str)
// Outputs a string through the serial port
{
    while (*str)
    {
        if (*str=='\n')
            aputch('\r'), aputch('\n');
        else
            aputch(*str);
        str++;
    }
}


void Asynch::aprintf(char *format, ...)
// Outputs a formatted string through the serial port
{
    #define MAXLEN 256
    va_list argptr;
    char str[256];

    va_start(argptr,format);            // access argument list
    vsprintf(str,format,argptr);        // create string using argument list
    va_end(argptr);                     // end access of argument list

    aputs(str);                         // output the formatted string
}
