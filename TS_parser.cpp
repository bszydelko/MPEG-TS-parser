#pragma once

#include "tsCommon.h"
#include "tsTransportStream.h"

std::ofstream log_file("../log.txt");


int main(int argc, char* argv[], char* envp[])
{
    const char* filename = "../example_new.ts";
    FILE* TransportStreamFile;
    fopen_s(&TransportStreamFile, filename, "rb");

    if (TransportStreamFile == NULL)
    {
        printf("wrong file name\n");
        return EXIT_FAILURE;
    }

    uint8_t             TS_PacketBuffer[xTS::TS_PacketLength];
    xTS_PacketHeader    TS_PacketHeader;
    xTS_AdaptationField TS_PacketAdaptationField;
    xPES_Assembler      PES_Assembler_PID_136;
    xPES_Assembler      PES_Assembler_PID_174;

    const int PID136 = 136;
    const int PID174 = 174;
    int currentPID = 0;

    PES_Assembler_PID_136.Init(PID136, ".mp2");
    PES_Assembler_PID_174.Init(PID174, ".264");

    int32_t TS_PacketId = 0;

    while (!feof(TransportStreamFile))
    {
        size_t NumRead = fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, TransportStreamFile);

        if (NumRead != xTS::TS_PacketLength) break;


        TS_PacketHeader.Reset();
        TS_PacketHeader.Parse(TS_PacketBuffer);

        TS_PacketAdaptationField.Reset();

        if (TS_PacketHeader.getSyncByte() == 'G' && (TS_PacketHeader.getPID() == PID136 || TS_PacketHeader.getPID() == PID174))
        {
            switch (TS_PacketHeader.getPID())
            {
            case (PID136):
            {
                if (TS_PacketHeader.hasAdaptationField())
                {
                    TS_PacketAdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAdaptationFieldControl());
                }

                log_file << std::setfill('0') << std::setw(10) << TS_PacketId;

                TS_PacketHeader.Print();

                if (TS_PacketHeader.hasAdaptationField())
                {
                    TS_PacketAdaptationField.Print();
                }

                xPES_Assembler::eResult Result = PES_Assembler_PID_136.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_PacketAdaptationField);

                switch (Result)
                {
                case xPES_Assembler::eResult::StreamPackedLost:
                {
                    log_file << "PcktLost ";
                    break;
                }
                case xPES_Assembler::eResult::AssemblingStarted:
                {
                    log_file << "Started  ";
                    PES_Assembler_PID_136.PrintPESH();
                    break;
                }
                case xPES_Assembler::eResult::AssemblingContinue:
                {
                    log_file << "Continue ";
                    break;
                }
                case xPES_Assembler::eResult::AssemblingFinished:
                {
                    fwrite(PES_Assembler_PID_136.getPacket(), sizeof(char), PES_Assembler_PID_136.getDataLength(), PES_Assembler_PID_136.getFileHandler());
                    log_file << "Finished ";
                    log_file << "PES: PcktLen=" << PES_Assembler_PID_136.getPacketLength();
                    log_file << " HeadLen=" << PES_Assembler_PID_136.getHeaderLength();
                    log_file << " DataLen=" << PES_Assembler_PID_136.getDataLength();
                    break;
                }
                default:
                    break;
                }

                break;
            }
            case(PID174):
            {
                if (TS_PacketHeader.hasAdaptationField())
                {
                    TS_PacketAdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAdaptationFieldControl());
                }

                //printf("%010d ", TS_PacketId);
                log_file << std::setfill('0') << std::setw(10) << TS_PacketId;


                TS_PacketHeader.Print();

                if (TS_PacketHeader.hasAdaptationField())
                {
                    TS_PacketAdaptationField.Print();
                }

                xPES_Assembler::eResult Result = PES_Assembler_PID_174.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_PacketAdaptationField);

                switch (Result)
                {
                case xPES_Assembler::eResult::StreamPackedLost:
                {
                    log_file << "PcktLost ";
                    break;
                }
                case xPES_Assembler::eResult::AssemblingStarted:
                {
                    log_file << "Started  ";
                    PES_Assembler_PID_174.PrintPESH();
                    break;
                }
                case xPES_Assembler::eResult::AssemblingContinue:
                {
                    log_file << "Continue ";
                    break;
                }
                case xPES_Assembler::eResult::AssemblingFinished:
                {
                    fwrite(PES_Assembler_PID_174.getPacket(), sizeof(char), PES_Assembler_PID_174.getDataLength(), PES_Assembler_PID_174.getFileHandler());
                    log_file << "Finished ";
                    log_file << "PES: PcktLen=" << PES_Assembler_PID_174.getPacketLength();
                    log_file << " HeadLen=" << PES_Assembler_PID_174.getHeaderLength();
                    log_file << " DataLen=" << PES_Assembler_PID_174.getDataLength();
                    break;
                }
                default:
                    break;
                }

                break;
            }
            }

            log_file << std::endl;

        }
        TS_PacketId++;
    }
}


