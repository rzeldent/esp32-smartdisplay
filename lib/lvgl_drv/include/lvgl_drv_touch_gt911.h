#pragma once

#define GT911_I2C_SLAVE_ADDR 0x5D

#define GT911_MAX_CONTACTS 5

#define GT911_PRODUCT_ID_LEN 4

#define GT911_REG_CFG 0x8047
#define GT911_REG_CHECKSUM 0x80FF
#define GT911_REG_DATA 0x8140
#define GT911_REG_ID 0x8140
#define GT911_REG_COORD_ADDR 0x814E

// Register Map of GT911
#define GT911_PRODUCT_ID1 0x8140
#define GT911_PRODUCT_ID2 0x8141
#define GT911_PRODUCT_ID3 0x8142
#define GT911_PRODUCT_ID4 0x8143
#define GT911_FIRMWARE_VER_L 0x8144
#define GT911_FIRMWARE_VER_H 0x8145
#define GT911_X_COORD_RES_L 0x8146
#define GT911_X_COORD_RES_H 0x8147
#define GT911_Y_COORD_RES_L 0x8148
#define GT911_Y_COORD_RES_H 0x8149
#define GT911_VENDOR_ID 0x814A

#define GT911_STATUS_REG 0x814E
#define GT911_STATUS_REG_BUF 0x80
#define GT911_STATUS_REG_LARGE 0x40
#define GT911_STATUS_REG_PROX_VALID 0x20
#define GT911_STATUS_REG_HAVEKEY 0x10
#define GT911_STATUS_REG_PT_MASK 0x0F

#define GT911_TRACK_ID1 0x814F
#define GT911_PT1_X_COORD_L 0x8150
#define GT911_PT1_X_COORD_H 0x8151
#define GT911_PT1_Y_COORD_L 0x8152
#define GT911_PT1_Y_COORD_H 0x8153
#define GT911_PT1_X_SIZE_L 0x8154
#define GT911_PT1_X_SIZE_H 0x8155

struct __attribute__((packed)) GTInfo
{
    char productId[4];    // 0x8140 - 0x8143
    uint16_t fwId;        // 0x8144 - 0x8145
    uint16_t xResolution; // 0x8146 - 0x8147
    uint16_t yResolution; // 0x8148 - 0x8149
    uint8_t vendorId;     // 0x814A
};

struct __attribute__((packed)) GTPoint
{
    // 0x814F-0x8156, ... 0x8176 (5 points)
    uint8_t trackId;
    uint16_t x;
    uint16_t y;
    uint16_t area;
    uint8_t reserved;
};

