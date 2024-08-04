type USBDevice = any; // Replace with actual USBDevice type if available

class Serial {
  static async getPorts(): Promise<SerialPort[]> {
    const devices = await navigator.usb.getDevices();
    return devices.map(device => new SerialPort(device));
  }

  static async requestPort(): Promise<SerialPort> {
    const filters = [{ 'vendorId': 0xcafe }];
    const device = await navigator.usb.requestDevice({ 'filters': filters });
    return new SerialPort(device);
  }
}

class SerialPort {
  private device_: USBDevice;
  private interfaceNumber: number;
  private endpointIn: number;
  private endpointOut: number;
  public onReceive: (data: DataView) => void = (data) => {
    console.log(data);
  };
  public onReceiveError: (error: DOMException) => void;

  constructor(device: USBDevice) {
    this.device_ = device;
    this.interfaceNumber = 0;
    this.endpointIn = 0;
    this.endpointOut = 0;
    this.onReceive = () => {};
    this.onReceiveError = () => {};
  }

  async connect(): Promise<void> {
    const readLoop = () => {
      this.device_.transferIn(this.endpointIn, 64).then(result => {
        this.onReceive(result.data);
        readLoop();
      }, error => {
        this.onReceiveError(error);
      });
    };

    await this.device_.open();
    if (this.device_.configuration === null) {
      await this.device_.selectConfiguration(1);
    }

    const interfaces = this.device_.configuration.interfaces;
    interfaces.forEach(element => {
      element.alternates.forEach(elementalt => {
        if (elementalt.interfaceClass === 0xFF) {
          this.interfaceNumber = element.interfaceNumber;
          elementalt.endpoints.forEach(elementendpoint => {
            if (elementendpoint.direction === 'out') {
              this.endpointOut = elementendpoint.endpointNumber;
            }
            if (elementendpoint.direction === 'in') {
              this.endpointIn = elementendpoint.endpointNumber;
            }
          });
        }
      });
    });

    await this.device_.claimInterface(this.interfaceNumber);
    await this.device_.selectAlternateInterface(this.interfaceNumber, 0);
    await this.device_.controlTransferOut({
      'requestType': 'class',
      'recipient': 'interface',
      'request': 0x22,
      'value': 0x01,
      'index': this.interfaceNumber
    });

    readLoop();
  }

  async disconnect(): Promise<void> {
    await this.device_.controlTransferOut({
      'requestType': 'class',
      'recipient': 'interface',
      'request': 0x22,
      'value': 0x00,
      'index': this.interfaceNumber
    });
    await this.device_.close();
  }

  async send(data: ArrayBuffer): Promise<void> {
    await this.device_.transferOut(this.endpointOut, data);
  }

  getDeviceName(): string {
    return this.device_.productName;
  }
}

export { Serial, SerialPort };
