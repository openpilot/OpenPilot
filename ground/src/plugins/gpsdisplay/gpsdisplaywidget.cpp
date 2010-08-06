/**
 ******************************************************************************
 *
 * @file       gpsdisplaywidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration 
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "gpsdisplaywidget.h"
#include "ui_gpsdisplaywidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"

#include <iostream>
#include <QtGui/QFileDialog>
#include <QDebug>
#include <QThread>

// constants/macros/typdefs
#define NMEA_BUFFERSIZE		128

// Message Codes
#define NMEA_NODATA		0	// No data. Packet not available, bad, or not decoded
#define NMEA_GPGGA		1	// Global Positioning System Fix Data
#define NMEA_GPVTG		2	// Course over ground and ground speed
#define NMEA_GPGLL		3	// Geographic position - latitude/longitude
#define NMEA_GPGSV		4	// GPS satellites in view
#define NMEA_GPGSA		5	// GPS DOP and active satellites
#define NMEA_GPRMC		6	// Recommended minimum specific GPS data
#define NMEA_UNKNOWN	0xFF// Packet received but not known

#define GPS_TIMEOUT_MS 500

// Debugging

//#define GPSDEBUG

#ifdef GPSDEBUG
        #define NMEA_DEBUG_PKT	///< define to enable debug of all NMEA messages
        #define NMEA_DEBUG_GGA	///< define to enable debug of GGA messages
        #define NMEA_DEBUG_VTG	///< define to enable debug of VTG messages
        #define NMEA_DEBUG_RMC	///< define to enable debug of RMC messages
        #define NMEA_DEBUG_GSA	///< define to enable debug of GSA messages
#endif

// Private functions

// Global variables

// Private constants
typedef struct struct_GpsData
{
        float Latitude;
        float Longitude;
        float Altitude;
        float Groundspeed;
        int SV;
        uint8_t channel;
        uint8_t value_h;
        uint8_t value_l;
        uint8_t sum;
}GpsData_t;

// Private types

// Private variables
cBuffer gpsRxBuffer;
static char gpsRxData[512];
char NmeaPacket[NMEA_BUFFERSIZE];
static uint32_t numUpdates;
static uint32_t numErrors;
static uint32_t timeOfLastUpdateMs;
static int32_t gpsRxOverflow=0;
GpsData_t GpsData;



class MyThread : public QThread
{
public:
    QextSerialPort *port;
    void setPort(QextSerialPort* port);
    void processInputStream();
    void run();
};



/*
 * Initialize the widget
 */
GpsDisplayWidget::GpsDisplayWidget(QWidget *parent) : QWidget(parent)
{
    bufferInit(&gpsRxBuffer, (unsigned char *)gpsRxData, 512);

    setMinimumSize(128,128);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
 
    widget = new Ui_GpsDisplayWidget();
    widget->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    QSvgRenderer *renderer = new QSvgRenderer();
    QGraphicsSvgItem *world = new QGraphicsSvgItem();
    renderer->load(QString(":/gpsgadget/images/gpsEarth.svg"));
    world->setSharedRenderer(renderer);
    world->setElementId("map");
    scene->addItem(world);
    scene->setSceneRect(world->boundingRect());
    widget->gpsWorld->setScene(scene);
    // Somehow fitInView does not work there at all? Makes
    // the 'world' element tiny tiny tiny. anyone knows why??
    //widget->gpsWorld->fitInView(world,Qt::KeepAspectRatio);
    qreal factor = widget->gpsWorld->size().height()/world->boundingRect().height();
    widget->gpsWorld->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    world->setScale(factor);


    connect(widget->connectButton, SIGNAL(clicked(bool)),
            this,SLOT(connectButtonClicked()));
}

GpsDisplayWidget::~GpsDisplayWidget()
{
   delete widget;
}


void GpsDisplayWidget::setPort(QextSerialPort* port)
{

    this->port=port;
}

void GpsDisplayWidget::connectButtonClicked() {
    MyThread* myThread = new MyThread();
    myThread->setPort(port);
    myThread->start();
}



/**
 * Prosesses NMEA sentence checksum
 * \param[in] Buffer for parsed nmea sentence
 * \return 0 checksum not valid
 * \return 1 checksum valid
 */
