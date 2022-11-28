#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Global includes

#include "hm_types.h"

// units
enum {UNIT_V = 0, UNIT_A, UNIT_W,  UNIT_WH, UNIT_KWH, UNIT_HZ, UNIT_C, UNIT_PCT, UNIT_VA, UNIT_NONE};
const char* const units[] = {"V", "A", "W", "Wh", "kWh", "Hz", "°C", "%","VAr",""};


// field types
enum {FLD_UDC = 0, FLD_IDC, FLD_PDC, FLD_YD, FLD_YW, FLD_YT,
        FLD_UAC, FLD_IAC, FLD_PAC, FLD_F, FLD_T, FLD_PCT, FLD_EFF,
        FLD_IRR, FLD_PRA,FLD_ALARM_MES_ID,FLD_FW_VERSION,FLD_FW_BUILD_YEAR,
        FLD_FW_BUILD_MONTH_DAY,FLD_HW_ID,FLD_ACT_PWR_LIMIT,FLD_LAST_ALARM_CODE};
        
const char* const fields[] = {"U_DC", "I_DC", "P_DC", "YieldDay", "YieldWeek", "YieldTotal",
        "U_AC", "I_AC", "P_AC", "Freq", "Temp", "Pct", "Efficiency", "Irradiation","P_ACr",
        "ALARM_MES_ID","FWVersion","FWBuildYear","FWBuildMonthDay","HWPartId","PowerLimit","LastAlarmCode"};

// indices to calculation functions, defined in hmInverter.h
enum {CALC_YT_CH0 = 0, CALC_YD_CH0, CALC_UDC_CH, CALC_PDC_CH0, CALC_EFF_CH0, CALC_IRR_CH};
enum {CMD_CALC = 0xffff};

//-------------------------------------
// HM-Series
//-------------------------------------
const HM_ByteAssign_t InfoAssignment[] = {
    { FLD_FW_VERSION,           UNIT_NONE,   CH0,  0, 2, 1 },
    { FLD_FW_BUILD_YEAR,        UNIT_NONE,   CH0,  2, 2, 1 },
    { FLD_FW_BUILD_MONTH_DAY,   UNIT_NONE,   CH0,  4, 2, 1 },
    { FLD_HW_ID,                UNIT_NONE,   CH0,  8, 2, 1 }
};
#define HMINFO_LIST_LEN     (sizeof(InfoAssignment) / sizeof(HM_ByteAssign_t))

const HM_ByteAssign_t SystemConfigParaAssignment[] = {
    { FLD_ACT_PWR_LIMIT,           UNIT_PCT,   CH0,  2, 2, 10 }
};
#define HMSYSTEM_LIST_LEN     (sizeof(SystemConfigParaAssignment) / sizeof(HM_ByteAssign_t))

const HM_ByteAssign_t AlarmDataAssignment[] = {
    { FLD_LAST_ALARM_CODE,           UNIT_NONE,   CH0,  0, 2, 1 }
};
#define HMALARMDATA_LIST_LEN     (sizeof(AlarmDataAssignment) / sizeof(HM_ByteAssign_t))


//-------------------------------------
// HM300, HM350, HM400
//-------------------------------------
const HM_ByteAssign_t hm1chAssignment[] = {
    { FLD_UDC, UNIT_V,   CH1,  2, 2, 10   },
    { FLD_IDC, UNIT_A,   CH1,  4, 2, 100  },
    { FLD_PDC, UNIT_W,   CH1,  6, 2, 10   },
    { FLD_YD,  UNIT_WH,  CH1, 12, 2, 1    },
    { FLD_YT,  UNIT_KWH, CH1,  8, 4, 1000 },
    { FLD_IRR, UNIT_PCT, CH1, CALC_IRR_CH, CH1, CMD_CALC },

    { FLD_UAC, UNIT_V,   CH0, 14, 2, 10   },
    { FLD_IAC, UNIT_A,   CH0, 22, 2, 100  },
    { FLD_PAC, UNIT_W,   CH0, 18, 2, 10   },
    { FLD_PRA, UNIT_VA,  CH0, 20, 2, 10   },
    { FLD_F,   UNIT_HZ,  CH0, 16, 2, 100  },
    { FLD_T,   UNIT_C,   CH0, 26, 2, 10   },
    { FLD_ALARM_MES_ID,   UNIT_NONE,   CH0, 24, 2, 1 },
    { FLD_YD,  UNIT_WH,  CH0, CALC_YD_CH0,  0, CMD_CALC },
    { FLD_YT,  UNIT_KWH, CH0, CALC_YT_CH0,  0, CMD_CALC },
    { FLD_PDC, UNIT_W,   CH0, CALC_PDC_CH0, 0, CMD_CALC },
    { FLD_EFF, UNIT_PCT, CH0, CALC_EFF_CH0, 0, CMD_CALC }
};
#define HM1CH_LIST_LEN     (sizeof(hm1chAssignment) / sizeof(HM_ByteAssign_t))


