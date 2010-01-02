/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

#if JUCE_USE_CDREADER

//***************************************************************************
//       %%% TARGET STATUS VALUES %%%
//***************************************************************************
#define STATUS_GOOD         0x00    // Status Good
#define STATUS_CHKCOND      0x02    // Check Condition
#define STATUS_CONDMET      0x04    // Condition Met
#define STATUS_BUSY         0x08    // Busy
#define STATUS_INTERM       0x10    // Intermediate
#define STATUS_INTCDMET     0x14    // Intermediate-condition met
#define STATUS_RESCONF      0x18    // Reservation conflict
#define STATUS_COMTERM      0x22    // Command Terminated
#define STATUS_QFULL        0x28    // Queue full

//***************************************************************************
//      %%% SCSI MISCELLANEOUS EQUATES %%%
//***************************************************************************
#define MAXLUN              7       // Maximum Logical Unit Id
#define MAXTARG             7       // Maximum Target Id
#define MAX_SCSI_LUNS       64      // Maximum Number of SCSI LUNs
#define MAX_NUM_HA          8       // Maximum Number of SCSI HA's

//***************************************************************************
//     %%% Commands for all Device Types %%%
//***************************************************************************
#define SCSI_CHANGE_DEF     0x40    // Change Definition (Optional)
#define SCSI_COMPARE        0x39    // Compare (O)
#define SCSI_COPY           0x18    // Copy (O)
#define SCSI_COP_VERIFY     0x3A    // Copy and Verify (O)
#define SCSI_INQUIRY        0x12    // Inquiry (MANDATORY)
#define SCSI_LOG_SELECT     0x4C    // Log Select (O)
#define SCSI_LOG_SENSE      0x4D    // Log Sense (O)
#define SCSI_MODE_SEL6      0x15    // Mode Select 6-byte (Device Specific)
#define SCSI_MODE_SEL10     0x55    // Mode Select 10-byte (Device Specific)
#define SCSI_MODE_SEN6      0x1A    // Mode Sense 6-byte (Device Specific)
#define SCSI_MODE_SEN10     0x5A    // Mode Sense 10-byte (Device Specific)
#define SCSI_READ_BUFF      0x3C    // Read Buffer (O)
#define SCSI_REQ_SENSE      0x03    // Request Sense (MANDATORY)
#define SCSI_SEND_DIAG      0x1D    // Send Diagnostic (O)
#define SCSI_TST_U_RDY      0x00    // Test Unit Ready (MANDATORY)
#define SCSI_WRITE_BUFF     0x3B    // Write Buffer (O)

//***************************************************************************
//     %%% Commands Unique to Direct Access Devices %%%
//***************************************************************************
#define SCSI_COMPARE        0x39    // Compare (O)
#define SCSI_FORMAT         0x04    // Format Unit (MANDATORY)
#define SCSI_LCK_UN_CAC     0x36    // Lock Unlock Cache (O)
#define SCSI_PREFETCH       0x34    // Prefetch (O)
#define SCSI_MED_REMOVL     0x1E    // Prevent/Allow medium Removal (O)
#define SCSI_READ6          0x08    // Read 6-byte (MANDATORY)
#define SCSI_READ10         0x28    // Read 10-byte (MANDATORY)
#define SCSI_RD_CAPAC       0x25    // Read Capacity (MANDATORY)
#define SCSI_RD_DEFECT      0x37    // Read Defect Data (O)
#define SCSI_READ_LONG      0x3E    // Read Long (O)
#define SCSI_REASS_BLK      0x07    // Reassign Blocks (O)
#define SCSI_RCV_DIAG       0x1C    // Receive Diagnostic Results (O)
#define SCSI_RELEASE        0x17    // Release Unit (MANDATORY)
#define SCSI_REZERO         0x01    // Rezero Unit (O)
#define SCSI_SRCH_DAT_E     0x31    // Search Data Equal (O)
#define SCSI_SRCH_DAT_H     0x30    // Search Data High (O)
#define SCSI_SRCH_DAT_L     0x32    // Search Data Low (O)
#define SCSI_SEEK6          0x0B    // Seek 6-Byte (O)
#define SCSI_SEEK10         0x2B    // Seek 10-Byte (O)
#define SCSI_SEND_DIAG      0x1D    // Send Diagnostics (MANDATORY)
#define SCSI_SET_LIMIT      0x33    // Set Limits (O)
#define SCSI_START_STP      0x1B    // Start/Stop Unit (O)
#define SCSI_SYNC_CACHE     0x35    // Synchronize Cache (O)
#define SCSI_VERIFY         0x2F    // Verify (O)
#define SCSI_WRITE6         0x0A    // Write 6-Byte (MANDATORY)
#define SCSI_WRITE10        0x2A    // Write 10-Byte (MANDATORY)
#define SCSI_WRT_VERIFY     0x2E    // Write and Verify (O)
#define SCSI_WRITE_LONG     0x3F    // Write Long (O)
#define SCSI_WRITE_SAME     0x41    // Write Same (O)

//***************************************************************************
//   %%% Commands Unique to Sequential Access Devices %%%
//***************************************************************************
#define SCSI_ERASE          0x19    // Erase (MANDATORY)
#define SCSI_LOAD_UN        0x1b    // Load/Unload (O)
#define SCSI_LOCATE         0x2B    // Locate (O)
#define SCSI_RD_BLK_LIM     0x05    // Read Block Limits (MANDATORY)
#define SCSI_READ_POS       0x34    // Read Position (O)
#define SCSI_READ_REV       0x0F    // Read Reverse (O)
#define SCSI_REC_BF_DAT     0x14    // Recover Buffer Data (O)
#define SCSI_RESERVE        0x16    // Reserve Unit (MANDATORY)
#define SCSI_REWIND         0x01    // Rewind (MANDATORY)
#define SCSI_SPACE          0x11    // Space (MANDATORY)
#define SCSI_VERIFY_T       0x13    // Verify (Tape) (O)
#define SCSI_WRT_FILE       0x10    // Write Filemarks (MANDATORY)

//***************************************************************************
//      %%% Commands Unique to Printer Devices %%%
//***************************************************************************
#define SCSI_PRINT          0x0A    // Print (MANDATORY)
#define SCSI_SLEW_PNT       0x0B    // Slew and Print (O)
#define SCSI_STOP_PNT       0x1B    // Stop Print (O)
#define SCSI_SYNC_BUFF      0x10    // Synchronize Buffer (O)

//***************************************************************************
//     %%% Commands Unique to Processor Devices %%%
//***************************************************************************
#define SCSI_RECEIVE        0x08    // Receive (O)
#define SCSI_SEND           0x0A    // Send (O)

//***************************************************************************
//    %%% Commands Unique to Write-Once Devices %%%
//***************************************************************************
#define SCSI_MEDIUM_SCN     0x38    // Medium Scan (O)
#define SCSI_SRCHDATE10     0x31    // Search Data Equal 10-Byte (O)
#define SCSI_SRCHDATE12     0xB1    // Search Data Equal 12-Byte (O)
#define SCSI_SRCHDATH10     0x30    // Search Data High 10-Byte (O)
#define SCSI_SRCHDATH12     0xB0    // Search Data High 12-Byte (O)
#define SCSI_SRCHDATL10     0x32    // Search Data Low 10-Byte (O)
#define SCSI_SRCHDATL12     0xB2    // Search Data Low 12-Byte (O)
#define SCSI_SET_LIM_10     0x33    // Set Limits 10-Byte (O)
#define SCSI_SET_LIM_12     0xB3    // Set Limits 10-Byte (O)
#define SCSI_VERIFY10       0x2F    // Verify 10-Byte (O)
#define SCSI_VERIFY12       0xAF    // Verify 12-Byte (O)
#define SCSI_WRITE12        0xAA    // Write 12-Byte (O)
#define SCSI_WRT_VER10      0x2E    // Write and Verify 10-Byte (O)
#define SCSI_WRT_VER12      0xAE    // Write and Verify 12-Byte (O)

//***************************************************************************
//      %%% Commands Unique to CD-ROM Devices %%%
//***************************************************************************
#define SCSI_PLAYAUD_10     0x45    // Play Audio 10-Byte (O)
#define SCSI_PLAYAUD_12     0xA5    // Play Audio 12-Byte 12-Byte (O)
#define SCSI_PLAYAUDMSF     0x47    // Play Audio MSF (O)
#define SCSI_PLAYA_TKIN     0x48    // Play Audio Track/Index (O)
#define SCSI_PLYTKREL10     0x49    // Play Track Relative 10-Byte (O)
#define SCSI_PLYTKREL12     0xA9    // Play Track Relative 12-Byte (O)
#define SCSI_READCDCAP      0x25    // Read CD-ROM Capacity (MANDATORY)
#define SCSI_READHEADER     0x44    // Read Header (O)
#define SCSI_SUBCHANNEL     0x42    // Read Subchannel (O)
#define SCSI_READ_TOC       0x43    // Read TOC (O)

//***************************************************************************
//      %%% Commands Unique to Scanner Devices %%%
//***************************************************************************
#define SCSI_GETDBSTAT      0x34    // Get Data Buffer Status (O)
#define SCSI_GETWINDOW      0x25    // Get Window (O)
#define SCSI_OBJECTPOS      0x31    // Object Postion (O)
#define SCSI_SCAN           0x1B    // Scan (O)
#define SCSI_SETWINDOW      0x24    // Set Window (MANDATORY)

