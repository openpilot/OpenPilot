#ifndef PJRC_RAWHID_H
#define PJRC_RAWHID_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <QDebug>

class pjrc_rawhid
{
public:
    pjrc_rawhid();
    int open(int max, int vid, int pid, int usage_page, int usage);
    int receive(int num, void *buf, int len, int timeout);
    void close(int num);
    int send(int num, void *buf, int len, int timeout);
    int getserial(int num, char *buf);

private:

};

#endif // PJRC_RAWHID_H
