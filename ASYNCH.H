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


#ifndef __ASYNCH_H
#define __ASYNCH_H

#ifndef __DOS_H
    #include <dos.h>
#endif


/////////////////////////////////////////////////////////////////////////////


// Definitions used by the object class Asynch
enum COM_PORTS {
    COM1 =  0x01,   // Com port 1
    COM2 =  0x02,   // Com port 2
    COM3 =  0x03,   // Com port 3
    COM4 =  0x04    // Com port 4
};

// Character input/output buffer length define
#define IBUF_LEN    2048    // Incoming buffer
#define OBUF_LEN    1024    // Outgoing buffer

// Port addresses for the 8259 Programmable Interrupt Controller (PIC)
#define ICR         0x20    // Interrupt Control Register port
#define IMR         0x21    // Interrupt Mask Register port

// An End Of Interrupt needs to be sent to the 8259 control port when
// a hardware interrupt ends
#define EOI         0x20    // End Of Interrupt

// TXR  Output data to the serial port
// RXR  Input data from the serial port
// LCR  Initialize the serial port
// IER  Controls interrupt generation
// IIR  Identifies interrupts
// MCR  Send control signals to the modem
// LSR  Monitor the status of the serial port
// MSR  Receive status of the modem
#define TXR         0x00    // Transmit Register    (WRITE)
#define RXR         0x00    // Receive Register     (READ)
#define IER         0x01    // Interrupt Enable Register
#define IIR         0x02    // Interrupt ID Register
#define LCR         0x03    // Line Control Register
#define MCR         0x04    // Modem Control Register
#define LSR         0x05    // Line Status Register
#define MSR         0x06    // Modem Status Register

#define DTR         0x01    // Data Terminal Ready
#define RTS         0x02    // Request To Send.  There is data to send.
#define MCI         0x08    // Modem Control Interrupt

#define CTS         0x10    // Clear To Send
#define DSR         0x20    // Data Set Ready


/////////////////////////////////////////////////////////////////////////////


// Asynchronous port information structure
typedef struct _ainfo_t {
    unsigned char   port;               // serial port
    volatile int    inhead,intail;      // pointers to the in buffer
    volatile int    outhead,outtail;    // pointers to the out buffer
    volatile char   inbuf[IBUF_LEN];    // in buffer
    volatile char   outbuf[OBUF_LEN];   // out buffer
    int             base;               // base address to this port
    int             irq;                // interrupt number for this port
    int             flow;               // high-speed modem flow control
    unsigned int    baud;               // maximum baud speed for modem
    int             nohangup;           // hang up at end of session?
} _ainfo_t;

extern _ainfo_t _ainfo;


/////////////////////////////////////////////////////////////////////////////


// Function prototype for serial interrupt
void far interrupt asynch_irq(...); // interrupt handler


/////////////////////////////////////////////////////////////////////////////


// Asynchronous object class definition
class Asynch {
private:
    // Private member functions
    void asynchInit();              // interrupt initialization
public:
    // Constructors/Destructors
    Asynch(unsigned char);          // constructors
    Asynch(unsigned char,unsigned int);
   ~Asynch(void);                   // destructor

    // Public member functions
    void setBaud(unsigned int);     // sets the baudrate
    int dtr(void);                  // returns 1 if dtr high, 0 if not
    inline void setDtr(void);       // sets DTR high
    void dropDtr(void);             // sets DTR low

    // Operator overloaders
    Asynch &operator<<(char);       // overloaded << operator for chars
    Asynch &operator<<(char *);     // overloaded << operator for strings
    Asynch &operator>>(char &);     // overloaded >> operator for chars

    // C type functions
    char ainkey(void);              // inputs character from port
    void aputch(char);              // outputs character to port
    void aputs(char *);             // outputs string to port
    void aprintf(char *, ...);      // outputs formatted string to port

    // Inline functions
    int  inCount(void)              // counts chars left in the in buffer
        {
            int len = _ainfo.intail - _ainfo.inhead;
            return((len>0) ? len:(-len));
        }
    int  outCount(void)             // counts chars left in the out buffer
        {
            int len = _ainfo.outtail - _ainfo.outhead;
            return((len>0) ? len:(-len));
        }
    void flushInBuf(void)           // dumps in buffer
        {
            disable();                          // disable interrupts
            _ainfo.inhead=_ainfo.intail=0;      // dump buffer
            enable();                           // enable interrupts
        }
    void flushOutBuf(void)          // dumps out buffer
        {
            disable();                          // disable interrupts
            _ainfo.outhead=_ainfo.outtail=0;    // dump buffer
            enable();                           // enable interrupts
        }
    void flushAllBufs(void)         // flushes both input and output buffers
        {
            flushInBuf(), flushOutBuf();
        }
};


#endif // __ASYNCH_H
