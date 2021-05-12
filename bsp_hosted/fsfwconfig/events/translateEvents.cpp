/**
 * @brief    Auto-generated event translation file. Contains 82 translations.
 * @details
 * Generated on: 2021-05-12 15:06:12
 */
#include "translateEvents.h"

const char *STORE_SEND_WRITE_FAILED_STRING = "STORE_SEND_WRITE_FAILED";
const char *STORE_WRITE_FAILED_STRING = "STORE_WRITE_FAILED";
const char *STORE_SEND_READ_FAILED_STRING = "STORE_SEND_READ_FAILED";
const char *STORE_READ_FAILED_STRING = "STORE_READ_FAILED";
const char *UNEXPECTED_MSG_STRING = "UNEXPECTED_MSG";
const char *STORING_FAILED_STRING = "STORING_FAILED";
const char *TM_DUMP_FAILED_STRING = "TM_DUMP_FAILED";
const char *STORE_INIT_FAILED_STRING = "STORE_INIT_FAILED";
const char *STORE_INIT_EMPTY_STRING = "STORE_INIT_EMPTY";
const char *STORE_CONTENT_CORRUPTED_STRING = "STORE_CONTENT_CORRUPTED";
const char *STORE_INITIALIZE_STRING = "STORE_INITIALIZE";
const char *INIT_DONE_STRING = "INIT_DONE";
const char *DUMP_FINISHED_STRING = "DUMP_FINISHED";
const char *DELETION_FINISHED_STRING = "DELETION_FINISHED";
const char *DELETION_FAILED_STRING = "DELETION_FAILED";
const char *AUTO_CATALOGS_SENDING_FAILED_STRING = "AUTO_CATALOGS_SENDING_FAILED";
const char *GET_DATA_FAILED_STRING = "GET_DATA_FAILED";
const char *STORE_DATA_FAILED_STRING = "STORE_DATA_FAILED";
const char *DEVICE_BUILDING_COMMAND_FAILED_STRING = "DEVICE_BUILDING_COMMAND_FAILED";
const char *DEVICE_SENDING_COMMAND_FAILED_STRING = "DEVICE_SENDING_COMMAND_FAILED";
const char *DEVICE_REQUESTING_REPLY_FAILED_STRING = "DEVICE_REQUESTING_REPLY_FAILED";
const char *DEVICE_READING_REPLY_FAILED_STRING = "DEVICE_READING_REPLY_FAILED";
const char *DEVICE_INTERPRETING_REPLY_FAILED_STRING = "DEVICE_INTERPRETING_REPLY_FAILED";
const char *DEVICE_MISSED_REPLY_STRING = "DEVICE_MISSED_REPLY";
const char *DEVICE_UNKNOWN_REPLY_STRING = "DEVICE_UNKNOWN_REPLY";
const char *DEVICE_UNREQUESTED_REPLY_STRING = "DEVICE_UNREQUESTED_REPLY";
const char *INVALID_DEVICE_COMMAND_STRING = "INVALID_DEVICE_COMMAND";
const char *MONITORING_LIMIT_EXCEEDED_STRING = "MONITORING_LIMIT_EXCEEDED";
const char *MONITORING_AMBIGUOUS_STRING = "MONITORING_AMBIGUOUS";
const char *FUSE_CURRENT_HIGH_STRING = "FUSE_CURRENT_HIGH";
const char *FUSE_WENT_OFF_STRING = "FUSE_WENT_OFF";
const char *POWER_ABOVE_HIGH_LIMIT_STRING = "POWER_ABOVE_HIGH_LIMIT";
const char *POWER_BELOW_LOW_LIMIT_STRING = "POWER_BELOW_LOW_LIMIT";
const char *SWITCH_WENT_OFF_STRING = "SWITCH_WENT_OFF";
const char *HEATER_ON_STRING = "HEATER_ON";
const char *HEATER_OFF_STRING = "HEATER_OFF";
const char *HEATER_TIMEOUT_STRING = "HEATER_TIMEOUT";
const char *HEATER_STAYED_ON_STRING = "HEATER_STAYED_ON";
const char *HEATER_STAYED_OFF_STRING = "HEATER_STAYED_OFF";
const char *TEMP_SENSOR_HIGH_STRING = "TEMP_SENSOR_HIGH";
const char *TEMP_SENSOR_LOW_STRING = "TEMP_SENSOR_LOW";
const char *TEMP_SENSOR_GRADIENT_STRING = "TEMP_SENSOR_GRADIENT";
const char *COMPONENT_TEMP_LOW_STRING = "COMPONENT_TEMP_LOW";
const char *COMPONENT_TEMP_HIGH_STRING = "COMPONENT_TEMP_HIGH";
const char *COMPONENT_TEMP_OOL_LOW_STRING = "COMPONENT_TEMP_OOL_LOW";
const char *COMPONENT_TEMP_OOL_HIGH_STRING = "COMPONENT_TEMP_OOL_HIGH";
const char *TEMP_NOT_IN_OP_RANGE_STRING = "TEMP_NOT_IN_OP_RANGE";
const char *FDIR_CHANGED_STATE_STRING = "FDIR_CHANGED_STATE";
const char *FDIR_STARTS_RECOVERY_STRING = "FDIR_STARTS_RECOVERY";
const char *FDIR_TURNS_OFF_DEVICE_STRING = "FDIR_TURNS_OFF_DEVICE";
const char *MONITOR_CHANGED_STATE_STRING = "MONITOR_CHANGED_STATE";
const char *VALUE_BELOW_LOW_LIMIT_STRING = "VALUE_BELOW_LOW_LIMIT";
const char *VALUE_ABOVE_HIGH_LIMIT_STRING = "VALUE_ABOVE_HIGH_LIMIT";
const char *VALUE_OUT_OF_RANGE_STRING = "VALUE_OUT_OF_RANGE";
const char *SWITCHING_TM_FAILED_STRING = "SWITCHING_TM_FAILED";
const char *CHANGING_MODE_STRING = "CHANGING_MODE";
const char *MODE_INFO_STRING = "MODE_INFO";
const char *FALLBACK_FAILED_STRING = "FALLBACK_FAILED";
const char *MODE_TRANSITION_FAILED_STRING = "MODE_TRANSITION_FAILED";
const char *CANT_KEEP_MODE_STRING = "CANT_KEEP_MODE";
const char *OBJECT_IN_INVALID_MODE_STRING = "OBJECT_IN_INVALID_MODE";
const char *FORCING_MODE_STRING = "FORCING_MODE";
const char *MODE_CMD_REJECTED_STRING = "MODE_CMD_REJECTED";
const char *HEALTH_INFO_STRING = "HEALTH_INFO";
const char *CHILD_CHANGED_HEALTH_STRING = "CHILD_CHANGED_HEALTH";
const char *CHILD_PROBLEMS_STRING = "CHILD_PROBLEMS";
const char *OVERWRITING_HEALTH_STRING = "OVERWRITING_HEALTH";
const char *TRYING_RECOVERY_STRING = "TRYING_RECOVERY";
const char *RECOVERY_STEP_STRING = "RECOVERY_STEP";
const char *RECOVERY_DONE_STRING = "RECOVERY_DONE";
const char *RF_AVAILABLE_STRING = "RF_AVAILABLE";
const char *RF_LOST_STRING = "RF_LOST";
const char *BIT_LOCK_STRING = "BIT_LOCK";
const char *BIT_LOCK_LOST_STRING = "BIT_LOCK_LOST";
const char *FRAME_PROCESSING_FAILED_STRING = "FRAME_PROCESSING_FAILED";
const char *CLOCK_SET_STRING = "CLOCK_SET";
const char *CLOCK_SET_FAILURE_STRING = "CLOCK_SET_FAILURE";
const char *TEST_STRING = "TEST";
const char *GPS_STARTUP_FAILED_STRING = "GPS_STARTUP_FAILED";
const char *GPS_FIX_STRING = "GPS_FIX";
const char *GPS_LOST_FIX_STRING = "GPS_LOST_FIX";
const char *CHANGE_OF_SETUP_PARAMETER_STRING = "CHANGE_OF_SETUP_PARAMETER";

