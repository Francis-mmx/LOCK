#ifndef IOCTL_INF_H
#define IOCTL_INF_H




#define IOCTL_SET_IRQ_NUM 				1
#define IOCTL_SET_PRIORITY 				2
#define IOCTL_SET_DATA_WIDTH 			3
#define IOCTL_SET_SPEED 				4
#define IOCTL_SET_DETECT_MODE 			5
#define IOCTL_SET_DETECT_FUNC 			6
#define IOCTL_SET_DETECT_TIME_INTERVAL  7
#define IOCTL_SET_PORT 					8
#define IOCTL_SET_PORT_FUNC 			9
#define IOCTL_SET_CS_PORT_FUNC 		 	10
#define IOCTL_SET_READ_MODE 			11
#define IOCTL_SET_WRITE_MODE 		    12
#define IOCTL_SET_WRITE_PROTECT 	    13
#define IOCTL_SET_START_BIT 			14
#define IOCTL_SET_STOP_BIT 				15
#define IOCTL_FLUSH 					16
#define IOCTL_REGISTER_IRQ_HANDLER      17
#define IOCTL_UNREGISTER_IRQ_HANDLER    18
#define IOCTL_GET_SYS_TIME              19
#define IOCTL_SET_SYS_TIME              20
#define IOCTL_GET_ALARM                 21
#define IOCTL_SET_ALARM                 22
#define IOCTL_SET_CAP_LOWSPEED_CARD	    23
#define IOCTL_SET_VDD50_EN              30
#define IOCTL_GET_WEEKDAY             	32
#define IOCTL_CLR_READ_MODE             33
#define IOCTL_SET_READ_CRC              34
#define IOCTL_GET_READ_CRC              35
#define IOCTL_GET_VOLUME                36
#define IOCTL_SET_VOLUME                37

#define IOCTL_GET_ID 					100
#define IOCTL_GET_SECTOR_SIZE			101
#define IOCTL_GET_BLOCK_SIZE			102
#define IOCTL_GET_CAPACITY 				103
#define IOCTL_GET_WIDTH 				104
#define IOCTL_GET_HEIGHT				105
#define IOCTL_GET_BLOCK_NUMBER          106
#define IOCTL_CHECK_WRITE_PROTECT       107
#define IOCTL_GET_STATUS                108
#define IOCTL_GET_TYPE                  109
#define IOCTL_GET_UNIQUE_ID             114
#define IOCTL_ERASE_OTP                 115
#define IOCTL_WRITE_OTP                 116
#define IOCTL_READ_OTP                  117
#define IOCTL_WRITE_READ        	    118

#define IOCTL_REFRESH_VAD				150

#define IOCTL_ERASE_SECTOR 				200
#define IOCTL_ERASE_BLOCK 				201
#define IOCTL_ERASE_CHIP 				202



struct ioctl_irq_handler {
    void *priv;
    void *handler;
};



#endif

