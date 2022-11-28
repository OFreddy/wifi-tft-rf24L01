/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include "Arduino.h"

#include "hm_crc.h"
#include "hm_inverter.h"
#include "hm_data.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Macros

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Macros

template <class T = float>
static T calcYieldTotalCh0(HM_Inverter<> *iv, uint8_t arg0);

template <class T = float>
static T calcYieldDayCh0(HM_Inverter<> *iv, uint8_t arg0);

template <class T = float>
static T calcUdcCh(HM_Inverter<> *iv, uint8_t arg0);

template <class T = float>
static T calcPowerDcCh0(HM_Inverter<> *iv, uint8_t arg0);

template <class T = float>
static T calcEffiencyCh0(HM_Inverter<> *iv, uint8_t arg0);

template <class T = float>
static T calcIrradiation(HM_Inverter<> *iv, uint8_t arg0);

template <class T = float>
using func_t = T(HM_Inverter<> *, uint8_t);

template <class T = float>
struct calcFunc_t
{
    uint8_t funcId;  // unique id
    func_t<T> *func; // function pointer
};

// list of all available functions, mapped in hmDefines.h
template <class T = float>
const calcFunc_t<T> calcFunctions[] = {
    {CALC_YT_CH0, &calcYieldTotalCh0},
    {CALC_YD_CH0, &calcYieldDayCh0},
    {CALC_UDC_CH, &calcUdcCh},
    {CALC_PDC_CH0, &calcPowerDcCh0},
    {CALC_EFF_CH0, &calcEffiencyCh0},
    {CALC_IRR_CH, &calcIrradiation}};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class RECORDTYPE>
