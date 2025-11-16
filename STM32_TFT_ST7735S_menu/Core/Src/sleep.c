

#include "sleep.h"
//#include "iwdg.h"

void sleep(void){


	HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 4096, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
	HAL_SuspendTick();
	HAL_PWR_EnterSTANDBYMode();

}

void disable_wakeup(void) {
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
    HAL_ResumeTick();
}


