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

#include "pendant.h"

// system includes
#include <algorithm>
#include <assert.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>

// 3rd party includes

// local library includes

// local includes
#include "./usb.h"
#include "./xhc-whb04b6.h"

using std::endl;

namespace XhcWhb04b6
{

	// ----------------------------------------------------------------------

	const ButtonsCode KeyCodes::Buttons;
	const MetaButtonsCodes KeyCodes::Meta(Buttons);
	const AxisRotaryButtonCodes KeyCodes::Axis;
	const FeedRotaryButtonCodes KeyCodes::Feed;

	// ----------------------------------------------------------------------

	KeyEventListener::~KeyEventListener()
	{
	}

	// ----------------------------------------------------------------------

	HandwheelStepModeStepSize::HandwheelStepModeStepSize() : mySequence{0.001, 0.01, 0.1, 1, 0, 0, 0}
	{
	}

	// ----------------------------------------------------------------------

	HandwheelStepModeStepSize::~HandwheelStepModeStepSize()
	{
	}

	// ----------------------------------------------------------------------

	float HandwheelStepModeStepSize::GetStepSize(PositionNameIndex buttonPosition) const
	{
		return mySequence[static_cast<uint8_t>(buttonPosition)];
	}

	// ----------------------------------------------------------------------

	bool HandwheelStepModeStepSize::IsPermitted(PositionNameIndex buttonPosition) const
	{
		return (GetStepSize(buttonPosition) > 0);
	}

	// ----------------------------------------------------------------------

	HandwheelContinuousModeStepSize::HandwheelContinuousModeStepSize() : mySequence{2, 5, 10, 30, 60, 100, 0}
	{
	}

	// ----------------------------------------------------------------------

	HandwheelContinuousModeStepSize::~HandwheelContinuousModeStepSize()
	{
	}

	// ----------------------------------------------------------------------

	float HandwheelContinuousModeStepSize::GetStepSize(PositionNameIndex buttonPosition) const
	{
		return mySequence[static_cast<uint8_t>(buttonPosition)];
	}

	// ----------------------------------------------------------------------

	bool HandwheelContinuousModeStepSize::IsPermitted(PositionNameIndex buttonPosition) const
	{
		return (GetStepSize(buttonPosition) > 0);
	}

	// ----------------------------------------------------------------------

	HandwheelLeadModeStepSize::HandwheelLeadModeStepSize() : mySequence{0, 0, 0, 0, 0, 0, 0.01}
	{
	}

	// ----------------------------------------------------------------------

	HandwheelLeadModeStepSize::~HandwheelLeadModeStepSize()
	{
	}

	// ----------------------------------------------------------------------

	float HandwheelLeadModeStepSize::GetStepSize(PositionNameIndex buttonPosition) const
	{
		return mySequence[static_cast<uint8_t>(buttonPosition)];
	}

	// ----------------------------------------------------------------------

	bool HandwheelLeadModeStepSize::IsPermitted(PositionNameIndex buttonPosition) const
	{
		return (buttonPosition == PositionNameIndex::LEAD);
	}

	// ----------------------------------------------------------------------

	KeyCode::KeyCode(uint8_t code, const char *text, const char *altText) : code(code),
																			text(text),
																			altText(altText)
	{
	}

	// ----------------------------------------------------------------------

	KeyCode::KeyCode(const KeyCode &other) : code(other.code),
											 text(other.text),
											 altText(other.altText)
	{
	}

	// ----------------------------------------------------------------------

	bool KeyCode::operator==(const KeyCode &other) const
	{
		return ((code == other.code) && (text == other.text) && (altText == other.altText));
	}

	// ----------------------------------------------------------------------

	bool KeyCode::operator!=(const KeyCode &other) const
	{
		return !(*this == other);
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const KeyCode &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);

		os << std::hex << std::setfill('0') << "{code=0x" << std::setw(2) << static_cast<uint16_t>(data.code)
		   << " text='" << data.text << "'"
		   << " altText='" << data.altText << "'}";

		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	MetaButtonCodes::MetaButtonCodes(const KeyCode &key, const KeyCode &modifier) : key(key),
																					modifier(modifier)
	{
	}

	// ----------------------------------------------------------------------