char GpsDisplayWidget::nmeaChecksum(char* gps_buffer)
{
        char checksum=0;
        char checksum_received=0;

        for(int x=0; x<NMEA_BUFFERSIZE; x++)
        {
                if(gps_buffer[x]=='*')
                {
                        //Parsing received checksum...
                        checksum_received = strtol(&gps_buffer[x+1],NULL,16);
                        break;
                }
                else
                {
                        //XOR the received data...
                        checksum^=gps_buffer[x];
                }
        }
        //PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%d=%d\r\n",checksum_received,checksum);
        if(checksum == checksum_received)
        {
                ++numUpdates;
                return 1;
        }
        else
        {
                ++numErrors;
                return 0;
        }
}

/**
 * Prosesses NMEA sentences
 * \param[in] cBuffer for prosessed nmea sentences
 * \return Message code for found packet
 * \return 0xFF NO packet found
 */
uint8_t GpsDisplayWidget::nmeaProcess(cBuffer* rxBuffer)
{
        uint8_t foundpacket = NMEA_NODATA;
        uint8_t startFlag = FALSE;
        //u08 data;
        uint16_t i,j;

        // process the receive buffer
        // go through buffer looking for packets
        while(rxBuffer->datalength)
        {
                // look for a start of NMEA packet
                if(bufferGetAtIndex(rxBuffer,0) == '$')
                {
                        // found start
                        startFlag = TRUE;
                        // when start is found, we leave it intact in the receive buffer
                        // in case the full NMEA string is not completely received.  The
                        // start will be detected in the next nmeaProcess iteration.

                        // done looking for start
                        break;
                }
                else
                        bufferGetFromFront(rxBuffer);
        }

        // if we detected a start, look for end of packet
        if(startFlag)
        {
                for(i=1; i<(rxBuffer->datalength)-1; i++)
                {
                        // check for end of NMEA packet <CR><LF>
                        if((bufferGetAtIndex(rxBuffer,i) == '\r') && (bufferGetAtIndex(rxBuffer,i+1) == '\n'))
                        {
                                // have a packet end
                                // dump initial '$'
                                bufferGetFromFront(rxBuffer);
                                // copy packet to NmeaPacket
                                for(j=0; j<(i-1); j++)
                                {
                                        // although NMEA strings should be 80 characters or less,
                                        // receive buffer errors can generate erroneous packets.
                                        // Protect against packet buffer overflow
                                        if(j<(NMEA_BUFFERSIZE-1))
                                                NmeaPacket[j] = bufferGetFromFront(rxBuffer);
                                        else
                                                bufferGetFromFront(rxBuffer);
                                }
                                // null terminate it
                                if (j<(NMEA_BUFFERSIZE-1)) {
                                        NmeaPacket[j] = 0;
                                } else {
                                        NmeaPacket[NMEA_BUFFERSIZE-1] = 0;
                                }
                                // dump <CR><LF> from rxBuffer
                                bufferGetFromFront(rxBuffer);
                                bufferGetFromFront(rxBuffer);
                                //DEBUG
                                #ifdef NMEA_DEBUG_PKT
                                //PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",NmeaPacket);
                                #endif
                                // found a packet
                                // done with this processing session
                                foundpacket = NMEA_UNKNOWN;
                                break;
                        }
                }
        }

        if(foundpacket)
        {
                // check message type and process appropriately
                if(!strncmp(NmeaPacket, "GPGGA", 5))
                {
                        // process packet of this type
                        nmeaProcessGPGGA(NmeaPacket);
                        // report packet type
                        foundpacket = NMEA_GPGGA;
                }
                else if(!strncmp(NmeaPacket, "GPVTG", 5))
                {
                        // process packet of this type
                        nmeaProcessGPVTG(NmeaPacket);
                        // report packet type
                        foundpacket = NMEA_GPVTG;
                }
                else if(!strncmp(NmeaPacket, "GPGSA", 5))
                {
                        // process packet of this type
                        nmeaProcessGPGSA(NmeaPacket);
                        // report packet type
                        foundpacket = NMEA_GPGSA;
                }
                else if(!strncmp(NmeaPacket, "GPRMC", 5))
                {
                        // process packet of this type
                        nmeaProcessGPRMC(NmeaPacket);
                        // report packet type
                        foundpacket = NMEA_GPRMC;
                }
        }
        else if(rxBuffer->datalength >= rxBuffer->size)
        {
                // if we found no packet, and the buffer is full
                // we're logjammed, flush entire buffer
                bufferFlush(rxBuffer);
        }
        return foundpacket;
}

/**
 * Prosesses NMEA GPGGA sentences
 * \param[in] Buffer for parsed nmea GPGGA sentence
 */