//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================

std::string xTS::getBitStream(const uint8_t* _Input, size_t _startByte, size_t _count)
{
    std::string c_bin = "";
    std::string s_bin = "";
    int bit = 0;
    int c = 0;

    for (size_t byte = _startByte; byte < _count + _startByte; byte++)
    {
        c_bin = "";
        c = int(_Input[byte]);

        bit = 0;
        while (bit < CHAR_BIT)
        {
            (c % 2) ? c_bin.push_back('1') : c_bin.push_back('0');
            c >>= 1;
            bit++;
        }
        std::reverse(c_bin.begin(), c_bin.end());
        s_bin.append(c_bin);
    }

    return s_bin;
}

//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================

void xTS_PacketHeader::Reset()
{
    m_Sync_byte.reset();
    m_Transport_error_indicator.reset();
    m_Payload_unit_start_indicator.reset();
    m_Transport_priority.reset();
    m_PID.reset();
    m_Transport_scrambling_control.reset();
    m_Adaptation_field_control.reset();
    m_Continuity_counter.reset();
}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input)
{
    H_bit_stream.str(xTS::getBitStream(Input, 0, xTS::TS_HeaderLength));

    H_bit_stream >> m_Sync_byte;
    H_bit_stream >> m_Transport_error_indicator;
    H_bit_stream >> m_Payload_unit_start_indicator;
    H_bit_stream >> m_Transport_priority;
    H_bit_stream >> m_PID;
    H_bit_stream >> m_Transport_scrambling_control;
    H_bit_stream >> m_Adaptation_field_control;
    H_bit_stream >> m_Continuity_counter;

    //rownie dobrze reszte bitstreamu mozna zapisac i udostepniac pozostalym klasom

    return int32_t();
}

void xTS_PacketHeader::Print() const
{
    log_file <<
        "TS:" <<
        " SB=" << m_Sync_byte.to_ulong() <<
        " E=" << m_Transport_error_indicator.to_ulong() <<
        " S=" << m_Payload_unit_start_indicator.to_ulong() <<
        " P=" << m_Transport_priority.to_ulong() <<
        " PID=" << std::setw(4) << m_PID.to_ulong() <<
        " TSC=" << m_Transport_scrambling_control.to_ulong() <<
        " AFC=" << m_Adaptation_field_control.to_ulong() <<
        " CC=" << std::setw(2) << m_Continuity_counter.to_ulong() << " ";

}

uint8_t xTS_PacketHeader::getSyncByte() const
{
    return uint8_t(m_Sync_byte.to_ulong());
}

uint8_t xTS_PacketHeader::getTransportErrorIndicator() const
{
    return uint8_t(m_Transport_error_indicator.to_ulong());
}

uint8_t xTS_PacketHeader::getPayloadUnitStartIndicator() const
{
    return uint8_t(m_Payload_unit_start_indicator.to_ulong());
}

uint8_t xTS_PacketHeader::getTransportPriority() const
{
    return uint8_t(m_Transport_priority.to_ulong());
}

uint16_t xTS_PacketHeader::getPID() const
{
    return uint16_t(m_PID.to_ulong());
}

uint8_t xTS_PacketHeader::getTransportScramblingControl() const
{
    return uint8_t(m_Transport_scrambling_control.to_ulong());
}

uint8_t xTS_PacketHeader::getAdaptationFieldControl() const
{
    return uint8_t(m_Adaptation_field_control.to_ulong());
}
uint8_t xTS_PacketHeader::getContinuityCounter() const
{
    return uint8_t(m_Continuity_counter.to_ulong());
}



//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================



xTS_AdaptationField::xTS_AdaptationField()
{
    m_Transport_private_data = nullptr;
    m_Stuffing_bytes_length = 0;
    m_Stuffing_bytes = nullptr;
}

