#include "./xhc-whb04b6.h"

// system includes
#include <assert.h>
#include <bitset>
#include <iomanip>
#include <iostream>

// 3rd party includes

// local library includes

// local includes

using std::endl;

namespace XhcWhb04b6
{

	// ----------------------------------------------------------------------

	const char *XhcWhb04b6Component::GetName() const
	{
		return myName;
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintCrcDebug(const UsbInPackage &inPackage, const UsbOutPackageData &outPackageBuffer) const
	{
		std::ios init(NULL);
		init.copyfmt(*myRxCout);
		*myRxCout << std::setfill('0') << std::hex;

		if (inPackage.buttonKeyCode1 == myKeyCodes.Buttons.undefined.code)
		{
			bool isValid = (inPackage.crc == (inPackage.randomByte & outPackageBuffer.seed));

			if (isValid)
			{
				if (myIsCrcDebuggingEnabled)
				{
					*myRxCout << "crc   checksum OK";
				}
			}
			else
			{
				*myRxCout << "warn  checksum error" << endl;
			}
			myRxCout->copyfmt(init);
			return;
		}

		std::bitset<8> random(inPackage.randomByte), buttonKeyCode(inPackage.buttonKeyCode1), crc(inPackage.crc);
		std::bitset<8> delta(0);

		if (inPackage.randomByte > inPackage.crc)
		{
			delta = inPackage.randomByte - inPackage.crc;
		}
		else
		{
			delta = inPackage.crc - inPackage.randomByte;
		}
		delta = inPackage.randomByte - inPackage.crc;

		if (myIsCrcDebuggingEnabled)
		{
			*myRxCout << endl;
			*myRxCout << "0x key " << std::setw(8) << static_cast<unsigned short>(inPackage.buttonKeyCode1)
					  << " random " << std::setw(8) << static_cast<unsigned short>(inPackage.randomByte)
					  << " crc " << std::setw(8) << static_cast<unsigned short>(inPackage.crc)
					  << " delta " << std::setw(8) << static_cast<unsigned short>(delta.to_ulong()) << endl;

			*myRxCout << "0b key " << buttonKeyCode
					  << " random " << random
					  << " crc " << crc
					  << " delta " << delta << endl;
		}

		std::bitset<8> seed(outPackageBuffer.seed), nonSeed(~seed);
		std::bitset<8> nonSeedAndRandom(nonSeed & random);
		std::bitset<8> keyXorNonSeedAndRandom(buttonKeyCode ^ nonSeedAndRandom);
		uint16_t expectedCrc = static_cast<unsigned short>(inPackage.crc);
		uint16_t calculatedCrc = static_cast<unsigned short>(0x00ff &
															 (random.to_ulong() - keyXorNonSeedAndRandom.to_ulong()));
		std::bitset<8> calculatedCrcBitset(calculatedCrc);
		bool isValid = (calculatedCrc == expectedCrc);

		if (myIsCrcDebuggingEnabled)
		{
			*myRxCout << endl
					  << "~seed                  " << nonSeed << endl
					  << "random                 " << random << endl
					  << "                       -------- &" << endl
					  << "~seed & random         " << nonSeedAndRandom << endl
					  << "key                    " << buttonKeyCode << endl
					  << "                       -------- ^" << endl
					  << "key ^ (~seed & random) " << keyXorNonSeedAndRandom
					  << " = calculated delta " << std::setw(2)
					  << static_cast<unsigned short>(keyXorNonSeedAndRandom.to_ulong())
					  << " vs " << std::setw(2) << static_cast<unsigned short>(delta.to_ulong())
					  << ((keyXorNonSeedAndRandom == delta) ? " OK" : " FAIL") << endl
					  << "calculated crc         " << calculatedCrcBitset << " " << std::setw(2) << calculatedCrc << " vs "
					  << std::setw(2)
					  << expectedCrc << ((isValid) ? "                    OK" : "                    FAIL")
					  << " (random - (key ^ (~seed & random))";
		}

		if (!isValid)
		{
			*myRxCout << "warn  checksum error";
		}

		myRxCout->copyfmt(init);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::OnInputDataReceived(const UsbInPackage &inPackage)
	{
		*myRxCout << "in    ";
		PrintHexdump(inPackage);
		if (inPackage.rotaryButtonFeedKeyCode != KeyCodes::Feed.undefined.code)
		{
			std::ios init(NULL);
			init.copyfmt(*myRxCout);
			*myRxCout << " delta " << std::setfill(' ') << std::setw(2)
					  << static_cast<unsigned short>(inPackage.rotaryButtonFeedKeyCode);
			myRxCout->copyfmt(init);
		}
		else
		{
			*myRxCout << " delta NA";
		}
		*myRxCout << " => ";
		PrintInputData(inPackage);
		PrintCrcDebug(inPackage, myUsb.GetOutputPackageData());
		*myRxCout << endl;

		uint8_t keyCode = inPackage.buttonKeyCode1;
		uint8_t modifierCode = inPackage.buttonKeyCode2;

		if (keyCode == myKeyCodes.Buttons.undefined.code)
		{
			keyCode = modifierCode;
			modifierCode = KeyCodes::Buttons.undefined.code;
		}

		if ((keyCode == KeyCodes::Buttons.function.code) &&
			(modifierCode != KeyCodes::Buttons.undefined.code))
		{
			keyCode = modifierCode;
			modifierCode = KeyCodes::Buttons.function.code;
		}

		if ((keyCode != KeyCodes::Buttons.undefined.code) &&
			(modifierCode != KeyCodes::Buttons.undefined.code) &&
			(modifierCode != KeyCodes::Buttons.function.code))
		{
			keyCode = modifierCode;
			modifierCode = KeyCodes::Buttons.undefined.code;
		}

		if (keyCode == KeyCodes::Buttons.undefined.code)
		{
			assert(modifierCode == KeyCodes::Buttons.undefined.code);
		}

		if (keyCode == KeyCodes::Buttons.function.code)
		{
			assert(modifierCode == KeyCodes::Buttons.undefined.code);
		}

		myPendant.ProcessEvent(keyCode, modifierCode,
							   inPackage.rotaryButtonAxisKeyCode,
							   inPackage.rotaryButtonFeedKeyCode,
							   inPackage.stepCount);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::InitWhb()
	{
		myIsRunning = true;
		myUsb.SetIsRunning(true);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::RequestTermination(int signal)
	{
		if (signal >= 0)
		{
			*myInitCout << "termination requested upon signal number " << signal << " ..." << endl;
		}
		else
		{
			*myInitCout << "termination requested ... " << endl;
		}
		myUsb.RequestTermination();
		myIsRunning = false;
	}

	// ----------------------------------------------------------------------

	bool XhcWhb04b6Component::IsRunning() const
	{
		return myIsRunning;
	}

	// ----------------------------------------------------------------------

	XhcWhb04b6Component::XhcWhb04b6Component() : myName("XHC-WHB04B-6"),
												 myKeyCodes(),
												 myMetaButtons{MetaButtonCodes(myKeyCodes.Buttons.reset, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.reset, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.stop, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.stop, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.start, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.start, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.feed_plus, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.feed_plus, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.feed_minus, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.feed_minus, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.spindle_plus, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.spindle_plus, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.spindle_minus, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.spindle_minus, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.machine_home, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.machine_home, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.safe_z, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.safe_z, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.workpiece_home, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.workpiece_home, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.spindle_on_off, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.spindle_on_off, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.function, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.probe_z, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.probe_z, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.macro10, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.macro10, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.manual_pulse_generator, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.manual_pulse_generator, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.step_continuous, myKeyCodes.Buttons.undefined),
															   MetaButtonCodes(myKeyCodes.Buttons.step_continuous, myKeyCodes.Buttons.function),
															   MetaButtonCodes(myKeyCodes.Buttons.undefined, myKeyCodes.Buttons.undefined)},
												 myUsb(myName, *this),
												 myTxCout(&myDevNull),
												 myRxCout(&myDevNull),
												 myKeyEventCout(&myDevNull),
												 myInitCout(&myDevNull),
												 myPackageReceivedEventReceiver(*this),
												 myPendant(myUsb.GetOutputPackageData())
	{
		SetSimulationMode(true);
		EnableVerboseRx(false);
		EnableVerboseTx(false);
		EnableVerboseInit(false);
	}

	// ----------------------------------------------------------------------

	XhcWhb04b6Component::~XhcWhb04b6Component()
	{
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::UpdateDisplay()
	{
		if (myIsRunning)
		{
			myPendant.UpdateDisplayData();
		}
		else
		{
			myPendant.ClearDisplayData();
		}
		myUsb.SendDisplayData();
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintPushButtonText(uint8_t keyCode, uint8_t modifierCode, std::ostream &out)
	{
		std::ios init(NULL);
		init.copyfmt(out);
		int indent = 15;
		out << std::setfill(' ');

		if (keyCode == myKeyCodes.Buttons.undefined.code)
		{
			out << std::setw(indent) << "";
			return;
		}

		const KeyCode &buttonKeyCode = myKeyCodes.Buttons.getKeyCode(keyCode);

		if (modifierCode == myKeyCodes.Buttons.function.code)
		{
			out << std::setw(indent) << buttonKeyCode.altText;
		}
		else
		{
			out << std::setw(indent) << buttonKeyCode.text;
		}
		out.copyfmt(init);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintRotaryButtonText(const KeyCode *keyCodesBase, uint8_t keyCode, std::ostream &out)
	{
		std::ios init(NULL);
		init.copyfmt(out);

		const KeyCode *buttonKeyCode = keyCodesBase;
		while (buttonKeyCode->code != 0)
		{
			if (buttonKeyCode->code == keyCode)
			{
				break;
			}
			buttonKeyCode++;
		}
		out << std::setw(5) << buttonKeyCode->text << "(" << std::setw(4) << buttonKeyCode->altText << ")";
		out.copyfmt(init);
	}

	// ----------------------------------------------------------------------

	DisplayIndicator::DisplayIndicator() : asByte(0)
	{
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintInputData(const UsbInPackage &inPackage, std::ostream &out)
	{
		std::ios init(NULL);
		init.copyfmt(out);

		out << "| " << std::setfill('0') << std::hex << std::setw(2) << static_cast<unsigned short>(inPackage.header)
			<< " | " << std::setw(2)
			<< static_cast<unsigned short>(inPackage.randomByte) << " | ";
		out.copyfmt(init);
		PrintPushButtonText(inPackage.buttonKeyCode1, inPackage.buttonKeyCode2, out);
		out << " | ";
		PrintPushButtonText(inPackage.buttonKeyCode2, inPackage.buttonKeyCode1, out);
		out << " | ";
		PrintRotaryButtonText((KeyCode *)&myKeyCodes.Feed, inPackage.rotaryButtonFeedKeyCode, out);
		out << " | ";
		PrintRotaryButtonText((KeyCode *)&myKeyCodes.Axis, inPackage.rotaryButtonAxisKeyCode, out);
		out << " | " << std::setfill(' ') << std::setw(3) << static_cast<short>(inPackage.stepCount) << " | " << std::hex
			<< std::setfill('0')
			<< std::setw(2) << static_cast<unsigned short>(inPackage.crc);

		out.copyfmt(init);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintHexdump(const UsbInPackage &inPackage, std::ostream &out)
	{
		std::ios init(NULL);
		init.copyfmt(out);

		out << std::setfill('0') << std::hex << "0x" << std::setw(2) << static_cast<unsigned short>(inPackage.header) << " "
			<< std::setw(2)
			<< static_cast<unsigned short>(inPackage.randomByte) << " " << std::setw(2)
			<< static_cast<unsigned short>(inPackage.buttonKeyCode1) << " " << std::setw(2)
			<< static_cast<unsigned short>(inPackage.buttonKeyCode2) << " " << std::setw(2)
			<< static_cast<unsigned short>(inPackage.rotaryButtonFeedKeyCode) << " " << std::setw(2)
			<< static_cast<unsigned short>(inPackage.rotaryButtonAxisKeyCode) << " " << std::setw(2)
			<< static_cast<unsigned short>(inPackage.stepCount & 0xff) << " " << std::setw(2)
			<< static_cast<unsigned short>(inPackage.crc);
		out.copyfmt(init);
	}

	// ----------------------------------------------------------------------

	int XhcWhb04b6Component::Run()
	{
		bool isReady = false;
		InitWhb();

		while (IsRunning())
		{
			InitWhb();
			if (!myUsb.Init())
			{
				return EXIT_FAILURE;
			}

			if (!isReady)
			{
				isReady = true;
			}

			if (myUsb.IsDeviceOpen())
			{
				*myInitCout << "init  enabling reception ...";
				if (!EnableReceiveAsyncTransfer())
				{
					std::cerr << endl
							  << "failed to enable reception" << endl;
					return EXIT_FAILURE;
				}
				*myInitCout << " ok" << endl;
			}
			Process();
			TeardownUsb();
		}

		return EXIT_SUCCESS;
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::LinuxcncSimulate()
	{
	}

	// ----------------------------------------------------------------------

	bool XhcWhb04b6Component::EnableReceiveAsyncTransfer()
	{
		return myUsb.SetupAsyncTransfer();
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::SetSimulationMode(bool enableSimulationMode)
	{
		myIsSimulationMode = enableSimulationMode;
		myUsb.SetSimulationMode(myIsSimulationMode);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::Process()
	{
		if (myUsb.IsDeviceOpen())
		{
			while (IsRunning() && !myUsb.GetDoReconnect())
			{
				tud_task();

				if (myIsSimulationMode)
				{
					LinuxcncSimulate();
				}
				UpdateDisplay();
			}
			UpdateDisplay();

			*myInitCout << "connection lost, cleaning up" << endl;

			myUsb.SetDeviceHandle(nullptr);
		}
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::TeardownUsb()
	{
		tud_disconnect();
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::EnableVerboseRx(bool enable)
	{
		myUsb.EnableVerboseRx(enable);
		if (enable)
		{
			myRxCout = &std::cout;
		}
		else
		{
			myRxCout = &myDevNull;
		}
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::EnableVerboseTx(bool enable)
	{
		myUsb.EnableVerboseTx(enable);
		if (enable)
		{
			myTxCout = &std::cout;
		}
		else
		{
			myTxCout = &myDevNull;
		}
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::EnableVerboseInit(bool enable)
	{
		myUsb.EnableVerboseInit(enable);
		if (enable)
		{
			myInitCout = &std::cout;
		}
		else
		{
			myInitCout = &myDevNull;
		}
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintPushButtonText(uint8_t keyCode, uint8_t modifierCode)
	{
		PrintPushButtonText(keyCode, modifierCode, *myRxCout);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintRotaryButtonText(const KeyCode *keyCodesBase, uint8_t keyCode)
	{
		PrintRotaryButtonText(keyCodesBase, keyCode, *myRxCout);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintInputData(const UsbInPackage &inPackage)
	{
		PrintInputData(inPackage, *myRxCout);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::PrintHexdump(const UsbInPackage &inPackage)
	{
		PrintHexdump(inPackage, *myRxCout);
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::SetWaitWithTimeout(uint8_t waitSecs)
	{
		myUsb.SetWaitWithTimeout(waitSecs);
	}

	// ----------------------------------------------------------------------

	bool XhcWhb04b6Component::IsSimulationModeEnabled() const
	{
		return myIsSimulationMode;
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::SetEnableVerboseKeyEvents(bool enable)
	{
		myUsb.EnableVerboseRx(enable);
		if (enable)
		{
			myKeyEventCout = &std::cout;
		}
		else
		{
			myKeyEventCout = &myDevNull;
		}
	}

	// ----------------------------------------------------------------------

	void XhcWhb04b6Component::EnableCrcDebugging(bool enable)
	{
		myIsCrcDebuggingEnabled = enable;
	}

} // namespace XhcWhb04b6
