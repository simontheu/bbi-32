//#include <iostream>
//using namespace std;

/* Linux */
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <getopt.h>

#include <time.h>

//Defines
#define VID_LBE		0x1dd2

#define PID_BBI_32	0x1150

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 */
#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif

#define HIDIOCGRAWNAME(len)     _IOC(_IOC_READ, 'H', 0x04, len)


int processCommandLineArguments(int argc, char **argv, int *enc, int *list, int *enable, int *type);

int main(int argc, char **argv)
{
      printf("Leo Bodnar BBI-32 config\n");
      
      int fd;
      int i, j, res, desc_size = 0;
      u_int8_t buf[60];
      uint32_t current_f;

      struct hidraw_devinfo info;

//      GPSSettings *currentSettings = new GPSSettings;

   /* Open the Device with non-blocking reads. In real life,
      don't use a hard coded path; use libudev instead. 
   */
      if (argc == 1)
      {
	    printf("Usage: bbi-32 /dev/hidraw??\n\n");
            printf("        --list:  list current setup\n               the frequency is saved in flash\n\n");
            return -1;
      }

      printf("Opening device %s\n", argv[1]);

      fd = open(argv[1], O_RDWR|O_NONBLOCK);

      if (fd < 0) 
      {
            perror("    Unable to open device");
            return 1;
      }

      //Device connected, setup report structs
      memset(&info, 0x0, sizeof(info));

      // Get Raw Info
      res = ioctl(fd, HIDIOCGRAWINFO, &info);
      
      if (res < 0) 
      {
            perror("HIDIOCGRAWINFO");
      } 
      else
      {
            if (info.vendor != VID_LBE || (info.product != PID_BBI_32)) {
                printf("    Not a valid BBI-32 Device\n\n");
                  printf("    Device Info:\n");
                  printf("        vendor: 0x%04hx\n", info.vendor);
                  printf("        product: 0x%04hx\n", info.product);
                  return -1;//Device not valid
            }
      }

      /* Get Raw Name */
      res = ioctl(fd, HIDIOCGRAWNAME(256), buf);

      if (res < 0) {
            perror("HIDIOCGRAWNAME");
      }
      else {
            printf("Connected To: %s\n\n", buf);
      }

      /* Get Feature */
      buf[0] = 0x9; /* Report Number */
      res = ioctl(fd, HIDIOCGFEATURE(256), buf);

      if (res < 0) {
            perror("HIDIOCGFEATURE");
      } else {
	      printf("  Status:\n");
            //currentSettings->setParamsFromReadBuffer(buf,res);
            if (buf[0] == 0) {
		    if ((buf[1] & 0x15) == 0x15) {
		    	printf("    Device OK");
		    } else {
		    	/*if ((buf[1] & GPS_LOCK_BIT) != GPS_LOCK_BIT) {
		    		printf("\n    No GPS lock\n");	
		    	}
		    	if ((buf[1] & ANT_OK_BIT) != ANT_OK_BIT) {
		    		printf("\n    GPS antenna short circuit\n");	
		    	}
		    	if ((buf[1] & OUT1_EN_BIT) != OUT1_EN_BIT) {
		    		printf("\n    GPS antenna short circuit\n");	
		    	}*/
		    }
		    current_f = (buf[5] << 24) + (buf[4] << 16) + (buf[3] << 8) + buf[2];
	            printf("\n    Current Frequency: %i\n", current_f);
            }

            printf("\n");
      }

      /* Get Raw Name */
      res = ioctl(fd, HIDIOCGRAWNAME(256), buf);

      if (res < 0) {
            perror("HIDIOCGRAWNAME");
      }
      else {

	//Get CLI values as vars
	int list = -1;
	int enable = -1;
	int type = -1;
	int enc = -1;
	
	int feature_reports[2] = {0x20, 0x40};
	int feature_report_count = 2;
	
	int encoder_settings[32];
	//Clear out contents to invalid setting;
	for (i=0; i<32; i++) {
	    encoder_settings[i] = -1;
	}
	
	processCommandLineArguments(argc, argv, &enc, &list, &enable, &type);
      	printf("  Changes:\n");
      	int changed = 0;
      	char type_string[4];
	if (enc != -1 && type != -1) {
	    //Set Frequency
	    switch (type) {
	    	case 0:
	    	    sprintf(type_string, "Off");
	    	    break;
	    	case 2:
	    	    sprintf(type_string, "1:1");
	    	    break;
	    	case 4:
	    	    sprintf(type_string, "1:2");
	    	    break;
	    	case 8:
	    	    sprintf(type_string, "1:4");
	    	    break;
	    	case 16:
	    	    sprintf(type_string, "Hat");
	    	    break;
	    }
	    printf ("    Setting Encoder: %i&%i  to: %s\n", enc, enc+1,type_string);
	    
	    buf[0] = 0;
	    buf[1] = 0xee;//Write to eeprom
	    buf[2] = (0x20 - 1) + enc;
	    if ((buf[2] & 0x01) != 0) {
	        perror("Invalid encoder type value");
	    }
	    buf[3] = 
	    /* Set Feature */
            res = ioctl(fd, HIDIOCSFEATURE(60), buf);
            if (res < 0) perror("HIDIOCSFEATURE");
            changed = 1;
	}
	if (enable != -1) {
	    buf[0] = 1;
	    buf[1] = enable & 0x01;
	    printf ("    Enable State :%i\n", enable);
	    /* Set Feature */
            res = ioctl(fd, HIDIOCSFEATURE(60), buf);
            if (res < 0) perror("HIDIOCSFEATURE");
            changed = 1;
	}
	if (list != -1) {
	    printf ("    Current Encoder Setup\n");
	    int all_received = -1;
	    //loop until all data is received
	    while (all_received == -1) {
                res = ioctl(fd, HIDIOCGFEATURE(256), buf);
                if (res) {
                    switch (buf[1]) {
                        case 0x20:
                            feature_reports[0] = 1;
                            for (j=0;j<32;j+=2) {
                                encoder_settings[j] = (buf[j+2] & 0x0f);
                                //printf ("j: %i : buf[j+2]: %i\n",j ,j+2);
                            }
                            break;
                        case 0x40:
                            feature_reports[1] = 1;
                            break;
                    }
                }
                //check to see if all reports have been received
                all_received = 1;
                for (i = 0; i < feature_report_count; i++) {
                    if (feature_reports[i] != 1) {
                        //printf("i: %i = %i",i,feature_reports[i]);
                    	all_received = -1;
                    }
                }
	    }
	    if (all_received) {
	    	for (i=0;i<32;i+=2) {
	    	   char type[4];
	    	   switch (encoder_settings[i]) {
	    	   	case 0:
	    	   	    sprintf(type,"Off");
	    	   	    break;
	    	   	case 2:
	    	   	    sprintf(type,"1:1");
	    	   	    break;
	    	   	case 4:
	    	   	    sprintf(type,"1:2");
	    	   	    break;
	    	   	case 8:
	    	   	    sprintf(type,"1:4");
	    	   	    break;
	    	   	case 16:
	    	   	    sprintf(type,"Hat");
	    	   	    break;
	    	   }
	    	   int encoder_setting[i];
	    		printf("\tInput %i&%i: \t%s\n", i+1,i+2, type);
	    	}
	    }
	    /* Set Feature */
            res = ioctl(fd, HIDIOCSFEATURE(60), buf);
            if (res < 0) perror("HIDIOCSFEATURE");
	}
	if (!changed) {
	    printf("    No changes made\n");
	}
      }
      close(fd);

      return 0;
}


int processCommandLineArguments(int argc, char **argv, int *enc, int *list, int *enable,int *type)
{
    int c;
    
    while (1)
    {
        static struct option long_options[] =
        {
                /* These options set a flag. */
                {"list", no_argument, 0, 0},//c = 0
                /* These options don’t set a flag.
                    We distinguish them by their indices. */
                {"enc",    required_argument, 0, 'a'},
                {"type",     required_argument, 0, 'b'},
                {"pulse",   required_argument, 0, 'c'},
                {"rot_pulse",   required_argument, 0, 'd'},
                {"rot_start",   required_argument, 0, 'e'},
                {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "abc:d:f:",
                    long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
        break;

        switch (c)
        {
            case 0://list
            	*list = 1;
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                        break;
                break;
            
            case 'a'://enc
                *enc = atoi(optarg);
                break;

            case 'b'://type
                *type = atoi(optarg);
                break;

            case 'c'://N2_HS
                *enable = atoi(optarg);
                break;
                
            case 'd'://N2_HS
                *enable = atoi(optarg);
                break;
                
            case 'e'://N2_HS
                *enable = atoi(optarg);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort ();
        }
    }
    return 0;
}

