#include <bsp_sam9g20/utility/TCTimerHandler.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <cstdio>
#include <limits>

extern "C" {
#include <at91/peripherals/aic/aic.h>
}


void TCTimerHandler::configureOneShotInterrupt(TcPeripherals tcSelect,
        void (*isrRoutine)(isr_args_t isrArgs), uint8_t interruptPriority,
        bool startImmediately, isr_args_t isrArgs) {
    TCTimerHandler::configureGenericInterrupt(tcSelect, 2, isrRoutine,
            interruptPriority, startImmediately, isrArgs, true);
}

void TCTimerHandler::configurePeriodicInterrupt(TcPeripherals tcSelect,
        uint32_t frequency, void (*isrRoutine)(isr_args_t isrArgs),
        uint8_t interruptPriority, bool startImmediately,
        isr_args_t isrArgs) {
    TCTimerHandler::configureGenericInterrupt(tcSelect, frequency, isrRoutine,
            interruptPriority, startImmediately, isrArgs, false);
}

void TCTimerHandler::configureOverflowInterrupt(TcPeripherals tcSelect,
        uint32_t frequency, void(*isrRoutine)(isr_args_t isrArgs),
        uint8_t interruptPriority, isr_args_t isrArgs) {
    TCTimerHandler::configureGenericInterrupt(tcSelect, frequency, isrRoutine,
            interruptPriority, true, isrArgs, false, true);
}

void TCTimerHandler::configureGenericInterrupt(TcPeripherals tcSelect,
        uint32_t frequency, void (*isrRoutine)(isr_args_t isrArgs),
        uint8_t interruptPriority, bool startImmediately, isr_args_t isrArgs,
        bool oneShotInterrupt, bool overflowInterrupt)
{
    AT91S_TC* peripheral = getPeripheral(tcSelect);
    uint32_t peripheralId = getPeripheralId(tcSelect);
    isr_routine_aic_t aicIsr;
    auto result = prepareParameters(tcSelect, aicIsr, interruptPriority,
            isrRoutine, oneShotInterrupt, isrArgs);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return;
    }

    uint32_t div = 0;
    uint32_t tcclks= 0;

    // Enable peripheral clock
    AT91C_BASE_PMC->PMC_PCER = 1 << peripheralId;

    // Configure TC for the specified frequency and trigger on RC compare
    // (RC is a register)
    int retval = TC_FindMckDivisor(frequency, BOARD_MCK,
            reinterpret_cast<unsigned int*>(&div),
            reinterpret_cast<unsigned int*>(&tcclks));
    if(retval == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TCTimerHandler::configureGenericInterrupt: Frequency "
                "too high!" << std::endl;
#else
        sif::printWarning( "TCTimerHandler::configureGenericInterrupt: Frequency "
                "too high!\n");
#endif
        return;
    }
    TC_Configure(peripheral, tcclks | AT91C_TC_CPCTRG);


    if(overflowInterrupt) {
        AT91C_BASE_TC0->TC_RC = 0xffff;
    }
    // timerFreq / desiredFreq equals the RC compare value.
    else if(((BOARD_MCK / div) / frequency) > std::numeric_limits<uint16_t>::max()) {
        //sif::warning << "TCTimerHandler::configureGenericInterrupt: Specified"
        //		"frequency too slow! Setting RC value to the maximum value.\n"
        //		<< std::flush;
        // silently set slowest value for now. (0.5 Hz für iOBC, SAM9G20-EK)
        AT91C_BASE_TC0->TC_RC = 0xffff;
    }
    else {
        AT91C_BASE_TC0->TC_RC = (BOARD_MCK / div) / frequency;
    }

    // Configure and enable interrupt on RC compare
    AIC_ConfigureIT(peripheralId, interruptPriority, aicIsr);
    peripheral->TC_IER = AT91C_TC_CPCS;
    AIC_EnableIT(peripheralId);
    if(startImmediately) {
        startTc(tcSelect);
    }
}

void TCTimerHandler::startTc(TcPeripherals tcPeripheral) {
    AT91S_TC* peripheral = getPeripheral(tcPeripheral);
    TC_Start(peripheral);
}

void TCTimerHandler::stopTc(TcPeripherals tcPeripheral) {
    AT91S_TC* peripheral = getPeripheral(tcPeripheral);
    TC_Stop(peripheral);
}

AT91S_TC* TCTimerHandler::getPeripheral(
        TcPeripherals peripheral) {
    switch(peripheral) {
    case(TcPeripherals::TC0): return AT91C_BASE_TC0;
    case(TcPeripherals::TC1): return AT91C_BASE_TC1;
    case(TcPeripherals::TC2): return AT91C_BASE_TC2;
    case(TcPeripherals::TC3): return AT91C_BASE_TC3;
    case(TcPeripherals::TC4): return AT91C_BASE_TC4;
    case(TcPeripherals::TC5): return AT91C_BASE_TC5;
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TCTimerHandler::configurePeriodicInterrupt: Invalid"
                "TC peripheral!" << std::endl;
#else
        sif::printError("TCTimerHandler::configurePeriodicInterrupt: "
                "Invalid TC peripheral!\n");
#endif
        return nullptr;
    }

    }
}