void GpsDisplayWidget::nmeaProcessGPGGA(char* packet)
{
        // start parsing just after "GPGGA,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }

        QString* nmeaString = new QString( packet );
        QStringList tokenslist = nmeaString->split(",");
        GpsData.Latitude = tokenslist.at(2).toFloat();
        GpsData.Longitude = tokenslist.at(4).toFloat();
        GpsData.Altitude = tokenslist.at(9).toFloat();
        GpsData.SV = tokenslist.at(7).toInt();

/*
        char *tokens;
    char *delimiter = ",";
    char *pEnd;
    char token_length=0;

    long deg,min,desim;

        #ifdef NMEA_DEBUG_GGA
                PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",packet);
        #endif

        // start parsing just after "GPGGA,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }

        // tokenizer for nmea sentence
        //GPGGA header
        tokens = strsep(&packet, delimiter);

        // get UTC time [hhmmss.sss]
        tokens = strsep(&packet, delimiter);
        //strcpy(GpsInfo.TimeOfFix,tokens);

        // next field: latitude
    // get latitude [ddmm.mmmmm]
        tokens = strsep(&packet, delimiter);
        token_length=strlen(tokens);
        deg=strtol (tokens,&pEnd,10);
        min=deg%100;
        deg=deg/100;
        desim=strtol (pEnd+1,NULL,10);
        if(token_length==10)// 5 desimal output
        {
                GpsData.Latitude=deg+(min+desim/100000.0)/60.0; // to desimal degrees
        }
        else if(token_length==9) // 4 desimal output
        {
                GpsData.Latitude=deg+(min+desim/10000.0)/60.0; // to desimal degrees
        }
        else if(token_length==11) // 6 desimal output OPGPS
        {
                GpsData.Latitude=deg+(min+desim/1000000.0)/60.0; // to desimal degrees
        }

    // next field: N/S indicator
        // correct latitute for N/S
        tokens = strsep(&packet, delimiter);
        if(tokens[0] == 'S') GpsData.Latitude = -GpsData.Latitude;

        // next field: longitude
        // get longitude [dddmm.mmmmm]
        tokens = strsep(&packet, delimiter);
        token_length=strlen(tokens);
        deg=strtol (tokens,&pEnd,10);
        min=deg%100;
        deg=deg/100;
        desim=strtol (pEnd+1,NULL,10);
        if(token_length==11)// 5 desimal output
        {
                GpsData.Longitude=deg+(min+desim/100000.0)/60.0; // to desimal degrees
        }
        else if(token_length==10) // 4 desimal output
        {
                GpsData.Longitude=deg+(min+desim/10000.0)/60.0; // to desimal degrees
        }
        else if(token_length==12) // 6 desimal output OPGPS
        {
                GpsData.Longitude=deg+(min+desim/1000000.0)/60.0; // to desimal degrees
        }
    // next field: E/W indicator
        // correct latitute for E/W
        tokens = strsep(&packet, delimiter);
        if(tokens[0] == 'W') GpsData.Longitude = -GpsData.Longitude;

    // next field: position fix status
        // position fix status
        // 0 = Invalid, 1 = Valid SPS, 2 = Valid DGPS, 3 = Valid PPS
        // check for good position fix
        tokens = strsep(&packet, delimiter);
    //if((tokens[0] != '0') || (tokens[0] != 0))
    //	GpsData.Updates++;

    // next field: satellites used
    // get number of satellites used in GPS solution
    tokens = strsep(&packet, delimiter);
        GpsData.Satellites=atoi(tokens);

        // next field: HDOP (horizontal dilution of precision)
        tokens = strsep(&packet, delimiter);

        // next field: altitude
        // get altitude (in meters mm.m)
        tokens = strsep(&packet, delimiter);
        //reuse variables for alt
        deg=strtol (tokens,&pEnd,10); // always 0.1m resolution?
        desim=strtol (pEnd+1,NULL,10);
        if(1) // OPGPS 3 desimal
                GpsData.Altitude=deg+desim/1000.0;
        else
                GpsData.Altitude=deg+desim/10.0;

        // next field: altitude units, always 'M'
        // next field: geoid seperation
        // next field: seperation units
        // next field: DGPS age
        // next field: DGPS station ID
        // next field: checksum
        */
}

/**
 * Prosesses NMEA GPRMC sentences
 * \param[in] Buffer for parsed nmea GPRMC sentence
 */
