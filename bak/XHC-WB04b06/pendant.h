/*
   Copyright (C) 2017 Raoul Rubien (github.com/rubienr)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the program; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.
 */

#pragma once

// local includes
#include "pendant-types.h"

// system includes
#include <list>
#include <stdint.h>
#include <type_traits>

#include <map>

// 3rd party includes

// local library includes

// forward declarations

// ----------------------------------------------------------------------

namespace XhcWhb04b6
{

	// forward declarations
	class XhcWhb04b6Component;
	class UsbOutPackageData;

	// ----------------------------------------------------------------------

	//! If hand wheel is in step mode (toggled by Step/Continuous" button) this
	//! speed setting is applied. In step mode the step is in machine units
	//! distance.
	class HandwheelStepModeStepSize
	{
	public:
		enum class PositionNameIndex : uint8_t
		{
			RotaryButton0001 = 0,
			RotaryButton0010 = 1,
			RotaryButton0100 = 2,
			RotaryButton100 = 3,
			NA0 = 4,
			NA1 = 5,
			NA2 = 6,
			RotaryButtonUndefined = 7,
			POSITIONS_COUNT = 8,
		};

		HandwheelStepModeStepSize();
		~HandwheelStepModeStepSize();

		//! Translates the button position to step metric units.
		//! \param buttonPosition
		//! \return the step size in units ℝ ∈ {0.001, 0.01, 0.1, 1.0, -1}
		float GetStepSize(PositionNameIndex buttonPosition) const;
		virtual bool IsPermitted(PositionNameIndex buttonPosition) const;

	private:
		const float mySequence[static_cast<
			std::underlying_type<HandwheelStepModeStepSize::PositionNameIndex>::type>(
			PositionNameIndex::POSITIONS_COUNT)];
	};

	// ----------------------------------------------------------------------

	//! If hand wheel is in continuous mode (toggled by Step/Continuous" button)
	//! this speed setting is applied. In continuous mode the step speed is in
	//! percent of max-velocity.
	class HandwheelContinuousModeStepSize
	{
	public:
		enum class PositionNameIndex : uint8_t
		{
			RotaryButton2percent = 0,
			RotaryButton5percent = 1,
			RotaryButton10percent = 2,
			RotaryButton30percent = 3,
			RotaryButton60percent = 4,
			RotaryButton100percent = 5,
			NA0 = 6,
			RotaryButtonUndefined = 7,
			POSITIONS_COUNT = 8
		};

		HandwheelContinuousModeStepSize();
		~HandwheelContinuousModeStepSize();

		//! Translates the button position to step size in %.
		//! \param buttonPosition
		//! \return the step size in percent ℕ ∈ {[0, 100], -1}
		float GetStepSize(PositionNameIndex buttonPosition) const;
		virtual bool IsPermitted(PositionNameIndex buttonPosition) const;

	private:
		const int8_t mySequence[static_cast<std::underlying_type<
			HandwheelContinuousModeStepSize::PositionNameIndex>::type>(
			PositionNameIndex::POSITIONS_COUNT)];
	};

	// ----------------------------------------------------------------------

	//! If hand wheel is in Lead mode (activated by the feed rotary button) this
	//! speed setting is applied.
	class HandwheelLeadModeStepSize
	{
	public:
		enum class PositionNameIndex : uint8_t
		{
			NA0 = 0,
			NA1 = 1,
			NA2 = 2,
			NA3 = 3,
			NA4 = 4,
			NA5 = 5,
			LEAD = 6,
			UNDEFINED = 7,
			POSITIONS_COUNT = 8
		};

		HandwheelLeadModeStepSize();
		~HandwheelLeadModeStepSize();

		//! Translates the button position to step size.
		//! \param buttonPosition
		//! \return the step size ℕ ∈ {1.0, -1.0}
		float GetStepSize(PositionNameIndex buttonPosition) const;
		virtual bool IsPermitted(PositionNameIndex buttonPosition) const;

	private:
		const float mySequence[static_cast<
			std::underlying_type<HandwheelLeadModeStepSize::PositionNameIndex>::type>(
			PositionNameIndex::POSITIONS_COUNT)];
	};

	// ----------------------------------------------------------------------

	//! pendant button key code description
	class KeyCode
	{
	public:
		const uint8_t code;
		//! default button text as written on pendant (if available)
		const char *text;
		//! alternative button text as written on pendant (if available)
		const char *altText;
		bool operator==(const KeyCode &other) const;
		bool operator!=(const KeyCode &other) const;
		KeyCode(uint8_t code, const char *text, const char *altText);
		KeyCode(const KeyCode &other);
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const KeyCode &data);

	// ----------------------------------------------------------------------

	//! meta-button state which is dependent on the "Fn" modifier button's state
	class MetaButtonCodes
	{
	public:
		const KeyCode &key;
		const KeyCode &modifier;

