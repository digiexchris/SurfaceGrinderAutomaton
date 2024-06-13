#pragma once

// system includes
#include <stdint.h>

// 3rd party includes

// local includes
#include "./pendant.h"
#include "./usb.h"

// forward declarations

namespace XhcWhb04b6
{

	// forward declarations

	//! The XHC WHB04B-6 user space component for Linuxcnc.
	class XhcWhb04b6Component : public OnUsbInputPackageListener
	{
	public:
		XhcWhb04b6Component();
		virtual ~XhcWhb04b6Component();
		void Process();
		void TeardownUsb();
		void InitWhb();
		bool EnableReceiveAsyncTransfer();
		void UpdateDisplay();
		void LinuxcncSimulate();
		void RequestTermination(int signal = -42);
		bool IsRunning() const;
		int Run();
		bool IsSimulationModeEnabled() const;
		void SetSimulationMode(bool enableSimulationMode);
		void SetEnableVerboseKeyEvents(bool enable);
		void EnableVerboseRx(bool enable);
		void EnableVerboseTx(bool enable);
		void EnableVerboseInit(bool enable);
		void EnableCrcDebugging(bool enable);
		void SetWaitWithTimeout(uint8_t waitSecs = 3);
		void PrintCrcDebug(const UsbInPackage &inPackage, const UsbOutPackageData &outPackageBuffer) const;
		void OfferMemory();
		void OnInputDataReceived(const UsbInPackage &inPackage) override;
		const char *GetName() const;

	private:
		const char *myName;
		const KeyCodes myKeyCodes;
		const MetaButtonCodes myMetaButtons[32];
		Usb myUsb;
		bool myIsRunning{false};
		bool myIsSimulationMode{false};
		std::ostream myDevNull{nullptr};
		std::ostream *myTxCout;
		std::ostream *myRxCout;
		std::ostream *myKeyEventCout;
		std::ostream *myInitCout;
		OnUsbInputPackageListener &myPackageReceivedEventReceiver;
		bool myIsCrcDebuggingEnabled{false};
		Pendant myPendant;

		void PrintPushButtonText(uint8_t keyCode, uint8_t modifierCode, std::ostream &out);
		void PrintPushButtonText(uint8_t keyCode, uint8_t modifierCode);
		void PrintRotaryButtonText(const KeyCode *keyCodesBase, uint8_t keyCode, std::ostream &out);
		void PrintRotaryButtonText(const KeyCode *keyCodesBase, uint8_t keyCode);
		void PrintInputData(const UsbInPackage &inPackage, std::ostream &out);
		void PrintInputData(const UsbInPackage &inPackage);
		void PrintHexdump(const UsbInPackage &inPackage, std::ostream &out);
		void PrintHexdump(const UsbInPackage &inPackage);
	};

}