void GpsDisplayWidget::nmeaProcessGPRMC(char* packet)
{
        // start parsing just after "GPRMC,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }

        QString* nmeaString = new QString( packet );
        QStringList tokenslist = nmeaString->split(",");
        GpsData.Groundspeed = tokenslist.at(7).toFloat();//(deg+(desim/100.0))*0.51444; //OPGPS style to m/s
        GpsData.Groundspeed = GpsData.Groundspeed*0.51444*3.6;

/*
        char *tokens;
    char *delimiter = ",";
    char *pEnd;
    char token_length=0;

    long deg,min,desim;

        #ifdef NMEA_DEBUG_RMC
                PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",packet);
        #endif

        // start parsing just after "GPRMC,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }

        // tokenizer for nmea sentence
        //GPRMC header
        tokens = strsep(&packet, delimiter);

        // get UTC time [hhmmss.sss]
        tokens = strsep(&packet, delimiter);
        //strcpy(GpsInfo.TimeOfFix,tokens);
        // next field: Navigation receiver warning A = OK, V = warning
        tokens = strsep(&packet, delimiter);

        // next field: latitude
    // get latitude [ddmm.mmmmm]
        tokens = strsep(&packet, delimiter);
        token_length=strlen(tokens);
        deg=strtol (tokens,&pEnd,10);
        min=deg%100;
        deg=deg/100;
        desim=strtol (pEnd+1,NULL,10);
        /*if(token_length==10)// 5 desimal output
        {
                GpsData.Latitude=deg+(min+desim/100000.0)/60.0; // to desimal degrees
        }
        else if(token_length==9) // 4 desimal output
        {
                GpsData.Latitude=deg+(min+desim/10000.0)/60.0; // to desimal degrees
        }
        else if(token_length==11) // 6 desimal output OPGPS
        {
                GpsData.Latitude=deg+(min+desim/1000000.0)/60.0; // to desimal degrees
        }*

    // next field: N/S indicator
        // correct latitute for N/S
        tokens = strsep(&packet, delimiter);
        //if(tokens[0] == 'S') GpsData.Latitude = -GpsData.Latitude;

        // next field: longitude
        // get longitude [dddmm.mmmmm]
        tokens = strsep(&packet, delimiter);
        token_length=strlen(tokens);
        deg=strtol (tokens,&pEnd,10);
        min=deg%100;
        deg=deg/100;
        desim=strtol (pEnd+1,NULL,10);
        /*if(token_length==11)// 5 desimal output
        {
                GpsData.Longitude=deg+(min+desim/100000.0)/60.0; // to desimal degrees
        }
        else if(token_length==10) // 4 desimal output
        {
                GpsData.Longitude=deg+(min+desim/10000.0)/60.0; // to desimal degrees
        }
        else if(token_length==12) // 6 desimal output OPGPS
        {
                GpsData.Longitude=deg+(min+desim/1000000.0)/60.0; // to desimal degrees
        }*
    // next field: E/W indicator
        // correct latitute for E/W
        tokens = strsep(&packet, delimiter);
        //if(tokens[0] == 'W') GpsData.Longitude = -GpsData.Longitude;

        // next field: speed (knots)
        // get speed in knots
        tokens = strsep(&packet, delimiter);
        deg=strtol (tokens,&pEnd,10);
        desim=strtol (pEnd+1,NULL,10);
        GpsData.Groundspeed = (deg+(desim/100.0))*0.51444; //OPGPS style to m/s

        // next field: True course
        // get True course
        tokens = strsep(&packet, delimiter);
        deg=strtol (tokens,&pEnd,10);
        desim=strtol (pEnd+1,NULL,10);
        GpsData.Heading = deg+(desim/100.0); //OPGPS style

        // next field: Date of fix
        // get Date of fix
        tokens = strsep(&packet, delimiter);

        // next field: Magnetic variation
        // next field: E or W
        // next field: checksum
        */
}


/**
 * Prosesses NMEA GPVTG sentences
 * \param[in] Buffer for parsed nmea GPVTG sentence
 */
