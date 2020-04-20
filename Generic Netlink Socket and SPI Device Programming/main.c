/* This is the user space program. It contains a lot of the code that was shared for the Generic Netlink. */




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include "IOinit.h"
#include "matrix_driver.h"


/* PATTERN 1 Standing and jumping man and default pattern
   PATTERN 2 Pacman open and close mouth   */


/* For the TA */  

#ifndef GRADING
#define MAX7219_CS_PIN 8
#define HCSR04_TRIGGER_PIN 0
#define HCSR04_ECHO_PIN 9
#define PATTERN 1
#endif


int num;
struct message_cmd mcmd;
static char message[GENL_TEST_ATTR_MSG_MAX];
static char message_struct[GENL_TEST_ATTR_MSG_MAX];
static int send_to_kernel;
static unsigned int mcgroups;		/* Mask of groups */



void gpio_unexport()
{ 

	int FDUnexport;
	FDUnexport = open("/sys/class/gpio/unexport", O_WRONLY);
	if (FDUnexport < 0)
	{
    		printf("\n gpio unexport open failed");
	}


//IO PIN 0
	write(FDUnexport, "11",2);
    		//printf("error FdExport 11");
		
	write(FDUnexport, "32",2);
    		//printf("error FdExport 32");
	


//IO PIN 1
	write(FDUnexport, "12",2);
	write(FDUnexport, "28",2);
	write(FDUnexport, "45",2);


//IO PIN 2
	write(FDUnexport, "13",2);
	write(FDUnexport, "34",2);
	write(FDUnexport, "77",2);
	write(FDUnexport, "61",2);

//IO PIN 3
	write(FDUnexport, "14",2);
	write(FDUnexport, "16",2);
	write(FDUnexport, "76",2);
	write(FDUnexport, "62",2);

// IO PIN4
	write(FDUnexport, "6",2);
	write(FDUnexport, "36",2);


// IO PIN 5
	write(FDUnexport, "0",2);
	write(FDUnexport, "18",2);
	write(FDUnexport, "66",2);

//IO PIN 6
	write(FDUnexport, "1",2);
	write(FDUnexport, "20",2);
	write(FDUnexport, "68",2);

//IO PIN 7
	write(FDUnexport, "38",2);

//IO PIN 8
	write(FDUnexport, "40",2);


//IO PIN 9
	write(FDUnexport, "4",2);
	write(FDUnexport, "22",2);
	write(FDUnexport, "70",2);


//IO PIN 10
	write(FDUnexport, "10",2);
	write(FDUnexport, "26",2);
	write(FDUnexport, "74",2);

//IO PIN 11
	write(FDUnexport, "5",2);
	write(FDUnexport, "24",2);
	write(FDUnexport, "44",2);


//IO PIN 12
	write(FDUnexport, "15",2);
	write(FDUnexport, "42",2);



//IO PIN 13
	write(FDUnexport, "7",2);
	write(FDUnexport, "30",2);
	write(FDUnexport, "46",2);


//PWM

//IO PIN 3
	write(FDUnexport, "16",2);   
	write(FDUnexport, "17",2);
	write(FDUnexport, "76",2);
	write(FDUnexport, "64",2);
    



//IO PIN 11
	write(FDUnexport, "24",2);  
	write(FDUnexport, "25",2);
	write(FDUnexport, "44",2);
	write(FDUnexport, "72",2);



//IO PIN 9
	write(FDUnexport, "22",2);
	write(FDUnexport, "23",2);  
	write(FDUnexport, "70",2);   


	close(FDUnexport);

}






static void usage(char* name)
{
	printf("Usage: %s\n"
		"	-h : this help message\n"
		"	-l : listen on one or more groups, comma separated\n"
		"	-m : the message to send, required with -s\n"
		"	-s : send to kernel\n"
		"NOTE: specify either l or s, not both\n",
		name);
}

static void add_group(char* group)
{
	unsigned int grp = strtoul(group, NULL, 10);

	if (grp > GENL_TEST_MCGRP_MAX-1) {
		fprintf(stderr, "Invalid group number %u. Values allowed 0:%u\n",
			grp, GENL_TEST_MCGRP_MAX-1);
		exit(EXIT_FAILURE);
	}

	mcgroups |= 1 << (grp);
}

static void parse_cmd_line(int argc, char** argv)
{
	char* opt_val;


	while (1) {
		int opt = getopt(argc, argv, "hl:m:s");

		if (opt == EOF)
			break;

		switch (opt) {
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);

		case 'l':
			opt_val = strtok(optarg, ",");
			add_group(opt_val); 
			while ((opt_val = strtok(NULL, ",")))
				add_group(opt_val); 

			break;

		case 'm':
			optarg = "Hello there it is me";
			strncpy(message, optarg, GENL_TEST_ATTR_MSG_MAX);
			message[GENL_TEST_ATTR_MSG_MAX - 1] = '\0';
			strncpy(message_struct, (char *)&mcmd, GENL_TEST_ATTR_MSG_MAX);
			message_struct[GENL_TEST_ATTR_MSG_MAX - 1] = '\0';


			break;

		case 's':
			send_to_kernel = 1;
			break;

		default:
			fprintf(stderr, "Unkown option %c !!\n", opt);
			exit(EXIT_FAILURE);
		}

	}

	/* sanity checks */
	if (send_to_kernel && mcgroups)  {
		fprintf(stderr, "I can either receive or send messages.\n\n");
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}	
	if (!send_to_kernel && !mcgroups)  {
		fprintf(stderr, "Nothing to do!\n");
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}
	if (send_to_kernel && message_struct[0]=='\0') {
		fprintf(stderr, "What is the message you want to send?\n");
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
}
static int send_msg_to_kernel(struct nl_sock *sock)
{
	struct nl_msg* msg;
	int family_id, err = 0;

	family_id = genl_ctrl_resolve(sock, GENL_TEST_FAMILY_NAME);
	if(family_id < 0){
		fprintf(stderr, "Unable to resolve family name!\n");
		exit(EXIT_FAILURE);
	}

	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		exit(EXIT_FAILURE);
	}



	if(!genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family_id, 0, 
		NLM_F_REQUEST, GENL_TEST_C_MSG, 0)) {
		fprintf(stderr, "failed to put nl hdr!\n");
		err = -ENOMEM;
		goto out;
	}

	err = nla_put(msg, GENL_TEST_ATTR_MSG, sizeof(struct message_cmd), (struct message_cmd*)&mcmd);

	//err = nla_put_string(msg, GENL_TEST_ATTR_MSG, message_struct);
	if (err) {
		fprintf(stderr, "failed to put nl string!\n");
		goto out;
	}

	err = nl_send_auto(sock, msg);
	if (err < 0) {
		fprintf(stderr, "failed to send nl message!\n");
	}



