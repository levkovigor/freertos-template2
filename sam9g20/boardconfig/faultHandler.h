#ifndef SAM9G20_BOARDCONFIG_FAULTHANDLER_H_
#define SAM9G20_BOARDCONFIG_FAULTHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

void defaultSpuriousHandler( void );
void defaultFiqHandler( void );
void defaultIrqHandler(void);

#ifdef __cplusplus
}
#endif


#endif /* SAM9G20_BOARDCONFIG_FAULTHANDLER_H_ */