void xTS_AdaptationField::Reset()
{
    m_Adaptation_field_length.reset();
    m_Discontinuity_indicator.reset();
    m_Random_access_indicator.reset();
    m_Elementary_stream_priority_indicator.reset();
    m_PCR_flag.reset();
    m_OPCR_flag.reset();
    m_Splicing_point_flag.reset();
    m_Transport_private_data_flag.reset();
    m_Adaptation_field_extension_flag.reset();

    m_Program_clock_reference_base.reset();
    m_Program_clock_reference_reserved.reset();
    m_Program_clock_reference_extension.reset();

    m_Original_program_clock_reference_base.reset();
    m_Original_program_clock_reference_reserved.reset();
    m_Original_program_clock_reference_extension.reset();

    m_Splice_countdown.reset();

    m_Transport_private_data_length.reset();

    m_Adaptation_field_extension_length.reset();
    //delete transport_private_data;

    m_Ltw_flag.reset();
    m_Piecewise_rate_flag.reset();
    m_Seamless_splice_flag.reset();
    m_Adaptation_field_extension_reserved.reset();

    m_Ltw_valid_flag.reset();
    m_Ltw_offset.reset();

    m_Piecewise_rate_reserved.reset();
    m_Piecewise_rate.reset();

    m_Splice_type.reset();
    m_DTS_next_access_unit.reset(); //do it later

    //optional_fields.reset();
    /*if (stuffing_bytes != nullptr)
        delete[] stuffing_bytes;*/

}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdapdationFieldControl)
{
    m_Adaptation_field_length = Input[xTS::TS_AdaptationFieldLengthByte];
    m_Stuffing_bytes_length = m_Adaptation_field_length.to_ulong() - 1; //substracted in IF statements

    AF_bit_stream.str(xTS::getBitStream(Input, xTS::TS_AdaptationFieldLengthByte + 1, m_Adaptation_field_length.to_ulong()));

    AF_bit_stream >> m_Discontinuity_indicator;
    AF_bit_stream >> m_Random_access_indicator;
    AF_bit_stream >> m_Elementary_stream_priority_indicator;
    AF_bit_stream >> m_PCR_flag;
    AF_bit_stream >> m_OPCR_flag;
    AF_bit_stream >> m_Splicing_point_flag;
    AF_bit_stream >> m_Transport_private_data_flag;
    AF_bit_stream >> m_Adaptation_field_extension_flag;

    //potrzeba sprawdzenia na "bogatszym" w pola pakiet
    if (m_PCR_flag == 0b1)
    {
        AF_bit_stream >> m_Program_clock_reference_base;
        AF_bit_stream >> m_Program_clock_reference_reserved;
        AF_bit_stream >> m_Program_clock_reference_extension;

        m_Stuffing_bytes_length -= 6;
    }
    if (m_OPCR_flag == 0b1)
    {
        AF_bit_stream >> m_Original_program_clock_reference_base;
        AF_bit_stream >> m_Original_program_clock_reference_reserved;
        AF_bit_stream >> m_Original_program_clock_reference_extension;

        m_Stuffing_bytes_length -= 6;
    }
    if (m_Splicing_point_flag == 0b1)
    {
        AF_bit_stream >> m_Splice_countdown;

        m_Stuffing_bytes_length -= 1;
    }

    if (m_Transport_private_data_flag == 0b1)
    {
        AF_bit_stream >> m_Transport_private_data_length;
        m_Transport_private_data = new uint8_t[m_Transport_private_data_length.to_ulong()];
        AF_bit_stream >> m_Transport_private_data;

        m_Stuffing_bytes_length -= m_Transport_private_data_length.to_ulong();
    }
    if (m_Adaptation_field_extension_flag == 0b1)
    {
        AF_bit_stream >> m_Adaptation_field_extension_length;
        AF_bit_stream >> m_Ltw_flag;
        AF_bit_stream >> m_Piecewise_rate_flag;
        AF_bit_stream >> m_Seamless_splice_flag;
        AF_bit_stream >> m_Adaptation_field_extension_reserved;

        m_Stuffing_bytes_length -= m_Adaptation_field_extension_length.to_ulong() - 1;

        if (m_Ltw_flag == 0b1)
        {
            AF_bit_stream >> m_Ltw_valid_flag;
            AF_bit_stream >> m_Ltw_offset;

            m_Stuffing_bytes_length -= 2;
        }
        if (m_Piecewise_rate_flag == 0b1)
        {
            AF_bit_stream >> m_Piecewise_rate_reserved;
            AF_bit_stream >> m_Piecewise_rate;

            m_Stuffing_bytes_length -= 3;
        }
        if (m_Seamless_splice_flag == 0b1)
        {
            AF_bit_stream >> m_Splice_type;
            AF_bit_stream >> m_DTS_next_access_unit;

            m_Stuffing_bytes_length -= 5;
        }

    }
    /*if (stuffing_bytes_length > 0)
    {
        stuffing_bytes = new uint8_t[stuffing_bytes_length];
        for (size_t byte = 0; byte < stuffing_bytes_length; byte++)
        {
            AF_bit_stream >> stuffing_bytes[byte];
        }
    }*/
    return int32_t();
}

