import { useState, useEffect } from "react";
import Header from "./components/Header";
import StatusPanel from "./components/StatusPanel";
import "./App.css";

function App() {
  const [status, setStatus] = useState<"Legitimate" | "Spoofed" | "Inactive">("Inactive");
  const [loadingType, setLoadingType] = useState<string | null>(null);

  useEffect(() => {
    let interval: number;

    const fetchStatus = async () => {
      try {
        const response = await fetch('/api/status');
        if (response.ok) {
          const data = await response.json();
          if (data.status) {
            setStatus(data.status);
          } else {
            setStatus("Inactive");
          }
        }
      } catch (err) {
        // If server down or no status
        if (status !== 'Inactive') setStatus("Inactive");
      }
    };

    if (loadingType || status !== "Inactive") {
      interval = setInterval(fetchStatus, 1000);
    }

    return () => clearInterval(interval);
  }, [loadingType, status]);

  const startStream = async (type: "spoof" | "legit") => {
    setLoadingType(type);
    setStatus("Inactive"); // Temporarily show inactive while loading new signal
    try {
      await fetch('/api/start', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ type })
      });
    } catch (e) {
      console.error('Failed to start stream', e);
      setLoadingType(null);
    }
  };

  const stopStream = async () => {
    setLoadingType(null);
    setStatus("Inactive");
    try {
      await fetch('/api/stop', { method: 'POST' });
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <div className="app-root">
      <Header />

      <div className="main-content">
        <StatusPanel status={status} />

        <div className="controls-container">
          <button
            className={`btn-acquire ${loadingType === 'legit' ? 'active' : ''}`}
            onClick={() => startStream('legit')}
          >
            {loadingType === 'legit' ? 'Running Normal Signal...' : 'Normal Signal Acquire'}
          </button>
          
          <button
            className={`btn-spoof ${loadingType === 'spoof' ? 'active' : ''}`}
            onClick={() => startStream('spoof')}
          >
            {loadingType === 'spoof' ? 'Running Spoofing Attack...' : 'Spoofing Attack'}
          </button>

          {(loadingType || status !== "Inactive") && (
             <button className="btn-stop" onClick={stopStream}>
               Stop Signal
             </button>
          )}
        </div>
      </div>
    </div>
  );
}

export default App;
