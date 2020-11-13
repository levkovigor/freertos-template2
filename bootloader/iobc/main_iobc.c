/**
 * @brief   Bootloader for the iOBC. Can be compiled for NOR-Flash and SRAM.
 * @author  R. Mueller
 */
int iobc_main()
{
#ifdef sram
	return iobc_sram();
#elif defined(norflash)
	return iobc_norflash();
#else
	return -1;
#endif
}