void xTS_AdaptationField::Print() const
{
    log_file <<
        "AF:" <<
        " L=" << std::setw(3) << m_Adaptation_field_length.to_ulong() <<
        " DC=" << m_Discontinuity_indicator <<
        " RA=" << m_Random_access_indicator <<
        " SPI=" << m_Elementary_stream_priority_indicator <<
        " PR=" << m_PCR_flag <<
        " OR=" << m_OPCR_flag <<
        " SPF=" << m_Splicing_point_flag <<
        " TP=" << m_Transport_private_data_flag <<
        " EX=" << m_Adaptation_field_extension_flag;

    if (m_PCR_flag == 0b1)
    {
        uint32_t PCR = m_Program_clock_reference_base.to_ulong() * xTS::BaseToExtendedClockMultiplier + m_Program_clock_reference_extension.to_ulong();
        double time = (double)PCR / (double)xTS::ExtendedClockFrequency_Hz; //wtf it works XD
        log_file << " PCR=" << PCR << " (Time=" << std::setprecision(7) << time << "s)";
    }

    if (m_OPCR_flag == 0b1)
    {
        uint32_t OPCR = m_Original_program_clock_reference_base.to_ulong() * xTS::BaseToExtendedClockMultiplier + m_Original_program_clock_reference_extension.to_ulong();
        double time = (double)OPCR / (double)xTS::ExtendedClockFrequency_Hz;
        log_file << " OPCR=" << OPCR << " (Time=" << std::setprecision(7) << time << "s)";
    }
    log_file << " Stuffing=" << m_Stuffing_bytes_length << " ";

}

uint32_t xTS_AdaptationField::getNumBytes() const
{
    return uint32_t();
}

uint8_t xTS_AdaptationField::getAdaptationFieldLength() const
{
    return uint8_t(m_Adaptation_field_length.to_ulong());
}

//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================



