"use client";

import { useState, useEffect } from 'react';
import { Serial, SerialPort } from './serial';
import { Button, Card } from "@nextui-org/react";

const SerialComponent: React.FC = () => {
  const [ports, setPorts] = useState<SerialPort[]>([]);
  const [connectedPort, setConnectedPort] = useState<SerialPort | null>(null);

  useEffect(() => {
    const fetchPorts = async () => {
      const availablePorts = await Serial.getPorts();
      setPorts(availablePorts);
    };

    fetchPorts();
  }, []);

  const handleRequestPort = async () => {
    if (connectedPort) {
      await handleDisconnect();
    }
    const newPort = await Serial.requestPort();
    await handleConnect(newPort);
    setPorts([...ports, newPort]);
  };

  const handleConnect = async (port: SerialPort) => {
    await port.connect();
    setConnectedPort(port);
  };

  const handleDisconnect = async () => {
    if (connectedPort) {
      await connectedPort.disconnect();
      setConnectedPort(null);
    }
  };

  const renderConnectedPort = () => {
    if (!connectedPort) {
      if (ports.length > 0) {
        return (
          <div>
            <Button onClick={() => handleConnect(ports[0])} color="secondary" auto>
              {/* Connect {ports[0].getDeviceName()} */}
              Connect
            </Button>
          </div>
        );
      }
    }
    return null;
  };

  const renderButtonLabel = () => {
    if (connectedPort) {
      return 'Disconnect';
    }
    return 'Select Device';
  };

  const renderButtonColor = () => {
    if (connectedPort) {
      return 'secondary';
    }
    return 'primary';
  };

  return (
    <div style={{ padding: '20px' }}>
      <div style={{ display: 'flex', gap: '10px', marginTop: '10px' }}>
        <Button
          onClick={connectedPort ? handleDisconnect : handleRequestPort}
          color={renderButtonColor()}
          auto
        >
          {renderButtonLabel()}
        </Button>
        {renderConnectedPort()}
      </div>
    </div>
  );
};

export default SerialComponent;