void GpsDisplayWidget::nmeaProcessGPVTG(char* packet)
{
        // start parsing just after "GPVTG,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }
/*
        char *tokens;
    char *delimiter = ",";

        #ifdef NMEA_DEBUG_VTG
                PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",packet);
        #endif

        // start parsing just after "GPVTG,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }
        // tokenizer for nmea sentence

        //GPVTG header
        tokens = strsep(&packet, delimiter);

        // get course (true north ref) in degrees [ddd.dd]
        tokens = strsep(&packet, delimiter);
    // next field: 'T'
        tokens = strsep(&packet, delimiter);

    // next field: course (magnetic north)
        // get course (magnetic north ref) in degrees [ddd.dd]
        tokens = strsep(&packet, delimiter);
        // next field: 'M'
        tokens = strsep(&packet, delimiter);

        // next field: speed (knots)
        // get speed in knots
        tokens = strsep(&packet, delimiter);
        // next field: 'N'
        tokens = strsep(&packet, delimiter);

        // next field: speed (km/h)
        // get speed in km/h
        tokens = strsep(&packet, delimiter);
        // next field: 'K'
        // next field: checksum
*/
}

/**
 * Prosesses NMEA GPGSA sentences
 * \param[in] Buffer for parsed nmea GPGSA sentence
 */
void GpsDisplayWidget::nmeaProcessGPGSA(char* packet)
{
        // start parsing just after "GPGSA,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }
/*
        char *tokens;
    char *delimiter = ",";
    char *pEnd;
    long value,desim;
    int mode;

        #ifdef NMEA_DEBUG_GSA
                PIOS_COM_SendFormattedStringNonBlocking(COM_DEBUG_USART,"$%s\r\n",packet);
        #endif

        // start parsing just after "GPGSA,"
        // attempt to reject empty packets right away
        if(packet[6]==',' && packet[7]==',')
                return;

        if(!nmeaChecksum(packet))
        {
                // checksum not valid
                return;
        }
        // tokenizer for nmea sentence

        //GPGSA header
        tokens = strsep(&packet, delimiter);

    // next field: Mode
        // Mode: M=Manual, forced to operate in 2D or 3D, A=Automatic, 3D/2D
        tokens = strsep(&packet, delimiter);
    // next field: Mode
    // Mode: 1=Fix not available, 2=2D, 3=3D
        tokens = strsep(&packet, delimiter);
        mode = atoi(tokens);
        if (mode == 1)
        {
                GpsData.Status = POSITIONACTUAL_STATUS_NOFIX;
        }
        else if (mode == 2)
        {
                GpsData.Status = POSITIONACTUAL_STATUS_FIX2D;
        }
        else if (mode == 3)
        {
                GpsData.Status = POSITIONACTUAL_STATUS_FIX3D;
        }

    // next field: 3-14 IDs of SVs used in position fix (null for unused fields)
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);
        tokens = strsep(&packet, delimiter);

        // next field: PDOP
        tokens = strsep(&packet, delimiter);
        value=strtol (tokens,&pEnd,10);
        desim=strtol (pEnd+1,NULL,10);
        GpsData.PDOP=value+desim/100.0;
        // next field: HDOP
        tokens = strsep(&packet, delimiter);
        value=strtol (tokens,&pEnd,10);
        desim=strtol (pEnd+1,NULL,10);
        GpsData.HDOP=value+desim/100.0;
        // next field: VDOP
        tokens = strsep(&packet, delimiter);
        value=strtol (tokens,&pEnd,10);
        desim=strtol (pEnd+1,NULL,10);
        GpsData.VDOP=value+desim/100.0;
        // next field: checksum
*/
}



/**
 * Called each time there are data in the input buffer
 */
void MyThread::processInputStream()
{
    GpsDisplayWidget *widget;
    //quint8 tmp;
        char c=0;
        port->read(&c,1);
        if( !bufferAddToEnd(&gpsRxBuffer, c) )
        {
                // no space in buffer
                // count overflow
                gpsRxOverflow++;
                return;
        }
        widget->nmeaProcess(&gpsRxBuffer);
}

void MyThread::setPort(QextSerialPort* port)
{

    this->port=port;
}

void MyThread::run()
{
    qDebug() <<  "Opening.";

    qDebug() <<  port->portName();

    bool isOpen =  port->open(QIODevice::ReadWrite);
    qDebug() <<  "Open: " << isOpen;

    char buf[1024];
    while(true) {
        qDebug() <<  "Reading.";
        /*qint64 bytesRead = port->readLine(buf, sizeof(buf));
        qDebug() << "bytesRead: " << bytesRead;
        if (bytesRead != -1) {
            qDebug() <<  "Result: '" << buf << "'";
        }*/
        while(port->bytesAvailable()>0)
        {
                processInputStream();
        }
        sleep(1);
    }
}