void xPES_PacketHeader::Reset()
{
    m_Packet_start_code_prefix.reset();
    m_Stream_id.reset();
    m_PES_packet_length.reset();
}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input)
{
    PES_bit_stream.str(xTS::getBitStream(Input, 0, xTS::PES_HeaderLength));

    PES_bit_stream >> m_Packet_start_code_prefix;
    PES_bit_stream >> m_Stream_id;
    PES_bit_stream >> m_PES_packet_length;

    if (m_Stream_id != eStreamId::eStreamId_program_stream_map
        && m_Stream_id != eStreamId::eStreamId_padding_stream
        && m_Stream_id != eStreamId::eStreamId_private_stream_2
        && m_Stream_id != eStreamId::eStreamId_ECM
        && m_Stream_id != eStreamId::eStreamId_EMM
        && m_Stream_id != eStreamId::eStreamId_program_stream_directory
        && m_Stream_id != eStreamId::eStreamId_DSMCC_stream
        && m_Stream_id != eStreamId::eStreamId_ITUT_H222_1_type_E)
    {
        //header length is not known, read 3 bytes for flags
        PES_bit_stream.str(xTS::getBitStream(Input, xTS::PES_HeaderLength, 3));

        PES_bit_stream >> m_Marker_bits;
        PES_bit_stream >> m_PES_scrambling_control;
        PES_bit_stream >> m_PES_priority;
        PES_bit_stream >> m_Data_alignment_indicator;
        PES_bit_stream >> m_Copyright;
        PES_bit_stream >> m_Original_or_copy;

        PES_bit_stream >> m_PTS_DTS_flags;
        PES_bit_stream >> m_ESCR_flag;
        PES_bit_stream >> m_ES_rate_flag;
        PES_bit_stream >> m_DSM_trick_mode_flag;
        PES_bit_stream >> m_Additional_copy_info_flag;
        PES_bit_stream >> m_PES_CRC_flag;
        PES_bit_stream >> m_PES_extension_flag;
        PES_bit_stream >> m_PES_header_data_length;

        // header length is known
        PES_bit_stream.str(xTS::getBitStream(Input, xTS::PES_HeaderLength + 3, m_PES_header_data_length.to_ulong()));


        if (m_PTS_DTS_flags == 0b10)
        {
            PES_bit_stream >> m_PTS_junk; //'0010'
            PES_bit_stream >> m_PTS_32_30;
            PES_bit_stream >> m_PTS_marker;
            PES_bit_stream >> m_PTS_29_15;
            PES_bit_stream >> m_PTS_marker;
            PES_bit_stream >> m_PTS_14_0;
            PES_bit_stream >> m_PTS_marker;

        }

        if (m_PTS_DTS_flags == 0b11)
        {
            PES_bit_stream >> m_PTS_junk; //'0010'
            PES_bit_stream >> m_PTS_32_30;
            PES_bit_stream >> m_PTS_marker;
            PES_bit_stream >> m_PTS_29_15;
            PES_bit_stream >> m_PTS_marker;
            PES_bit_stream >> m_PTS_14_0;
            PES_bit_stream >> m_PTS_marker;

            PES_bit_stream >> m_DTS_junk; //'0001'
            PES_bit_stream >> m_DTS_32_30;
            PES_bit_stream >> m_DTS_marker;
            PES_bit_stream >> m_DTS_29_15;
            PES_bit_stream >> m_DTS_marker;
            PES_bit_stream >> m_DTS_14_0;
            PES_bit_stream >> m_DTS_marker;
        }

        if (m_ESCR_flag == 0b1)
        {
            PES_bit_stream >> m_ESCR_data;
        }

        if (m_ES_rate_flag == 0b1)
        {
            PES_bit_stream >> m_ES_rate_data;
        }

        //rest of the fields blah blah blah
    }
    else if (m_Stream_id == eStreamId::eStreamId_program_stream_map
        || m_Stream_id == eStreamId::eStreamId_private_stream_2
        || m_Stream_id == eStreamId::eStreamId_ECM
        || m_Stream_id == eStreamId::eStreamId_EMM
        || m_Stream_id == eStreamId::eStreamId_program_stream_directory
        || m_Stream_id == eStreamId::eStreamId_DSMCC_stream
        || m_Stream_id == eStreamId::eStreamId_ITUT_H222_1_type_E)
    {
        //to powinno byc jako parsowanie calego pakietu a nie naglowka
        for (size_t i = 0; i < m_PES_packet_length.to_ulong(); i++)
        {

        }
    }
    else if (m_Stream_id == eStreamId::eStreamId_padding_stream)
    {
        //tu tak samo jak wyzej
        for (size_t i = 0; i < m_PES_packet_length.to_ulong(); i++)
        {

        }
    }

    return int32_t();
}

void xPES_PacketHeader::Print() const
{
    log_file << "PES:"
        << " PSCP=" << m_Packet_start_code_prefix.to_ulong()
        << " SID=" << m_Stream_id.to_ulong()
        << " L=" << m_PES_packet_length.to_ulong();

    if (m_PTS_DTS_flags == 0b10)
    {
        uint32_t PES = std::bitset<33>(std::string(m_PTS_32_30.to_string() + m_PTS_29_15.to_string() + m_PTS_14_0.to_string())).to_ulong();
        float PES_time = float(PES) / float(xTS::BaseClockFrequency_Hz);

        log_file << " PTS=" << PES << "(Time=" << PES_time << "s)";
    }

    if (m_PTS_DTS_flags == 0b11)
    {
        uint32_t PES = std::bitset<33>(std::string(m_PTS_32_30.to_string() + m_PTS_29_15.to_string() + m_PTS_14_0.to_string())).to_ulong();
        uint32_t DES = std::bitset<33>(std::string(m_DTS_32_30.to_string() + m_DTS_29_15.to_string() + m_DTS_14_0.to_string())).to_ulong();

        float PES_time = float(PES) / float(xTS::BaseClockFrequency_Hz);
        float DES_time = float(DES) / float(xTS::BaseClockFrequency_Hz);

        log_file << " PTS=" << PES << "(Time=" << PES_time << "s)" << " DTS=" << DES << "(Time=" << DES_time << "s)";
    }
}

//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================
//=============================================================================================================================================================================