struct __attribute__((packed)) GTConfig
{
    uint8_t configVersion;        // 0x8047
    uint16_t xResolution;         // 0x8048 - 0x8049
    uint16_t yResolution;         // 0x804A - 0x804B
    uint8_t touchNumber;          // 0x804C
    uint8_t moduleSwitch1;        // 0x804D
    uint8_t moduleSwitch2;        // 0x804E
    uint8_t shakeCount;           // 0x804F
    uint8_t filter;               // 0x8050
    uint8_t largeTouch;           // 0x8051
    uint8_t noiseReduction;       // 0x8052
    uint8_t screenTouchLevel;     // 0x8053
    uint8_t screenLeaveLevel;     // 0x8054
    uint8_t lowPowerControl;      // 0x8055
    uint8_t refreshRate;          // 0x8056
    uint8_t xThreshold;           // 0x8057
    uint8_t yThreshold;           // 0x8058
    uint8_t xSpeedLimit;          // 0x8059 - reserved
    uint8_t ySpeedLimit;          // 0x805A - reserved
    uint8_t vSpace;               // 0x805B
    uint8_t hSpace;               // 0x805C
    uint8_t miniFilter;           // 0x805D
    uint8_t stretchR0;            // 0x805E
    uint8_t stretchR1;            // 0x805F
    uint8_t stretchR2;            // 0x8060
    uint8_t stretchRM;            // 0x8061
    uint8_t drvGroupANum;         // 0x8062
    uint8_t drvGroupBNum;         // 0x8063
    uint8_t sensorNum;            // 0x8064
    uint8_t freqAFactor;          // 0x8065
    uint8_t freqBFactor;          // 0x8066
    uint16_t pannelBitFreq;       // 0x8067 - 0x8068
    uint16_t pannelSensorTime;    // 0x8069 - 0x806A
    uint8_t pannelTxGain;         // 0x806B
    uint8_t pannelRxGain;         // 0x806C
    uint8_t pannelDumpShift;      // 0x806D
    uint8_t drvFrameControl;      // 0x806E
    uint8_t chargingLevelUp;      // 0x806F
    uint8_t moduleSwitch3;        // 0x8070
    uint8_t gestureDis;           // 0x8071
    uint8_t gestureLongPressTime; // 0x8072
    uint8_t xySlopeAdjust;        // 0x8073
    uint8_t gestureControl;       // 0x8074
    uint8_t gestureSwitch1;       // 0x8075
    uint8_t gestureSwitch2;       // 0x8076
    uint8_t gestureRefreshRate;   // 0x8077
    uint8_t gestureTouchLevel;    // 0x8078
    uint8_t newGreenWakeUpLevel;  // 0x8079
    uint8_t freqHoppingStart;     // 0x807A
    uint8_t freqHoppingEnd;       // 0x807B
    uint8_t noiseDetectTimes;     // 0x807C
    uint8_t hoppingFlag;          // 0x807D
    uint8_t hoppingThreshold;     // 0x807E
    uint8_t noiseThreshold;       // 0x807F
    uint8_t noiseMinThreshold;    // 0x8080
    uint8_t NC_1;                 // 0x8081
    uint8_t hoppingSensorGroup;   // 0x8082
    uint8_t hoppingSeg1Normalize; // 0x8083
    uint8_t hoppingSeg1Factor;    // 0x8084
    uint8_t mainClockAjdust;      // 0x8085
    uint8_t hoppingSeg2Normalize; // 0x8086
    uint8_t hoppingSeg2Factor;    // 0x8087
    uint8_t NC_2;                 // 0x8088
    uint8_t hoppingSeg3Normalize; // 0x8089
    uint8_t hoppingSeg3Factor;    // 0x808A
    uint8_t NC_3;                 // 0x808B
    uint8_t hoppingSeg4Normalize; // 0x808C
    uint8_t hoppingSeg4Factor;    // 0x808D
    uint8_t NC_4;                 // 0x808E
    uint8_t hoppingSeg5Normalize; // 0x808F
    uint8_t hoppingSeg5Factor;    // 0x8090
    uint8_t NC_5;                 // 0x8091
    uint8_t hoppingSeg6Normalize; // 0x8092
    uint8_t key[4];               // 0x8093 - 0x8096
    uint8_t keyArea;              // 0x8097
    uint8_t keyTouchLevel;        // 0x8098
    uint8_t keyLeaveLevel;        // 0x8099
    uint8_t keySens[2];           // 0x809A - 0x809B
    uint8_t keyRestrain;          // 0x809C
    uint8_t keyRestrainTime;      // 0x809D
    uint8_t gestureLargeTouch;    // 0x809E
    uint8_t NC_6[2];              // 0x809F - 0x80A0
    uint8_t hotknotNoiseMap;      // 0x80A1
    uint8_t linkThreshold;        // 0x80A2
    uint8_t pxyThreshold;         // 0x80A3
    uint8_t gHotDumpShift;        // 0x80A4
    uint8_t gHotRxGain;           // 0x80A5
    uint8_t freqGain[4];          // 0x80A6 - 0x80A9
    uint8_t NC_7[9];              // 0x80AA - 0x80B2
    uint8_t combineDis;           // 0x80B3
    uint8_t splitSet;             // 0x80B4
    uint8_t NC_8[2];              // 0x80B5 - 0x80B6
    uint8_t sensorCH[14];         // 0x80B7 - 0x80C4
    uint8_t NC_9[16];             // 0x80C5 - 0x80D4
    uint8_t driverCH[26];         // 0x80D5 - 0x80EE
    uint8_t NC_10[16];            // 0x80EF - 0x80FE
};