bool HM_Inverter<RECORDTYPE>::getAssignment()
{
   bool ret = true;
   // Default assignment;
   if (INV_TYPE_1CH == type)
   {
      listLen = (uint8_t)(HM1CH_LIST_LEN);
      assign = (HM_ByteAssign_t *)hm1chAssignment;
      channels = 1;
   }
   else if (INV_TYPE_2CH == type)
   {
      listLen = (uint8_t)(HM2CH_LIST_LEN);
      assign = (HM_ByteAssign_t *)hm2chAssignment;
      channels = 2;
   }
   else if (INV_TYPE_4CH == type)
   {
      listLen = (uint8_t)(HM4CH_LIST_LEN);
      assign = (HM_ByteAssign_t *)hm4chAssignment;
      channels = 4;
   }
   else
   {
      listLen = 0;
      channels = 0;
      assign = NULL;
      ret = false;
   }

   switch (getQueuedCmd())
   {
   case eCmd_RealTimeRunData_Debug:
      // Do nothing will use default
      break;
   case eCmd_InverterDevInform_All:
      listLen = (uint8_t)(HMINFO_LIST_LEN);
      assign = (HM_ByteAssign_t *)InfoAssignment;
      break;
   case eCmd_SystemConfigPara:
      listLen = (uint8_t)(HMSYSTEM_LIST_LEN);
      assign = (HM_ByteAssign_t *)SystemConfigParaAssignment;
      break;
   case eCmd_AlarmData:
      listLen = (uint8_t)(HMALARMDATA_LIST_LEN);
      assign = (HM_ByteAssign_t *)AlarmDataAssignment;
      break;
   default:
      HM_PRINTF(HM_PSTR("Command parser not implemented"));
      break;
   }

   return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class RECORDTYPE>
uint8_t HM_Inverter<RECORDTYPE>::getPosByChFld(uint8_t channel, uint8_t fieldId)
{
   uint8_t pos = 0;
   for (; pos < listLen; pos++)
   {
      if ((assign[pos].ch == channel) && (assign[pos].fieldId == fieldId))
         break;
   }
   return (pos >= listLen) ? 0xff : pos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class RECORDTYPE>
void HM_Inverter<RECORDTYPE>::addValue(uint8_t pos, uint8_t buf[])
{
   uint8_t cmd = getQueuedCmd();
   uint8_t ptr = assign[pos].start;
   uint8_t end = ptr + assign[pos].num;
   uint16_t div = assign[pos].div;
   if (CMD_CALC != div)
   {
      uint32_t val = 0;
      do
      {
         val <<= 8;
         val |= buf[ptr];
      } while (++ptr != end);
      if ((RECORDTYPE)(div) > 1)
      {
         record[pos] = (RECORDTYPE)(val) / (RECORDTYPE)(div);
      }
      else
      {
         record[pos] = (RECORDTYPE)(val);
      }
   }
   if (cmd == eCmd_RealTimeRunData_Debug)
   {
      // get last alarm message index and save it in the inverter object
      if (getPosByChFld(0, FLD_ALARM_MES_ID) == pos)
      {
         if (alarmMesIndex < record[pos])
         {
            alarmMesIndex = record[pos];
            // enqueCommand<InfoCommand>(AlarmUpdate); // What is the function of AlarmUpdate?
            enqueCommand<CommandInfo>(eCmd_AlarmData);
         }
         else
         {
            alarmMesIndex = record[pos]; // no change
         }
      }
   }
   if (cmd == eCmd_InverterDevInform_All)
   {
      // get at least the firmware version and save it to the inverter object
      if (getPosByChFld(0, FLD_FW_VERSION) == pos)
      {
         fwVersion = record[pos];
         HM_PRINTF(HM_PSTR("Inverter FW-Version: %u"), fwVersion);
      }
   }
   if (cmd == eCmd_SystemConfigPara)
   {
      // get at least the firmware version and save it to the inverter object
      if (getPosByChFld(0, FLD_ACT_PWR_LIMIT) == pos)
      {
         actPowerLimit = record[pos];
         HM_PRINTF(HM_PSTR("Inverter actual power limit: %u"), actPowerLimit);
      }
   }
   if (cmd == eCmd_AlarmData)
   {
      if (getPosByChFld(0, FLD_LAST_ALARM_CODE) == pos)
      {
         lastAlarmMsg = String("Unknown"); //getAlarmStr(record[pos]);
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class RECORDTYPE>
void HM_Inverter<RECORDTYPE>::doCalculations()
{
   uint8_t cmd = getQueuedCmd();
	getAssignment();
	if (cmd == eCmd_RealTimeRunData_Debug)
	{
		for (uint8_t i = 0; i < listLen; i++)
		{
			if (CMD_CALC == assign[i].div)
			{
				record[i] = calcFunctions<RECORDTYPE>[assign[i].start].func(this, assign[i].num);
			}
			yield();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * To calculate values which are not transmitted by the unit there is a generic
 * list of functions which can be linked to the assignment.
 * The special command 0xff (CMDFF) must be used.
 */

template <class T = float>
static T calcYieldTotalCh0(HM_Inverter<> *iv, uint8_t arg0)
{
    if (NULL != iv)
    {
        T yield = 0;
        for (uint8_t i = 1; i <= iv->channels; i++)
        {
            uint8_t pos = iv->getPosByChFld(i, FLD_YT);
            yield += iv->getValue(pos);
        }
        return yield;
    }
    return 0.0;
}

template <class T = float>
static T calcYieldDayCh0(HM_Inverter<> *iv, uint8_t arg0)
{
    if (NULL != iv)
    {
        T yield = 0;
        for (uint8_t i = 1; i <= iv->channels; i++)
        {
            uint8_t pos = iv->getPosByChFld(i, FLD_YD);
            yield += iv->getValue(pos);
        }
        return yield;
    }
    return 0.0;
}

template <class T = float>
static T calcUdcCh(HM_Inverter<> *iv, uint8_t arg0)
{
    // arg0 = channel of source
    for (uint8_t i = 0; i < iv->listLen; i++)
    {
        if ((FLD_UDC == iv->assign[i].fieldId) && (arg0 == iv->assign[i].ch))
        {
            return iv->getValue(i);
        }
    }

    return 0.0;
}

template <class T = float>
static T calcPowerDcCh0(HM_Inverter<> *iv, uint8_t arg0)
{
    if (NULL != iv)
    {
        T dcPower = 0;
        for (uint8_t i = 1; i <= iv->channels; i++)
        {
            uint8_t pos = iv->getPosByChFld(i, FLD_PDC);
            dcPower += iv->getValue(pos);
        }
        return dcPower;
    }
    return 0.0;
}

template <class T = float>
static T calcEffiencyCh0(HM_Inverter<> *iv, uint8_t arg0)
{
    if (NULL != iv)
    {
        uint8_t pos = iv->getPosByChFld(CH0, FLD_PAC);
        T acPower = iv->getValue(pos);
        T dcPower = 0;
        for (uint8_t i = 1; i <= iv->channels; i++)
        {
            pos = iv->getPosByChFld(i, FLD_PDC);
            dcPower += iv->getValue(pos);
        }
        if (dcPower > 0)
            return acPower / dcPower * 100.0f;
    }
    return 0.0;
}

template <class T = float>
static T calcIrradiation(HM_Inverter<> *iv, uint8_t arg0)
{
    // arg0 = channel
    if (NULL != iv)
    {
        uint8_t pos = iv->getPosByChFld(arg0, FLD_PDC);
        if (iv->chMaxPwr[arg0 - 1] > 0)
            return iv->getValue(pos) / iv->chMaxPwr[arg0 - 1] * 100.0f;
    }
    return 0.0;
}


// Forward declaration for every used template to activate linker
template class HM_Inverter<float>;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
