/**
 * @file	dataPoolInit.cpp
 *
 * @brief	Auto-Generated datapool initialization
 * @date	02.05.2020
 */
#include <fsfwconfig/cdatapool/dataPoolInit.h>

void datapool::dataPoolInit(poolMap * poolMap) {

	/* FSFW */
	poolMap->emplace(datapool::INTERNAL_ERROR_STORE_FULL,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::INTERNAL_ERROR_MISSED_LIVE_TM,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::INTERNAL_ERROR_FULL_MSG_QUEUES,
			new PoolEntry<uint32_t>({0},1));

	/* GPS 0 */
	poolMap->emplace(datapool::GPS0_FIX_MODE,
			new PoolEntry<uint8_t>({0},1));
	poolMap->emplace(datapool::GPS0_NUMBER_OF_SV_IN_FIX,
			new PoolEntry<uint8_t>({0},1));
	poolMap->emplace(datapool::GPS0_GNSS_WEEK,
			new PoolEntry<uint16_t>({0},1));
	poolMap->emplace(datapool::GPS0_TIME_OF_WEEK,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS0_LATITUDE,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS0_LONGITUDE,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS0_MEAN_SEA_ALTITUDE,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS0_POSITION,
			new PoolEntry<double>({0, 0, 0},3));
	poolMap->emplace(datapool::GPS0_VELOCITY,
			new PoolEntry<double>({0, 0, 0},3));

	/* GPS 1 */
	poolMap->emplace(datapool::GPS1_FIX_MODE,
			new PoolEntry<uint8_t>({0},1));
	poolMap->emplace(datapool::GPS1_NUMBER_OF_SV_IN_FIX,
			new PoolEntry<uint8_t>({0},1));
	poolMap->emplace(datapool::GPS1_GNSS_WEEK,
			new PoolEntry<uint16_t>({0},1));
	poolMap->emplace(datapool::GPS1_TIME_OF_WEEK,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS1_LATITUDE,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS1_LONGITUDE,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS1_MEAN_SEA_ALTITUDE,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::GPS1_POSITION,
			new PoolEntry<double>({0, 0, 0},3));
	poolMap->emplace(datapool::GPS1_VELOCITY,
			new PoolEntry<double>({0, 0, 0},3));

	/* TEST */
	poolMap->emplace(datapool::TEST_BOOLEAN,
			new PoolEntry<bool>({0},1));
	poolMap->emplace(datapool::TEST_UINT8,
			new PoolEntry<uint8_t>({0},1));
	poolMap->emplace(datapool::TEST_UINT16,
			new PoolEntry<uint16_t>({0},1));
	poolMap->emplace(datapool::TEST_UINT32,
			new PoolEntry<uint32_t>({0},1));
	poolMap->emplace(datapool::TEST_FLOAT_VECTOR,
			new PoolEntry<float>({0, 0},2));

}
