import { useState, useEffect, useRef } from "react";
import Header from "./components/Header";
import StatusPanel from "./components/StatusPanel";
import "./App.css";

type StatusType = "Legitimate" | "Spoofed" | "Inactive";

type IterationHistory = {
  iteration: number;
  time: string;
  status: string;
  legitProb: number;
  spoofProb: number;
};

function formatTime(seconds: number) {
  const m = Math.floor(seconds / 60).toString().padStart(2, '0');
  const s = (seconds % 60).toString().padStart(2, '0');
  return `${m}:${s}`;
}

function App() {
  const [status, setStatus] = useState<StatusType>("Inactive");
  const [loadingType, setLoadingType] = useState<string | null>(null);
  const [activeData, setActiveData] = useState<any>(null);

  // Timer & History State
  const [startTime, setStartTime] = useState<number | null>(null);
  const [elapsed, setElapsed] = useState(0);
  const [history, setHistory] = useState<IterationHistory[]>([]);
  const [showHistory, setShowHistory] = useState(false);
  const [iterationOffset, setIterationOffset] = useState<number>(0);

  // To avoid duplicate history insertions
  const lastIterationRef = useRef<number>(-1);

  // Poll exactly every 1 second
  useEffect(() => {
    let interval: number;

    const fetchStatus = async () => {
      try {
        const response = await fetch('/api/status');
        if (response.ok) {
          const data = await response.json();
          if (data.status && data.status !== "Inactive") {
            setStatus(data.status as StatusType);
            setActiveData(data);

            // Time Tracking
            if (!startTime) setStartTime(Date.now());

            // History Log Tracking
            if (data.iteration && data.iteration !== lastIterationRef.current) {
              lastIterationRef.current = data.iteration;
              setHistory(prev => [{
                iteration: data.iteration + iterationOffset,
                time: data.timestamp_iso || new Date().toISOString(),
                status: data.status,
                legitProb: data.result?.avg_legitimate_probability || 0,
                spoofProb: data.result?.avg_spoof_probability || 0
              }, ...prev]);
            }
          } else {
             // File was potentially locally deleted by starting a new stream
             // If we aren't loading, actually drop to Inactive
             if (!loadingType) {
               setStatus("Inactive");
             }
          }
        }
      } catch (err) {
        if (!loadingType && status !== 'Inactive') setStatus("Inactive");
      }
    };

    if (loadingType || status !== "Inactive") {
      interval = setInterval(fetchStatus, 1000);
    }

    return () => clearInterval(interval);
  }, [loadingType, status, startTime, iterationOffset]);

  // Handle local 1s timer updates independently to prevent lag
  useEffect(() => {
    let tInterval: number;
    if (startTime && status !== "Inactive") {
      tInterval = setInterval(() => {
        setElapsed(Math.floor((Date.now() - startTime) / 1000));
      }, 1000);
    }
    return () => clearInterval(tInterval);
  }, [startTime, status]);

  const startStream = async (type: "spoof" | "legit") => {
    if (status === "Inactive") {
      setStatus("Inactive");
      setStartTime(null);
      setElapsed(0);
      setHistory([]);
      setActiveData(null);
      lastIterationRef.current = -1;
      setIterationOffset(0);
      setShowHistory(false);
    } else {
      // Smooth Transition: We are switching streams live! Do not kill history.
      // Offset so the new file's iterations stack continuously.
      setIterationOffset(prev => prev + (activeData?.iteration || 0));
      lastIterationRef.current = -1;
    }
    setLoadingType(type);
    
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
    try {
      await fetch('/api/stop', { method: 'POST' });
    } catch (e) {
      console.error(e);
    }
    setLoadingType(null);
    setStatus("Inactive");
    setStartTime(null);
  };

  const legitProb = activeData?.result?.avg_legitimate_probability || 0;
  const spoofProb = activeData?.result?.avg_spoof_probability || 0;

  const resetTimerAndLog = () => {
    setHistory([]);
    setElapsed(0);
    setIterationOffset(0);
    if (status !== "Inactive" || loadingType) {
      setStartTime(Date.now());
      lastIterationRef.current = -1;
    } else {
      setStartTime(null);
    }
  };

  return (
    <div className="app-root">
      <Header />

      <div className="main-content">
        {/* Dynamic Display */}
        <div className="dashboard-grid">
          {/* Main Panel */}
          <div className="dashboard-main">
            <StatusPanel status={status} />
          </div>

          {/* Side Probability Panel (ALWAYS SHOWN for absolute horizontal consistency) */}
          <div className={`stats-panel ${status === "Spoofed" ? "alert" : ""} ${status === "Inactive" ? "inactive-panel" : ""}`}>
            <h3>Signal Probabilities</h3>
            <div className="stat-row">
              <span className="stat-label">Legitimate Avg</span>
              <span className={`stat-value ${status !== "Inactive" && legitProb > 0.5 ? "text-green" : "text-gray"}`}>
                {status !== "Inactive" ? legitProb.toFixed(4) : "N/A"}
              </span>
            </div>
            <div className="stat-row">
              <span className="stat-label">Spoofed Avg</span>
              <span className={`stat-value ${status !== "Inactive" && spoofProb > 0.5 ? "text-red" : "text-gray"}`}>
                {status !== "Inactive" ? spoofProb.toFixed(4) : "N/A"}
              </span>
            </div>
          </div>
        </div>

        {/* Timer & History Widget (ALWAYS SHOWN for absolute vertical consistency) */}
        <div className="history-widget-container">
          <div className="timer-controls-row">
            <button 
              className="timer-btn" 
              onClick={() => setShowHistory(!showHistory)}
            >
              <span className="timer-icon">⏱️</span> 
              <span>Session Time: {formatTime(elapsed)}</span>
              <span className="timer-sub">Tracked Iterations: {history.length} ▼</span>
            </button>

            <button className="reset-btn" onClick={resetTimerAndLog} title="Reset Timer and Logs">
              ↻ Reset
            </button>
          </div>
          
          <div className={`history-dropdown-wrapper ${showHistory ? 'open' : ''}`}>
             {showHistory && (
              <div className="history-dropdown">
                <h4>Session Event Log</h4>
                {history.length === 0 ? (
                  <div style={{textAlign: "center", color: "#64748b", margin: "1rem"}}>No data logged yet</div>
                ) : (
                  <div className="history-list">
                    {history.map((log, i) => (
                      <div key={i} className={`history-row ${log.status === "Spoofed" ? "row-red" : "row-green"}`}>
                        <span className="h-iter">#{log.iteration}</span>
                        <span className="h-time">{new Date(log.time).toLocaleTimeString()}</span>
                        <span className="h-type">{log.status}</span>
                        <span className="h-prob">Spoof: {log.spoofProb.toFixed(2)}</span>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            )}
          </div>
        </div>

        {/* SDR Simulation Tags */}
        {loadingType && (
          <div className="sdr-indicator-container">
            {loadingType === 'legit' && (
              <span className="sdr-text blinking">
                ► Feeding I/Q signals from SDR to NEMESIS model...
              </span>
            )}
            {loadingType === 'spoof' && (
              <span className="sdr-text blinking alert">
                ► Simulating spoofing attack... Feeding I/Q signals from SDR to NEMESIS model...
              </span>
            )}
          </div>
        )}

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
