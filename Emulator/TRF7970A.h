#define CMD                 				0b10000000
#define READ                				0b01000000
#define WRITE               				0b00000000
#define CONT                				0b00100000

#define CHIP_STATUS                         0x00 
#define ISO_CONTROL                         0x01 

//Protocol Subsetting Registers
#define ISO14443B_TX                        0x02 
#define ISO14443A_RATE					    0x03 
#define TX_TIMER_HIGH			            0x04 
#define TX_TIMER_LOW                        0x05 
#define TX_PULSE_LENGTH                     0x06 
#define RX_NO_RESPONSE_WAIT_TIME            0x07 
#define RX_WAIT_TIME			            0x08 
#define MODULATOR_AND_SYS_CLK_CONTROL       0x09 
#define RX_SPECIAL		                    0x0A 
#define REG_IO_CONTROL                      0x0B 
#define FIFO_IRQ_LEVELS          			0x14 
#define NFC_LOW_FIELD_LEVEL        			0x16 
#define NFCID1_NUMBER              			0x17 
#define NFC_TARGET_DETECTION_LEVEL 			0x18 
#define NFC_TARGET_PROTOCOL        			0x19 

//Status Registers
#define IRQ_STATUS 							0x0C
#define IRQ_MASK                            0x0D
#define COLLISION                           0x0E
#define RSSI                                0x0F

//FIFO Registers
#define FIFO_STATUS                         0x1C
#define TX_LENGTH_1                         0x1D
#define TX_LENGTH_2                         0x1E
#define FIFO_IO                             0x1F

//Commands
#define IDLE                                0x00
#define RESET                               0x03
#define RF_COLLISION                        0x04
#define RESET_FIFO                          0x0F
#define TX_NO_CRC                           0x10
#define TX_CRC                              0x11
#define TX_DELAY_NO_CRC                     0x12
#define TX_DELAY_CRC 	                    0x13
#define BLOCK_RECEIVER                      0x16
#define ENABLE_RECEIVER                     0x17
#define TEST_INTERNAL_RF                    0x18
#define TEST_EXTERNAL_RF                    0x19

//Chip_status fields
#define stby								0b10000000
#define direct								0b01000000
#define rf_on								0b00100000
#define rf_pwr								0b00010000		
#define pm_on								0b00001000
#define rec_on								0b00000010	
#define vrs5_3								0b00000001

//ISO_Control fields
#define rx_crc_n							0b10000000
#define dir_mode							0b01000000
#define rfid								0b00100000
#define iso_4								0b00010000		
#define iso_3								0b00001000
#define iso_2								0b00000100
#define iso_1								0b00000010	
#define iso_0								0b00000001

//ISO14443B_TX fields
#define egt2								0b10000000
#define egt1								0b01000000
#define egt0								0b00100000
#define eof_l0								0b00010000		
#define sof_l1								0b00001000
#define sof_l0								0b00000100
#define l_egt								0b00000010	
#define SDD_SAK								0b00000001