	MetaButtonCodes::~MetaButtonCodes()
	{
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const MetaButtonCodes &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << "{key=" << data.key << " modifier=" << data.modifier << "}";
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	bool MetaButtonCodes::ContainsKeys(const KeyCode &key, const KeyCode &modifier) const
	{
		return (this->key.code == key.code) && (this->modifier.code == modifier.code);
	}

	// ----------------------------------------------------------------------

	bool MetaButtonCodes::operator==(const MetaButtonCodes &other) const
	{
		return (key == other.key) && (modifier == other.modifier);
	}

	// ----------------------------------------------------------------------

	bool MetaButtonCodes::operator!=(const MetaButtonCodes &other) const
	{
		return !(*this == other);
	}

	// ----------------------------------------------------------------------

	MetaButtonsCodes::MetaButtonsCodes(const ButtonsCode &buttons) : reset(buttons.reset, buttons.undefined),
																	 macro11(buttons.reset, buttons.function),
																	 stop(buttons.stop, buttons.undefined),
																	 macro12(buttons.stop, buttons.function),
																	 start(buttons.start, buttons.undefined),
																	 macro13(buttons.start, buttons.function),
																	 feed_plus(buttons.feed_plus, buttons.undefined),
																	 macro1(buttons.feed_plus, buttons.function),
																	 feed_minus(buttons.feed_minus, buttons.undefined),
																	 macro2(buttons.feed_minus, buttons.function),
																	 spindle_plus(buttons.spindle_plus, buttons.undefined),
																	 macro3(buttons.spindle_plus, buttons.function),
																	 spindle_minus(buttons.spindle_minus, buttons.undefined),
																	 macro4(buttons.spindle_minus, buttons.function),
																	 machine_home(buttons.machine_home, buttons.undefined),
																	 macro5(buttons.machine_home, buttons.function),
																	 safe_z(buttons.safe_z, buttons.undefined),
																	 macro6(buttons.safe_z, buttons.function),
																	 workpiece_home(buttons.workpiece_home, buttons.undefined),
																	 macro7(buttons.workpiece_home, buttons.function),
																	 spindle_on_off(buttons.spindle_on_off, buttons.undefined),
																	 macro8(buttons.spindle_on_off, buttons.function),
																	 function(buttons.function, buttons.undefined),
																	 probe_z(buttons.probe_z, buttons.undefined),
																	 macro9(buttons.probe_z, buttons.function),
																	 macro10(buttons.macro10, buttons.undefined),
																	 macro14(buttons.macro10, buttons.function),
																	 manual_pulse_generator(buttons.manual_pulse_generator, buttons.undefined),
																	 macro15(buttons.manual_pulse_generator, buttons.function),
																	 step_continuous(buttons.step_continuous, buttons.undefined),
																	 macro16(buttons.step_continuous, buttons.function),
																	 undefined(buttons.undefined, buttons.undefined),
																	 buttons{
																		 {&reset},
																		 {&macro11},
																		 {&stop},
																		 {&macro12},
																		 {&start},
																		 {&macro13},
																		 {&feed_plus},
																		 {&macro1},
																		 {&feed_minus},
																		 {&macro2},
																		 {&spindle_plus},
																		 {&macro3},
																		 {&spindle_minus},
																		 {&macro4},
																		 {&machine_home},
																		 {&macro5},
																		 {&safe_z},
																		 {&macro6},
																		 {&workpiece_home},
																		 {&macro7},
																		 {&spindle_on_off},
																		 {&macro8},
																		 {&function},
																		 {&probe_z},
																		 {&macro9},
																		 {&macro10},
																		 {&macro14},
																		 {&manual_pulse_generator},
																		 {&macro15},
																		 {&step_continuous},
																		 {&macro16},
																		 {&undefined}}
	{
	}

	// ----------------------------------------------------------------------

	const MetaButtonCodes &MetaButtonsCodes::Find(const KeyCode &keyCode, const KeyCode &modifierCode) const
	{

		std::function<bool(const MetaButtonCodes *)> comparator = [&keyCode, &modifierCode](
																	  const MetaButtonCodes *metaButton)
		{
			return metaButton->ContainsKeys(keyCode, modifierCode);
		};

		std::list<const MetaButtonCodes *>::const_iterator button = std::find_if(buttons.begin(), buttons.end(), comparator);

		if (button == buttons.end())
		{
			std::cerr << "failed to find metaButton={ keyCode={" << keyCode << "} modifierCode={" << modifierCode << "}}"
					  << endl;
		}
		assert(button != buttons.end());

		return **button;
	}

	// ----------------------------------------------------------------------

	MetaButtonsCodes::~MetaButtonsCodes()
	{
	}

	// ----------------------------------------------------------------------

	Button::Button(const KeyCode &key) : myKey(&key)
	{
	}

	// ----------------------------------------------------------------------

	Button::~Button()
	{
	}

	// ----------------------------------------------------------------------

	Button &Button::operator=(const Button &other)
	{
		myKey = other.myKey;
		return *this;
	}

	// ----------------------------------------------------------------------

	AxisRotaryButtonCodes::AxisRotaryButtonCodes() : off(0x06, "OFF", ""),
													 x(0x11, "X", ""),
													 y(0x12, "Y", ""),
													 z(0x13, "Z", ""),
													 a(0x14, "A", ""),
													 b(0x15, "B", ""),
													 c(0x16, "C", ""),
													 undefined(0x00, "", ""),
													 codeMap{
														 {off.code, &off},
														 {x.code, &x},
														 {y.code, &y},
														 {z.code, &z},
														 {a.code, &a},
														 {b.code, &b},
														 {c.code, &c},
														 {undefined.code, &undefined}}
	{
	}

	// ----------------------------------------------------------------------

	FeedRotaryButtonCodes::FeedRotaryButtonCodes() : speed_0_001(0x0d, "0.001", "2%"),
													 speed_0_01(0x0e, "0.01", "5%"),
													 speed_0_1(0x0f, "0.1", "10%"),
													 speed_1(0x10, "1", "30%"),
													 percent_60(0x1a, "", "60%"),
													 percent_100(0x1b, "", "100%"),
													 lead(0x1c, "Lead", ""),
													 undefined(0x00, "", ""),
													 codeMap{
														 {speed_0_001.code, &speed_0_001},
														 {speed_0_01.code, &speed_0_01},
														 {speed_0_1.code, &speed_0_1},
														 {speed_1.code, &speed_1},
														 {percent_60.code, &percent_60},
														 {percent_100.code, &percent_100},
														 {lead.code, &lead},
														 {undefined.code, &undefined}}
	{
	}

	// ----------------------------------------------------------------------

	ButtonsCode::ButtonsCode() : reset(0x01, "reset", "macro-11"),
								 stop(0x02, "stop", "macro-12"),
								 start(0x03, "start-pause", "macro-13"),
								 feed_plus(0x04, "feed-plus", "macro-1"),
								 feed_minus(0x05, "feed-minus", "macro-2"),
								 spindle_plus(0x06, "spindle-plus", "macro-3"),
								 spindle_minus(0x07, "spindle-minus", "macro-4"),
								 machine_home(0x08, "m-home", "macro-5"),
								 safe_z(0x09, "safe-z", "macro-6"),
								 workpiece_home(0x0a, "w-home", "macro-7"),
								 spindle_on_off(0x0b, "s-on-off", "macro-8"),
								 function(0x0c, "fn", "<unused>"),
								 probe_z(0x0d, "probe-z", "macro-9"),
								 macro10(0x10, "macro-10", "macro-14"),
								 manual_pulse_generator(0x0e, "mode-continuous", "macro-15"),
								 step_continuous(0x0f, "mode-step", "macro-16"),
								 undefined(0x00, "", ""),
								 codeMap{
									 {reset.code, &reset},
									 {stop.code, &stop},
									 {start.code, &start},
									 {feed_plus.code, &feed_plus},
									 {feed_minus.code, &feed_minus},
									 {spindle_plus.code, &spindle_plus},
									 {spindle_minus.code, &spindle_minus},
									 {machine_home.code, &machine_home},
									 {safe_z.code, &safe_z},
									 {workpiece_home.code, &workpiece_home},
									 {spindle_on_off.code, &spindle_on_off},
									 {function.code, &function},
									 {probe_z.code, &probe_z},
									 {macro10.code, &macro10},
									 {manual_pulse_generator.code, &manual_pulse_generator},
									 {step_continuous.code, &step_continuous},
									 {undefined.code, &undefined}}
	{
	}

	// ----------------------------------------------------------------------

	const KeyCode &ButtonsCode::GetKeyCode(uint8_t keyCode) const
	{
		const KeyCode *buttonKeyCode = reinterpret_cast<const KeyCode *>(this);

		while (buttonKeyCode->code != 0)
		{
			if (buttonKeyCode->code == keyCode)
			{
				break;
			}
			buttonKeyCode++;
		}

		assert(nullptr != buttonKeyCode);

		return *buttonKeyCode;
	}

	// ----------------------------------------------------------------------

	const KeyCode &Button::KeyCode() const
	{
		return *myKey;
	}

	// ----------------------------------------------------------------------

	bool Button::SetKeyCode(const KeyCode &keyCode)
	{
		bool isNewButton = *myKey != keyCode;
		myKey = &keyCode;
		return isNewButton;
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const Button &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << "{key=" << data.KeyCode() << "}";
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	ToggleButton::ToggleButton(const KeyCode &key, const KeyCode &modifier) : Button(key),
																			  myModifier(&modifier)
	{
	}

	// ----------------------------------------------------------------------

	ToggleButton::~ToggleButton()
	{
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const ToggleButton &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << "{" << *static_cast<const Button *>(&data) << " modifier=" << data.ModifierCode() << "}";
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	ToggleButton &ToggleButton::operator=(const ToggleButton &other)
	{
		Button::operator=(other);
		myModifier = other.myModifier;
		return *this;
	}

	// ----------------------------------------------------------------------

	const KeyCode &ToggleButton::ModifierCode() const
	{
		return *myModifier;
	}

	// ----------------------------------------------------------------------

	void ToggleButton::SetModifierCode(KeyCode &modifierCode)
	{
		myModifier = &modifierCode;
	}

	// ----------------------------------------------------------------------

	bool ToggleButton::ContainsKeys(const KeyCode &key, const KeyCode &modifier) const
	{
		return ((key.code == myKey->code) && (modifier.code == myModifier->code));
	}

	// ----------------------------------------------------------------------

	RotaryButton::RotaryButton(const KeyCode &keyCode) : Button(keyCode)
	{
	}

	// ----------------------------------------------------------------------

	RotaryButton::~RotaryButton()
	{
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const RotaryButton &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << "{" << *static_cast<const Button *>(&data) << "}";
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	RotaryButton &RotaryButton::operator=(const RotaryButton &other)
	{
		Button::operator=(other);
		return *this;
	}

	// ----------------------------------------------------------------------
	const HandwheelStepModeStepSize FeedRotaryButton::myStepStepSizeMapper;
	const HandwheelContinuousModeStepSize FeedRotaryButton::myContinuousSizeMapper;
	const HandwheelLeadModeStepSize FeedRotaryButton::myLeadStepSizeMapper;

	const std::map<const KeyCode *, HandwheelStepModeStepSize::PositionNameIndex> FeedRotaryButton::myStepKeycodeLut{
		{&KeyCodes::Feed.speed_0_001, HandwheelStepModeStepSize::PositionNameIndex::RotaryButton0001},
		{&KeyCodes::Feed.speed_0_01, HandwheelStepModeStepSize::PositionNameIndex::RotaryButton0010},
		{&KeyCodes::Feed.speed_0_1, HandwheelStepModeStepSize::PositionNameIndex::RotaryButton0100},
		{&KeyCodes::Feed.speed_1, HandwheelStepModeStepSize::PositionNameIndex::RotaryButton100},
		{&KeyCodes::Feed.percent_60, HandwheelStepModeStepSize::PositionNameIndex::RotaryButtonUndefined},
		{&KeyCodes::Feed.percent_100, HandwheelStepModeStepSize::PositionNameIndex::RotaryButtonUndefined},
		{&KeyCodes::Feed.lead, HandwheelStepModeStepSize::PositionNameIndex::RotaryButtonUndefined}};
	const std::map<const KeyCode *, HandwheelContinuousModeStepSize::PositionNameIndex> FeedRotaryButton::myContinuousKeycodeLut{
		{&KeyCodes::Feed.speed_0_001, HandwheelContinuousModeStepSize::PositionNameIndex::RotaryButton2percent},
		{&KeyCodes::Feed.speed_0_01, HandwheelContinuousModeStepSize::PositionNameIndex::RotaryButton5percent},
		{&KeyCodes::Feed.speed_0_1, HandwheelContinuousModeStepSize::PositionNameIndex::RotaryButton10percent},
		{&KeyCodes::Feed.speed_1, HandwheelContinuousModeStepSize::PositionNameIndex::RotaryButton30percent},
		{&KeyCodes::Feed.percent_60, HandwheelContinuousModeStepSize::PositionNameIndex::RotaryButton60percent},
		{&KeyCodes::Feed.percent_100, HandwheelContinuousModeStepSize::PositionNameIndex::RotaryButton100percent},
		{&KeyCodes::Feed.lead, HandwheelContinuousModeStepSize::PositionNameIndex::RotaryButtonUndefined}};
	const std::map<const KeyCode *, HandwheelLeadModeStepSize::PositionNameIndex> FeedRotaryButton::myLeadKeycodeLut{
		{&KeyCodes::Feed.speed_0_001, HandwheelLeadModeStepSize::PositionNameIndex::UNDEFINED},
		{&KeyCodes::Feed.speed_0_01, HandwheelLeadModeStepSize::PositionNameIndex::UNDEFINED},
		{&KeyCodes::Feed.speed_0_1, HandwheelLeadModeStepSize::PositionNameIndex::UNDEFINED},
		{&KeyCodes::Feed.speed_1, HandwheelLeadModeStepSize::PositionNameIndex::UNDEFINED},
		{&KeyCodes::Feed.percent_60, HandwheelLeadModeStepSize::PositionNameIndex::UNDEFINED},
		{&KeyCodes::Feed.percent_100, HandwheelLeadModeStepSize::PositionNameIndex::UNDEFINED},
		{&KeyCodes::Feed.lead, HandwheelLeadModeStepSize::PositionNameIndex::LEAD}};

	// ----------------------------------------------------------------------

	FeedRotaryButton::FeedRotaryButton(const KeyCode &keyCode,
									   HandwheelStepmodes::Mode stepMode,
									   KeyEventListener *listener) : RotaryButton(keyCode),
																	 myStepMode(stepMode),
																	 myIsPermitted(false),
																	 myStepSize(0),
																	 myEventListener(listener)
	{
	}

	// ----------------------------------------------------------------------

	FeedRotaryButton::~FeedRotaryButton()
	{
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const FeedRotaryButton &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << "{" << *static_cast<const RotaryButton *>(&data) << " "
		   << "IsPermitted=" << ((data.IsPermitted()) ? "TRUE" : "FALSE") << " "
		   << "StepSize=" << data.StepSize() << " "
		   << "StepMode=0x" << std::setfill('0') << std::hex << std::setw(2)
		   << static_cast<int16_t>(data.StepMode()) << "}";
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	FeedRotaryButton &FeedRotaryButton::operator=(const FeedRotaryButton &other)
	{
		RotaryButton::operator=(other);
		myStepMode = other.myStepMode;
		return *this;
	}

	// ----------------------------------------------------------------------

	bool FeedRotaryButton::SetKeyCode(const KeyCode &keyCode)
	{
		bool hasChanged = Button::SetKeyCode(keyCode);
		Update();
		return hasChanged;
	}

	// ----------------------------------------------------------------------

	void FeedRotaryButton::SetStepMode(HandwheelStepmodes::Mode stepMode)
	{
		myStepMode = stepMode;
		Update();
	}

	// ----------------------------------------------------------------------

	HandwheelStepmodes::Mode FeedRotaryButton::StepMode() const
	{
		return myStepMode;
	}

	// ----------------------------------------------------------------------

	float FeedRotaryButton::StepSize() const
	{
		return myStepSize;
	}

	// ----------------------------------------------------------------------

	void FeedRotaryButton::Update()
	{
		if (*myKey == KeyCodes::Feed.undefined)
		{
			myIsPermitted = false;
			return;
		}

		if (*myKey == KeyCodes::Feed.lead)
		{
			myStepSize = myLeadStepSizeMapper.GetStepSize(HandwheelLeadModeStepSize::PositionNameIndex::LEAD);
			myIsPermitted = myLeadStepSizeMapper.IsPermitted(HandwheelLeadModeStepSize::PositionNameIndex::LEAD);
		}
		else if (myStepMode == HandwheelStepmodes::Mode::CONTINUOUS)
		{
			auto enumValue = myContinuousKeycodeLut.find(myKey);
			assert(enumValue != myContinuousKeycodeLut.end());
			auto second = enumValue->second;
			myStepSize = myContinuousSizeMapper.GetStepSize(second);
			myIsPermitted = myContinuousSizeMapper.IsPermitted(second);
		}
		else if (myStepMode == HandwheelStepmodes::Mode::STEP)
		{
			auto enumValue = myStepKeycodeLut.find(myKey);
			assert(enumValue != myStepKeycodeLut.end());
			auto second = enumValue->second;
			myStepSize = myStepStepSizeMapper.GetStepSize(second);
			myIsPermitted = myStepStepSizeMapper.IsPermitted(second);
		}
		else
		{
			assert(false);
		}
	}

	// ----------------------------------------------------------------------

	bool FeedRotaryButton::IsPermitted() const
	{
		return myIsPermitted;
	}

	// ----------------------------------------------------------------------

	AxisRotaryButton::AxisRotaryButton(const KeyCode &keyCode, KeyEventListener *listener) : RotaryButton(keyCode),
																							 myEventListener(listener)
	{
	}

	// ----------------------------------------------------------------------

	AxisRotaryButton::~AxisRotaryButton()
	{
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const AxisRotaryButton &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << "{" << *static_cast<const RotaryButton *>(&data)
		   << " IsPermitted=" << ((data.IsPermitted()) ? "TRUE" : "FALSE") << "}";
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	AxisRotaryButton &AxisRotaryButton::operator=(const AxisRotaryButton &other)
	{
		RotaryButton::operator=(other);
		return *this;
	}

	// ----------------------------------------------------------------------

	bool AxisRotaryButton::IsPermitted() const
	{
		return (*myKey != KeyCodes::Axis.undefined) && (*myKey != KeyCodes::Axis.off);
	}

	// ----------------------------------------------------------------------

	Handwheel::Handwheel(const FeedRotaryButton &feedButton, KeyEventListener *listener) : myCounters(),
																						   myFeedButton(feedButton),
																						   myEventListener(listener),
																						   myWheelCout(&std::cout),
																						   myPrefix("pndnt ")
	{
	}

	// ----------------------------------------------------------------------

	Handwheel::~Handwheel()
	{
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const Handwheel &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);
		os << "{Counters=" << data.Counters() << "}";
		return os;
	}

	// ----------------------------------------------------------------------

	const HandWheelCounters &Handwheel::Counters() const
	{
		return static_cast<const HandWheelCounters &>(
			static_cast<Handwheel>(*this).Counters());
	}

	// ----------------------------------------------------------------------

	void Handwheel::SetEnabled(bool enabled)
	{
		myIsEnabled = enabled;
	}
	// ----------------------------------------------------------------------

	HandWheelCounters &Handwheel::Counters()
	{
		return myCounters;
	}

	// ----------------------------------------------------------------------

	void Handwheel::SetMode(HandWheelCounters::CounterNameToIndex activeCounterMode)
	{
		myCounters.SetActiveCounter(activeCounterMode);
	}

	// ----------------------------------------------------------------------

	void Handwheel::Count(int8_t delta)
	{
		assert(myEventListener != nullptr);

		if (myIsEnabled)
		{
			myCounters.Count(delta);
			myEventListener->OnJogDialEvent(myCounters, delta);
		}

		std::ios init(NULL);
		init.copyfmt(*myWheelCout);
		*myWheelCout << myPrefix << "handwheel total counts " << std::setfill(' ') << std::setw(5) << myCounters
					 << endl;
		myWheelCout->copyfmt(init);
	}

	// ----------------------------------------------------------------------

	ButtonsState::ButtonsState(KeyEventListener *listener, const ButtonsState *previousState) : myPressedButtons(),
																								myCurrentMetaButton(&KeyCodes::Meta.undefined),
																								myAxisButton(KeyCodes::Axis.undefined, listener),
																								myFeedButton(KeyCodes::Feed.undefined, HandwheelStepmodes::Mode::CONTINUOUS, listener),
																								myPreviousState(previousState),
																								myEventListener(listener)
	{
	}

	// ----------------------------------------------------------------------

	ButtonsState::~ButtonsState()
	{
	}

	// ----------------------------------------------------------------------

	void ButtonsState::Update(const KeyCode &keyCode,
							  const KeyCode &modifierCode,
							  const KeyCode &axisButton,
							  const KeyCode &feedButton)
	{
		//! propagate push button events
		const MetaButtonCodes &newButton = KeyCodes::Meta.Find(keyCode, modifierCode);
		if (*myCurrentMetaButton != newButton)
		{
			if (*myCurrentMetaButton != KeyCodes::Meta.undefined)
			{
				if (myEventListener != nullptr)
				{
					myEventListener->OnButtonReleasedEvent(*myCurrentMetaButton);
				}
			}

			myCurrentMetaButton = &newButton;
			if (*myCurrentMetaButton != KeyCodes::Meta.undefined)
			{
				if (myEventListener != nullptr)
				{
					myEventListener->OnButtonPressedEvent(*myCurrentMetaButton);
				}
			}
		}

		//! propagate axis rotary button events
		const KeyCode &oldAxisKeyCode = myAxisButton.KeyCode();
		if (myAxisButton.SetKeyCode(axisButton))
		{
			myEventListener->OnAxisInactiveEvent(oldAxisKeyCode);
			myEventListener->OnAxisActiveEvent(myAxisButton.KeyCode());
		}

		//! propagate feed rotary button events
		const KeyCode &oldFeedKeyCode = myFeedButton.KeyCode();
		if (myFeedButton.SetKeyCode(feedButton))
		{
			myEventListener->OnFeedInactiveEvent(oldFeedKeyCode);
			myEventListener->OnFeedActiveEvent(myFeedButton.KeyCode());
		}
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const ButtonsState &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);

		os << "{pressed buttons=";
		for (const KeyCode *pb : data.PressedButtons())
		{
			assert(pb != nullptr);
			os << *pb << " ";
		}
		os << " metaButton=" << *data.CurrentMetaButton()
		   << " axisButton=" << data.AxisButton()
		   << " feedButton=" << data.FeedButton() << "}";
		os.copyfmt(init);
		return os;
	}

	// ----------------------------------------------------------------------

	ButtonsState &ButtonsState::operator=(const ButtonsState &other)
	{
		myPressedButtons = other.myPressedButtons;
		myCurrentMetaButton = other.myCurrentMetaButton;
		myAxisButton = other.myAxisButton;
		myFeedButton = other.myFeedButton;
		return *this;
	}

	// ----------------------------------------------------------------------

	void ButtonsState::ClearPressedButtons()
	{
		myPressedButtons.clear();
	}

	// ----------------------------------------------------------------------

	const std::list<const KeyCode *> &ButtonsState::PressedButtons() const
	{
		return myPressedButtons;
	}

	// ----------------------------------------------------------------------

	const MetaButtonCodes *ButtonsState::CurrentMetaButton() const
	{
		return myCurrentMetaButton;
	}

	// ----------------------------------------------------------------------

	const AxisRotaryButton &ButtonsState::AxisButton() const
	{
		return myAxisButton;
	}

	// ----------------------------------------------------------------------

	const FeedRotaryButton &ButtonsState::FeedButton() const
	{
		return myFeedButton;
	}

	// ----------------------------------------------------------------------

	FeedRotaryButton &ButtonsState::FeedButton()
	{
		return myFeedButton;
	}

	// ----------------------------------------------------------------------

	Pendant::Pendant(UsbOutPackageData &displayOutData) : myPreviousButtonsState(this),
														  myCurrentButtonsState(this, &myPreviousButtonsState),
														  myHandWheel(myCurrentButtonsState.FeedButton(), this),
														  myDisplay(myCurrentButtonsState, displayOutData),
														  myPrefix("pndnt "),
														  myPendantCout(&std::cout)
	{
	}

	// ----------------------------------------------------------------------

	Pendant::~Pendant()
	{
	}

	// ----------------------------------------------------------------------

	std::ostream &operator<<(std::ostream &os, const Pendant &data)
	{
		std::ios init(NULL);
		init.copyfmt(os);

		os << "{CurrentButtonState=" << data.CurrentButtonsState() << " "
		   << "PreviousButtonState=" << data.PreviousButtonsState() << " "
		   << "handwheel= " << data.HandWheel() << "}";
		return os;
	}

	// ----------------------------------------------------------------------

	void Pendant::ProcessEvent(uint8_t keyCode,
							   uint8_t modifierCode,
							   uint8_t rotaryButtonAxisKeyCode,
							   uint8_t rotaryButtonFeedKeyCode,
							   int8_t handWheelStepCount)
	{
		ShiftButtonState();

		auto key = KeyCodes::Buttons.codeMap.find(keyCode);
		auto modifier = KeyCodes::Buttons.codeMap.find(modifierCode);
		auto axis = KeyCodes::Axis.codeMap.find(rotaryButtonAxisKeyCode);
		auto feed = KeyCodes::Feed.codeMap.find(rotaryButtonFeedKeyCode);

		if (key == KeyCodes::Buttons.codeMap.end())
		{
			*myPendantCout << myPrefix << "failed to interpret key code keyCode={" << keyCode << "}" << endl;
		}
		if (modifier == KeyCodes::Buttons.codeMap.end())
		{
			*myPendantCout << myPrefix << "failed to interpret modifier code keyCode={" << modifierCode << "}" << endl;
		}
		if (axis == KeyCodes::Axis.codeMap.end())
		{
			*myPendantCout << myPrefix << "failed to interpret axis code axisCode={" << modifierCode << "}" << endl;
		}
		if (feed == KeyCodes::Feed.codeMap.end())
		{
			*myPendantCout << myPrefix << "failed to interpret axis code axisCode={" << modifierCode << "}" << endl;
		}

		ProcessEvent(*key->second, *modifier->second, *axis->second, *feed->second, handWheelStepCount);
	}

	// ----------------------------------------------------------------------

	void Pendant::ProcessEvent(const KeyCode &keyCode,
							   const KeyCode &modifierCode,
							   const KeyCode &rotaryButtonAxisKeyCode,
							   const KeyCode &rotaryButtonFeedKeyCode,
							   int8_t handWheelStepCount)
	{
		myHandWheel.SetEnabled(true);
		myCurrentButtonsState.Update(keyCode, modifierCode, rotaryButtonAxisKeyCode, rotaryButtonFeedKeyCode);
		myHandWheel.Count(handWheelStepCount);
		myDisplay.UpdateData();
	}

	// ----------------------------------------------------------------------

	void Pendant::UpdateDisplayData()
	{
		myDisplay.UpdateData();
	}

	// ----------------------------------------------------------------------

	void Pendant::ClearDisplayData()
	{
		myDisplay.ClearData();
	}

	// ----------------------------------------------------------------------

	void Pendant::ShiftButtonState()
	{
		myPreviousButtonsState = myCurrentButtonsState;
		myCurrentButtonsState.ClearPressedButtons();
	}

	// ----------------------------------------------------------------------

	const ButtonsState &Pendant::CurrentButtonsState() const
	{
		return myCurrentButtonsState;
	}

	// ----------------------------------------------------------------------

	const ButtonsState &Pendant::PreviousButtonsState() const
	{
		return myPreviousButtonsState;
	}

	// ----------------------------------------------------------------------

	const Handwheel &Pendant::HandWheel() const
	{
		return myHandWheel;
	}

	// ----------------------------------------------------------------------

	Handwheel &Pendant::HandWheel()
	{
		return const_cast<Handwheel &>(const_cast<const Pendant *>(this)->HandWheel());
	}

	// ----------------------------------------------------------------------

	bool Pendant::OnButtonPressedEvent(const MetaButtonCodes &metaButton)
	{
		*myPendantCout << myPrefix << "button pressed  event metaButton=" << metaButton << endl;
		bool isHandled = false;
		if (metaButton == KeyCodes::Meta.reset)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.stop)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.start)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.feed_plus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.feed_minus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.spindle_plus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.spindle_minus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.machine_home)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.safe_z)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.workpiece_home)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.spindle_on_off)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.probe_z)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro10)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.manual_pulse_generator)
		{
			myCurrentButtonsState.FeedButton().SetStepMode(HandwheelStepmodes::Mode::CONTINUOUS);
			DispatchFeedValue();
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.step_continuous)
		{
			myCurrentButtonsState.FeedButton().SetStepMode(HandwheelStepmodes::Mode::STEP);
			DispatchFeedValue();
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro11)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro12)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro13)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro1)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro2)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro3)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro4)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro5)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro6)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro7)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro8)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro9)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro14)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro15)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro16)
		{
			isHandled = true;
		}

		myDisplay.OnButtonPressedEvent(metaButton);
		return isHandled;
	}

	// ----------------------------------------------------------------------

	bool Pendant::OnButtonReleasedEvent(const MetaButtonCodes &metaButton)
	{
		*myPendantCout << myPrefix << "button released event metaButton=" << metaButton << endl;
		bool isHandled = false;
		if (metaButton == KeyCodes::Meta.reset)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.stop)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.start)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.feed_plus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.feed_minus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.spindle_plus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.spindle_minus)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.machine_home)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.safe_z)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.workpiece_home)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.spindle_on_off)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.probe_z)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro10)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.manual_pulse_generator)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.step_continuous)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro11)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro12)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro13)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro1)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro2)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro3)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro4)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro5)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro6)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro7)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro8)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro9)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro14)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro15)
		{
			isHandled = true;
		}
		else if (metaButton == KeyCodes::Meta.macro16)
		{
			isHandled = true;
		}

		myDisplay.OnButtonReleasedEvent(metaButton);
		return isHandled;
	}

	// ----------------------------------------------------------------------

	void Pendant::OnAxisActiveEvent(const KeyCode &axis)
	{
		*myPendantCout << myPrefix << "axis   active   event axis=" << axis
					   << " axisButton=" << myCurrentButtonsState.AxisButton() << endl;
		DispatchAxisEventToHandwheel(axis, true);
		DispatchAxisEvent(axis, true);
		myDisplay.OnAxisActiveEvent(axis);
	}

	// ----------------------------------------------------------------------

	void Pendant::OnAxisInactiveEvent(const KeyCode &axis)
	{
		*myPendantCout << myPrefix << "axis   inactive event axis=" << axis
					   << " axisButton=" << myCurrentButtonsState.AxisButton() << endl;
		DispatchAxisEventToHandwheel(axis, false);
		DispatchAxisEvent(axis, false);
		myDisplay.OnAxisInactiveEvent(axis);
	}

	// ----------------------------------------------------------------------

	void Pendant::OnFeedActiveEvent(const KeyCode &feed)
	{
		(*myPendantCout) << myPrefix << "feed   active   event feed=" << feed
						 << " feedButton=" << myCurrentButtonsState.FeedButton() << endl;

		DispatchFeedEventToHandwheel(feed, true);
		DispatchFeedValue(feed);
		DispatchActiveFeed(feed, true);
		myDisplay.OnFeedActiveEvent(feed);
	}

	// ----------------------------------------------------------------------

	void Pendant::DispatchFeedEventToHandwheel(const KeyCode &feed, bool isActive)
	{
		if (feed.code == KeyCodes::Feed.lead.code)
		{
			myHandWheel.Counters().SetLeadValueLimit(100, 1000);
			myHandWheel.Counters().EnableLeadCounter(isActive);
		}
	}

	// ----------------------------------------------------------------------

	void Pendant::DispatchActiveFeed(const KeyCode &feed, bool isActive)
	{
		if (feed.code == KeyCodes::Feed.speed_0_001.code)
		{
		}
		else if (feed.code == KeyCodes::Feed.speed_0_01.code)
		{
		}
		else if (feed.code == KeyCodes::Feed.speed_0_1.code)
		{
		}
		else if (feed.code == KeyCodes::Feed.speed_1.code)
		{
		}
	}

	// ----------------------------------------------------------------------

	void Pendant::DispatchFeedValue(const KeyCode &keyCode)
	{
		// on feed rotary button change and lead is active
		if (keyCode.code == KeyCodes::Feed.lead.code)
		{
			return;
		}
		// on feed rotary button change and lead is inactive
		else
		{
		}
		DispatchFeedValue();
	}

	// ----------------------------------------------------------------------

	void Pendant::DispatchFeedValue()
	{
		// on feed rotary button change
		FeedRotaryButton &feedButton = myCurrentButtonsState.FeedButton();
		if (feedButton.IsPermitted())
		{
			float axisJogStepSize = 0;
			if (feedButton.StepMode() == HandwheelStepmodes::Mode::STEP)
			{
				axisJogStepSize = feedButton.StepSize();
			}
			else if (feedButton.StepMode() == HandwheelStepmodes::Mode::CONTINUOUS)
			{
				axisJogStepSize = 2;
			}
			else
			{
			}
		}
		else
		{
		}
	}

	// ----------------------------------------------------------------------

	void Pendant::OnFeedInactiveEvent(const KeyCode &feed)
	{
		*myPendantCout << myPrefix << "feed   inactive event feed=" << feed
					   << " feedButton=" << myCurrentButtonsState.FeedButton() << endl;
		DispatchFeedEventToHandwheel(feed, false);
		DispatchActiveFeed(feed, false);
		myDisplay.OnFeedInactiveEvent(feed);
	}

	// ----------------------------------------------------------------------

	bool Pendant::OnJogDialEvent(const HandWheelCounters &counters, int8_t delta)
	{

		if (HandWheelCounters::CounterNameToIndex::UNDEFINED != counters.ActiveCounter() &&
			counters.Counts() != 0)
		{
			*myPendantCout << myPrefix << "wheel  event " << counters.Counts() << endl;

			if (HandWheelCounters::CounterNameToIndex::LEAD != counters.ActiveCounter())
			{
				myHandWheel.Counters().SetLeadValueLimit(100, 1000);
			}
			myDisplay.OnJogDialEvent(counters, delta);
			return true;
		}
		return false;
	}

	// ----------------------------------------------------------------------

	void Pendant::DispatchAxisEventToHandwheel(const KeyCode &axis, bool isActive)
	{
		if (!isActive)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::UNDEFINED);
		}
		else if (axis.code == KeyCodes::Axis.off.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::UNDEFINED);
		}
		else if (axis.code == KeyCodes::Axis.x.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::AXIS_X);
		}
		else if (axis.code == KeyCodes::Axis.y.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::AXIS_Y);
		}
		else if (axis.code == KeyCodes::Axis.z.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::AXIS_Z);
		}
		else if (axis.code == KeyCodes::Axis.a.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::AXIS_A);
		}
		else if (axis.code == KeyCodes::Axis.b.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::AXIS_B);
		}
		else if (axis.code == KeyCodes::Axis.c.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::AXIS_C);
		}
		else if (axis.code == KeyCodes::Axis.undefined.code)
		{
			myHandWheel.Counters().SetActiveCounter(HandWheelCounters::CounterNameToIndex::UNDEFINED);
		}
	}

	// ----------------------------------------------------------------------

	void Pendant::DispatchAxisEvent(const KeyCode &axis, bool isActive)
	{
		if (axis.code == KeyCodes::Axis.off.code)
		{
		}
		else if (axis.code == KeyCodes::Axis.x.code)
		{
		}
		else if (axis.code == KeyCodes::Axis.y.code)
		{
		}
		else if (axis.code == KeyCodes::Axis.z.code)
		{
		}
		else if (axis.code == KeyCodes::Axis.a.code)
		{
		}
		else if (axis.code == KeyCodes::Axis.b.code)
		{
		}
		else if (axis.code == KeyCodes::Axis.c.code)
		{
		}
		else if (axis.code == KeyCodes::Axis.undefined.code)
		{
		}
	}

	// ----------------------------------------------------------------------

	Display::Display(const ButtonsState &currentButtonsState, UsbOutPackageData &displayData) : myCurrentButtonsState(currentButtonsState),
																								myDisplayData(displayData),
																								myAxisPositionMethod(AxisPositionMethod::ABSOLUTE),
																								myActiveAxisGroup(AxisGroup::XYZ)
	{
	}

	// ----------------------------------------------------------------------

	Display::~Display()
	{
	}

	// ----------------------------------------------------------------------

	bool Display::OnButtonPressedEvent(const MetaButtonCodes &metaButton)
	{
		if (metaButton == KeyCodes::Meta.manual_pulse_generator)
		{
			myDisplayData.displayModeFlags.asBitFields.stepMode =
				static_cast<typename std::underlying_type<DisplayIndicatorStepMode::StepMode>::type>(
					DisplayIndicatorStepMode::StepMode::MANUAL_PULSE_GENERATOR);
			return true;
		}
		else if (metaButton == KeyCodes::Meta.step_continuous)
		{
			myDisplayData.displayModeFlags.asBitFields.stepMode =
				static_cast<typename std::underlying_type<DisplayIndicatorStepMode::StepMode>::type>(
					DisplayIndicatorStepMode::StepMode::STEP);
			return true;
		}
		else if (metaButton == KeyCodes::Meta.macro5)
		{
			myAxisPositionMethod = AxisPositionMethod::ABSOLUTE;
			return true;
		}
		else if (metaButton == KeyCodes::Meta.macro7)
		{
			myAxisPositionMethod = AxisPositionMethod::RELATIVE;
			return true;
		}
		return false;
	}

	// ----------------------------------------------------------------------

	bool Display::OnButtonReleasedEvent(const MetaButtonCodes &metaButton)
	{
		return false;
	}

	// ----------------------------------------------------------------------

	void Display::OnAxisActiveEvent(const KeyCode &axis)
	{
		if ((axis.code == KeyCodes::Axis.x.code) ||
			(axis.code == KeyCodes::Axis.y.code) ||
			(axis.code == KeyCodes::Axis.z.code))
		{
			myActiveAxisGroup = AxisGroup::XYZ;
		}
		else
		{ // a || b || c
			myActiveAxisGroup = AxisGroup::ABC;
		}
	}

	// ----------------------------------------------------------------------

	void Display::OnAxisInactiveEvent(const KeyCode &axis)
	{
	}

	// ----------------------------------------------------------------------

	void Display::OnFeedActiveEvent(const KeyCode &feed)
	{
		if (myCurrentButtonsState.FeedButton().StepMode() == HandwheelStepmodes::Mode::STEP)
		{
			myDisplayData.displayModeFlags.asBitFields.stepMode =
				static_cast<typename std::underlying_type<DisplayIndicatorStepMode::StepMode>::type>(
					DisplayIndicatorStepMode::StepMode::STEP);
		}
		else if (myCurrentButtonsState.FeedButton().StepMode() == HandwheelStepmodes::Mode::CONTINUOUS)
		{
			myDisplayData.displayModeFlags.asBitFields.stepMode =
				static_cast<typename std::underlying_type<DisplayIndicatorStepMode::StepMode>::type>(
					DisplayIndicatorStepMode::StepMode::MANUAL_PULSE_GENERATOR);
		}
	}

	// ----------------------------------------------------------------------

	void Display::OnFeedInactiveEvent(const KeyCode &feed)
	{
	}

	// ----------------------------------------------------------------------

	bool Display::OnJogDialEvent(const HandWheelCounters &counters, int8_t delta)
	{
		return false;
	}

	// ----------------------------------------------------------------------

	void Display::UpdateData()
	{
		myDisplayData.displayModeFlags.asBitFields.isReset = true;

		uint32_t spindleSpeed = 100;
		uint32_t feedRate = 200;

		assert(spindleSpeed <= std::numeric_limits<uint16_t>::max());
		assert(feedRate <= std::numeric_limits<uint16_t>::max());

		myDisplayData.spindleSpeed = spindleSpeed;
		myDisplayData.feedRate = feedRate;

		bool isAbsolutePositionRequest = (myAxisPositionMethod == AxisPositionMethod::ABSOLUTE);
		myDisplayData.displayModeFlags.asBitFields.isRelativeCoordinate = !isAbsolutePositionRequest;
		if (myActiveAxisGroup == AxisGroup::XYZ)
		{
			myDisplayData.row1Coordinate.SetCoordinate(10);
			myDisplayData.row2Coordinate.SetCoordinate(20);
			myDisplayData.row3Coordinate.SetCoordinate(30);
		}
		else
		{
			myDisplayData.row1Coordinate.SetCoordinate(40);
			myDisplayData.row2Coordinate.SetCoordinate(50);
			myDisplayData.row3Coordinate.SetCoordinate(60);
		}
	}

	void Display::ClearData()
	{
		myDisplayData.feedRate = 0;
		myDisplayData.spindleSpeed = 0;
		myDisplayData.displayModeFlags.asBitFields.stepMode =
			static_cast<typename std::underlying_type<DisplayIndicatorStepMode::StepMode>::type>(
				DisplayIndicatorStepMode::StepMode::MANUAL_PULSE_GENERATOR);
		myDisplayData.displayModeFlags.asBitFields.isReset = true;
		myDisplayData.displayModeFlags.asBitFields.isRelativeCoordinate = false;
		myDisplayData.row1Coordinate.Clear();
		myDisplayData.row2Coordinate.Clear();
		myDisplayData.row3Coordinate.Clear();
	}
} // namespace XhcWhb04b6
