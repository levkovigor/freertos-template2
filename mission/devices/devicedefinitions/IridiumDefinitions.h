#ifndef MISSION_DEVICES_DEVICEDEFINITIONS_IRIDIUMDEFINITIONS_H_
#define MISSION_DEVICES_DEVICEDEFINITIONS_IRIDIUMDEFINITIONS_H_

#include <fsfw/devicehandlers/DeviceHandlerIF.h>

namespace Iridium {

/*********************************************************************
Abbreviations:
ISU: Iridium System
MO: Mobile Originated Buffer. Telemetry is written here.
MT: Mobile Terminated Buffer. Telecommands are read from there
SBD: Short Burst Data. Communication type.
*********************************************************************/

/*********************************************************************
General information:
Commands are generally sent as ASCII strings which are terminated
with a <CR> character.

When sending a command, the reply (if enabled) coming from the ISU generally
consists of two  or three messages

1. <CR><LF><Sent Command><CR><LF>
2. (Optional) <CR><LF><Reponse><CR><LF>
3. <CR><LF><Result><CR><LF>

Result can be "ERROR" or "OK" depending on whether the sent command
is valid or invalid (command or parameters).
*********************************************************************/

static constexpr size_t MAX_SEND_SIZE = 340;
static constexpr size_t MAX_RECEIVE_SIZE = 270;

static constexpr DeviceCommandId_t PING = 0;
// This is a const char * const type
constexpr auto PING_CMD = "AT\r";

static constexpr DeviceCommandId_t ENABLE_ECHO = 1;
static constexpr DeviceCommandId_t DISABLE_ECHO = 2;
constexpr auto DISABLE_ECHO_CMD = "ATE0\r";
constexpr auto ENABLE_ECHO_CMD = "ATE1\r";

static constexpr DeviceCommandId_t ENABLE_ALL_RESPONSES = 3;
static constexpr DeviceCommandId_t DISABLE_ALL_RESPONSES = 4;
constexpr auto ENABLE_ALL_RESPONSES_CMD = "ATQ0\r";
constexpr auto DISABLE_ALL_RESPONSES_CMD = "ATE1\r";

static constexpr DeviceCommandId_t ENABLE_NUMERIC_RESPONSE = 5;
static constexpr DeviceCommandId_t DISABLE_NUMERIC_RESPONSE = 6;
constexpr auto DISABLE_NUMERIC_RESPONSE_CMD = "ATV0\r";
constexpr auto ENABLE_NUMERIC_RESPONSE_CMD = "ATV1\r";

static constexpr DeviceCommandId_t ENABLE_RTS_CTS_FLOW_CONTROL = 7;
static constexpr DeviceCommandId_t ENABLE_RTS_CTS_FLOW_CONTROL = 8;
constexpr auto ENABLE_RTS_CTS_CMD = "AT&K3\r";
constexpr auto DISBLE_RTS_CTS_CMD = "AT&K0\r";

static constexpr DeviceCommandId_t QUERY_RING_ALERT_SETTINGS = 9;
constexpr auto QUERY_RING_ALERT_SETTINGS_CMD = "AT+SBDMTA?\r";

static constexpr DeviceCommandId_t ENABLE_RING_ALERTS = 10;
static constexpr DeviceCommandId_t DISABLE_RING_ALERTS = 11;
constexpr auto ENABLE_RING_ALERTS_CMD = "AT+SBDMTA=0\r";
constexpr auto DISABLE_RING_ALERTS_CMD = "AT+SBDMTA=1\r";

static constexpr DeviceCommandId_t QUERY_INDICATOR_EVENT_RPRT_MODE = 12;
constexpr auto QUERY_INDICATOR_EVENT_RPRT_MODE_CMD = "AT+CIER?\r";

static constexpr DeviceCommandId_t SET_INDICATOR_EVENT_RPRT_MODE = 13;
/**
 * @brief   This is only the command header.
 * @details
 * AT+CIER=<mode>,<sigind>,<svcind>,<antind>,<sv_beam_coords_int>
 *
 * <mode>=0: Disable indicator event reporting, do not send +CIEV onsolicited
 * result codes to the DTE, buffer the most recent indicator event for each
 * indiicator in the Data Module (default)
 * <mode>=1: Enable indicator reportin events, buffer the most recent +CIEV
 * unsolicited result code for eahc indicator when the data port is reserved
 * (e.g. in SBD data mode) and flush them to the DTE after reservation,
 * otherwise, forward them directly to the DTE.
 *
 * <sigind>=0: Disable "signal quality" indicator reporting
 * <sigind>=1: Enable "signal quality" indicator reporting
 *
 * <svcind>=0: Disbale "service availability" indicator reporting
 * <svdind>=1: Enable
 *
 * <antind>=0: Disable "antenna fault" indicator reporting
 * <andint>=1: Enable
 *
 * <sv_beam_coords_int>=0: Disable "SV and beam coordinates" indicator reporting
 * <sv_beam_coords_int>=1: Enable
 */
constexpr auto SET_INDICATOR_EVENT_RPRT_MODE_HEADER = "AT+CIER=";
constexpr auto GENERIC_CIER_DISABLE_CHAR = "0";
constexpr auto GENERIC_CIER_ENABLE_CHAR = "1";
constexpr auto COMMAND_CIER_SEPARATOR = ",";


static constexpr DeviceCommandId_t QUERY_CURRENTLY_RCVD_SIGNAL_STRENGTH = 14;
constexpr auto QUERY_SINGAL_STRENGTH_CMD = "AT+CSQ\r";
/**
 * Signal strength will be displayed as a number from 0 to 5 behind the singal
 * strength header.
 * 0 is equivalent to 0 bars displayed on the ISU signal strength indicator.
 * 5 is equivalent to 5 bars displayed on the ISU signal strength indicator.
 */
constexpr auto QUERY_SIGNAL_STRENGTH_REPLY_HEADER = "+CSQ:";

static constexpr DeviceCommandId_t QUERY_LATEST_IRD_NETWORK_SYSTEM_TIME = 15;
constexpr auto QUERY_LATEST_IRD_NETWORK_SYSTEM_TIME_CMD = "AT-MSSTM\r";
/**
 * The reply header is followed by the <system_time> which can take the
 * following form:
 *
 * "no network service" -> The ISU has not yet received system time
 * "XXXXXXXX2 -> Current Iridium system time which is a 32 bit integer count
 * of 90 milliseconds intervals that have elapsed since epoch.
 * The returnvalue is formatted as an ASCII hex number without leading zeros.
 * Result should not be used as a time source for user applications, because
 * it will rollover approx. every 12 years.
 *
 */
constexpr auto QUERY_LATEST_IRD_NETWORK_SYSTEM_TIME_REPLY_HDR = "-MSSTM:";

/**
 * All buffers will be cleared when power-cycling the device.
 * ISU MT buffer is cleared automatically when a SBD session is initiated.
 * Sending a message from the ISU MO buffer does not automatically clear it.
 * Receiving a message from the ISU MT buffer does not automatically clear it.
 */
static constexpr DeviceCommandId_t CLEAR_BUFFERS = 16;
constexpr auto CLEAR_BUFFERS_CMD = "AT+SBDD";
constexpr auto CLEAR_MOBILE_ORIGINATED_BUFFER = "0";
constexpr auto CLEAR_MOBILE_TERMINATED_BUFFER = "1";
constexpr auto CLEAR_BOTH_BUFFERs = "2";
constexpr auto CLEAR_SUCCESS = "0";
constexpr auto CLEAR_FAILURE = "1";

/**
 * This is the command to send data to ground via Iridium.
 * Data is written to the ISU MO buffer.
 *
 * General procedure:
 *  1. Write "AT+SBDWB=<message_length><CR>
 *  2. Wait for response to indicate readiness: "<CR><LF>READY<CR><LF>"
 *  3. Send  data in the following format: "<binary SBD messsage><checksum>"
 *  4. Wait for two reponses with follwing format: "<CR><LF><result_num><CR><LF>
 *     and "<CR><LF><result_code><CR><LF>
 *
 * The result number will be a number from 0 to 3. The result number reply
 * will always be followed by a result code.
 *  0: SBD message written to ISU successfully. Result code "OK",
 *  1: SBD message timeout (60 seconds). Result code "ERROR",
 *  2: SBD message checksum not equal to calculated one at ISU.
 *     Result Code "OK",
 *  3: SBD message size incorrect. Result code "OK",
 *
 * The <checksum> is a 2 byte checksum which is
 * calculated by summing up all bytes of the SBD message and extracting
 * the least significant 2 bytes.
 *
 * Example: Sending "hello". Hex Repr = 68 65 6c 6c 6f . Checksum is 02 14.
 * So sent message is [0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x02, 0x14]
 */
static constexpr DeviceCommandId_t WRITE_BINARY_MSG_TO_MO_BUFFER = 17;
constexpr auto REPLY_READY = "READY";
constexpr auto REPLY_NUMBER_OK = "0";
constexpr auto REPLY_NUMBER_NOK_TIMEOUT = "1";
constexpr auto REPLY_NUMBER_NOK_CHECKSUM = "2";
constexpr auto REPLY_NUMBER_NOK_SIZE = "3";

}



#endif /* MISSION_DEVICES_DEVICEDEFINITIONS_IRIDIUMDEFINITIONS_H_ */