//***************************************************************************
//    %%% Commands Unique to Optical Memory Devices %%%
//***************************************************************************
#define SCSI_UpdateBlk      0x3D    // Update Block (O)

//***************************************************************************
//    %%% Commands Unique to Medium Changer Devices %%%
//***************************************************************************
#define SCSI_EXCHMEDIUM     0xA6    // Exchange Medium (O)
#define SCSI_INITELSTAT     0x07    // Initialize Element Status (O)
#define SCSI_POSTOELEM      0x2B    // Position to Element (O)
#define SCSI_REQ_VE_ADD     0xB5    // Request Volume Element Address (O)
#define SCSI_SENDVOLTAG     0xB6    // Send Volume Tag (O)

//***************************************************************************
//     %%% Commands Unique to Communication Devices %%%
//***************************************************************************
#define SCSI_GET_MSG_6      0x08    // Get Message 6-Byte (MANDATORY)
#define SCSI_GET_MSG_10     0x28    // Get Message 10-Byte (O)
#define SCSI_GET_MSG_12     0xA8    // Get Message 12-Byte (O)
#define SCSI_SND_MSG_6      0x0A    // Send Message 6-Byte (MANDATORY)
#define SCSI_SND_MSG_10     0x2A    // Send Message 10-Byte (O)
#define SCSI_SND_MSG_12     0xAA    // Send Message 12-Byte (O)


//***************************************************************************
//      %%% Request Sense Data Format %%%
//***************************************************************************
typedef struct {
 BYTE  ErrorCode;       // Error Code (70H or 71H)
 BYTE  SegmentNum;      // Number of current segment descriptor
 BYTE  SenseKey;        // Sense Key(See bit definitions too)
 BYTE  InfoByte0;       // Information MSB
 BYTE  InfoByte1;       // Information MID
 BYTE  InfoByte2;       // Information MID
 BYTE  InfoByte3;       // Information LSB
 BYTE  AddSenLen;       // Additional Sense Length
 BYTE  ComSpecInf0;     // Command Specific Information MSB
 BYTE  ComSpecInf1;     // Command Specific Information MID
 BYTE  ComSpecInf2;     // Command Specific Information MID
 BYTE  ComSpecInf3;     // Command Specific Information LSB
 BYTE  AddSenseCode;    // Additional Sense Code
 BYTE  AddSenQual;      // Additional Sense Code Qualifier
 BYTE  FieldRepUCode;   // Field Replaceable Unit Code
 BYTE  SenKeySpec15;    // Sense Key Specific 15th byte
 BYTE  SenKeySpec16;    // Sense Key Specific 16th byte
 BYTE  SenKeySpec17;    // Sense Key Specific 17th byte
 BYTE  AddSenseBytes;   // Additional Sense Bytes
} SENSE_DATA_FMT;

//***************************************************************************
//       %%% REQUEST SENSE ERROR CODE %%%
//***************************************************************************
#define SERROR_CURRENT      0x70    // Current Errors
#define SERROR_DEFERED      0x71    // Deferred Errors

//***************************************************************************
//      %%% REQUEST SENSE BIT DEFINITIONS %%%
//***************************************************************************
#define SENSE_VALID         0x80    // Byte 0 Bit 7
#define SENSE_FILEMRK       0x80    // Byte 2 Bit 7
#define SENSE_EOM           0x40    // Byte 2 Bit 6
#define SENSE_ILI           0x20    // Byte 2 Bit 5

//***************************************************************************
//     %%% REQUEST SENSE SENSE KEY DEFINITIONS %%%
//***************************************************************************
#define KEY_NOSENSE         0x00    // No Sense
#define KEY_RECERROR        0x01    // Recovered Error
#define KEY_NOTREADY        0x02    // Not Ready
#define KEY_MEDIUMERR       0x03    // Medium Error
#define KEY_HARDERROR       0x04    // Hardware Error
#define KEY_ILLGLREQ        0x05    // Illegal Request
#define KEY_UNITATT         0x06    // Unit Attention
#define KEY_DATAPROT        0x07    // Data Protect
#define KEY_BLANKCHK        0x08    // Blank Check
#define KEY_VENDSPEC        0x09    // Vendor Specific
#define KEY_COPYABORT       0x0A    // Copy Abort
#define KEY_EQUAL           0x0C    // Equal (Search)
#define KEY_VOLOVRFLW       0x0D    // Volume Overflow
#define KEY_MISCOMP         0x0E    // Miscompare (Search)
#define KEY_RESERVED        0x0F    // Reserved

//***************************************************************************
//      %%% PERIPHERAL DEVICE TYPE DEFINITIONS %%%
//***************************************************************************
#define DTYPE_DASD          0x00    // Disk Device
#define DTYPE_SEQD          0x01    // Tape Device
#define DTYPE_PRNT          0x02    // Printer
#define DTYPE_PROC          0x03    // Processor
#define DTYPE_WORM          0x04    // Write-once read-multiple
#define DTYPE_CROM          0x05    // CD-ROM device
#define DTYPE_SCAN          0x06    // Scanner device
#define DTYPE_OPTI          0x07    // Optical memory device
#define DTYPE_JUKE          0x08    // Medium Changer device
#define DTYPE_COMM          0x09    // Communications device
#define DTYPE_RESL          0x0A    // Reserved (low)
#define DTYPE_RESH          0x1E    // Reserved (high)
#define DTYPE_UNKNOWN       0x1F    // Unknown or no device type

//***************************************************************************
//      %%% ANSI APPROVED VERSION DEFINITIONS %%%
//***************************************************************************
#define ANSI_MAYBE          0x0     // Device may or may not be ANSI approved stand
#define ANSI_SCSI1          0x1     // Device complies to ANSI X3.131-1986 (SCSI-1)
#define ANSI_SCSI2          0x2     // Device complies to SCSI-2
#define ANSI_RESLO          0x3     // Reserved (low)
#define ANSI_RESHI          0x7     // Reserved (high)


typedef struct
{
  USHORT Length;
  UCHAR  ScsiStatus;
  UCHAR  PathId;
  UCHAR  TargetId;
  UCHAR  Lun;
  UCHAR  CdbLength;
  UCHAR  SenseInfoLength;
  UCHAR  DataIn;
  ULONG  DataTransferLength;
  ULONG  TimeOutValue;
  ULONG  DataBufferOffset;
  ULONG  SenseInfoOffset;
  UCHAR  Cdb[16];
} SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;

