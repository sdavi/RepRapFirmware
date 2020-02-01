/*
 * Fan.h
 *
 *  Created on: 29 Jun 2016
 *      Author: David
 */

#ifndef SRC_FAN_H_
#define SRC_FAN_H_

#include "RepRapFirmware.h"
#include <ObjectModel/ObjectModel.h>
#include "Hardware/IoPorts.h"
#include "GCodes/GCodeResult.h"

#if SUPPORT_CAN_EXPANSION
# include "CanId.h"
#endif

class GCodeBuffer;

class Fan INHERIT_OBJECT_MODEL
{
public:
	Fan(unsigned int fanNum) noexcept;

	virtual ~Fan() noexcept { }
	virtual bool Check() noexcept = 0;								// update the fan PWM returning true if it is a thermostatic fan that is on
	virtual GCodeResult SetPwmFrequency(PwmFrequency freq, const StringRef& reply) noexcept = 0;
	virtual bool IsEnabled() const noexcept = 0;
	virtual GCodeResult ReportPortDetails(const StringRef& str) const noexcept = 0;
#if SUPPORT_CAN_EXPANSION
	virtual void UpdateRpmFromRemote(CanAddress src, int32_t rpm) noexcept = 0;
#endif

	// Set or report the parameters for this fan
	// If 'mCode' is an M-code used to set parameters (which should only ever be 106 or 107)
	// then search for parameters used to configure the fan. If any are found, perform appropriate actions and return true.
	// If errors were discovered while processing parameters, put an appropriate error message in 'reply' and set 'error' to true.
	// If no relevant parameters are found, print the existing ones to 'reply' and return false.
	bool Configure(unsigned int mcode, size_t fanNum, GCodeBuffer& gb, const StringRef& reply, bool& error);

	float GetConfiguredPwm() const noexcept { return val; }			// returns the configured PWM. Actual PWM may be different, e.g. due to blipping or for thermostatic fans.
	int32_t GetRPM() const noexcept;

	GCodeResult SetPwm(float speed, const StringRef& reply) noexcept;
	bool HasMonitoredSensors() const noexcept { return sensorsMonitored.IsNonEmpty(); }
	const char *GetName() const noexcept { return name.c_str(); }

#if HAS_MASS_STORAGE
	bool WriteSettings(FileStore *f, size_t fanNum) const noexcept;	// save the settings of this fan if it isn't thermostatic
#endif

protected:
	DECLARE_OBJECT_MODEL

	static constexpr uint32_t RpmReadingTimeout = 2000;		// any reading older than this number of milliseconds is considered unreliable

	virtual GCodeResult Refresh(const StringRef& reply) noexcept = 0;
	virtual bool UpdateFanConfiguration(const StringRef& reply) noexcept = 0;

	void SetLastRpm(float rpm) noexcept { lastRpm = rpm; whenLastRpmSet = millis(); }

	unsigned int fanNumber;

	// Variables that control the fan
	float val;
	float lastVal;
	float minVal;
	float maxVal;
	mutable int32_t lastRpm;
	uint32_t whenLastRpmSet;
	float triggerTemperatures[2];
	uint32_t blipTime;										// in milliseconds
	SensorsBitmap sensorsMonitored;

	String<MaxFanNameLength> name;
};

#endif /* SRC_FAN_H_ */
