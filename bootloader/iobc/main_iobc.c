#ifdef sram
extern int iobc_sram();
#elif defined(norflash)
extern int boot_iobc_from_norflash();
#endif /* sram */

/**
 * @brief   Bootloader for the iOBC. Can be compiled for NOR-Flash and SRAM.
 * @author  R. Mueller
 */
int iobc_main()
{
#ifdef sram
	return iobc_sram();
#elif defined(norflash)
	return boot_iobc_from_norflash();
#else
	return -1;
#endif /* sram */
}

