#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <stdint.h>

#include "IOinit.h"

//exporting and setting all the required pins

void IOSetup()
{
	int FdExport, Fd15, Fd44, Fd46, Fd13, Fd14, Fd34, Fd16, Fv44, Fv46, Fv72, Fv77, Fv76, Fv34, Fv16, Fv13, Fv14, Fv64; 

	FdExport = open("/sys/class/gpio/export", O_WRONLY);

	if (FdExport < 0)
	{
		printf("gpio export open failed \n");
	}
	
	if(0 > write(FdExport,"44",2))
		printf("error FdExport 44 \n");
	if(0 > write(FdExport,"72",2))
		printf("error FdExport 72 \n");
	if(0 > write(FdExport,"46",2))
		printf("error FdExport 46 \n");
	if(0 > write(FdExport,"15",2))
		printf("error FdExport 15 \n");
	if(0> write(FdExport,"77",2))
		printf("error Fdexport 77 \n");
	if(0> write(FdExport,"76",2))
		printf("error Fdexport 76 \n");
	if(0> write(FdExport,"13",2))
		printf("error Fdexport 13 \n");
	if(0> write(FdExport,"14",2))
		printf("error Fdexport 14 \n");
	if(0> write(FdExport,"34",2))
		printf("error Fdexport 34 \n");
	if(0> write(FdExport,"16",2))
		printf("error Fdexport 16 \n");
	if(0> write(FdExport,"64",2))
		printf("error Fdexport 64 \n");

	close(FdExport);

	Fd44 = open("/sys/class/gpio/gpio44/direction", O_WRONLY);
	if (Fd44 < 0)
	{
		printf("GPIO44 direction open failed. \n");
	}
	if(0 > write(Fd44,"out",3))
		printf("error Fd44 \n");
	close(Fd44);

	Fd46 = open("/sys/class/gpio/gpio46/direction", O_WRONLY);
	if (Fd46 < 0)
	{
		printf("GPIO46 direction open failed. \n");
	}
	if(0 > write(Fd46,"out",3))
		printf("error Fd46 \n");
	close(Fd46);

	Fd15 = open("/sys/class/gpio/gpio15/direction", O_WRONLY);
	if (Fd15 < 0)

	{
		printf("GPIO15 direction open failed. \n");
	}
	if(0 > write(Fd15,"out",3))
		printf("error Fd15 \n");
	close(Fd15);

	Fd34 = open("/sys/class/gpio/gpio34/direction", O_WRONLY);
	if (Fd34 < 0)
	{
		printf("\n gpio34 direction file could not be opened");
	}
	if(0> write(Fd34,"out",3))
		printf(" \ndirection writing error - fd34");
	close(Fd34);

	Fd16 = open("/sys/class/gpio/gpio16/direction", O_WRONLY);
	if (Fd16 < 0)
	{
		printf("\n gpio16 direction file could not be opened");
	}
	if(0> write(Fd16,"out",3))
		printf(" \ndirection writing error - fd16");
	close(Fd16);	

	Fd13 = open("/sys/class/gpio/gpio13/direction", O_WRONLY);
	if (Fd13 < 0)
	{
		printf("\n gpio13 direction file could not be opened");
	}
	if(0> write(Fd13,"out",3))
		printf(" \ndirection writing error - fd13");
	close(Fd13);

	Fd14 = open("/sys/class/gpio/gpio14/direction", O_WRONLY);
	if (Fd14 < 0)
	{
		printf("\n gpio14 direction file could not be opened");
	}
	if(0> write(Fd14,"out",3))
		printf(" \ndirection writing error - fd14");	
	close(Fd14);	

	Fv44 = open("/sys/class/gpio/gpio44/value", O_WRONLY);
	if(0 > write(Fv44,"1",1)){
		printf("Error Fv44");
	}
	Fv72 = open("/sys/class/gpio/gpio72/value", O_WRONLY);
	if(0 > write(Fv72,"0",1)){
		printf("Error Fv72");
	}
	Fv46 = open("/sys/class/gpio/gpio46/value", O_WRONLY);
	if(0 > write(Fv46,"1",1)){
		printf("Error Fv46");
	}
	Fv77 = open("/sys/class/gpio/gpio77/value", O_WRONLY);
	if(0 > write(Fv77,"0",1)){
		printf("Error Fv77");
	}
	Fv76 = open("/sys/class/gpio/gpio76/value", O_WRONLY);
	if(0 > write(Fv76,"0",1)){
		printf("Error Fv76");
	}
	Fv34 = open("/sys/class/gpio/gpio34/value", O_WRONLY);
	if(0 > write(Fv34,"0",1)){
		printf("Error Fv34");
	}
	Fv16 = open("/sys/class/gpio/gpio16/value", O_WRONLY);
	if(0 > write(Fv16,"1",1)){
		printf("Error Fv16");
	}
	Fv13 = open("/sys/class/gpio/gpio13/value", O_WRONLY);
	if(0 > write(Fv13,"0",1)){
		printf("Error Fv13");
	}
	Fv14 = open("/sys/class/gpio/gpio14/value", O_WRONLY);
	if(0 > write(Fv14,"0",1)){
		printf("Error Fv14");
	}
	Fv64 = open("/sys/class/gpio/gpio64/value", O_WRONLY);
	if(0 > write(Fv64,"0",1)){
		printf("Error Fv64");
	}
}