const char * translateEvents(Event event) {
	switch( (event & 0xffff) ) {
	case(2200):
		return STORE_SEND_WRITE_FAILED_STRING;
	case(2201):
		return STORE_WRITE_FAILED_STRING;
	case(2202):
		return STORE_SEND_READ_FAILED_STRING;
	case(2203):
		return STORE_READ_FAILED_STRING;
	case(2204):
		return UNEXPECTED_MSG_STRING;
	case(2205):
		return STORING_FAILED_STRING;
	case(2206):
		return TM_DUMP_FAILED_STRING;
	case(2207):
		return STORE_INIT_FAILED_STRING;
	case(2208):
		return STORE_INIT_EMPTY_STRING;
	case(2209):
		return STORE_CONTENT_CORRUPTED_STRING;
	case(2210):
		return STORE_INITIALIZE_STRING;
	case(2211):
		return INIT_DONE_STRING;
	case(2212):
		return DUMP_FINISHED_STRING;
	case(2213):
		return DELETION_FINISHED_STRING;
	case(2214):
		return DELETION_FAILED_STRING;
	case(2215):
		return AUTO_CATALOGS_SENDING_FAILED_STRING;
	case(2600):
		return GET_DATA_FAILED_STRING;
	case(2601):
		return STORE_DATA_FAILED_STRING;
	case(2800):
		return DEVICE_BUILDING_COMMAND_FAILED_STRING;
	case(2801):
		return DEVICE_SENDING_COMMAND_FAILED_STRING;
	case(2802):
		return DEVICE_REQUESTING_REPLY_FAILED_STRING;
	case(2803):
		return DEVICE_READING_REPLY_FAILED_STRING;
	case(2804):
		return DEVICE_INTERPRETING_REPLY_FAILED_STRING;
	case(2805):
		return DEVICE_MISSED_REPLY_STRING;
	case(2806):
		return DEVICE_UNKNOWN_REPLY_STRING;
	case(2807):
		return DEVICE_UNREQUESTED_REPLY_STRING;
	case(2808):
		return INVALID_DEVICE_COMMAND_STRING;
	case(2809):
		return MONITORING_LIMIT_EXCEEDED_STRING;
	case(2810):
		return MONITORING_AMBIGUOUS_STRING;
	case(4201):
		return FUSE_CURRENT_HIGH_STRING;
	case(4202):
		return FUSE_WENT_OFF_STRING;
	case(4204):
		return POWER_ABOVE_HIGH_LIMIT_STRING;
	case(4205):
		return POWER_BELOW_LOW_LIMIT_STRING;
	case(4300):
		return SWITCH_WENT_OFF_STRING;
	case(5000):
		return HEATER_ON_STRING;
	case(5001):
		return HEATER_OFF_STRING;
	case(5002):
		return HEATER_TIMEOUT_STRING;
	case(5003):
		return HEATER_STAYED_ON_STRING;
	case(5004):
		return HEATER_STAYED_OFF_STRING;
	case(5200):
		return TEMP_SENSOR_HIGH_STRING;
	case(5201):
		return TEMP_SENSOR_LOW_STRING;
	case(5202):
		return TEMP_SENSOR_GRADIENT_STRING;
	case(5901):
		return COMPONENT_TEMP_LOW_STRING;
	case(5902):
		return COMPONENT_TEMP_HIGH_STRING;
	case(5903):
		return COMPONENT_TEMP_OOL_LOW_STRING;
	case(5904):
		return COMPONENT_TEMP_OOL_HIGH_STRING;
	case(5905):
		return TEMP_NOT_IN_OP_RANGE_STRING;
	case(7101):
		return FDIR_CHANGED_STATE_STRING;
	case(7102):
		return FDIR_STARTS_RECOVERY_STRING;
	case(7103):
		return FDIR_TURNS_OFF_DEVICE_STRING;
	case(7201):
		return MONITOR_CHANGED_STATE_STRING;
	case(7202):
		return VALUE_BELOW_LOW_LIMIT_STRING;
	case(7203):
		return VALUE_ABOVE_HIGH_LIMIT_STRING;
	case(7204):
		return VALUE_OUT_OF_RANGE_STRING;
	case(7301):
		return SWITCHING_TM_FAILED_STRING;
	case(7400):
		return CHANGING_MODE_STRING;
	case(7401):
		return MODE_INFO_STRING;
	case(7402):
		return FALLBACK_FAILED_STRING;
	case(7403):
		return MODE_TRANSITION_FAILED_STRING;
	case(7404):
		return CANT_KEEP_MODE_STRING;
	case(7405):
		return OBJECT_IN_INVALID_MODE_STRING;
	case(7406):
		return FORCING_MODE_STRING;
	case(7407):
		return MODE_CMD_REJECTED_STRING;
	case(7506):
		return HEALTH_INFO_STRING;
	case(7507):
		return CHILD_CHANGED_HEALTH_STRING;
	case(7508):
		return CHILD_PROBLEMS_STRING;
	case(7509):
		return OVERWRITING_HEALTH_STRING;
	case(7510):
		return TRYING_RECOVERY_STRING;
	case(7511):
		return RECOVERY_STEP_STRING;
	case(7512):
		return RECOVERY_DONE_STRING;
	case(7900):
		return RF_AVAILABLE_STRING;
	case(7901):
		return RF_LOST_STRING;
	case(7902):
		return BIT_LOCK_STRING;
	case(7903):
		return BIT_LOCK_LOST_STRING;
	case(7905):
		return FRAME_PROCESSING_FAILED_STRING;
	case(8900):
		return CLOCK_SET_STRING;
	case(8901):
		return CLOCK_SET_FAILURE_STRING;
	case(9700):
		return TEST_STRING;
	case(12900):
		return GPS_STARTUP_FAILED_STRING;
	case(12901):
		return GPS_FIX_STRING;
	case(12902):
		return GPS_LOST_FIX_STRING;
	case(13000):
		return CHANGE_OF_SETUP_PARAMETER_STRING;
	default:
		return "UNKNOWN_EVENT";
	}
	return 0;
}
