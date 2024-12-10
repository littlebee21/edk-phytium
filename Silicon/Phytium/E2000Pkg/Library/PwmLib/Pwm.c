/** @file
 *
**/
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PhytiumGpioLib.h>
#include <Library/PhytiumPwmLib.h>

UINTN pwm_reg_base[]={
	0x2804a400,
	0x2804a800,
	0x2804b400,
	0x2804b800,
	0x2804c400,
	0x2804c800,
	0x2804d400,
	0x2804d800,
	0x2804e400,
	0x2804e800,
	0x2804f400,
	0x2804f800,
	0x28050400,
	0x28050800,
	0x28051400,
	0x28051800,
};
VOID
PwmInit(
  IN UINT8 PwmPort
  )
{
  MmioWrite32(pwm_reg_base[PwmPort]+0x4,0x770000);
  MmioWrite32(pwm_reg_base[PwmPort]+0xc,0x64);
  MmioWrite32(pwm_reg_base[PwmPort]+0x10,0x44);
  MmioWrite32(pwm_reg_base[PwmPort]+0x14,0x32);
  MmioWrite32(pwm_reg_base[PwmPort]+0x4,0x770002);
  if((PwmPort % 2 )==0)
  {
	  MmioWrite32(0x2807e020,1 << PwmPort/2);
  }else{
	  MmioWrite32(0x2807e020,1 << (PwmPort-1)/2);
  }
  //DEBUG((EFI_D_INFO,"PwmPort: PWM%d, addrbase:0x%x\n",PwmPort,pwm_reg_base[PwmPort]));

}

VOID 
EdpPowerGpioEnable(
  IN UINT8 PowerOn
  )
{
  UINT8 BacklightPwrIndex;
  UINT8 BacklightPwrPort;
  
  //背光电源引脚配置
  BacklightPwrIndex = (PcdGet8(PcdBacklightPwrGpio) >> 4) & 0xF;         
  BacklightPwrPort = (PcdGet8(PcdBacklightPwrGpio) >> 0) & 0xF;
 // DEBUG((EFI_D_INFO,"BacklightpwrIndex:%dBacklightpwrPort: %d\n",BacklightPwrIndex,BacklightPwrPort));

  if (1 == PowerOn){
     DirectSetGpioOutPutLevel(BacklightPwrIndex,BacklightPwrPort,1);
     DEBUG((EFI_D_INFO,"edp backlightpwr on\n"));
  }
  else{
     DirectSetGpioOutPutLevel(BacklightPwrIndex,BacklightPwrPort,0);
     DEBUG((EFI_D_INFO,"edp backlightpwr off\n"));
  }    
} 

VOID
EdpBacklightPortEnable(
  IN UINT8 LightOpen
  )
{
  UINT8 BacklightEnableIndex;
  UINT8 BacklightEnablePort;
  UINT8 PwmPortNum;


  /*背光使能引脚配置*/
  BacklightEnableIndex = (PcdGet8(PcdBacklightEnableGpio) >> 4) & 0xF;
  BacklightEnablePort = (PcdGet8(PcdBacklightEnableGpio) >> 0) & 0xF;
  PwmPortNum = PcdGet8(PcdBacklightPwmPort);
 // DEBUG((EFI_D_INFO,"BacklightEnableIndex:%dBacklightEnablePort: %d\n",BacklightEnableIndex,BacklightEnablePort));

  if (1 == LightOpen){
     /*背光PWM配置*/
     DirectSetGpioOutPutLevel(BacklightEnableIndex,BacklightEnablePort,1);
     PwmInit(PwmPortNum);
     DEBUG((EFI_D_INFO,"edp backlight Enable\n"));
  }
  else{
     DirectSetGpioOutPutLevel(BacklightEnableIndex,BacklightEnablePort,0);
     DEBUG((EFI_D_INFO,"edp backlight Disable\n"));
  }
}