//exporting all the required level shifters

void level_shifter(){

	int FdExport1, Fd32, Fd28, Fd36, Fd18, Fd20, Fd22, Fd26, Fd24, Fd42, Fd30;

	FdExport1 = open("/sys/class/gpio/export", O_WRONLY);
	if (FdExport1 < 0)
	{
		printf("gpio export open failed \n");
	}

	if(0 > write(FdExport1,"32",2))
		printf("error in FdExport for level shifter 32 \n");
	if(0 > write(FdExport1,"28",2))
		printf("error in FdExport for level shifter 28 \n");
	if(0 > write(FdExport1,"36",2))
		printf("error in FdExport for level shifter 36 \n");
	if(0 > write(FdExport1,"18",2))
		printf("error in FdExport for level shifter 18 \n");
	if(0 > write(FdExport1,"20",2))
		printf("error in FdExport for level shifter 20 \n");
	if(0 > write(FdExport1,"22",2))
		printf("error in FdExport for level shifter 22 \n");
	if(0 > write(FdExport1,"26",2))
		printf("error in FdExport for level shifter 26 \n");
	if(0 > write(FdExport1,"24",2))
		printf("error in FdExport for level shifter 24 \n");
	if(0 > write(FdExport1,"42",2))
		printf("error in FdExport for level shifter 42 \n");
	if(0 > write(FdExport1,"30",2))
		printf("error in FdExport for level shifter 30 \n");
	close(FdExport1);

	Fd32 = open("/sys/class/gpio/gpio32/direction", O_WRONLY);
	if (Fd32 < 0)
	{
		printf("\n gpio32 direction open failed  \n");
	}
	Fd28 = open("/sys/class/gpio/gpio28/direction", O_WRONLY);
	if (Fd28 < 0)
	{
		printf("\n gpio28 direction open failed \n");
	}
	Fd36 = open("/sys/class/gpio/gpio36/direction", O_WRONLY);
	if (Fd36 < 0)
	{
		printf("\n gpio36 direction open failed \n");
	}
	Fd18 = open("/sys/class/gpio/gpio18/direction", O_WRONLY);
	if (Fd18 < 0)
	{
		printf("\n gpio18 direction open failed \n");
	}
	Fd20 = open("/sys/class/gpio/gpio20/direction", O_WRONLY);
	if (Fd20 < 0)
	{
		printf("\n gpio20 direction open failed \n");
	}
	Fd22 = open("/sys/class/gpio/gpio22/direction", O_WRONLY);
	if (Fd22 < 0)
	{
		printf("\n gpio22 direction open failed \n");
	}
	Fd26 = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
	if (Fd26 < 0)
	{
		printf("\n gpio26 direction open failed \n");
	}
	Fd24 = open("/sys/class/gpio/gpio24/direction", O_WRONLY);
	if (Fd24 < 0)
	{
		printf("\n gpio24 direction open failed \n");
	}
	Fd42 = open("/sys/class/gpio/gpio42/direction", O_WRONLY);
	if (Fd42 < 0)
	{
		printf("\n gpio42 direction open failed \n");
	}
	Fd30 = open("/sys/class/gpio/gpio30/direction", O_WRONLY);
	if (Fd30 < 0)
	{
		printf("\n gpio30 direction open failed \n");
	}

	if(0 > write(Fd32,"out",3))
		printf("error Fd32 \n");
	if(0 > write(Fd28,"out",3))
		printf("error Fd28 \n");
	if(0 > write(Fd36,"out",3))
		printf("error Fd36 \n");
	if(0 > write(Fd18,"out",3))
		printf("error Fd18 \n");
	if(0 > write(Fd20,"out",3))
		printf("error Fd20 \n");
	if(0 > write(Fd22,"out",3))
		printf("error Fd22 \n");
	if(0 > write(Fd26,"out",3))
		printf("error Fd26 \n");
	if(0 > write(Fd24,"out",3))
		printf("error Fd24 \n");
	if(0 > write(Fd42,"out",3))
		printf("error Fd42 \n");
	if(0 > write(Fd30,"out",3))
		printf("error Fd30 \n");
}

