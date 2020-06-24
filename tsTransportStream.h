#pragma once
#include "tsCommon.h"
#include <string>
#include <bitset>




/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/


//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================

class xTS
{
public:
    static constexpr uint32_t TS_PacketLength = 188;
    static constexpr uint32_t TS_HeaderLength = 4;

    static constexpr uint32_t PES_HeaderLength = 6;

    static constexpr uint32_t BaseClockFrequency_Hz = 90000; //Hz
    static constexpr uint32_t ExtendedClockFrequency_Hz = 27000000; //Hz
    static constexpr uint32_t BaseClockFrequency_kHz = 90; //kHz
    static constexpr uint32_t ExtendedClockFrequency_kHz = 27000; //kHz
    static constexpr uint32_t BaseToExtendedClockMultiplier = 300;


    //Input - input stream
    //startByte - :)
    //count - nubmer of bytes to return
    static std::string getBitStream(const uint8_t* _Input, size_t _startByte, size_t _count);
    static constexpr uint32_t TS_AdaptationFieldLengthByte = 4; //of course it's 5

};



//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================

class xTS_PacketHeader
{
public:
    enum class ePID : uint16_t
    {
        PAT = 0x0000,
        CAT = 0x0001,
        TSDT = 0x0002,
        IPMT = 0x0003,
        NIT = 0x0010, //DVB specific PID
        SDT = 0x0011, //DVB specific PID
        NuLL = 0x1FFF,
    };

protected:
    std::bitset<8>  m_Sync_byte;
    std::bitset<1>  m_Transport_error_indicator;
    std::bitset<1>  m_Payload_unit_start_indicator;
    std::bitset<1>  m_Transport_priority;
    std::bitset<13> m_PID;
    std::bitset<2>  m_Transport_scrambling_control;
    std::bitset<2>  m_Adaptation_field_control;
    std::bitset<4>  m_Continuity_counter;

    std::istringstream H_bit_stream;

public:
    void     Reset();
    int32_t  Parse(const uint8_t* Input);
    void     Print() const;

public:
    //direct acces to header values
    uint8_t  getSyncByte()                   const;
    uint8_t  getTransportErrorIndicator()    const;
    uint8_t  getPayloadUnitStartIndicator()  const;
    uint8_t  getTransportPriority()          const;
    uint16_t getPID()                        const;
    uint8_t  getTransportScramblingControl() const;
    uint8_t  getAdaptationFieldControl()     const;
    uint8_t  getContinuityCounter()          const;

public:
    bool     hasAdaptationField() const { return (m_Adaptation_field_control == 0b10 || m_Adaptation_field_control == 0b11) ? true : false; }
    bool     hasPayload()         const { return (m_Adaptation_field_control == 0b01 || m_Adaptation_field_control == 0b11) ? true : false; }
};


//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================


class xTS_AdaptationField
{
public:
    xTS_AdaptationField();
protected:
    //AF lenght
    std::bitset<8>  m_Adaptation_field_length;
    //mandatory field
    std::bitset<1>  m_Discontinuity_indicator;
    std::bitset<1>  m_Random_access_indicator;
    std::bitset<1>  m_Elementary_stream_priority_indicator;
    std::bitset<1>  m_PCR_flag;
    std::bitset<1>  m_OPCR_flag;
    std::bitset<1>  m_Splicing_point_flag;
    std::bitset<1>  m_Transport_private_data_flag;
    std::bitset<1>  m_Adaptation_field_extension_flag;

    //zad dod 1 - optional fields
    //PCR
    std::bitset<33> m_Program_clock_reference_base;
    std::bitset<6>  m_Program_clock_reference_reserved;
    std::bitset<9>  m_Program_clock_reference_extension;
    //OPCR
    std::bitset<33> m_Original_program_clock_reference_base;
    std::bitset<6>  m_Original_program_clock_reference_reserved;
    std::bitset<9>  m_Original_program_clock_reference_extension;

    std::bitset<8>  m_Splice_countdown;

    std::bitset<8>  m_Transport_private_data_length;
    uint8_t* m_Transport_private_data;
    //AdaptationFieldExtension
    std::bitset<8>  m_Adaptation_field_extension_length;
    std::bitset<1>  m_Ltw_flag;
    std::bitset<1>  m_Piecewise_rate_flag;
    std::bitset<1>  m_Seamless_splice_flag;
    std::bitset<5>  m_Adaptation_field_extension_reserved;
    //LTW
    std::bitset<1>  m_Ltw_valid_flag;
    std::bitset<15> m_Ltw_offset;
    //PiecewiseRate
    std::bitset<2>  m_Piecewise_rate_reserved;
    std::bitset<22> m_Piecewise_rate;
    //SeamlessSplice
    std::bitset<4>  m_Splice_type;
    std::bitset<36> m_DTS_next_access_unit; //do it later

