//
// COPYRIGHT 2014 Winbond Electronics Corp.
// SPI FLASH Reference Code
// version 1.03 2014/12/29

// version update log
// 1.02: add DTR function
// 1.03: add Die select,3byte-4byte
// 1.04: update relate issue

/*** COMMAND ***/
#include <stdint.h> // for uint32_t, etc
#include "DrvGPIO.h"
#include "DrvSPIHost.h"


void WRENVSR(void);
void WRDI(void);
unsigned char Read_RDSR1_StatusRegister(void);
unsigned char Read_RDSR2_StatusRegister(void);
uint32_t Read_SenXorExternal_Flash(uint32_t addr,uint32_t count, uint8_t* ReadDataBuffer);
unsigned int Read4Byte(unsigned int addr,char* ReadDataBuffer,int count);
unsigned int FastRead(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead4Byte(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int DTR_FR(unsigned int addr,int* ReadDataBuffer,int count);
unsigned int FastRead_DO(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead_DO4Byte(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead_QO(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead_QO4Byte(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead_DIO(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead_DIO4byte(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int DTR_FR_DIO(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead_QIO(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int FastRead_QIO4byte(unsigned int addr, int* ReadDataBuffer,int count);
unsigned int DTR_FR_QIO(unsigned int addr, int* ReadDataBuffer,int count);
void SetWrap(unsigned int BW);
unsigned int Write_SenXorExternal_Flash(uint32_t addr,uint32_t count, uint8_t* updatebuffer);
unsigned int Program4byte(unsigned int addr, char* updatebuffer,int count);
unsigned int QIProgram(unsigned int addr, char* updatebuffer,int count);
unsigned int QIProgram4byte(unsigned int addr, char* updatebuffer,int count);
unsigned int SectorErase(unsigned int addr);
unsigned int HalfBlockErase(unsigned int addr);
unsigned int BlockErase(unsigned int addr);
unsigned int ChipErase(unsigned int addr);
unsigned int Suspend(void);
unsigned int Resume(void);
void PowerDown(void);
void ReleasePowerDown(void);
void Device_ID(char* ID_buffer);
void ManufacturerDevice_ID(char* ID_buffer);
void ManufacturerDevice_ID_DO(char* ID_buffer);
void ManufacturerDevice_ID_QO(char* ID_buffer);
void UID(char* ID_buffer);
void JEDECID(char* ID_buffer);
void SFDP(unsigned int addr,char* ID_buffer,int count);
unsigned int EraseSecurityRegisters(char Select_Security_Register);
unsigned int ProgramSecurityRegisters(char* updatebuffer,char Select_Security_Register);
void SetReadParameters(int parameters);
void BR_Wrap(char* buffer,int addr,int count);
void DTR_BR_Wrap(char* buffer,int addr,int count);
int individualBlockSectorLock(unsigned int addr);
int IndividualBlockSectorUnlock(unsigned int addr);
unsigned int ReadLock(unsigned int addr);
unsigned int GlobalLock(void);
unsigned int GlobalUnlock(void);
unsigned char RDEAR(void);
void WREAR(char ear);
void ENTER4B(void);
void DieSelect(char DieID);
int QPIstatus(int qpi_flag);
int ADPstatus(void);


unsigned char CheckBusy(void);
void DummyClock(unsigned int count);
unsigned char CheckSuspend(void);
unsigned char CheckWPS(void);
void EnterQPI(void);
void ExitQPI(void);
void EnableReset(void);
void Reset(void);

 #define QIO 1
 #define DIO 3
 #define SIO 0
/*** W25Q series command hex code definition ***/
//ID commands
//0.read "ID" commands
#define  SPI_JEDECID_CMD    0x9F	//Read JEDEC ID
#define  SPI_DVID_CMD       0xAB	//Read device ID
#define  SPI_MDID_CMD       0x90	//Read Manufacture /Device ID
#define  SPI_MDIDD_CMD      0x92	//Read Manu/devi id for Dual I/O
#define  SPI_MDIDQ_CMD      0x94	//Read Manu/devi id for Quad I/O
#define  SPI_UID_CMD        0x4B	//Read Unique ID

//1.Status "R"egister commands
#define  SPI_RDSR1_CMD      0x05	//Read status register -1
#define  SPI_RDSR2_CMD      0x35	//Read Status register -2
#define  SPI_WRSR_CMD       0x01	//Write Status Register
#define  SPI_WRSR2_CMD      0x31    //Write Status Register -2
#define  SPI_WRSR3_CMD      0x11    //Write Status Register -3
#define  SPI_PGSCUR_CMD     0x42	//Program Security Registers
#define  SPI_ERSCUR_CMD     0x44	//Erase Security Registers
#define  SPI_RDSCUR_CMD     0x48	//Read Security Registers
#define  SPI_SFDP_CMD       0x5A	//Read SFDP Register

//Read "RD" Commands
#define  SPI_RD_CMD         0x03	//READ (1x I/O)
#define  SPI_RD4Byte_CMD    0x13    //Read Data with 4-Byte
#define  SPI_FRD_CMD        0x0B	//Fast read (1In,1Out)
#define  SPI_FRD4Byte_CMD   0x0C    //Fast Read with 4-Byte Address
#define  SPI_FRD2O_CMD      0x3B	//Fast read dual output(1In,2Out)
#define  SPI_FRD204Byte_CMD 0x3C    //Fast read dual output with 4 Byte
#define  SPI_FRD4O_CMD      0x6B	//Fast read quad output(1In,4Out)
#define  SPI_FRD4O4Byte_CMD 0x6C    //Fast Read Quad Output with 4-Byte Address
#define  SPI_2RD_CMD        0xBB	//Fast READ (2x I/O)
#define  SPI_2RD4Byte_CMD   0xBC    //Fast READ (2x I/O) with 4-Byte address
#define  SPI_4RD_CMD        0xEB	//Fast READ (4x I/O)(4dummy clocks)
#define  SPI_4RD4Byte_CMD   0xEC    //Fast READ (4x I/O)(4dummy clocks)with 4-Byte address
#define  SPI_WRDQ_CMD       0xE7	//Word Read Quad I/O(only 2 dummy clocks)
#define  SPI_OWRDQ_CMD      0xE3	//Octal word read quad I/O(0 dummy clocks)
#define  SPI_DTRFR_CMD      0x0D    //DTR Fast Read
#define  SPI_DTR_FR_DIO_CMD 0xBD    //DTR Fast Read Dual I/O
#define  SPI_DTR_FR_QIO_CMD 0xED    //DTR Fast Read Quad I/O
#define  SPI_CRMRST_CMD     0xFF	//Continuous Read Mode Reset(quad I/O)
#define  SPI_CRMRSTD_CMD    0xFFFF	//Continuous Read Mode Reset(dual I/O)
#define  SPI_BRWP_CMD       0x77	//Set Burst with Wrap (/FRD)

//"P"rogram Commands
#define  SPI_WREN_CMD       0x06	//Write Enable
#define  SPI_WRENVSR_CMD    0x50	//Write Enable for volatile Status register
#define  SPI_WRDI_CMD       0x04	//Write Disable
#define  SPI_PP_CMD         0x02	//Page program
#define  SPI_PP_4Byte_CMD   0x12    //Page program with 4-Byte Address
#define  SPI_4PP_CMD        0x32	//Quad Page Program
#define  SPI_4PP4Byte_CMD   0x34    //Quad Page Program with 4-Byte Address
//"E"rase commands
#define  SPI_SE_CMD         0x20	//Sector erase(4kb)
#define  SPI_SE4byte_CMD    0x21    //Sector erase(4kb) with 4-Byte Address
#define  SPI_BE32_CMD       0x52	//Block Erase(32kb)
#define  SPI_BE_CMD         0xD8	//Block Erase(64kb)
#define  SPI_BE4byte_CMD    0xDC    //Block Erase(64kb) with 4-Bye Address
#define  SPI_CE_CMD         0x60	//Chip Erase hex code 60 or C7
#define  SPI_CE2_CME        0xC7    //Chip Erase hex code 60 or C7
//"M"ode setting commands
#define  SPI_PD_CMD         0xB9	//Power Down
#define  SPI_RPD_CMD        0xAB	//Release power down

//For large IC
#define  SPI_RDEAR_CMD      0xC8    //Read Extended Address Register
#define  SPI_WREAR_CMD      0xC5    //Write Extended Address Register
#define  SPI_ENTER4B_CMD    0xB7    //Enter 4-Byte Address Mode
#define  SPI_EXIT4B_CMD     0xE9    //Exit 4-Byte Address Mode

//Die select
#define  SPI_DIESELECT_CME  0xC2    //Software Die Select


//"O"ther
#define  SPI_EPSP_CMD       0x75	//Erase/Program Suspend
#define  SPI_EPSR_CMD       0x7A	//Erase/Program Resume
#define  QPI_SRP_CMD        0xC0    //Set Read Parameters
#define  QPI_BRW_CMD        0x0C    //Burst Read with Wrap
#define  QPI_DTBRW_CMD      0x0E    //DTR Burst Read with Wrap
#define  QPI_Enter_CMD      0x38    //Enter QPI
#define  QPI_Exit_CMD       0xFF    //EXit QPI
#define  SPI_IBLKLK_CMD     0x36    //Individual Block Sector Lock
#define  SPI_IBLKULK_CMD    0x39    //Individual Block Sector Unlock
#define  SPI_RDBLKLK_CMD    0x3D    //Read Block/Sector Lock
#define  SPI_GBSLK_CMD      0x7E    //Global Block/Sector Lock
#define  SPI_GBSULK_CMD     0x98    //Global Block/Sector Lock
#define  SPI_RSTEN_CMD      0x66    //Enable Reset
#define  SPI_RST_CMD        0x99    //Reset

