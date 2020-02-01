/*
 * Board Config
 *
 *  Created on: 3 Feb 2019
 *      Author: sdavi
 */

#ifndef BOARDCONFIG_H_
#define BOARDCONFIG_H_

#include "Core.h"
#include "RepRapFirmware.h"
#include "MessageType.h"
#include "Storage/FileStore.h"
#include "RepRap.h"


enum configValueType{
    cvPinType = 0,
    cvBoolType,
    cvUint8Type,
    cvUint16Type,
    cvUint32Type,
    cvFloatType,
    cvStringType,
};

struct boardConfigEntry_t
{
    const char* key;
    void *variable;
    const size_t *maxArrayEntries;
    configValueType type;
};


class Platform; //fwd decl

class BoardConfig {

public:
    static void Init() noexcept;

    static void Diagnostics(MessageType mtype) noexcept;
    static Pin StringToPin(const char *strvalue) noexcept;
    static Pin LookupPin(char *strvalue) noexcept;

    
private:
    BoardConfig()  noexcept;
    static bool GetConfigKeys(FileStore *configFile, const boardConfigEntry_t *boardConfigEntryArray, const size_t numConfigs) noexcept;
    static void SetValueFromString(configValueType type, void *variable, char *valuePtr) noexcept;
    static void PrintValue(MessageType mtype, configValueType configType, void *variable) noexcept;
    static void PrintPinArray(MessageType mtype, Pin arr[], uint16_t numEntries) noexcept;
};

#endif /* BOARDCONFIG_H_ */