		bool operator==(const MetaButtonCodes &other) const;
		bool operator!=(const MetaButtonCodes &other) const;
		MetaButtonCodes(const KeyCode &key, const KeyCode &modifier);
		virtual ~MetaButtonCodes();
		bool ContainsKeys(const KeyCode &key, const KeyCode &modifier) const;
	};

	// ----------------------------------------------------------------------

	//! rotary axis selection button related parameters
	class AxisRotaryButtonCodes
	{
	public:
		const KeyCode off;
		const KeyCode x;
		const KeyCode y;
		const KeyCode z;
		const KeyCode a;
		const KeyCode b;
		const KeyCode c;
		const KeyCode undefined;
		const std::map<uint8_t, const KeyCode *> codeMap;

		AxisRotaryButtonCodes();
	};

	// ----------------------------------------------------------------------

	//! rotary feed button related parameters
	class FeedRotaryButtonCodes
	{
	public:
		const KeyCode speed_0_001;
		const KeyCode speed_0_01;
		const KeyCode speed_0_1;
		const KeyCode speed_1;
		const KeyCode percent_60;
		const KeyCode percent_100;
		const KeyCode lead;
		const KeyCode undefined;
		const std::map<uint8_t, const KeyCode *> codeMap;

		FeedRotaryButtonCodes();
	};

	// ----------------------------------------------------------------------

	//! pendant button related parameters
	class ButtonsCode
	{
	public:
		const KeyCode reset;
		const KeyCode stop;
		const KeyCode start;
		const KeyCode feed_plus;
		const KeyCode feed_minus;
		const KeyCode spindle_plus;
		const KeyCode spindle_minus;
		const KeyCode machine_home;
		const KeyCode safe_z;
		const KeyCode workpiece_home;
		const KeyCode spindle_on_off;
		const KeyCode function;
		const KeyCode probe_z;
		const KeyCode macro10;
		const KeyCode manual_pulse_generator;
		const KeyCode step_continuous;
		const KeyCode undefined;
		const KeyCode &GetKeyCode(uint8_t keyCode) const;
		const std::map<uint8_t, const KeyCode *> codeMap;

		ButtonsCode();
	};

	// ----------------------------------------------------------------------

	class KeyEventListener
	{
	public:
		//! Called when button is pressed.
		//! \param softwareButton the button pressed
		//! \return true if a subsequent re-evaluation should be performed.
		//! Example: A button event changes the feed rotary buttons step mode from
		//! step to continuous. The button must be re-evaluated, otherwise the
		//! button state remains untouched until the next button's event.
		virtual bool OnButtonPressedEvent(const MetaButtonCodes &metaButton) = 0;
		//! Called when button is released.
		//! \param softwareButton the button released
		//! \return true if a subsequent re-evaluation should be performed.
		//! Example: A button event changes the feed rotary buttons step mode from
		//! step to continuous. The button must be re-evaluated, otherwise the
		//! button state remains untouched until the next button's event.
		virtual bool OnButtonReleasedEvent(const MetaButtonCodes &metaButton) = 0;
		virtual void OnAxisActiveEvent(const KeyCode &axis) = 0;
		virtual void OnAxisInactiveEvent(const KeyCode &axis) = 0;
		virtual void OnFeedActiveEvent(const KeyCode &feed) = 0;
		virtual void OnFeedInactiveEvent(const KeyCode &feed) = 0;
		virtual bool OnJogDialEvent(const HandWheelCounters &counters,
									int8_t delta) = 0;
		virtual ~KeyEventListener();
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const MetaButtonCodes &data);

	// ----------------------------------------------------------------------

	class MetaButtonsCodes
	{
	public:
		const MetaButtonCodes reset;
		const MetaButtonCodes macro11;
		const MetaButtonCodes stop;
		const MetaButtonCodes macro12;
		const MetaButtonCodes start;
		const MetaButtonCodes macro13;
		const MetaButtonCodes feed_plus;
		const MetaButtonCodes macro1;
		const MetaButtonCodes feed_minus;
		const MetaButtonCodes macro2;
		const MetaButtonCodes spindle_plus;
		const MetaButtonCodes macro3;
		const MetaButtonCodes spindle_minus;
		const MetaButtonCodes macro4;
		const MetaButtonCodes machine_home;
		const MetaButtonCodes macro5;
		const MetaButtonCodes safe_z;
		const MetaButtonCodes macro6;
		const MetaButtonCodes workpiece_home;
		const MetaButtonCodes macro7;
		const MetaButtonCodes spindle_on_off;
		const MetaButtonCodes macro8;
		const MetaButtonCodes function;
		const MetaButtonCodes probe_z;
		const MetaButtonCodes macro9;
		const MetaButtonCodes macro10;
		const MetaButtonCodes macro14;
		const MetaButtonCodes manual_pulse_generator;
		const MetaButtonCodes macro15;
		const MetaButtonCodes step_continuous;
		const MetaButtonCodes macro16;
		const MetaButtonCodes undefined;