uint32_t TCTimerHandler::getPeripheralId(TcPeripherals peripheral) {
    switch(peripheral) {
    case(TcPeripherals::TC0): return AT91C_ID_TC0;
    case(TcPeripherals::TC1): return AT91C_ID_TC1;
    case(TcPeripherals::TC2): return AT91C_ID_TC2;
    case(TcPeripherals::TC3): return AT91C_ID_TC3;
    case(TcPeripherals::TC4): return AT91C_ID_TC4;
    case(TcPeripherals::TC5): return AT91C_ID_TC5;
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TCTimerHandler::configurePeriodicInterrupt: Invalid"
                "TC peripheral. Returning peripheral 0!" << std::endl;
#else
        sif::printError("TCTimerHandler::configurePeriodicInterrupt: Invalid"
                "TC peripheral. Returning peripheral 0!\n");
#endif
        return AT91C_ID_TC0;
    }

    }
}

ReturnValue_t TCTimerHandler::prepareParameters(TcPeripherals tcPeripheral,
        isr_routine_aic_t& aicIsr,uint8_t& interruptPrio,
        void (*isrRoutine)(isr_args_t args), bool oneShotInterrupt,
        isr_args_t isrArgs) {
    if(interruptPrio > HIGHEST_ISR_PRIORITY) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "TCTimerHandler::configurePeriodicInterrupt: Invalid"
                << "priority. Setting to lowest priority!" << std::endl;
#else
        sif::printWarning("TCTimerHandler::configurePeriodicInterrupt: Invalid"
                "priority. Setting to lowest priority!\n");
#endif
        interruptPrio = LOWEST_ISR_PRIORITY;
    }
    switch(tcPeripheral) {
    case(TcPeripherals::TC0): {
        aicIsr = tcGenericIsrTc0;
        break;
    }
    case(TcPeripherals::TC1): {
        aicIsr = tcGenericIsrTc1;
        break;
    }
    case(TcPeripherals::TC2): {
        aicIsr = tcGenericIsrTc2;
        break;
    }
    case(TcPeripherals::TC3): {
        aicIsr = tcGenericIsrTc3;
        break;
    }
    case(TcPeripherals::TC4): {
        aicIsr = tcGenericIsrTc4;
        break;
    }
    case(TcPeripherals::TC5): {
        aicIsr = tcGenericIsrTc5;
        break;
    }
    default: {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TCTimerHandler::configurePeriodicInterrupt: Invalid"
                "TC peripheral!" << std::endl;
#else
        sif::printError("TCTimerHandler::configurePeriodicInterrupt: Invalid"
                "TC peripheral!\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    }
    if(isrRoutine == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "TCTimerHandler::configurePeriodicInterrupt: Passed "
                "callback function is a nullptr" << std::endl;
#else
        sif::printError("TCTimerHandler::configurePeriodicInterrupt: Passed "
                "callback function is a nullptr\n");
#endif
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    int peripheralNumber = static_cast<int>(tcPeripheral);
    oneShotIsr[peripheralNumber] = oneShotInterrupt;
    isrArgsBuffer[peripheralNumber] = isrArgs;
    isrRoutinesBuffer[peripheralNumber] = isrRoutine;
    return HasReturnvaluesIF::RETURN_OK;
}

void TCTimerHandler::tcGenericIsrTc0(void) {
    // Call the passed interrupt routine
    int peripheralId = static_cast<int>(TcPeripherals::TC0);
    isrRoutinesBuffer[peripheralId]((isrArgsBuffer[peripheralId]));
    if(oneShotIsr[peripheralId]) {
        TC_Stop(AT91C_BASE_TC0);
    }
    AT91C_BASE_TC0->TC_SR;
}

void TCTimerHandler::tcGenericIsrTc1(void) {
    // Call the passed interrupt routine
    int peripheralId = static_cast<int>(TcPeripherals::TC1);
    isrRoutinesBuffer[peripheralId](isrArgsBuffer[peripheralId]);
    if(oneShotIsr[peripheralId]) {
        TC_Stop(AT91C_BASE_TC1);
    }
    AT91C_BASE_TC1->TC_SR;
}

void TCTimerHandler::tcGenericIsrTc2(void) {
    // Call the passed interrupt routine
    int peripheralId = static_cast<int>(TcPeripherals::TC2);
    isrRoutinesBuffer[peripheralId](isrArgsBuffer[peripheralId]);
    if(oneShotIsr[peripheralId]) {
        TC_Stop(AT91C_BASE_TC2);
    }
}

void TCTimerHandler::tcGenericIsrTc3() {
    int peripheralId = static_cast<int>(TcPeripherals::TC3);
    isrRoutinesBuffer[peripheralId]((isrArgsBuffer[peripheralId]));
    if(oneShotIsr[peripheralId]) {
        TC_Stop(AT91C_BASE_TC3);
    }
    AT91C_BASE_TC3->TC_SR;
}

void TCTimerHandler::tcGenericIsrTc4() {
    int peripheralId = static_cast<int>(TcPeripherals::TC4);
    isrRoutinesBuffer[peripheralId]((isrArgsBuffer[peripheralId]));
    if(oneShotIsr[peripheralId]) {
        TC_Stop(AT91C_BASE_TC4);
    }
    AT91C_BASE_TC4->TC_SR;
}

void TCTimerHandler::tcGenericIsrTc5() {
    int peripheralId = static_cast<int>(TcPeripherals::TC5);
    isrRoutinesBuffer[peripheralId]((isrArgsBuffer[peripheralId]));
    if(oneShotIsr[peripheralId]) {
        TC_Stop(AT91C_BASE_TC5);
    }
    AT91C_BASE_TC5->TC_SR;
}