    //stuffing bytes
    size_t          m_Stuffing_bytes_length;
    uint8_t* m_Stuffing_bytes;

    std::istringstream AF_bit_stream;

public:
    void    Reset();
    int32_t Parse(const uint8_t* Input, uint8_t AdapdationFieldControl);
    void    Print() const;

public:
    //derrived values
    uint32_t getNumBytes() const;
    uint8_t  getAdaptationFieldLength() const;
};


//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================


class xPES_PacketHeader
{
public:
    enum eStreamId : uint8_t
    {
        eStreamId_program_stream_map = 0xBC,
        eStreamId_padding_stream = 0xBE,
        eStreamId_private_stream_2 = 0xBF,
        eStreamId_ECM = 0xF0,
        eStreamId_EMM = 0xF1,
        eStreamId_program_stream_directory = 0xFF,
        eStreamId_DSMCC_stream = 0xF2,
        eStreamId_ITUT_H222_1_type_E = 0xF8,
    };

protected:
    //PES packet header
    std::bitset<24> m_Packet_start_code_prefix;
    std::bitset<8>  m_Stream_id;
    std::bitset<16> m_PES_packet_length;
    //Optional PES header - zad dod 2
    std::bitset<2>  m_Marker_bits;
    std::bitset<2>  m_PES_scrambling_control;
    std::bitset<1>  m_PES_priority;
    std::bitset<1>  m_Data_alignment_indicator;
    std::bitset<1>  m_Copyright;
    std::bitset<1>  m_Original_or_copy;
    std::bitset<2>  m_PTS_DTS_flags;
    std::bitset<1>  m_ESCR_flag;
    std::bitset<1>  m_ES_rate_flag;
    std::bitset<1>  m_DSM_trick_mode_flag;
    std::bitset<1>  m_Additional_copy_info_flag;
    std::bitset<1>  m_PES_CRC_flag;
    std::bitset<1>  m_PES_extension_flag;
    std::bitset<8>  m_PES_header_data_length;

    std::bitset<4>  m_PTS_junk;
    std::bitset<1>  m_PTS_marker;
    std::bitset<3>  m_PTS_32_30;
    std::bitset<15> m_PTS_29_15;
    std::bitset<15> m_PTS_14_0;

    std::bitset<4>  m_DTS_junk;
    std::bitset<1>  m_DTS_marker;
    std::bitset<3>  m_DTS_32_30;
    std::bitset<15> m_DTS_29_15;
    std::bitset<15> m_DTS_14_0;

    std::bitset<48> m_ESCR_data;

    std::bitset<24> m_ES_rate_data;

    std::istringstream PES_bit_stream;


public:
    void     Reset();
    int32_t  Parse(const uint8_t* Input);
    void     Print() const;

public:
    //PES packet header
    uint32_t getPacketStartCodePrefix() const { return m_Packet_start_code_prefix.to_ulong(); }
    uint8_t  getStreamId()              const { return m_Stream_id.to_ulong(); }
    uint16_t getPacketLength()          const { return m_PES_packet_length.to_ulong(); }
    uint16_t getHeaderDataLength()      const { return m_PES_header_data_length.to_ulong() + 3; }
};

//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================


class xPES_Assembler
{
public:
    enum class eResult : int32_t
    {
        UnexpectedPID = 1,
        StreamPackedLost,
        AssemblingStarted,
        AssemblingContinue,
        AssemblingFinished,
    };

protected:
    //setup
    int32_t           m_PID;
    //buffer
    uint8_t* m_Buffer;
    uint32_t          m_BufferSize;
    uint32_t          m_DataOffset;
    //operation
    int8_t            m_LastContinuityCounter;
    bool              m_Started;
    xPES_PacketHeader m_PESH;

    //file
    const char* m_Filename;
    FILE* m_File;

public:
    xPES_Assembler();
    ~xPES_Assembler();

    void    Init(int32_t PID, std::string file_extension);
    eResult AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField);

    void     PrintPESH()        const { m_PESH.Print(); }
    uint8_t* getPacket() { return &m_Buffer[this->getHeaderLength()]; }
    int32_t  getPacketLength()  const { return m_DataOffset; }
    int32_t  getHeaderLength()  const { return m_PESH.getHeaderDataLength() + xTS::PES_HeaderLength; }
    int32_t  getDataLength()    const { return m_DataOffset - this->getHeaderLength(); }
    //
    const char* getFilename()   const { return m_Filename; }
    FILE* getFileHandler()      const { return m_File; }

protected:
    void xBufferReset();
    void xBufferAppend(const uint8_t* Data, int32_t Size);
};