		const std::list<const MetaButtonCodes *> buttons;

		MetaButtonsCodes(const ButtonsCode &buttons);
		~MetaButtonsCodes();

		const MetaButtonCodes &Find(const KeyCode &keyCode,
									const KeyCode &modifierCode) const;
	};

	// ----------------------------------------------------------------------

	class KeyCodes
	{
	public:
		static const ButtonsCode Buttons;
		static const MetaButtonsCodes Meta;
		static const AxisRotaryButtonCodes Axis;
		static const FeedRotaryButtonCodes Feed;
	};

	// ----------------------------------------------------------------------

	class Button
	{
	public:
		Button(const KeyCode &key);
		virtual ~Button();
		virtual const KeyCode &KeyCode() const;
		virtual bool SetKeyCode(const KeyCode &keyCode);
		Button &operator=(const Button &other);

	protected:
		const KeyCode *myKey;
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const Button &data);

	// ----------------------------------------------------------------------

	//! meta-button state which is dependent on the "Fn" modifier button's state
	class ToggleButton : public Button
	{
	public:
		ToggleButton(const KeyCode &key, const KeyCode &modifier);
		virtual ~ToggleButton();
		virtual const KeyCode &ModifierCode() const;
		virtual void SetModifierCode(KeyCode &modifierCode);
		bool ContainsKeys(const KeyCode &key, const KeyCode &modifier) const;
		ToggleButton &operator=(const ToggleButton &other);

	private:
		const KeyCode *myModifier;
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const ToggleButton &data);

	// ----------------------------------------------------------------------

	class RotaryButton : public Button
	{
	public:
		RotaryButton(const KeyCode &keyCode);
		virtual ~RotaryButton();
		virtual bool IsPermitted() const = 0;
		RotaryButton &operator=(const RotaryButton &other);
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const RotaryButton &data);

	// ----------------------------------------------------------------------

	class FeedRotaryButton : public RotaryButton
	{
	public:
		FeedRotaryButton(
			const KeyCode &keyCode = KeyCodes::Feed.undefined,
			HandwheelStepmodes::Mode stepMode = HandwheelStepmodes::Mode::CONTINUOUS,
			KeyEventListener *listener = nullptr);
		~FeedRotaryButton();
		virtual bool SetKeyCode(const KeyCode &keyCode) override;
		void SetStepMode(HandwheelStepmodes::Mode stepMode);
		HandwheelStepmodes::Mode StepMode() const;
		float StepSize() const;
		bool IsPermitted() const override;
		FeedRotaryButton &operator=(const FeedRotaryButton &other);

	private:
		HandwheelStepmodes::Mode myStepMode;
		bool myIsPermitted;
		float myStepSize;
		KeyEventListener *myEventListener;

		static const HandwheelStepModeStepSize myStepStepSizeMapper;
		static const HandwheelContinuousModeStepSize myContinuousSizeMapper;
		static const HandwheelLeadModeStepSize myLeadStepSizeMapper;

		static const std::map<const KeyCode *,
							  HandwheelStepModeStepSize::PositionNameIndex>
			myStepKeycodeLut;
		static const std::map<const KeyCode *,
							  HandwheelContinuousModeStepSize::PositionNameIndex>
			myContinuousKeycodeLut;
		static const std::map<const KeyCode *,
							  HandwheelLeadModeStepSize::PositionNameIndex>
			myLeadKeycodeLut;

		void Update();
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const FeedRotaryButton &data);

	// ----------------------------------------------------------------------

	class AxisRotaryButton : public RotaryButton
	{
	public:
		AxisRotaryButton(const KeyCode &keyCode = KeyCodes::Axis.undefined,
						 KeyEventListener *listener = nullptr);
		virtual ~AxisRotaryButton();
		bool IsPermitted() const override;
		AxisRotaryButton &operator=(const AxisRotaryButton &other);

	private:
		KeyEventListener *myEventListener;
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const AxisRotaryButton &data);

	// ----------------------------------------------------------------------

	class Handwheel
	{
	public:
		Handwheel(const FeedRotaryButton &feedButton,
				  KeyEventListener *listener = nullptr);
		~Handwheel();
		void SetMode(HandWheelCounters::CounterNameToIndex mode);
		void Count(int8_t delta);
		const HandWheelCounters &Counters() const;
		HandWheelCounters &Counters();
		//! En-/disables the incremental counter.
		//! \param enabled false if counting should be inhibited, true otherwise
		void SetEnabled(bool enabled);