//printf("Error number after sending auto is: %d\n", err);

out:
	nlmsg_free(msg);
	return err;
}

static int skip_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}



static int print_rx_msg(struct nl_msg *msg, void* arg)
{
	struct nlattr *attr[GENL_TEST_ATTR_MAX+1];
	struct message_cmd *temp= NULL;

	genlmsg_parse(nlmsg_hdr(msg), 0, attr, 
			GENL_TEST_ATTR_MAX, genl_test_policy);

	if (!attr[GENL_TEST_ATTR_MSG]) {
		fprintf(stdout, "Kernel sent empty message!!\n");
		return NL_OK;
	}


	 //fprintf(stdout, "Kernel says: %llu \n", 
	 	//(uint64_t)nla_get_string(attr[GENL_TEST_ATTR_MSG]));
	temp = (struct message_cmd *)(nla_data(attr[GENL_TEST_ATTR_MSG]));
	nla_memcpy(temp, attr[GENL_TEST_ATTR_MSG], sizeof(struct message_cmd));
	//temp = (struct message_cmd *)nla_get_string(attr[GENL_TEST_ATTR_MSG]);

	fprintf(stdout, "The average distance is: %llucm \n", 
		temp->distance);	



	return NL_OK;
}

static void prep_nl_sock(struct nl_sock** nlsock)
{
	int family_id, grp_id;
	unsigned int bit = 0;
	
	*nlsock = nl_socket_alloc();
	if(!*nlsock) {
		fprintf(stderr, "Unable to alloc nl socket!\n");
		exit(EXIT_FAILURE);
	}

	/* disable seq checks on multicast sockets */
	nl_socket_disable_seq_check(*nlsock);
	nl_socket_disable_auto_ack(*nlsock);

	/* connect to genl */
	if (genl_connect(*nlsock)) {
		fprintf(stderr, "Unable to connect to genl!\n");
		goto exit_err;
	}

	/* resolve the generic nl family id*/
	family_id = genl_ctrl_resolve(*nlsock, GENL_TEST_FAMILY_NAME);
	if(family_id < 0){
		fprintf(stderr, "Unable to resolve family name!\n");
		goto exit_err;
	}

	if (!mcgroups)
		return;

	while (bit < sizeof(unsigned int)) {
		if (!(mcgroups & (1 << bit)))
			goto next;

		grp_id = genl_ctrl_resolve_grp(*nlsock, GENL_TEST_FAMILY_NAME,
				genl_test_mcgrp_names[bit]);

		if (grp_id < 0)	{
			fprintf(stderr, "Unable to resolve group name for %u!\n",
				(1 << bit));
            goto exit_err;
		}
		if (nl_socket_add_membership(*nlsock, grp_id)) {
			fprintf(stderr, "Unable to join group %u!\n", 
				(1 << bit));
            goto exit_err;
		}
next:
		bit++;
	}

    return;

exit_err:
    nl_socket_free(*nlsock); // this call closes the socket as well
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
	struct nl_sock* nlsock = NULL;
	struct nl_cb *cb = NULL;
	int ret;

	gpio_unexport();
	IOSetup();
	level_shifter();

	mcmd.trigger_pin = HCSR04_TRIGGER_PIN;
	mcmd.echo_pin = HCSR04_ECHO_PIN; 
	mcmd.chip_pin = MAX7219_CS_PIN; 
	mcmd.pattern = PATTERN;

	//printf("argv[1], argv[2] argv[3]  %s %s %s \n", argv[1], argv[2], argv[3]);
	if((strcmp(argv[1],"-s")==0)){

		if((strcmp(argv[3],"start")==0))
		mcmd.enable = 1;

   		else if((strcmp(argv[3],"stop")==0))
		mcmd.enable = 0;

		else{
			printf("Please enter a valid option!\n");
			return 0;
		}

}


	parse_cmd_line(argc, argv);

	prep_nl_sock(&nlsock);

	if (send_to_kernel) {
		ret = send_msg_to_kernel(nlsock);
		exit(EXIT_SUCCESS);
	}

	//printf("Message is sent\n");
	/* prep the cb */
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, skip_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_rx_msg, NULL);
	do {
		ret = nl_recvmsgs(nlsock, cb);
	} while (!ret);
	
	nl_cb_put(cb);
    nl_socket_free(nlsock);
	return 0;
}