xPES_Assembler::xPES_Assembler()
{
    m_BufferSize = 0;
    m_Started = false;
    m_PID = -1;
    m_LastContinuityCounter = -1;
    m_DataOffset = xTS::PES_HeaderLength;

    m_File = nullptr;
    m_Filename = "";

    m_Buffer = nullptr;
}

xPES_Assembler::~xPES_Assembler()
{
    fclose(m_File);
    delete[] m_Buffer;
}

void xPES_Assembler::Init(int32_t PID, std::string file_extension)
{
    m_PID = PID;
    std::string name_builder = "../PID" + std::to_string(PID) + file_extension;
    m_Filename = name_builder.c_str();
    fopen_s(&m_File, m_Filename, "wb");
}

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket
(
    const uint8_t* TransportStreamPacket,
    const xTS_PacketHeader* PacketHeader,
    const xTS_AdaptationField* AdaptationField)
{
    int32_t start_byte = 0;
    int32_t payload_size = 0;

    if (m_LastContinuityCounter != -1
        && (PacketHeader->getContinuityCounter() - m_LastContinuityCounter != 1
            && (int32_t)PacketHeader->getContinuityCounter() - m_LastContinuityCounter != -15))
    {
        xBufferReset();
        return eResult::StreamPackedLost;
    }

    //start of PES packet, parse PES header
    if (PacketHeader->getPayloadUnitStartIndicator() == 1)
    {
        if (m_Started && m_PESH.getPacketLength() == 0)
        {
            m_Started = false;
            size_t writed = fwrite(&m_Buffer[this->getHeaderLength()], sizeof(char), this->getDataLength(), m_File);
            xBufferReset();
        }

        if (m_Buffer != nullptr) xBufferReset();

        m_Started = true;
        m_LastContinuityCounter = PacketHeader->getContinuityCounter();

        start_byte = xTS::TS_HeaderLength + AdaptationField->getAdaptationFieldLength() + 1;
        payload_size =
            xTS::TS_PacketLength
            - xTS::TS_HeaderLength
            - (AdaptationField->getAdaptationFieldLength() + 1);
        if (AdaptationField->getAdaptationFieldLength() == 0)
        {
            start_byte--;
            payload_size++;
        }

        m_PESH.Parse(&TransportStreamPacket[start_byte]);

        xBufferAppend(&TransportStreamPacket[start_byte], payload_size);
        return eResult::AssemblingStarted;
    }

    //still assembling 
    if (m_Started)
    {
        m_LastContinuityCounter = PacketHeader->getContinuityCounter();

        if (PacketHeader->hasAdaptationField()) //maybe last PES packet part
        {
            start_byte = xTS::TS_HeaderLength + AdaptationField->getAdaptationFieldLength() + 1;
            payload_size =
                xTS::TS_PacketLength
                - xTS::TS_HeaderLength
                - (AdaptationField->getAdaptationFieldLength() + 1);
        }
        else
        {
            start_byte = xTS::TS_HeaderLength;
            payload_size = xTS::TS_PacketLength - xTS::TS_HeaderLength;
        }

        xBufferAppend(&TransportStreamPacket[start_byte], payload_size);

        //last PES packet part, length without header
        if (m_PESH.getPacketLength() == m_BufferSize - xTS::PES_HeaderLength)
        {
            m_Started = false;
            return eResult::AssemblingFinished;
        }

        return eResult::AssemblingContinue;
    }
}

void xPES_Assembler::xBufferReset()
{
    m_BufferSize = 0;
    m_DataOffset = 0;

    delete[] m_Buffer;
    m_Buffer = nullptr;
}

void xPES_Assembler::xBufferAppend(const uint8_t* Data, int32_t Size)
{
    m_BufferSize += Size;
    m_DataOffset += Size;

    if (m_Buffer == nullptr) //after reset
    {
        m_Buffer = new uint8_t[m_BufferSize];
        std::memcpy(m_Buffer, Data, m_BufferSize);
        return;
    }

    uint8_t* temp_buffer = new uint8_t[m_BufferSize];

    int32_t new_data_start_byte = m_BufferSize - Size;

    std::memcpy(temp_buffer, m_Buffer, new_data_start_byte);
    std::memcpy(&temp_buffer[new_data_start_byte], Data, Size); //append new data

    delete[] m_Buffer;
    m_Buffer = new uint8_t[m_BufferSize];

    std::memcpy(m_Buffer, temp_buffer, m_BufferSize);//fill buffer

    delete[] temp_buffer;
}