	private:
		HandWheelCounters myCounters;
		bool myIsEnabled{false};
		const FeedRotaryButton &myFeedButton;
		KeyEventListener *myEventListener;
		std::ostream *myWheelCout;
		const char *myPrefix;
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const Handwheel &data);

	// ----------------------------------------------------------------------

	class ButtonsState
	{
	public:
		ButtonsState(KeyEventListener *listener = nullptr,
					 const ButtonsState *previousState = nullptr);
		~ButtonsState();

		ButtonsState &operator=(const ButtonsState &other);

		void Update(const KeyCode &keyCode, const KeyCode &modifierCode,
					const KeyCode &axisButton, const KeyCode &feedButton);

		void ClearPressedButtons();

		const std::list<const KeyCode *> &PressedButtons() const;
		const MetaButtonCodes *CurrentMetaButton() const;
		const AxisRotaryButton &AxisButton() const;
		const FeedRotaryButton &FeedButton() const;
		FeedRotaryButton &FeedButton();

	private:
		std::list<const KeyCode *> myPressedButtons;
		const MetaButtonCodes *myCurrentMetaButton;
		AxisRotaryButton myAxisButton;
		FeedRotaryButton myFeedButton;
		const ButtonsState *myPreviousState;
		KeyEventListener *myEventListener;
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const ButtonsState &data);

	// ----------------------------------------------------------------------

	class Display : public KeyEventListener
	{
	public:
		enum class AxisPositionMethod
		{
			RELATIVE,
			ABSOLUTE
		};
		enum class AxisGroup
		{
			XYZ,
			ABC
		};

		Display(const ButtonsState &currentButtonsState, UsbOutPackageData &displayData);

		Display();

		~Display();

		virtual bool OnButtonPressedEvent(const MetaButtonCodes &metaButton) override;

		virtual bool
		OnButtonReleasedEvent(const MetaButtonCodes &metaButton) override;

		virtual void OnAxisActiveEvent(const KeyCode &axis) override;

		virtual void OnAxisInactiveEvent(const KeyCode &axis) override;

		virtual void OnFeedActiveEvent(const KeyCode &feed) override;

		virtual void OnFeedInactiveEvent(const KeyCode &feed) override;

		virtual bool OnJogDialEvent(const HandWheelCounters &counters,
									int8_t delta) override;

		void UpdateData();

		void ClearData();

	private:
		const ButtonsState &myCurrentButtonsState;
		UsbOutPackageData &myDisplayData;
		AxisPositionMethod myAxisPositionMethod;
		AxisGroup myActiveAxisGroup;
	};

	// ----------------------------------------------------------------------

	class Pendant : public KeyEventListener
	{
	public:
		Pendant(UsbOutPackageData &displayOutData);
		~Pendant();

		void ProcessEvent(uint8_t keyCode, uint8_t modifierCode,
						  uint8_t rotaryButtonAxisKeyCode,
						  uint8_t rotaryButtonFeedKeyCode, int8_t handWheelStepCount);

		void UpdateDisplayData();
		void ClearDisplayData();

		const ButtonsState &CurrentButtonsState() const;
		const ButtonsState &PreviousButtonsState() const;
		const Handwheel &HandWheel() const;
		Handwheel &HandWheel();

		virtual bool OnButtonPressedEvent(const MetaButtonCodes &metaButton) override;
		virtual bool
		OnButtonReleasedEvent(const MetaButtonCodes &metaButton) override;
		virtual void OnAxisActiveEvent(const KeyCode &axis) override;
		virtual void OnAxisInactiveEvent(const KeyCode &axis) override;
		virtual void OnFeedActiveEvent(const KeyCode &axis) override;
		virtual void OnFeedInactiveEvent(const KeyCode &axis) override;
		virtual bool OnJogDialEvent(const HandWheelCounters &counters,
									int8_t delta) override;

	private:
		ButtonsState myPreviousButtonsState;
		ButtonsState myCurrentButtonsState;
		Handwheel myHandWheel;
		Display myDisplay;

		float myScale;
		float myMaxVelocity;

		const char *myPrefix;
		std::ostream *myPendantCout;

		void ShiftButtonState();

		void ProcessEvent(const KeyCode &keyCode, const KeyCode &modifierCode,
						  const KeyCode &rotaryButtonAxisKeyCode,
						  const KeyCode &rotaryButtonFeedKeyCode,
						  int8_t handWheelStepCount);
		void DispatchFeedEventToHandwheel(const KeyCode &feed, bool isActive);
		void DispatchAxisEventToHandwheel(const KeyCode &axis, bool isActive);
		void DispatchAxisEvent(const KeyCode &axis, bool isActive);
		void DispatchActiveFeed(const KeyCode &feed, bool isActive);
		void DispatchFeedValue();
		void DispatchFeedValue(const KeyCode &feed);
	};

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const Pendant &data);
} // namespace XhcWhb04b6
