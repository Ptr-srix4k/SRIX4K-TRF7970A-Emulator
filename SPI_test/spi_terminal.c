#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "./ftd2xx.h"

int spi_tx (FT_HANDLE ftHandle0, unsigned char *tx_data, unsigned char *rx_data, unsigned char to_write)
{
    FT_STATUS	ftStatus;
    DWORD 	    dwBytesWritten;
    DWORD       bytesReceived = 0;
    unsigned char 	tmp[1];
    int k;
    
    //Write first the number of bytes you want to transmit
    tmp[0] = to_write;
    ftStatus = FT_Write(ftHandle0, tmp, 1, &dwBytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Error FT_Write(%d)\n", (int)ftStatus);
	    return 0;
	}
	if (dwBytesWritten != 1)
	{ 
		printf("FT_Write only wrote %d (of %d) bytes\n", (int)dwBytesWritten, to_write);
        return 0;
    }
    
    ftStatus = FT_Write(ftHandle0, tx_data, to_write, &dwBytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Error FT_Write(%d)\n", (int)ftStatus);
	    return 0;
	}
	if (dwBytesWritten != to_write)
	{ 
		printf("FT_Write only wrote %d (of %d) bytes\n", (int)dwBytesWritten, to_write);
        return 0;
    }
    
    while(bytesReceived < dwBytesWritten)
    {
        FT_GetQueueStatus(ftHandle0, &bytesReceived);
    }
    //Need another request otherwise the RX seems broken (don't know why...)
    FT_GetQueueStatus(ftHandle0, &bytesReceived);
    
    ftStatus = FT_Read(ftHandle0, rx_data, to_write, &dwBytesWritten);  
    if (ftStatus != FT_OK) 
	{
		printf("Error FT_Read(%d)\n", (int)ftStatus);
	    return 0;
	}
	if (dwBytesWritten != to_write)
	{ 
		printf("FT_Read only read %d (of %d) bytes\n", (int)dwBytesWritten, to_write);
        return 0;
    }  
    
    return 1;
}

int main(int argc, char *argv[])
{
	FT_STATUS	ftStatus;
	FT_HANDLE	ftHandle0;
	int iport;
	int i=0;
	static FT_DEVICE ftDevice;
	DWORD libraryVersion = 0;
	DWORD 	dwBytesWritten;
	DWORD 	dwBytesWrite;
	DWORD 	bytesReceived=0;
	int retCode = 0;
	unsigned char 	cBufWrite[10];
	unsigned char 	cBufRead[10];
	
	char hex_inp[256];
    char single[3];
    int num;
    unsigned char *ptr;
    int len_buf;

	ftStatus = FT_GetLibraryVersion(&libraryVersion);
	if (ftStatus == FT_OK)
	    printf("Library version = 0x%x\n", (unsigned int)libraryVersion);
	else
	{
		printf("Error reading library version.\n");
		return 1;
	}
	
	ftStatus = FT_Open(iport, &ftHandle0);
	if(ftStatus != FT_OK) {
		printf("FT_Open(%d) failed\n", iport);
		return 1;
	}
	
	printf("FT_Open succeeded.  Handle is %p\n", ftHandle0);

	ftStatus = FT_GetDeviceInfo(ftHandle0,
	                            &ftDevice,
	                            NULL,
	                            NULL,
	                            NULL,
	                            NULL); 
	if (ftStatus != FT_OK) 
	{ 
		printf("FT_GetDeviceType FAILED!\n");
		retCode = 1;
		goto exit;
	}  

	printf("FT_GetDeviceInfo succeeded.  Device is type %d.\n", (int)ftDevice);
	
	FT_ResetDevice(ftHandle0);
	FT_SetBaudRate(ftHandle0, 9600);
	FT_SetDataCharacteristics(ftHandle0, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);

	
	while(strcmp(hex_inp, "exit")!=0)
	{
	    printf("> ");
        scanf("%s", hex_inp);
        
        if (strcmp(hex_inp, "exit")==0)
        {
            printf("Exit... \n");
        }
        else if((strlen(hex_inp) % 2) != 0)
        {
            printf("Error: Uneven number of bytes");
        }
        else if(strlen(hex_inp) > 0)
        {
            //Allocate memory for input buffer
            len_buf = (strlen(hex_inp) / 2);
            ptr = (unsigned char*)malloc(len_buf * sizeof(unsigned char));
        
            //Scan input string for hex string
            for(i=0;i<strlen(hex_inp);i=i+2)
            {
                memcpy(single, &hex_inp[i], 2 );
                single[2] = '\0';
                num = (int)strtol(single, NULL, 16); 
                ptr[i/2] = num;
            }
            
            spi_tx(ftHandle0, ptr, cBufRead, len_buf);
            
            printf("> ");
            for(i=0;i<len_buf;i++)
                printf("%x ", cBufRead[i]);
	        printf("\n");
            
            free(ptr);
        }
    }
	
exit:

	FT_Close(ftHandle0);
	printf("Returning %d\n", retCode);
	return retCode;
}
