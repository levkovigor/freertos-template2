#include "SerialReceiverTask.h"
#include <utility/trace.h>


SerialReceiverTask::SerialReceiverTask(object_id_t objectId,
		uint32_t baudRate, uint16_t timeoutBaudTicks): SystemObject(objectId) {
	uartConfig.baudrate = baudRate;
	uartConfig.rxtimeout = timeoutBaudTicks;
}

ReturnValue_t SerialReceiverTask::performOperation(uint8_t opCode) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t SerialReceiverTask::initialize() {
	uartConfig.mode = mode;
	uartConfig.busType = rs232_uart;
	uartConfig.timeGuard = 0;
	int result = UART_start(bus0_uart, uartConfig);
	return result;
}

void SerialReceiverTask::handleBinaryReception() {
}



//void uart_polling_task() {
//    for(;;) {
//        // should be mutex protected..
//        int result = 0;
//        BaseType_t mutex_lock = xSemaphoreTake(uartMutex, 20);
//        if(mutex_lock == pdFALSE) {
//            // should not happen
//        }
//        uint8_t buffer_state = 0;
//        if(!buffer_0_full) {
//            buffer_state = 0;
//        }
//        else if(!buffer_1_full) {
//            buffer_state = 1;
//        }
//        else {
//            buffer_state = 2;
//            // both buffers full, apparently the data is not handled fast
//            // enough. output error.
//            return;
//        }
//        xSemaphoreGive(uartMutex);
//
//        if(buffer_state < 2) {
//            result = UART_read(bus0_uart, reception_buffer[0], RECEPTION_BUFFER_SIZE);
//        }
//
//        if(result != 0) {
//            // handle bus errors here
//        }
//
//        // Update bytes read.
//        mutex_lock = xSemaphoreTake(uartMutex, 20);
//        size_t prev_bytes_read = UART_getPrevBytesRead(bus0_uart);
//
//        // Received packet is smaller, notify analyzer task so it doesn't
//        // scan full buffer.
//        if(buffer_state == 0) {
//            buffer_0_bytes_received = prev_bytes_read;
//        }
//        else if(buffer_state == 1) {
//            buffer_1_bytes_received = prev_bytes_read;
//        }
//        xSemaphoreGive(uartMutex);
//    }
//}
//
//void handle_binary_reception() {
//    bool binaryReceived = false;
//    binary_type_t bin_type = BOOTLOADER;
//    uint16_t packetIndex = 0;
//    uint32_t hamming_code_offset = 0;
//    uint32_t binary_size = 0;
//    uint16_t number_of_packets = 0;
//    uint8_t decoding_buffer[RECEPTION_BUFFER_SIZE];
//
//    while(!binaryReceived) {
//
//        size_t size_received = 0;
//        size_t packet_size = 0;
//        // Attempt to recveive a packet.
//        if(!receive_packet(&size_received)) {
//            // Something went wrong
//            continue;
//        }
//
//        // Decode the DLE encoded packet.
//        if(current_buffer == NULL) {
//            // config error
//            TRACE_ERROR("handle_binary_reception: Current buffer is NULL!\n\r");
//            continue;
//        }
//        dle_returncodes_t decode_result = decode_dle(current_buffer,
//                size_received, NULL, decoding_buffer,
//                RECEPTION_BUFFER_SIZE, &packet_size);
//        if(decode_result != DLE_OK) {
//            // Something went wrong
//            continue;
//        }
//
//        if(packetIndex == 0) {
//            handle_lead_packet(decoding_buffer, &bin_type, &binary_size,
//                    &number_of_packets, &hamming_code_offset);
//            packetIndex++;
//        }
//        else if(packetIndex == number_of_packets) {
//            // should be the last packet. verify by checking subservice
//        }
//        else {
//            // verify subservice
//        }
//
//        // handle lead packet and cache expected number of packets, binary
//        // size and start position of hamming code.
//
//    }
//}
//
//
//bool receive_packet(size_t* size_received) {
//    uint32_t lastDotPrint = 0;
//    uint8_t packetIndex = 0;
//    bool stxReceived = false;
//    for(;;) {
////        if(!stxReceived && u32_ms_counter - lastDotPrint >= 2000) {
////            TRACE_INFO_WP(".");
////            lastDotPrint = u32_ms_counter;
////        }
//
//        size_t bytes_last_transfer = 0;
//        BaseType_t mutex_lock = xSemaphoreTake(uartMutex, 20);
//        if(mutex_lock == pdFALSE) {
//            // should not happen!
//        }
//
//        if(buffer_0_full) {
//            // assign pointer of buffer
//            current_buffer = reception_buffer[0];
//            bytes_last_transfer = buffer_0_bytes_received;
//            if(buffer_1_full) {
//                // config error
//            }
//        }
//        else if(buffer_1_full) {
//            current_buffer = reception_buffer[1];
//            bytes_last_transfer = buffer_1_bytes_received;
//            if(buffer_0_full) {
//                // config error
//            }
//        }
//        else {
//            // nothing to do
//            mutex_lock = xSemaphoreGive(uartMutex);
//            continue;
//        }
//        mutex_lock = xSemaphoreGive(uartMutex);
//
//        // Analyse buffer here.
//        for(uint16_t buff_idx = 0; buff_idx < bytes_last_transfer; buff_idx++) {
//            char recvChar = current_buffer[buff_idx];
//            // start of packet detected
//            if(recvChar == STX_CHAR) {
//                if(!stxReceived) {
//                    stxReceived = true;
//                    packetIndex++;
//                }
//                else {
//                    return false;
//                }
//            }
//            // End of packet detected
//            else if(recvChar == ETX_CHAR) {
//                if(stxReceived) {
//                    //reception_buffer[packetIndex] = recvChar;
//                    packetIndex++;
//                    *size_received = packetIndex;
//                    return true;
//                }
//            }
//            // Regular char of packet
//            else if(stxReceived) {
//                //reception_buffer[packetIndex] = recvChar;
//                packetIndex++;
//            }
//        }
//
//        if(packetIndex == sizeof(reception_buffer)) {
//            // Invalid packet.
//            // Send NACK
//            return false;
//        }
//    }
//    return true;
//}
//
//void handle_lead_packet(uint8_t* packet_ptr, binary_type_t* bin_type,
//        uint32_t* binary_size, uint16_t * number_of_packets,
//        uint32_t* hamming_code_offset) {
//    // check packet format / validity. Assign expected number
//    // of packets, binary size and start position of hamming code
//    // verify subservice
//
//    // TC packet
//    TcPacketPointer* tc_packet = (TcPacketPointer*) packet_ptr;
//    tc_check_retcode_t tc_check = checkPacket(tc_packet);
//    if(tc_check != TC_CHECK_OK) {
//        // failure
//    }
//
//    // now verify subservice
//
//    const uint8_t* lead_packet_data = get_application_data(tc_packet);
//    size_t lead_packet_data_size = get_application_data_size(tc_packet);
//    if(lead_packet_data_size != sizeof(lead_packet_data_t)) {
//        // data invalid
//
//    }
//
//    size_t deserialized_size = 0;
//    *bin_type = lead_packet_data[0];
//    // check whether bin type is valid.
//    if(*bin_type != BOOTLOADER) {
//        // does only support bootloader for now
//    }
//
//    deserialized_size += sizeof(*bin_type);
//
//    uint16_t numer_of_packet_le = 0;
//    memcpy(&number_of_packets, lead_packet_data + deserialized_size,
//            sizeof(number_of_packets));
//    *number_of_packets = __builtin_bswap16(numer_of_packet_le);
//    deserialized_size += sizeof(numer_of_packet_le);
//    // add sanity check for size (should be less than ~ 970kB / 256)
//    // and even smaller if binary type is bootloader)
//
//    uint32_t binary_size_le = 0;
//    memcpy(&binary_size_le, lead_packet_data + deserialized_size,
//            sizeof(binary_size_le));
//    *binary_size = __builtin_bswap32(binary_size_le);
//    deserialized_size += sizeof(binary_size_le);
//    // add sanity check for size (should be less than ~ 970 kB
//    // and even smaller if binary type is bootloader)
//    if(binary_size){}
//
//    uint32_t hamming_code_offset_le = 0;
//    memcpy(&hamming_code_offset_le,
//            lead_packet_data + deserialized_size,
//            sizeof(hamming_code_offset_le));
//    *hamming_code_offset = __builtin_bswap32(hamming_code_offset_le);
//    deserialized_size += sizeof(hamming_code_offset_le);
//    // add sanity check for offset (should be at the end of
//    // the designated memory, so less than binary_size)
//
//    // if all three variables are ok, write them into
//    // into the FRAM.
//
//
//
//
//    // send NACK, packet invalid.
//
//    // send ACK to sender
//}