//-------------------------------------
// HM600, HM700, HM800
//-------------------------------------
const HM_ByteAssign_t hm2chAssignment[] = {
    { FLD_UDC, UNIT_V,   CH1,  2, 2, 10   },
    { FLD_IDC, UNIT_A,   CH1,  4, 2, 100  },
    { FLD_PDC, UNIT_W,   CH1,  6, 2, 10   },
    { FLD_YD,  UNIT_WH,  CH1, 22, 2, 1    },
    { FLD_YT,  UNIT_KWH, CH1, 14, 4, 1000 },
    { FLD_IRR, UNIT_PCT, CH1, CALC_IRR_CH, CH1, CMD_CALC },

    { FLD_UDC, UNIT_V,   CH2,  8, 2, 10   },
    { FLD_IDC, UNIT_A,   CH2, 10, 2, 100  },
    { FLD_PDC, UNIT_W,   CH2, 12, 2, 10   },
    { FLD_YD,  UNIT_WH,  CH2, 24, 2, 1    },
    { FLD_YT,  UNIT_KWH, CH2, 18, 4, 1000 },
    { FLD_IRR, UNIT_PCT, CH2, CALC_IRR_CH, CH2, CMD_CALC },

    { FLD_UAC, UNIT_V,   CH0, 26, 2, 10   },
    { FLD_IAC, UNIT_A,   CH0, 34, 2, 100  },
    { FLD_PAC, UNIT_W,   CH0, 30, 2, 10   },
    { FLD_PRA, UNIT_VA,  CH0, 32, 2, 10   },
    { FLD_F,   UNIT_HZ,  CH0, 28, 2, 100  },
    { FLD_T,   UNIT_C,   CH0, 38, 2, 10   },
    { FLD_ALARM_MES_ID,   UNIT_NONE,   CH0, 40, 2, 1   },
    { FLD_YD,  UNIT_WH,  CH0, CALC_YD_CH0,  0, CMD_CALC },
    { FLD_YT,  UNIT_KWH, CH0, CALC_YT_CH0,  0, CMD_CALC },
    { FLD_PDC, UNIT_W,   CH0, CALC_PDC_CH0, 0, CMD_CALC },
    { FLD_EFF, UNIT_PCT, CH0, CALC_EFF_CH0, 0, CMD_CALC }

};
#define HM2CH_LIST_LEN     (sizeof(hm2chAssignment) / sizeof(HM_ByteAssign_t))


//-------------------------------------
// HM1200, HM1500
//-------------------------------------
const HM_ByteAssign_t hm4chAssignment[] = {
    { FLD_UDC, UNIT_V,   CH1,  2, 2, 10   },
    { FLD_IDC, UNIT_A,   CH1,  4, 2, 100  },
    { FLD_PDC, UNIT_W,   CH1,  8, 2, 10   },
    { FLD_YD,  UNIT_WH,  CH1, 20, 2, 1    },
    { FLD_YT,  UNIT_KWH, CH1, 12, 4, 1000 },
    { FLD_IRR, UNIT_PCT, CH1, CALC_IRR_CH, CH1, CMD_CALC },

    { FLD_UDC, UNIT_V,   CH2, CALC_UDC_CH, CH1, CMD_CALC },
    { FLD_IDC, UNIT_A,   CH2,  6, 2, 100  },
    { FLD_PDC, UNIT_W,   CH2, 10, 2, 10   },
    { FLD_YD,  UNIT_WH,  CH2, 22, 2, 1    },
    { FLD_YT,  UNIT_KWH, CH2, 16, 4, 1000 },
    { FLD_IRR, UNIT_PCT, CH2, CALC_IRR_CH, CH2, CMD_CALC },

    { FLD_UDC, UNIT_V,   CH3, 24, 2, 10   },
    { FLD_IDC, UNIT_A,   CH3, 26, 2, 100  },
    { FLD_PDC, UNIT_W,   CH3, 30, 2, 10   },
    { FLD_YD,  UNIT_WH,  CH3, 42, 2, 1    },
    { FLD_YT,  UNIT_KWH, CH3, 34, 4, 1000 },
    { FLD_IRR, UNIT_PCT, CH3, CALC_IRR_CH, CH3, CMD_CALC },

    { FLD_UDC, UNIT_V,   CH4, CALC_UDC_CH, CH3, CMD_CALC },
    { FLD_IDC, UNIT_A,   CH4, 28, 2, 100  },
    { FLD_PDC, UNIT_W,   CH4, 32, 2, 10   },
    { FLD_YD,  UNIT_WH,  CH4, 44, 2, 1    },
    { FLD_YT,  UNIT_KWH, CH4, 38, 4, 1000 },
    { FLD_IRR, UNIT_PCT, CH4, CALC_IRR_CH, CH4, CMD_CALC },

    { FLD_UAC, UNIT_V,   CH0, 46, 2, 10   },
    { FLD_IAC, UNIT_A,   CH0, 54, 2, 100  },
    { FLD_PAC, UNIT_W,   CH0, 50, 2, 10   },
    { FLD_PRA, UNIT_VA,  CH0, 52, 2, 10   },
    { FLD_F,   UNIT_HZ,  CH0, 48, 2, 100  },
    { FLD_PCT, UNIT_PCT, CH0, 56, 2, 10   },
    { FLD_T,   UNIT_C,   CH0, 58, 2, 10   },
    { FLD_ALARM_MES_ID,   UNIT_NONE,   CH0, 60, 2, 1   },
    { FLD_YD,  UNIT_WH,  CH0, CALC_YD_CH0,  0, CMD_CALC },
    { FLD_YT,  UNIT_KWH, CH0, CALC_YT_CH0,  0, CMD_CALC },
    { FLD_PDC, UNIT_W,   CH0, CALC_PDC_CH0, 0, CMD_CALC },
    { FLD_EFF, UNIT_PCT, CH0, CALC_EFF_CH0, 0, CMD_CALC }
};
#define HM4CH_LIST_LEN     (sizeof(hm4chAssignment) / sizeof(HM_ByteAssign_t))

