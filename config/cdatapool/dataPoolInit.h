/**
 * @file dataPoolInit.h
 *
 * @date 06.12.2018
 * @author jakob
 */

#ifndef CONFIG_CDATAPOOL_DATAPOOLINIT_H_
#define CONFIG_CDATAPOOL_DATAPOOLINIT_H_

#include <fsfw/datapoolglob/GlobalDataPool.h>

namespace datapool{
	void dataPoolInit(GlobPoolMap * poolMap);

	enum datapoolvariables {
		NO_PARAMETER = 0,

		/** [EXPORT] : [GROUP] FSFW */
		INTERNAL_ERROR_STORE_FULL = 0xEE000001, 	 //!< [EXPORT] : [NAME] Internal Error Store Entry [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		INTERNAL_ERROR_MISSED_LIVE_TM = 0xEE000001,  //!< [EXPORT] : [NAME] Internal Error Missed Live Tm [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		INTERNAL_ERROR_FULL_MSG_QUEUES = 0xEE000001, //!< [EXPORT] : [NAME] Internal Error Full Msg Queue [UNIT] (-) [SIZE] 1 [TYPE] uint32_t

		/** [EXPORT] : [GROUP] GPS 0 */
		GPS0_FIX_MODE = 0x44001F00, 			//!< [EXPORT] : [NAME] GPS 0 Fix Mode  [UNIT] (-) [SIZE] 1 [TYPE] uint8_t
		GPS0_NUMBER_OF_SV_IN_FIX = 0x44001F05, 	//!< [EXPORT] : [NAME] GPS 0 Sv in Fix [UNIT] (-) [SIZE] 1 [TYPE] uint8_t
		GPS0_GNSS_WEEK = 0x44001F10, 			//!< [EXPORT] : [NAME] GPS 0 GNSS Week [UNIT] (-) [SIZE] 1 [TYPE] uint16_t
		GPS0_TIME_OF_WEEK = 0x44001F20, 		//!< [EXPORT] : [NAME] GPS 0 Time of Week [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS0_LATITUDE = 0x44001F30, 			//!< [EXPORT] : [NAME] GPS 0 Latitude [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS0_LONGITUDE = 0x44001F40, 			//!< [EXPORT] : [NAME] GPS 0 Longitude [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS0_MEAN_SEA_ALTITUDE = 0x44001F50, 	//!< [EXPORT] : [NAME] GPS 0 Mean Sea Altitude [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS0_POSITION = 0x44001F60, 			//!< [EXPORT] : [NAME] GPS 0 Position [UNIT] (-) [SIZE] 3 [TYPE] double
		GPS0_VELOCITY = 0x44001F70, 			//!< [EXPORT] : [NAME] GPS 0 Velocity [UNIT] (m/s) [SIZE] 3 [TYPE] double

		/** [EXPORT] : [GROUP] GPS 1 */
		GPS1_FIX_MODE = 0x44002000, 			//!< [EXPORT] : [NAME] GPS 1 Fix Mode [UNIT] (-) [SIZE] 1 [TYPE] uint8_t
		GPS1_NUMBER_OF_SV_IN_FIX = 0x44002005, 	//!< [EXPORT] : [NAME] GPS 1 Sv in Fix [UNIT] (-) [SIZE] 1 [TYPE] uint8_t
		GPS1_GNSS_WEEK = 0x44002010, 			//!< [EXPORT] : [NAME] GPS 1 GNSS Week [UNIT] (-) [SIZE] 1 [TYPE] uint16_t
		GPS1_TIME_OF_WEEK = 0x44002015, 		//!< [EXPORT] : [NAME] GPS 1 Time of Week [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS1_LATITUDE = 0x44002020, 			//!< [EXPORT] : [NAME] GPS 1 Latitude [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS1_LONGITUDE = 0x44002025, 			//!< [EXPORT] : [NAME] GPS 1 Longitude [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS1_MEAN_SEA_ALTITUDE = 0x44002030, 	//!< [EXPORT] : [NAME] GPS 1 Mean Sea Altitude [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		GPS1_POSITION = 0x44002035, 			//!< [EXPORT] : [NAME] GPS 1 Position [UNIT] (-) [SIZE] 3 [TYPE] double
		GPS1_VELOCITY = 0x44002040, 			//!< [EXPORT] : [NAME] GPS 1 Velocity [UNIT] (m/s) [SIZE] 3 [TYPE] double

		/** [EXPORT] : [GROUP] TEST */
		TEST_BOOLEAN = 0x01010102, 				//!< [EXPORT] : [NAME] Test Boolean [UNIT] (-) [SIZE] 1 [TYPE] bool
		TEST_UINT8 = 0x02020204,				//!< [EXPORT] : [NAME] Test Byte [UNIT] (-) [SIZE] 1 [TYPE] uint8_t
		TEST_UINT16 = 0x03030306,				//!< [EXPORT] : [NAME] Test UINT16 [UNIT] (-) [SIZE] 1 [TYPE] uint16_t
		TEST_UINT32 = 0x04040408,				//!< [EXPORT] : [NAME] Test UINT32 [UNIT] (-) [SIZE] 1 [TYPE] uint32_t
		TEST_FLOAT_VECTOR = 0x05050510			//!< [EXPORT] : [NAME] Test Float [UNIT] (-) [SIZE] 2 [TYPE] float
	};
}
#endif /* CONFIG_CDATAPOOL_DATAPOOLINIT_H_ */
