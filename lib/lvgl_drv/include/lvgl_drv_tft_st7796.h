#pragma once

// Command table 1
#define CMD_NOP 0x00       // No Operation
#define CMD_SWRESET 0x01   // Software Reset
#define CMD_RDDID 0x04     // Read Display ID
#define CMD_RDNUMED 0x05   // Read Number of the Errors on DSI
#define CMD_RDDST 0x09     // Read Display Status
#define CMD_RDDPM 0x0A     // Read Display Power Mode
#define CMD_RDDMADCTL 0x0B // Read Display MADCTL
#define CMD_RDDCOLMOD 0x0C // Read Display Pixel Format
#define CMD_RDDIM 0x0D     // Read Display Image Mode
#define CMD_RDDSM 0x0E     // Read Display Signal Mode
#define CMD_RDDSDR 0x0F    // Read Display Self-Diagnostic Result
#define CMD_SLPIN 0x10     // Sleep in
#define CMD_SLPOUT 0x11    // Sleep out
#define CMD_PTLON 0x12     // Partial Display Mode On
#define CMD_NORON 0x13     // Normal Display Mode On
#define CMD_INVOFF 0x20    // Display Inversion Off
#define CMD_INVON 0x21     // Display Inversion On
#define CMD_DISPOFF 0x28   // Display Off
#define CMD_DISPON 0x29    // Display On
#define CMD_CASET 0x2A     // Column Address Set
#define CMD_RASET 0x2B     // Row Address Set
#define CMD_RAMWR 0x2C     // Memory Write
#define CMD_RAMRD 0x2E     // Memory Read
#define CMD_PTLAR 0x30     // Partial Area
#define CMD_VSCRDEF 0x33   // Vertical Scrolling Definition
#define CMD_TEOFF 0x34     // Tearing Effect Line Off
#define CMD_TEON 0x35      // Tearing Effect Line On
#define CMD_MADCTL 0x36    // Memory Data Access Control
#define CMD_VSCSAD 0x37    // Vertical Scroll Start Address of RAM
#define CMD_IDMOFF 0x38    // Idle Mode Off
#define CMD_IDMON 0x39     // Idle Mode On
#define CMD_COLMOD 0x3A    // Interface Pixel Format
#define CMD_WRMEMC 0x3C    // Write Memory Continue
#define CMD_RDMEMC 0x3E    // Read Memory Continue
#define CMD_STE 0x44       // Set Tear Scanline
#define CMD_GSCAN 0x45     // Get Scanline
#define CMD_WRDISBV 0x51   // Write Display Brightness Value
#define CMD_RDDISBV 0x52   // Read Display Brightness Value
#define CMD_WRCTRLD 0x53   // Write CTRL Display
#define CMD_RDCTRLD 0x54   // Read CTRL Display
#define CMD_WRCABC 0x55    // Write Content Adaptive Brightness Control
#define CMD_RDCABC 0x56    // Read Content Adaptive Brightness Control
#define CMD_WRCABCMB 0x5E  // Write CABC Minimum Brightness
#define CMD_RDCABCMB 0x5F  // Read CABC Minimum Brightness
#define CMD_RDFCS 0xAA     // Read First Checksum
#define CMD_RDCFCS 0xAF    // Read Continue Checksum
#define CMD_RDID1 0xDA     // Read ID1
#define CMD_RDID2 0xDB     // Read ID2
#define CMD_RDID13 0xDC    // Read ID3

// Command table 2
#define CMD_IFMODE 0xB0    // Interface Mode Control
#define CMD_FRMCTR1 0xB1   // Frame Rate Control (In Normal Mode/Full Colors)
#define CMD_FRMCTR2 0xB2   // Frame Rate Control2 (In Idle Mode/8 colors)
#define CMD_FRMCTR3 0xB3   // Frame Rate Control3 (In Partial Mode/Full Colors)
#define CMD_DIC 0xB4       // Display Inversion Control
#define CMD_BPC 0xB5       // Blanking Porch Control
#define CMD_DFC 0xB6       // Display Function Control
#define CMD_EM 0xB7        // Entry Mode Set
#define CMD_PWCTR1 0xC0    // Power Control 1
#define CMD_PWCTR2 0xC1    // Power Control 2
#define CMD_PWCTR3 0xC2    // Power Control 3
#define CMD_VCMPCTL 0xC5   // VCOM Control
#define CMD_VCMOFFSET 0xC6 // VCOM Offset Register
#define CMD_NVMADW 0xD0    // NVM Address/Data Write
#define CMD_NVMBPROG 0xD1  // NVM Byte Program
#define CMD_NVMSTRD 0xD2   // NVM Status Read
#define CMD_RDID4 0xD3     // Read ID4
#define CMD_PGC 0E0        // Positive Gamma Control
#define CMD_NGC 0xE1       // Negative Gamma Control
#define CMD_DGC1 0xE2      // Digital Gamma Control 1
#define CMD_DGC2 0xE3      // Digital Gamma Control 2
#define CMD_DOCA 0xE8      // Display Output Ctrl Adjust
#define CMD_CSCON 0xF0     // Command Set Control
#define CMD_SPIRC 0xFB     // SPI Read Control

// MADCTL flags
#define MADCTL_MY 0x80  // Row Address Order - 0=Increment (Top to Bottom), 1=Decrement (Bottom to Top)
#define MADCTL_MX 0x40  // Column Address Order - 0=Increment (Left to Right), 1=Decrement (Right to Left)
#define MADCTL_MV 0x20  // Row/Column exchange - 0=Normal, 1=Row/Column exchanged
#define MADCTL_ML 0x10  // Scan Address Order - 0=Decrement, 1=Increment
#define MADCTL_BGR 0x08 // RGB/BGR Order - BGR
#define MADCTL_MH 0x04  // Horizontal Order - 0=Decrement (Left to Right), 1=Increment (Right to Left)
#define MADCTL_HF 0x02
#define MADCTL_VF 0x01
#define MADCTL_RGB 0x00 // RGB/BGR Order - RGB

// COLMOD modes
#define COLMOD_RGB_12BIT 0x30
#define COLMOD_RGB_16BIT 0x50
#define COLMOD_RGB_18BIT 0x60
#define COLMOD_CTRL_12BIT 0x03
#define COLMOD_CTRL_16BIT 0x05
#define COLMOD_CTRL_18BIT 0x06
#define COLMOD_CTRL_24BIT 0x07
#define COLMOD_RGB444 (COLMOD_RGB_12BIT | COLMOD_CTRL_12BIT)
#define COLMOD_RGB656 (COLMOD_RGB_16BIT | COLMOD_CTRL_16BIT)
#define COLMOD_RGB (COLMOD_RGB_18BIT | COLMOD_CTRL_18BIT)

// OPTIONS
#define OPTIONS_WRAP_V 0x01
#define OPTIONS_WRAP_H 0x02
#define OPTIONS_WRAP 0x03