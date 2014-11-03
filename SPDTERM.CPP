/* A terminal program which I wrote to practice programming in C++ */

#include <conio.h>
#include <iostream.h>
#include <stdio.h>

#include "spdterm.h"        // structures and global vars and defines
#include "asynch.h"         // Asynch class definition


static Asynch com2(COM2,2400);


inline char readch()
{
    if (kbhit())
    {
        char ch = getch();
        if (!ch)
        {
            if (getch()==45)    // Alt-X hit
                return(-1);
        }
        return(ch);
    }
    return(0);
}


inline void writech(char ch)
{
    if (ch)
        cout << ch;
}


void terminal(void)
{
    clrscr();
    cout << "Terminal ready... hit Alt-X to exit.\n";
    char modem=0,key=0;
    int done=0;
    do {
        key=readch();
        switch(key) {
            case -1: // exit terminal
                done=1;
                break;
            default:
                break;
        }
        com2 << key;
        com2 >> modem;
        writech(modem);
    } while (!done);
}


void answer(void)
{
    cout << "Waiting for call... ";
    while(!com2.inCount() && !kbhit())
        ;
    char s[81];
    int i=0;
    while(com2.inCount())
        com2 >> s[i++];
    s[i]=0;
    cout << s;
    com2 << "ata\r";
}


void main()
{
    com2 << "at m1 l1 s0=0 s7=90 e1 q0 v1 x4 &c1 &d2\r";
//    if (!com2.dtr())  // not really necessary
//        com2.setDtr();
    terminal();
//    answer();     // still doesn't work right
//    com2.dropDtr();
}