typedef struct
{
  USHORT Length;
  UCHAR  ScsiStatus;
  UCHAR  PathId;
  UCHAR  TargetId;
  UCHAR  Lun;
  UCHAR  CdbLength;
  UCHAR  SenseInfoLength;
  UCHAR  DataIn;
  ULONG  DataTransferLength;
  ULONG  TimeOutValue;
  PVOID  DataBuffer;
  ULONG  SenseInfoOffset;
  UCHAR  Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

typedef struct
{
  SCSI_PASS_THROUGH_DIRECT spt;
  ULONG Filler;
  UCHAR ucSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

typedef struct
{
  ULONG Length;
  UCHAR PortNumber;
  UCHAR PathId;
  UCHAR TargetId;
  UCHAR Lun;
} SCSI_ADDRESS, *PSCSI_ADDRESS;


#define  METHOD_BUFFERED     0
#define  METHOD_IN_DIRECT    1
#define  METHOD_OUT_DIRECT   2
#define  METHOD_NEITHER      3

#define  FILE_ANY_ACCESS      0
#ifndef FILE_READ_ACCESS
#define  FILE_READ_ACCESS     (0x0001)
#endif
#ifndef FILE_WRITE_ACCESS
#define  FILE_WRITE_ACCESS    (0x0002)
#endif

#define IOCTL_SCSI_BASE    0x00000004

#define  SCSI_IOCTL_DATA_OUT          0
#define  SCSI_IOCTL_DATA_IN           1
#define  SCSI_IOCTL_DATA_UNSPECIFIED  2

#define CTL_CODE2( DevType, Function, Method, Access ) (                 \
    ((DevType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE2( IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_GET_CAPABILITIES     CTL_CODE2( IOCTL_SCSI_BASE, 0x0404, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE2( IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_GET_ADDRESS          CTL_CODE2( IOCTL_SCSI_BASE, 0x0406, METHOD_BUFFERED, FILE_ANY_ACCESS )


#define SENSE_LEN                 14
#define SRB_DIR_SCSI              0x00
#define SRB_POSTING               0x01
#define SRB_ENABLE_RESIDUAL_COUNT 0x04
#define SRB_DIR_IN                0x08
#define SRB_DIR_OUT               0x10
#define SRB_EVENT_NOTIFY          0x40
#define RESIDUAL_COUNT_SUPPORTED  0x02
#define MAX_SRB_TIMEOUT       1080001u
#define DEFAULT_SRB_TIMEOUT   1080001u

#define SC_HA_INQUIRY             0x00
#define SC_GET_DEV_TYPE           0x01
#define SC_EXEC_SCSI_CMD          0x02
#define SC_ABORT_SRB              0x03
#define SC_RESET_DEV              0x04
#define SC_SET_HA_PARMS           0x05
#define SC_GET_DISK_INFO          0x06
#define SC_RESCAN_SCSI_BUS        0x07
#define SC_GETSET_TIMEOUTS        0x08

#define SS_PENDING                0x00
#define SS_COMP                   0x01
#define SS_ABORTED                0x02
#define SS_ABORT_FAIL             0x03
#define SS_ERR                    0x04
#define SS_INVALID_CMD            0x80
#define SS_INVALID_HA             0x81
#define SS_NO_DEVICE              0x82
#define SS_INVALID_SRB            0xE0
#define SS_OLD_MANAGER            0xE1
#define SS_BUFFER_ALIGN           0xE1
#define SS_ILLEGAL_MODE           0xE2
#define SS_NO_ASPI                0xE3
#define SS_FAILED_INIT            0xE4
#define SS_ASPI_IS_BUSY           0xE5
#define SS_BUFFER_TO_BIG          0xE6
#define SS_BUFFER_TOO_BIG         0xE6
#define SS_MISMATCHED_COMPONENTS  0xE7
#define SS_NO_ADAPTERS            0xE8
#define SS_INSUFFICIENT_RESOURCES 0xE9
#define SS_ASPI_IS_SHUTDOWN       0xEA
#define SS_BAD_INSTALL            0xEB

#define HASTAT_OK                 0x00
#define HASTAT_SEL_TO             0x11
#define HASTAT_DO_DU              0x12
#define HASTAT_BUS_FREE           0x13
#define HASTAT_PHASE_ERR          0x14
#define HASTAT_TIMEOUT            0x09
#define HASTAT_COMMAND_TIMEOUT    0x0B
#define HASTAT_MESSAGE_REJECT     0x0D
#define HASTAT_BUS_RESET          0x0E
#define HASTAT_PARITY_ERROR       0x0F
#define HASTAT_REQUEST_SENSE_FAILED 0x10

#define PACKED
#pragma pack(1)

typedef struct
{
  BYTE     SRB_Cmd;
  BYTE     SRB_Status;
  BYTE     SRB_HaID;
  BYTE     SRB_Flags;
  DWORD    SRB_Hdr_Rsvd;
  BYTE     HA_Count;
  BYTE     HA_SCSI_ID;
  BYTE     HA_ManagerId[16];
  BYTE     HA_Identifier[16];
  BYTE     HA_Unique[16];
  WORD     HA_Rsvd1;
  BYTE     pad[20];
} PACKED SRB_HAInquiry, *PSRB_HAInquiry, FAR *LPSRB_HAInquiry;


typedef struct
{
  BYTE     SRB_Cmd;
  BYTE     SRB_Status;
  BYTE     SRB_HaID;
  BYTE     SRB_Flags;
  DWORD    SRB_Hdr_Rsvd;
  BYTE     SRB_Target;
  BYTE     SRB_Lun;
  BYTE     SRB_DeviceType;
  BYTE     SRB_Rsvd1;
  BYTE     pad[68];
} PACKED SRB_GDEVBlock, *PSRB_GDEVBlock, FAR *LPSRB_GDEVBlock;


typedef struct
{
  BYTE     SRB_Cmd;
  BYTE     SRB_Status;
  BYTE     SRB_HaID;
  BYTE     SRB_Flags;
  DWORD    SRB_Hdr_Rsvd;
  BYTE     SRB_Target;
  BYTE     SRB_Lun;
  WORD     SRB_Rsvd1;
  DWORD    SRB_BufLen;
  BYTE FAR *SRB_BufPointer;
  BYTE     SRB_SenseLen;
  BYTE     SRB_CDBLen;
  BYTE     SRB_HaStat;
  BYTE     SRB_TargStat;
  VOID FAR *SRB_PostProc;
  BYTE     SRB_Rsvd2[20];
  BYTE     CDBByte[16];
  BYTE SenseArea[SENSE_LEN+2];
} PACKED SRB_ExecSCSICmd, *PSRB_ExecSCSICmd, FAR *LPSRB_ExecSCSICmd;


typedef struct
{
  BYTE      SRB_Cmd;
  BYTE      SRB_Status;
  BYTE      SRB_HaId;
  BYTE      SRB_Flags;
  DWORD     SRB_Hdr_Rsvd;
} PACKED SRB, *PSRB, FAR *LPSRB;

#pragma pack()


//==============================================================================
struct CDDeviceInfo
{
    char vendor[9];
    char productId[17];
    char rev[5];
    char vendorSpec[21];

    BYTE ha;
    BYTE tgt;
    BYTE lun;
    char scsiDriveLetter; // will be 0 if not using scsi
};


//==============================================================================
class CDReadBuffer
{
public:
    int startFrame;
    int numFrames;
    int dataStartOffset;
    int dataLength;
    BYTE* buffer;
    int bufferSize;
    int index;
    bool wantsIndex;

    //==============================================================================
    CDReadBuffer (const int numberOfFrames)
        : startFrame (0),
          numFrames (0),
          dataStartOffset (0),
          dataLength (0),
          index (0),
          wantsIndex (false)
    {
        bufferSize = 2352 * numberOfFrames;
        buffer = (BYTE*) malloc (bufferSize);
    }

    ~CDReadBuffer()
    {
        free (buffer);
    }

    bool isZero() const
    {
        BYTE* p = buffer + dataStartOffset;

        for (int i = dataLength; --i >= 0;)
            if (*p++ != 0)
                return false;

        return true;
    }
};

class CDDeviceHandle;

class CDController
{
public:
    CDController();
    virtual ~CDController();

    virtual bool read (CDReadBuffer* t) = 0;
    virtual void shutDown();

    bool readAudio (CDReadBuffer* t, CDReadBuffer* overlapBuffer = 0);
    int getLastIndex();

public:
    bool initialised;

    CDDeviceHandle* deviceInfo;
    int framesToCheck, framesOverlap;

    void prepare (SRB_ExecSCSICmd& s);
    void perform (SRB_ExecSCSICmd& s);
    void setPaused (bool paused);
};


//==============================================================================
#pragma pack(1)

struct TOCTRACK
{
    BYTE rsvd;
    BYTE ADR;
    BYTE trackNumber;
    BYTE rsvd2;
    BYTE addr[4];
};

struct TOC
{
    WORD tocLen;
    BYTE firstTrack;
    BYTE lastTrack;
    TOCTRACK tracks[100];
};

#pragma pack()

enum
{
    READTYPE_ANY = 0,
    READTYPE_ATAPI1 = 1,
    READTYPE_ATAPI2 = 2,
    READTYPE_READ6 = 3,
    READTYPE_READ10 = 4,
    READTYPE_READ_D8 = 5,
    READTYPE_READ_D4 = 6,
    READTYPE_READ_D4_1 = 7,
    READTYPE_READ10_2 = 8
};


//==============================================================================
class CDDeviceHandle
{
public:
    CDDeviceHandle (const CDDeviceInfo* const device)
        : scsiHandle (0),
          readType (READTYPE_ANY),
          controller (0)
    {
        memcpy (&info, device, sizeof (info));
    }

    ~CDDeviceHandle()
    {
        if (controller != 0)
        {
            controller->shutDown();
            delete controller;
        }

        if (scsiHandle != 0)
            CloseHandle (scsiHandle);
    }

    bool readTOC (TOC* lpToc, bool useMSF);
    bool readAudio (CDReadBuffer* buffer, CDReadBuffer* overlapBuffer = 0);
    void openDrawer (bool shouldBeOpen);

    CDDeviceInfo info;
    HANDLE scsiHandle;
    BYTE readType;

private:
    CDController* controller;

    bool testController (const int readType,
                         CDController* const newController,
                         CDReadBuffer* const bufferToUse);
};


//==============================================================================
DWORD (*fGetASPI32SupportInfo)(void);
DWORD (*fSendASPI32Command)(LPSRB);

//==============================================================================
static HINSTANCE winAspiLib = 0;
static bool usingScsi = false;
static bool initialised = false;


static bool InitialiseCDRipper()
{
    if (! initialised)
    {
        initialised = true;

        OSVERSIONINFO info;
        info.dwOSVersionInfoSize = sizeof (info);
        GetVersionEx (&info);

        usingScsi = (info.dwPlatformId == VER_PLATFORM_WIN32_NT) && (info.dwMajorVersion > 4);

        if (! usingScsi)
        {
            fGetASPI32SupportInfo = 0;
            fSendASPI32Command = 0;
            winAspiLib = LoadLibrary (_T("WNASPI32.DLL"));

            if (winAspiLib != 0)
            {
                fGetASPI32SupportInfo = (DWORD(*)(void))  GetProcAddress (winAspiLib, "GetASPI32SupportInfo");
                fSendASPI32Command    = (DWORD(*)(LPSRB)) GetProcAddress (winAspiLib, "SendASPI32Command");

                if (fGetASPI32SupportInfo == 0 || fSendASPI32Command == 0)
                    return false;
            }
            else
            {
                usingScsi = true;
            }
        }
    }

    return true;
}

static void DeinitialiseCDRipper()
{
    if (winAspiLib != 0)
    {
        fGetASPI32SupportInfo = 0;
        fSendASPI32Command = 0;
        FreeLibrary (winAspiLib);
        winAspiLib = 0;
    }

    initialised = false;
}

//==============================================================================
static HANDLE CreateSCSIDeviceHandle (char driveLetter)
{
    TCHAR devicePath[8];
    devicePath[0] = '\\';
    devicePath[1] = '\\';
    devicePath[2] = '.';
    devicePath[3] = '\\';
    devicePath[4] = driveLetter;
    devicePath[5] = ':';
    devicePath[6] = 0;

    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof (info);
    GetVersionEx (&info);

    DWORD flags = GENERIC_READ;

    if ((info.dwPlatformId == VER_PLATFORM_WIN32_NT) && (info.dwMajorVersion > 4))
        flags = GENERIC_READ | GENERIC_WRITE;

    HANDLE h = CreateFile (devicePath, flags, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (h == INVALID_HANDLE_VALUE)
    {
        flags ^= GENERIC_WRITE;
        h = CreateFile (devicePath, flags, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }

    return h;
}

static DWORD performScsiPassThroughCommand (const LPSRB_ExecSCSICmd srb,
                                            const char driveLetter,
                                            HANDLE& deviceHandle,
                                            const bool retryOnFailure = true)
{
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER s;
    zerostruct (s);

    s.spt.Length = sizeof (SCSI_PASS_THROUGH);
    s.spt.CdbLength = srb->SRB_CDBLen;

    s.spt.DataIn = (BYTE) ((srb->SRB_Flags & SRB_DIR_IN)
                            ? SCSI_IOCTL_DATA_IN
                            : ((srb->SRB_Flags & SRB_DIR_OUT)
                                ? SCSI_IOCTL_DATA_OUT
                                : SCSI_IOCTL_DATA_UNSPECIFIED));

    s.spt.DataTransferLength = srb->SRB_BufLen;
    s.spt.TimeOutValue = 5;
    s.spt.DataBuffer = srb->SRB_BufPointer;
    s.spt.SenseInfoOffset = offsetof (SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);

    memcpy (s.spt.Cdb, srb->CDBByte, srb->SRB_CDBLen);

    srb->SRB_Status = SS_ERR;
    srb->SRB_TargStat = 0x0004;

    DWORD bytesReturned = 0;

    if (DeviceIoControl (deviceHandle, IOCTL_SCSI_PASS_THROUGH_DIRECT,
                         &s, sizeof (s),
                         &s, sizeof (s),
                         &bytesReturned, 0) != 0)
    {
        srb->SRB_Status = SS_COMP;
    }
    else if (retryOnFailure)
    {
        const DWORD error = GetLastError();

        if ((error == ERROR_MEDIA_CHANGED) || (error == ERROR_INVALID_HANDLE))
        {
            if (error != ERROR_INVALID_HANDLE)
                CloseHandle (deviceHandle);

            deviceHandle = CreateSCSIDeviceHandle (driveLetter);

            return performScsiPassThroughCommand (srb, driveLetter, deviceHandle, false);
        }
    }

    return srb->SRB_Status;
}


//==============================================================================
// Controller types..

class ControllerType1  : public CDController
{
public:
    ControllerType1() {}
    ~ControllerType1() {}

    bool read (CDReadBuffer* rb)
    {
        if (rb->numFrames * 2352 > rb->bufferSize)
            return false;

        SRB_ExecSCSICmd s;
        prepare (s);
        s.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
        s.SRB_BufLen = rb->bufferSize;
        s.SRB_BufPointer = rb->buffer;
        s.SRB_CDBLen = 12;
        s.CDBByte[0] = 0xBE;
        s.CDBByte[3] = (BYTE)((rb->startFrame >> 16) & 0xFF);
        s.CDBByte[4] = (BYTE)((rb->startFrame >> 8) & 0xFF);
        s.CDBByte[5] = (BYTE)(rb->startFrame & 0xFF);
        s.CDBByte[8] = (BYTE)(rb->numFrames & 0xFF);
        s.CDBByte[9] = (BYTE)((deviceInfo->readType == READTYPE_ATAPI1) ? 0x10 : 0xF0);
        perform (s);

        if (s.SRB_Status != SS_COMP)
            return false;

        rb->dataLength = rb->numFrames * 2352;
        rb->dataStartOffset = 0;
        return true;
    }
};


//==============================================================================
class ControllerType2  : public CDController
{
public:
    ControllerType2() {}
    ~ControllerType2() {}

    void shutDown()
    {
        if (initialised)
        {
            BYTE bufPointer[] = { 0, 0, 0, 8, 83, 0, 0, 0, 0, 0, 8, 0 };

            SRB_ExecSCSICmd s;
            prepare (s);
            s.SRB_Flags = SRB_EVENT_NOTIFY | SRB_ENABLE_RESIDUAL_COUNT;
            s.SRB_BufLen = 0x0C;
            s.SRB_BufPointer = bufPointer;
            s.SRB_CDBLen = 6;
            s.CDBByte[0] = 0x15;
            s.CDBByte[4] = 0x0C;
            perform (s);
        }
    }

    bool init()
    {
        SRB_ExecSCSICmd s;
        s.SRB_Status = SS_ERR;

        if (deviceInfo->readType == READTYPE_READ10_2)
        {
            BYTE bufPointer1[] = { 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 9, 48, 35, 6, 0, 0, 0, 0, 0, 128 };
            BYTE bufPointer2[] = { 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 9, 48, 1, 6, 32, 7, 0, 0, 0, 0 };

            for (int i = 0; i < 2; ++i)
            {
                prepare (s);
                s.SRB_Flags = SRB_EVENT_NOTIFY;
                s.SRB_BufLen = 0x14;
                s.SRB_BufPointer = (i == 0) ? bufPointer1 : bufPointer2;
                s.SRB_CDBLen = 6;
                s.CDBByte[0] = 0x15;
                s.CDBByte[1] = 0x10;
                s.CDBByte[4] = 0x14;
                perform (s);

                if (s.SRB_Status != SS_COMP)
                    return false;
            }
        }
        else
        {
            BYTE bufPointer[] = { 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 9, 48 };

            prepare (s);
            s.SRB_Flags = SRB_EVENT_NOTIFY;
            s.SRB_BufLen = 0x0C;
            s.SRB_BufPointer = bufPointer;
            s.SRB_CDBLen = 6;
            s.CDBByte[0] = 0x15;
            s.CDBByte[4] = 0x0C;
            perform (s);
        }

        return s.SRB_Status == SS_COMP;
    }

    bool read (CDReadBuffer* rb)
    {
        if (rb->numFrames * 2352 > rb->bufferSize)
            return false;

        if (!initialised)
        {
            initialised = init();

            if (!initialised)
                return false;
        }

        SRB_ExecSCSICmd s;
        prepare (s);
        s.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
        s.SRB_BufLen = rb->bufferSize;
        s.SRB_BufPointer = rb->buffer;
        s.SRB_CDBLen = 10;
        s.CDBByte[0] = 0x28;
        s.CDBByte[1] = (BYTE)(deviceInfo->info.lun << 5);
        s.CDBByte[3] = (BYTE)((rb->startFrame >> 16) & 0xFF);
        s.CDBByte[4] = (BYTE)((rb->startFrame >> 8) & 0xFF);
        s.CDBByte[5] = (BYTE)(rb->startFrame & 0xFF);
        s.CDBByte[8] = (BYTE)(rb->numFrames & 0xFF);
        perform (s);

        if (s.SRB_Status != SS_COMP)
            return false;

        rb->dataLength = rb->numFrames * 2352;
        rb->dataStartOffset = 0;

        return true;
    }
};


//==============================================================================
class ControllerType3  : public CDController
{
public:
    ControllerType3() {}
    ~ControllerType3() {}

    bool read (CDReadBuffer* rb)
    {
        if (rb->numFrames * 2352 > rb->bufferSize)
            return false;

        if (!initialised)
        {
            setPaused (false);
            initialised = true;
        }

        SRB_ExecSCSICmd s;
        prepare (s);
        s.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
        s.SRB_BufLen = rb->numFrames * 2352;
        s.SRB_BufPointer = rb->buffer;
        s.SRB_CDBLen = 12;
        s.CDBByte[0] = 0xD8;
        s.CDBByte[3] = (BYTE)((rb->startFrame >> 16) & 0xFF);
        s.CDBByte[4] = (BYTE)((rb->startFrame >> 8) & 0xFF);
        s.CDBByte[5] = (BYTE)(rb->startFrame & 0xFF);
        s.CDBByte[9] = (BYTE)(rb->numFrames & 0xFF);
        perform (s);

        if (s.SRB_Status != SS_COMP)
            return false;

        rb->dataLength = rb->numFrames * 2352;
        rb->dataStartOffset = 0;

        return true;
    }
};


//==============================================================================
class ControllerType4  : public CDController
{
public:
    ControllerType4() {}
    ~ControllerType4() {}

    bool selectD4Mode()
    {
        BYTE bufPointer[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 48 };

        SRB_ExecSCSICmd s;
        prepare (s);
        s.SRB_Flags = SRB_EVENT_NOTIFY;
        s.SRB_CDBLen = 6;
        s.SRB_BufLen = 12;
        s.SRB_BufPointer = bufPointer;
        s.CDBByte[0] = 0x15;
        s.CDBByte[1] = 0x10;
        s.CDBByte[4] = 0x08;
        perform (s);

        return s.SRB_Status == SS_COMP;
    }

    bool read (CDReadBuffer* rb)
    {
        if (rb->numFrames * 2352 > rb->bufferSize)
            return false;

        if (!initialised)
        {
            setPaused (true);

            if (deviceInfo->readType == READTYPE_READ_D4_1)
                selectD4Mode();

            initialised = true;
        }

        SRB_ExecSCSICmd s;
        prepare (s);
        s.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
        s.SRB_BufLen = rb->bufferSize;
        s.SRB_BufPointer = rb->buffer;
        s.SRB_CDBLen = 10;
        s.CDBByte[0] = 0xD4;
        s.CDBByte[3] = (BYTE)((rb->startFrame >> 16) & 0xFF);
        s.CDBByte[4] = (BYTE)((rb->startFrame >> 8) & 0xFF);
        s.CDBByte[5] = (BYTE)(rb->startFrame & 0xFF);
        s.CDBByte[8] = (BYTE)(rb->numFrames & 0xFF);
        perform (s);

        if (s.SRB_Status != SS_COMP)
            return false;

        rb->dataLength = rb->numFrames * 2352;
        rb->dataStartOffset = 0;

        return true;
    }
};


//==============================================================================
CDController::CDController()  : initialised (false)
{
}

CDController::~CDController()
{
}

void CDController::prepare (SRB_ExecSCSICmd& s)
{
    zerostruct (s);

    s.SRB_Cmd = SC_EXEC_SCSI_CMD;
    s.SRB_HaID = deviceInfo->info.ha;
    s.SRB_Target = deviceInfo->info.tgt;
    s.SRB_Lun = deviceInfo->info.lun;
    s.SRB_SenseLen = SENSE_LEN;
}

void CDController::perform (SRB_ExecSCSICmd& s)
{
    HANDLE event = CreateEvent (0, TRUE, FALSE, 0);
    s.SRB_PostProc = (void*)event;

    ResetEvent (event);

    DWORD status = (usingScsi) ? performScsiPassThroughCommand ((LPSRB_ExecSCSICmd)&s,
                                                                deviceInfo->info.scsiDriveLetter,
                                                                deviceInfo->scsiHandle)
                               : fSendASPI32Command ((LPSRB)&s);

    if (status == SS_PENDING)
        WaitForSingleObject (event, 4000);

    CloseHandle (event);
}

void CDController::setPaused (bool paused)
{
    SRB_ExecSCSICmd s;
    prepare (s);
    s.SRB_Flags = SRB_EVENT_NOTIFY;
    s.SRB_CDBLen = 10;
    s.CDBByte[0] = 0x4B;
    s.CDBByte[8] = (BYTE) (paused ? 0 : 1);
    perform (s);
}

void CDController::shutDown()
{
}

bool CDController::readAudio (CDReadBuffer* rb, CDReadBuffer* overlapBuffer)
{
    if (overlapBuffer != 0)
    {
        const bool canDoJitter = (overlapBuffer->bufferSize >= 2352 * framesToCheck);
        const bool doJitter = canDoJitter && ! overlapBuffer->isZero();

        if (doJitter
             && overlapBuffer->startFrame > 0
             && overlapBuffer->numFrames > 0
             && overlapBuffer->dataLength > 0)
        {
            const int numFrames = rb->numFrames;

            if (overlapBuffer->startFrame == (rb->startFrame - framesToCheck))
            {
                rb->startFrame -= framesOverlap;

                if (framesToCheck < framesOverlap
                     && numFrames + framesOverlap <= rb->bufferSize / 2352)
                    rb->numFrames += framesOverlap;
            }
            else
            {
                overlapBuffer->dataLength = 0;
                overlapBuffer->startFrame = 0;
                overlapBuffer->numFrames = 0;
            }
        }

        if (! read (rb))
            return false;

        if (doJitter)
        {
            const int checkLen = framesToCheck * 2352;
            const int maxToCheck = rb->dataLength - checkLen;

            if (overlapBuffer->dataLength == 0 || overlapBuffer->isZero())
                return true;

            BYTE* const p = overlapBuffer->buffer + overlapBuffer->dataStartOffset;
            bool found = false;

            for (int i = 0; i < maxToCheck; ++i)
            {
                if (!memcmp (p, rb->buffer + i, checkLen))
                {
                    i += checkLen;
                    rb->dataStartOffset = i;
                    rb->dataLength -= i;
                    rb->startFrame = overlapBuffer->startFrame + framesToCheck;
                    found = true;
                    break;
                }
            }

            rb->numFrames = rb->dataLength / 2352;
            rb->dataLength = 2352 * rb->numFrames;

            if (!found)
                return false;
        }

        if (canDoJitter)
        {
            memcpy (overlapBuffer->buffer,
                    rb->buffer + rb->dataStartOffset + 2352 * (rb->numFrames - framesToCheck),
                    2352 * framesToCheck);

            overlapBuffer->startFrame = rb->startFrame + rb->numFrames - framesToCheck;
            overlapBuffer->numFrames = framesToCheck;
            overlapBuffer->dataLength = 2352 * framesToCheck;
            overlapBuffer->dataStartOffset = 0;
        }
        else
        {
            overlapBuffer->startFrame = 0;
            overlapBuffer->numFrames = 0;
            overlapBuffer->dataLength = 0;
        }

        return true;
    }
    else
    {
        return read (rb);
    }
}

int CDController::getLastIndex()
{
    char qdata[100];

    SRB_ExecSCSICmd s;
    prepare (s);
    s.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
    s.SRB_BufLen = sizeof (qdata);
    s.SRB_BufPointer = (BYTE*)qdata;
    s.SRB_CDBLen = 12;
    s.CDBByte[0] = 0x42;
    s.CDBByte[1] = (BYTE)(deviceInfo->info.lun << 5);
    s.CDBByte[2] = 64;
    s.CDBByte[3] = 1; // get current position
    s.CDBByte[7] = 0;
    s.CDBByte[8] = (BYTE)sizeof (qdata);
    perform (s);

    if (s.SRB_Status == SS_COMP)
        return qdata[7];

    return 0;
}

//==============================================================================
bool CDDeviceHandle::readTOC (TOC* lpToc, bool useMSF)
{
    HANDLE event = CreateEvent (0, TRUE, FALSE, 0);

    SRB_ExecSCSICmd s;
    zerostruct (s);

    s.SRB_Cmd = SC_EXEC_SCSI_CMD;
    s.SRB_HaID = info.ha;
    s.SRB_Target = info.tgt;
    s.SRB_Lun = info.lun;
    s.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
    s.SRB_BufLen = 0x324;
    s.SRB_BufPointer = (BYTE*)lpToc;
    s.SRB_SenseLen = 0x0E;
    s.SRB_CDBLen = 0x0A;
    s.SRB_PostProc = (void*)event;
    s.CDBByte[0] = 0x43;
    s.CDBByte[1] = (BYTE)(useMSF ? 0x02 : 0x00);
    s.CDBByte[7] = 0x03;
    s.CDBByte[8] = 0x24;

    ResetEvent (event);
    DWORD status = (usingScsi) ? performScsiPassThroughCommand ((LPSRB_ExecSCSICmd)&s, info.scsiDriveLetter, scsiHandle)
                               : fSendASPI32Command ((LPSRB)&s);

    if (status == SS_PENDING)
        WaitForSingleObject (event, 4000);

    CloseHandle (event);
    return (s.SRB_Status == SS_COMP);
}

bool CDDeviceHandle::readAudio (CDReadBuffer* const buffer,
                                CDReadBuffer* const overlapBuffer)
{
    if (controller == 0)
    {
           testController (READTYPE_ATAPI2,    new ControllerType1(), buffer)
        || testController (READTYPE_ATAPI1,    new ControllerType1(), buffer)
        || testController (READTYPE_READ10_2,  new ControllerType2(), buffer)
        || testController (READTYPE_READ10,    new ControllerType2(), buffer)
        || testController (READTYPE_READ_D8,   new ControllerType3(), buffer)
        || testController (READTYPE_READ_D4,   new ControllerType4(), buffer)
        || testController (READTYPE_READ_D4_1, new ControllerType4(), buffer);
    }

    buffer->index = 0;

    if ((controller != 0)
        && controller->readAudio (buffer, overlapBuffer))
    {
        if (buffer->wantsIndex)
            buffer->index = controller->getLastIndex();

        return true;
    }

    return false;
}

void CDDeviceHandle::openDrawer (bool shouldBeOpen)
{
    if (shouldBeOpen)
    {
        if (controller != 0)
        {
            controller->shutDown();
            delete controller;
            controller = 0;
        }

        if (scsiHandle != 0)
        {
            CloseHandle (scsiHandle);
            scsiHandle = 0;
        }
    }

    SRB_ExecSCSICmd s;
    zerostruct (s);

    s.SRB_Cmd = SC_EXEC_SCSI_CMD;
    s.SRB_HaID = info.ha;
    s.SRB_Target = info.tgt;
    s.SRB_Lun = info.lun;
    s.SRB_SenseLen = SENSE_LEN;
    s.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
    s.SRB_BufLen = 0;
    s.SRB_BufPointer = 0;
    s.SRB_CDBLen = 12;
    s.CDBByte[0] = 0x1b;
    s.CDBByte[1] = (BYTE)(info.lun << 5);
    s.CDBByte[4] = (BYTE)((shouldBeOpen) ? 2 : 3);

    HANDLE event = CreateEvent (0, TRUE, FALSE, 0);
    s.SRB_PostProc = (void*)event;

    ResetEvent (event);

    DWORD status = (usingScsi) ? performScsiPassThroughCommand  ((LPSRB_ExecSCSICmd)&s, info.scsiDriveLetter, scsiHandle)
                               : fSendASPI32Command ((LPSRB)&s);

    if (status == SS_PENDING)
        WaitForSingleObject (event, 4000);

    CloseHandle (event);
}

bool CDDeviceHandle::testController (const int type,
                                     CDController* const newController,
                                     CDReadBuffer* const rb)
{
    controller = newController;
    readType = (BYTE)type;

    controller->deviceInfo = this;
    controller->framesToCheck = 1;
    controller->framesOverlap = 3;

    bool passed = false;

    memset (rb->buffer, 0xcd, rb->bufferSize);

    if (controller->read (rb))
    {
        passed = true;
        int* p = (int*) (rb->buffer + rb->dataStartOffset);
        int wrong = 0;

        for (int i = rb->dataLength / 4; --i >= 0;)
        {
            if (*p++ == (int) 0xcdcdcdcd)
            {
                if (++wrong == 4)
                {
                    passed = false;
                    break;
                }
            }
            else
            {
                wrong = 0;
            }
        }
    }

    if (! passed)
    {
        controller->shutDown();
        delete controller;
        controller = 0;
    }

    return passed;
}


//==============================================================================
static void GetAspiDeviceInfo (CDDeviceInfo* dev, BYTE ha, BYTE tgt, BYTE lun)
{
    HANDLE event = CreateEvent (0, TRUE, FALSE, 0);

    const int bufSize = 128;
    BYTE buffer[bufSize];
    zeromem (buffer, bufSize);

    SRB_ExecSCSICmd s;
    zerostruct (s);

    s.SRB_Cmd        = SC_EXEC_SCSI_CMD;
    s.SRB_HaID       = ha;
    s.SRB_Target     = tgt;
    s.SRB_Lun        = lun;
    s.SRB_Flags      = SRB_DIR_IN | SRB_EVENT_NOTIFY;
    s.SRB_BufLen     = bufSize;
    s.SRB_BufPointer = buffer;
    s.SRB_SenseLen   = SENSE_LEN;
    s.SRB_CDBLen     = 6;
    s.SRB_PostProc   = (void*)event;
    s.CDBByte[0]     = SCSI_INQUIRY;
    s.CDBByte[4]     = 100;

    ResetEvent (event);

    if (fSendASPI32Command ((LPSRB)&s) == SS_PENDING)
        WaitForSingleObject (event, 4000);

    CloseHandle (event);

    if (s.SRB_Status == SS_COMP)
    {
        memcpy (dev->vendor,     &buffer[8], 8);
        memcpy (dev->productId,  &buffer[16], 16);
        memcpy (dev->rev,        &buffer[32], 4);
        memcpy (dev->vendorSpec, &buffer[36], 20);
    }
}

static int FindCDDevices (CDDeviceInfo* const list,
                          int maxItems)
{
    int count = 0;

    if (usingScsi)
    {
        for (char driveLetter = 'b'; driveLetter <= 'z'; ++driveLetter)
        {
            TCHAR drivePath[8];
            drivePath[0] = driveLetter;
            drivePath[1] = ':';
            drivePath[2] = '\\';
            drivePath[3] = 0;

            if (GetDriveType (drivePath) == DRIVE_CDROM)
            {
                HANDLE h = CreateSCSIDeviceHandle (driveLetter);

                if (h != INVALID_HANDLE_VALUE)
                {
                    BYTE buffer[100], passThroughStruct[1024];
                    zeromem (buffer, sizeof (buffer));
                    zeromem (passThroughStruct, sizeof (passThroughStruct));

                    PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER p = (PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)passThroughStruct;

                    p->spt.Length             = sizeof (SCSI_PASS_THROUGH);
                    p->spt.CdbLength          = 6;
                    p->spt.SenseInfoLength    = 24;
                    p->spt.DataIn             = SCSI_IOCTL_DATA_IN;
                    p->spt.DataTransferLength = 100;
                    p->spt.TimeOutValue       = 2;
                    p->spt.DataBuffer         = buffer;
                    p->spt.SenseInfoOffset    = offsetof (SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);
                    p->spt.Cdb[0]             = 0x12;
                    p->spt.Cdb[4]             = 100;

                    DWORD bytesReturned = 0;

                    if (DeviceIoControl (h, IOCTL_SCSI_PASS_THROUGH_DIRECT,
                                         p, sizeof (SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
                                         p, sizeof (SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER),
                                         &bytesReturned, 0) != 0)
                    {
                        zeromem (&list[count], sizeof (CDDeviceInfo));

                        list[count].scsiDriveLetter = driveLetter;

                        memcpy (list[count].vendor,     &buffer[8], 8);
                        memcpy (list[count].productId,  &buffer[16], 16);
                        memcpy (list[count].rev,        &buffer[32], 4);
                        memcpy (list[count].vendorSpec, &buffer[36], 20);

                        zeromem (passThroughStruct, sizeof (passThroughStruct));
                        PSCSI_ADDRESS scsiAddr = (PSCSI_ADDRESS)passThroughStruct;

                        scsiAddr->Length = sizeof (SCSI_ADDRESS);

                        if (DeviceIoControl (h, IOCTL_SCSI_GET_ADDRESS,
                                             0, 0, scsiAddr, sizeof (SCSI_ADDRESS),
                                             &bytesReturned, 0) != 0)
                        {
                            list[count].ha = scsiAddr->PortNumber;
                            list[count].tgt = scsiAddr->TargetId;
                            list[count].lun = scsiAddr->Lun;
                            ++count;
                        }
                    }

                    CloseHandle (h);
                }
            }
        }
    }
    else
    {
        const DWORD d = fGetASPI32SupportInfo();
        BYTE status = HIBYTE (LOWORD (d));

        if (status != SS_COMP || status == SS_NO_ADAPTERS)
            return 0;

        const int numAdapters = LOBYTE (LOWORD (d));

        for (BYTE ha = 0; ha < numAdapters; ++ha)
        {
            SRB_HAInquiry s;
            zerostruct (s);

            s.SRB_Cmd = SC_HA_INQUIRY;
            s.SRB_HaID = ha;
            fSendASPI32Command ((LPSRB)&s);

            if (s.SRB_Status == SS_COMP)
            {
                maxItems = (int)s.HA_Unique[3];

                if (maxItems == 0)
                    maxItems = 8;

                for (BYTE tgt = 0; tgt < maxItems; ++tgt)
                {
                    for (BYTE lun = 0; lun < 8; ++lun)
                    {
                        SRB_GDEVBlock sb;
                        zerostruct (sb);

                        sb.SRB_Cmd = SC_GET_DEV_TYPE;
                        sb.SRB_HaID = ha;
                        sb.SRB_Target = tgt;
                        sb.SRB_Lun = lun;
                        fSendASPI32Command ((LPSRB) &sb);

                        if (sb.SRB_Status == SS_COMP
                             && sb.SRB_DeviceType == DTYPE_CROM)
                        {
                            zeromem (&list[count], sizeof (CDDeviceInfo));

                            list[count].ha = ha;
                            list[count].tgt = tgt;
                            list[count].lun = lun;

                            GetAspiDeviceInfo (&(list[count]), ha, tgt, lun);

                            ++count;
                        }
                    }
                }
            }
        }
    }

    return count;
}


//==============================================================================
static int ripperUsers = 0;
static bool initialisedOk = false;

class DeinitialiseTimer  : private Timer,
                           private DeletedAtShutdown
{
    DeinitialiseTimer (const DeinitialiseTimer&);
    const DeinitialiseTimer& operator= (const DeinitialiseTimer&);

public:
    DeinitialiseTimer()
    {
        startTimer (4000);
    }

    ~DeinitialiseTimer()
    {
        if (--ripperUsers == 0)
            DeinitialiseCDRipper();
    }

    void timerCallback()
    {
        delete this;
    }

    juce_UseDebuggingNewOperator
};

static void incUserCount()
{
    if (ripperUsers++ == 0)
        initialisedOk = InitialiseCDRipper();
}

static void decUserCount()
{
    new DeinitialiseTimer();
}

//==============================================================================
struct CDDeviceWrapper
{
    CDDeviceHandle* cdH;
    CDReadBuffer* overlapBuffer;
    bool jitter;
};

//==============================================================================
static int getAddressOf (const TOCTRACK* const t)
{
    return (((DWORD)t->addr[0]) << 24) + (((DWORD)t->addr[1]) << 16) +
           (((DWORD)t->addr[2]) << 8) + ((DWORD)t->addr[3]);
}

static int getMSFAddressOf (const TOCTRACK* const t)
{
    return 60 * t->addr[1] + t->addr[2];
}

static const int samplesPerFrame = 44100 / 75;
static const int bytesPerFrame = samplesPerFrame * 4;


//==============================================================================
const StringArray AudioCDReader::getAvailableCDNames()
{
    StringArray results;
    incUserCount();

    if (initialisedOk)
    {
        CDDeviceInfo list[8];
        const int num = FindCDDevices (list, 8);

        decUserCount();

        for (int i = 0; i < num; ++i)
        {
            String s;

            if (list[i].scsiDriveLetter > 0)
                s << String::charToString (list[i].scsiDriveLetter).toUpperCase() << T(": ");

            s << String (list[i].vendor).trim()
              << T(" ") << String (list[i].productId).trim()
              << T(" ") << String (list[i].rev).trim();

            results.add (s);
        }
    }

    return results;
}

static CDDeviceHandle* openHandle (const CDDeviceInfo* const device)
{
    SRB_GDEVBlock s;
    zerostruct (s);

    s.SRB_Cmd = SC_GET_DEV_TYPE;
    s.SRB_HaID = device->ha;
    s.SRB_Target = device->tgt;
    s.SRB_Lun = device->lun;

    if (usingScsi)
    {
        HANDLE h = CreateSCSIDeviceHandle (device->scsiDriveLetter);

        if (h != INVALID_HANDLE_VALUE)
        {
            CDDeviceHandle* cdh = new CDDeviceHandle (device);
            cdh->scsiHandle = h;
            return cdh;
        }
    }
    else
    {
        if (fSendASPI32Command ((LPSRB)&s) == SS_COMP
             && s.SRB_DeviceType == DTYPE_CROM)
        {
            return new CDDeviceHandle (device);
        }
    }

    return 0;
}

AudioCDReader* AudioCDReader::createReaderForCD (const int deviceIndex)
{
    incUserCount();

    if (initialisedOk)
    {
        CDDeviceInfo list[8];
        const int num = FindCDDevices (list, 8);

        if (((unsigned int) deviceIndex) < (unsigned int) num)
        {
            CDDeviceHandle* const handle = openHandle (&(list[deviceIndex]));

            if (handle != 0)
            {
                CDDeviceWrapper* const d = new CDDeviceWrapper();
                d->cdH = handle;
                d->overlapBuffer = new CDReadBuffer(3);

                return new AudioCDReader (d);
            }
        }
    }

    decUserCount();
    return 0;
}

AudioCDReader::AudioCDReader (void* handle_)
    : AudioFormatReader (0, T("CD Audio")),
      handle (handle_),
      indexingEnabled (false),
      lastIndex (0),
      firstFrameInBuffer (0),
      samplesInBuffer (0)
{
    jassert (handle_ != 0);

    refreshTrackLengths();

    sampleRate = 44100.0;
    bitsPerSample = 16;
    lengthInSamples = getPositionOfTrackStart (numTracks);
    numChannels = 2;
    usesFloatingPointData = false;

    buffer.setSize (4 * bytesPerFrame, true);
}

AudioCDReader::~AudioCDReader()
{
    CDDeviceWrapper* const device = (CDDeviceWrapper*)handle;

    delete device->cdH;
    delete device->overlapBuffer;
    delete device;

    decUserCount();
}

bool AudioCDReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                 int64 startSampleInFile, int numSamples)
{
    CDDeviceWrapper* const device = (CDDeviceWrapper*) handle;

    bool ok = true;

    while (numSamples > 0)
    {
        const int bufferStartSample = firstFrameInBuffer * samplesPerFrame;
        const int bufferEndSample = bufferStartSample + samplesInBuffer;

        if (startSampleInFile >= bufferStartSample
             && startSampleInFile < bufferEndSample)
        {
            const int toDo = (int) jmin ((int64) numSamples, bufferEndSample - startSampleInFile);

            int* const l = destSamples[0] + startOffsetInDestBuffer;
            int* const r = numDestChannels > 1 ? (destSamples[1] + startOffsetInDestBuffer) : 0;
            const short* src = (const short*) buffer.getData();
            src += 2 * (startSampleInFile - bufferStartSample);

            for (int i = 0; i < toDo; ++i)
            {
                l[i] = src [i << 1] << 16;

                if (r != 0)
                    r[i] = src [(i << 1) + 1] << 16;
            }

            startOffsetInDestBuffer += toDo;
            startSampleInFile += toDo;
            numSamples -= toDo;
        }
        else
        {
            const int framesInBuffer = buffer.getSize() / bytesPerFrame;
            const int frameNeeded = (int) (startSampleInFile / samplesPerFrame);

            if (firstFrameInBuffer + framesInBuffer != frameNeeded)
            {
                device->overlapBuffer->dataLength = 0;
                device->overlapBuffer->startFrame = 0;
                device->overlapBuffer->numFrames = 0;
                device->jitter = false;
            }

            firstFrameInBuffer = frameNeeded;
            lastIndex = 0;

            CDReadBuffer readBuffer (framesInBuffer + 4);
            readBuffer.wantsIndex = indexingEnabled;

            int i;
            for (i = 5; --i >= 0;)
            {
                readBuffer.startFrame = frameNeeded;
                readBuffer.numFrames = framesInBuffer;

                if (device->cdH->readAudio (&readBuffer, (device->jitter) ? device->overlapBuffer : 0))
                    break;
                else
                    device->overlapBuffer->dataLength = 0;
            }

            if (i >= 0)
            {
                memcpy ((char*) buffer.getData(),
                        readBuffer.buffer + readBuffer.dataStartOffset,
                        readBuffer.dataLength);

                samplesInBuffer = readBuffer.dataLength >> 2;
                lastIndex = readBuffer.index;
            }
            else
            {
                int* l = destSamples[0] + startOffsetInDestBuffer;
                int* r = numDestChannels > 1 ? (destSamples[1] + startOffsetInDestBuffer) : 0;

                while (--numSamples >= 0)
                {
                    *l++ = 0;

                    if (r != 0)
                        *r++ = 0;
                }

                // sometimes the read fails for just the very last couple of blocks, so
                // we'll ignore and errors in the last half-second of the disk..
                ok = startSampleInFile > (trackStarts [numTracks] - 20000);
                break;
            }
        }
    }

    return ok;
}

bool AudioCDReader::isCDStillPresent() const
{
    TOC toc;
    zerostruct (toc);

    return ((CDDeviceWrapper*)handle)->cdH->readTOC (&toc, false);
}

int AudioCDReader::getNumTracks() const
{
    return numTracks;
}

int AudioCDReader::getPositionOfTrackStart (int trackNum) const
{
    return (trackNum >= 0 && trackNum <= numTracks) ? trackStarts [trackNum] * samplesPerFrame
                                                    : 0;
}

void AudioCDReader::refreshTrackLengths()
{
    zeromem (trackStarts, sizeof (trackStarts));
    zeromem (audioTracks, sizeof (audioTracks));

    TOC toc;
    zerostruct (toc);

    if (((CDDeviceWrapper*)handle)->cdH->readTOC (&toc, false))
    {
        numTracks = 1 + toc.lastTrack - toc.firstTrack;

        for (int i = 0; i <= numTracks; ++i)
        {
            trackStarts[i] = getAddressOf (&toc.tracks[i]);
            audioTracks[i] = ((toc.tracks[i].ADR & 4) == 0);
        }
    }
    else
    {
        numTracks = 0;
    }
}

bool AudioCDReader::isTrackAudio (int trackNum) const
{
    return (trackNum >= 0 && trackNum <= numTracks) ? audioTracks [trackNum]
                                                    : false;
}

void AudioCDReader::enableIndexScanning (bool b)
{
    indexingEnabled = b;
}

int AudioCDReader::getLastIndex() const
{
    return lastIndex;
}

const int framesPerIndexRead = 4;

int AudioCDReader::getIndexAt (int samplePos)
{
    CDDeviceWrapper* const device = (CDDeviceWrapper*) handle;

    const int frameNeeded = samplePos / samplesPerFrame;

    device->overlapBuffer->dataLength = 0;
    device->overlapBuffer->startFrame = 0;
    device->overlapBuffer->numFrames = 0;
    device->jitter = false;

    firstFrameInBuffer = 0;
    lastIndex = 0;

    CDReadBuffer readBuffer (4 + framesPerIndexRead);
    readBuffer.wantsIndex = true;

    int i;
    for (i = 5; --i >= 0;)
    {
        readBuffer.startFrame = frameNeeded;
        readBuffer.numFrames = framesPerIndexRead;

        if (device->cdH->readAudio (&readBuffer, (false) ? device->overlapBuffer : 0))
            break;
    }

    if (i >= 0)
        return readBuffer.index;

    return -1;
}

const Array <int> AudioCDReader::findIndexesInTrack (const int trackNumber)
{
    Array <int> indexes;

    const int trackStart = getPositionOfTrackStart (trackNumber);
    const int trackEnd = getPositionOfTrackStart (trackNumber + 1);

    bool needToScan = true;

    if (trackEnd - trackStart > 20 * 44100)
    {
        // check the end of the track for indexes before scanning the whole thing
        needToScan = false;
        int pos = jmax (trackStart, trackEnd - 44100 * 5);
        bool seenAnIndex = false;

        while (pos <= trackEnd - samplesPerFrame)
        {
            const int index = getIndexAt (pos);

            if (index == 0)
            {
                // lead-out, so skip back a bit if we've not found any indexes yet..
                if (seenAnIndex)
                    break;

                pos -= 44100 * 5;

                if (pos < trackStart)
                    break;
            }
            else
            {
                if (index > 0)
                    seenAnIndex = true;

                if (index > 1)
                {
                    needToScan = true;
                    break;
                }

                pos += samplesPerFrame * framesPerIndexRead;
            }
        }
    }

    if (needToScan)
    {
        CDDeviceWrapper* const device = (CDDeviceWrapper*) handle;

        int pos = trackStart;
        int last = -1;

        while (pos < trackEnd - samplesPerFrame * 10)
        {
            const int frameNeeded = pos / samplesPerFrame;

            device->overlapBuffer->dataLength = 0;
            device->overlapBuffer->startFrame = 0;
            device->overlapBuffer->numFrames = 0;
            device->jitter = false;

            firstFrameInBuffer = 0;

            CDReadBuffer readBuffer (4);
            readBuffer.wantsIndex = true;

            int i;
            for (i = 5; --i >= 0;)
            {
                readBuffer.startFrame = frameNeeded;
                readBuffer.numFrames = framesPerIndexRead;

                if (device->cdH->readAudio (&readBuffer, (false) ? device->overlapBuffer : 0))
                    break;
            }

            if (i < 0)
                break;

            if (readBuffer.index > last && readBuffer.index > 1)
            {
                last = readBuffer.index;
                indexes.add (pos);
            }

            pos += samplesPerFrame * framesPerIndexRead;
        }

        indexes.removeValue (trackStart);
    }

    return indexes;
}

int AudioCDReader::getCDDBId()
{
    refreshTrackLengths();

    if (numTracks > 0)
    {
        TOC toc;
        zerostruct (toc);

        if (((CDDeviceWrapper*) handle)->cdH->readTOC (&toc, true))
        {
            int n = 0;

            for (int i = numTracks; --i >= 0;)
            {
                int j = getMSFAddressOf (&toc.tracks[i]);

                while (j > 0)
                {
                    n += (j % 10);
                    j /= 10;
                }
            }

            if (n != 0)
            {
                const int t = getMSFAddressOf (&toc.tracks[numTracks])
                                - getMSFAddressOf (&toc.tracks[0]);

                return ((n % 0xff) << 24) | (t << 8) | numTracks;
            }
        }
    }

    return 0;
}

void AudioCDReader::ejectDisk()
{
    ((CDDeviceWrapper*) handle)->cdH->openDrawer (true);
}

#endif

#if JUCE_USE_CDBURNER

//==============================================================================
static IDiscRecorder* enumCDBurners (StringArray* list, int indexToOpen, IDiscMaster** master)
{
    CoInitialize (0);

    IDiscMaster* dm;
    IDiscRecorder* result = 0;

    if (SUCCEEDED (CoCreateInstance (CLSID_MSDiscMasterObj, 0,
                                     CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                                     IID_IDiscMaster,
                                     (void**) &dm)))
    {
        if (SUCCEEDED (dm->Open()))
        {
            IEnumDiscRecorders* drEnum = 0;

            if (SUCCEEDED (dm->EnumDiscRecorders (&drEnum)))
            {
                IDiscRecorder* dr = 0;
                DWORD dummy;
                int index = 0;

                while (drEnum->Next (1, &dr, &dummy) == S_OK)
                {
                    if (indexToOpen == index)
                    {
                        result = dr;
                        break;
                    }
                    else if (list != 0)
                    {
                        BSTR path;

                        if (SUCCEEDED (dr->GetPath (&path)))
                            list->add ((const WCHAR*) path);
                    }

                    ++index;
                    dr->Release();
                }

                drEnum->Release();
            }

            if (master == 0)
                dm->Close();
        }

        if (master != 0)
            *master = dm;
        else
            dm->Release();
    }

    return result;
}

const StringArray AudioCDBurner::findAvailableDevices()
{
    StringArray devs;
    enumCDBurners (&devs, -1, 0);
    return devs;
}

AudioCDBurner* AudioCDBurner::openDevice (const int deviceIndex)
{
    AudioCDBurner* b = new AudioCDBurner (deviceIndex);

    if (b->internal == 0)
        deleteAndZero (b);

    return b;
}

class CDBurnerInfo  : public IDiscMasterProgressEvents
{
public:
    CDBurnerInfo()
        : refCount (1),
          progress (0),
          shouldCancel (false),
          listener (0)
    {
    }

    ~CDBurnerInfo()
    {
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (result == 0)
            return E_POINTER;

        if (id == IID_IUnknown || id == IID_IDiscMasterProgressEvents)
        {
            AddRef();
            *result = this;
            return S_OK;
        }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { jassert (refCount > 0); const int r = --refCount; if (r == 0) delete this; return r; }

    HRESULT __stdcall QueryCancel (boolean* pbCancel)
    {
        if (listener != 0 && ! shouldCancel)
            shouldCancel = listener->audioCDBurnProgress (progress);

        *pbCancel = shouldCancel;

        return S_OK;
    }

    HRESULT __stdcall NotifyBlockProgress (long nCompleted, long nTotal)
    {
        progress = nCompleted / (float) nTotal;
        shouldCancel = listener != 0 && listener->audioCDBurnProgress (progress);

        return E_NOTIMPL;
    }

    HRESULT __stdcall NotifyPnPActivity (void)                              { return E_NOTIMPL; }
    HRESULT __stdcall NotifyAddProgress (long /*nCompletedSteps*/, long /*nTotalSteps*/)    { return E_NOTIMPL; }
    HRESULT __stdcall NotifyTrackProgress (long /*nCurrentTrack*/, long /*nTotalTracks*/)   { return E_NOTIMPL; }
    HRESULT __stdcall NotifyPreparingBurn (long /*nEstimatedSeconds*/)      { return E_NOTIMPL; }
    HRESULT __stdcall NotifyClosingDisc (long /*nEstimatedSeconds*/)        { return E_NOTIMPL; }
    HRESULT __stdcall NotifyBurnComplete (HRESULT /*status*/)               { return E_NOTIMPL; }
    HRESULT __stdcall NotifyEraseComplete (HRESULT /*status*/)              { return E_NOTIMPL; }

    IDiscMaster* discMaster;
    IDiscRecorder* discRecorder;
    IRedbookDiscMaster* redbook;
    AudioCDBurner::BurnProgressListener* listener;
    float progress;
    bool shouldCancel;

private:
    int refCount;
};

AudioCDBurner::AudioCDBurner (const int deviceIndex)
    : internal (0)
{
    IDiscMaster* discMaster;
    IDiscRecorder* dr = enumCDBurners (0, deviceIndex, &discMaster);

    if (dr != 0)
    {
        IRedbookDiscMaster* redbook;
        HRESULT hr = discMaster->SetActiveDiscMasterFormat (IID_IRedbookDiscMaster, (void**) &redbook);

        hr = discMaster->SetActiveDiscRecorder (dr);

        CDBurnerInfo* const info = new CDBurnerInfo();
        internal = info;

        info->discMaster = discMaster;
        info->discRecorder = dr;
        info->redbook = redbook;
    }
}

AudioCDBurner::~AudioCDBurner()
{
    CDBurnerInfo* const info = (CDBurnerInfo*) internal;

    if (info != 0)
    {
        info->discRecorder->Close();
        info->redbook->Release();
        info->discRecorder->Release();
        info->discMaster->Release();

        info->Release();
    }
}

bool AudioCDBurner::isDiskPresent() const
{
    CDBurnerInfo* const info = (CDBurnerInfo*) internal;

    HRESULT hr = info->discRecorder->OpenExclusive();

    long type, flags;
    hr = info->discRecorder->QueryMediaType (&type, &flags);

    info->discRecorder->Close();
    return hr == S_OK && type != 0 && (flags & MEDIA_WRITABLE) != 0;
}

int AudioCDBurner::getNumAvailableAudioBlocks() const
{
    CDBurnerInfo* const info = (CDBurnerInfo*) internal;
    long blocksFree = 0;
    info->redbook->GetAvailableAudioTrackBlocks (&blocksFree);
    return blocksFree;
}

const String AudioCDBurner::burn (AudioCDBurner::BurnProgressListener* listener,
                                  const bool ejectDiscAfterwards,
                                  const bool performFakeBurnForTesting)
{
    CDBurnerInfo* const info = (CDBurnerInfo*) internal;

    info->listener = listener;
    info->progress = 0;
    info->shouldCancel = false;

    UINT_PTR cookie;
    HRESULT hr = info->discMaster->ProgressAdvise (info, &cookie);

    hr = info->discMaster->RecordDisc (performFakeBurnForTesting,
                                       ejectDiscAfterwards);

    String error;
    if (hr != S_OK)
    {
        const char* e = "Couldn't open or write to the CD device";

        if (hr == IMAPI_E_USERABORT)
            e = "User cancelled the write operation";
        else if (hr == IMAPI_E_MEDIUM_NOTPRESENT || hr == IMAPI_E_TRACKOPEN)
            e = "No Disk present";

        error = e;
    }

    info->discMaster->ProgressUnadvise (cookie);
    info->listener = 0;

    return error;
}

bool AudioCDBurner::addAudioTrack (AudioSource* source, int numSamples)
{
    if (source == 0)
        return false;

    CDBurnerInfo* const info = (CDBurnerInfo*) internal;

    long bytesPerBlock;
    HRESULT hr = info->redbook->GetAudioBlockSize (&bytesPerBlock);

    const int samplesPerBlock = bytesPerBlock / 4;
    bool ok = true;

    hr = info->redbook->CreateAudioTrack ((long) numSamples / (bytesPerBlock * 4));

    HeapBlock <byte> buffer (bytesPerBlock);

    AudioSampleBuffer sourceBuffer (2, samplesPerBlock);
    int samplesDone = 0;

    source->prepareToPlay (samplesPerBlock, 44100.0);

    while (ok)
    {
        {
            AudioSourceChannelInfo info;
            info.buffer = &sourceBuffer;
            info.numSamples = samplesPerBlock;
            info.startSample = 0;
            sourceBuffer.clear();

            source->getNextAudioBlock (info);
        }

        zeromem (buffer, bytesPerBlock);

        AudioDataConverters::convertFloatToInt16LE (sourceBuffer.getSampleData (0, 0),
                                                    buffer, samplesPerBlock, 4);

        AudioDataConverters::convertFloatToInt16LE (sourceBuffer.getSampleData (1, 0),
                                                    buffer + 2, samplesPerBlock, 4);

        hr = info->redbook->AddAudioTrackBlocks (buffer, bytesPerBlock);

        if (hr != S_OK)
            ok = false;

        samplesDone += samplesPerBlock;

        if (samplesDone >= numSamples)
            break;
    }

    hr = info->redbook->CloseAudioTrack();

    delete source;

    return ok && hr == S_OK;
}

#endif
#endif
